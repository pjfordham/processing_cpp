#include "processing_pgraphics.h"
#include "processing_utils.h"
#include "processing_debug.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

static std::vector<std::weak_ptr<PGraphicsImpl>> &graphicsHandles() {
   static std::vector<std::weak_ptr<PGraphicsImpl>> handles;
   return handles;
}

static PVector posOnUnitSquare( float angle ) {
   float x = 2 * sin(-angle + HALF_PI);
   float y = 2 * cos(-angle + HALF_PI);
   if ( x > 1.0 ) {
      y = y / x;
      x = 1.0;
   } else if ( x < -1.0 ) {
         y = y / -x;
         x = -1.0;
   }
   if ( y > 1.0 ) {
      x = x / y;
      y = 1.0;
   } else if ( y < -1.0 ) {
      x = x / -y;
      y = -1.0;
   }
   return {x,y};
}

static PVector posOnUnitCircle( float angle ) {
   return { sinf(-angle + HALF_PI), cosf(-angle + HALF_PI) };
}

class PGraphicsImpl {
public:
   static void init();

   static void close();

   bool pixels_current = false;
   int textureMode_ = IMAGE;
   gl::context glc;

   gl::framebuffer localFrame;
   gl::framebuffer windowFrame;
   gl::batch_t batch;
   gl::scene_t scene;

   int ellipse_mode = CENTER;
   int rect_mode = CORNER;
   int image_mode = CORNER;

   int width = 0;
   int height = 0;
   float aaFactor;

   int xTextAlign = LEFT;
   int yTextAlign = TOP;

   float xsphere_ures = 30;
   float xsphere_vres = 30;

   PShape _shape;
   std::vector<unsigned int> pixels;

   std::vector<PMatrix> matrix_stack;

   PShader defaultShader;
   PShader currentShader;

   int flushes = 0;

   int getWidth() const { return width; }
   int getHeight() const {return height; }
   unsigned int *getPixels() { return pixels.data(); }
   GLuint getAsTexture() { return localFrame.getColorBufferID(); }
   PImage getAsPImage() { return createImageFromTexture(localFrame.getColorBufferID()); }

   ~PGraphicsImpl() {
      DEBUG_METHOD();
   }

   PGraphicsImpl(int width, int height, int mode, int aaMode = MSAA, int aaFactor = 2) :
      localFrame(width, height, aaMode, aaFactor),
      windowFrame( gl::framebuffer::constructMainFrame( width, height ) ),
      _shape( createShape() ) {

      DEBUG_METHOD();
      this->width = width;
      this->height = height;
      this->aaFactor = aaFactor;

      glc.init();

      defaultShader = loadShader();
      shader( defaultShader );

      noLights();
      camera();
      perspective();

      background(DEFAULT_GRAY);
   }

   void releaseResources() {
      width = 0;
      height = 0;
      defaultShader = {};
      currentShader = {};
      localFrame = {};
      scene = {};
      batch = {};
   }

   void flush() {
      static PShader flat = flatShader();
      if ( batch.size() > 0 ) {
         flushes+=batch.size();
         if (currentShader == defaultShader && scene.lights.size() == 0 && batch.usesTextures() == false && batch.usesCircles() == false) {
            glc.setShader( flat.getShader(), scene, batch );
            flat.set_uniforms();
            flat.bind();
         } else {
            glc.setShader( currentShader.getShader(), scene, batch );
            currentShader.set_uniforms();
            currentShader.bind();
         }
         localFrame.bind();
         scene.set();
         batch.compile();
         batch.draw();
         batch.clear();
      }
   }

   void directDraw( gl::batch_t &batch, const PMatrix &transform ) {
      static PShader flat = flatShader();
      if ( batch.size() > 0 ) {
         flushes+=batch.size();
         if (currentShader == defaultShader && scene.lights.size() == 0 && batch.usesTextures() == false && batch.usesCircles() == false) {
            glc.setShader( flat.getShader(), scene, batch );
            flat.set_uniforms();
            flat.bind();
         } else {
            glc.setShader( currentShader.getShader(), scene, batch );
            currentShader.set_uniforms();
            currentShader.bind();
         }
         localFrame.bind();
         scene.set();
         batch.draw(transform.glm_data());
      }
   }

   void drawPImageWithCPU( PImage img, int x, int y ) {
      img.loadPixels();
      loadPixels();
      for (int row = 0; row < img.width; row++) {
         for (int col = 0; col < img.height; col++) {
            // Need to blend properly since font image is alpha only
            unsigned char fontp = color(img.pixels[(img.height - col-1)*img.width + row]).a;
            pixels[(col+y)*width+(row+x)] = color(fontp,fontp,fontp);
         }
      }
      updatePixels();
   }

   void save( const std::string &fileName ) {
      flush();
      localFrame.blit( windowFrame );
      PImage image = createImage(width, height, 0);
      windowFrame.saveFrame( image.pixels );
      image.save_as( fileName );
   }

   void saveFrame( std::string fileName = "frame-####.png" ) {
      static int counter = 0;
      int c = counter;
      std::size_t pos = fileName.rfind('#');
      while (pos != std::string::npos) {
         fileName[pos] = '0' + (c % 10);
         c /= 10;
         pos = fileName.rfind('#', pos - 1);
      }
      save( fileName );
      counter++;
   }

   void pushMatrix() {
      matrix_stack.push_back(_shape.getShapeMatrix());
   }

   void popMatrix() {
      _shape.resetMatrix();
      _shape.transform( matrix_stack.back() );
      matrix_stack.pop_back();
   }

   void ortho(float left, float right, float bottom, float top, float near, float far) {
      float tx = -(right + left) / (right - left);
      float ty = -(top + bottom) / (top - bottom);
      float tz = -(far + near) / (far - near);
      glm::mat4 projection =  {
         { 2/(right-left),               0,             0, tx},
         {              0,  2/(top-bottom),             0, ty},
         {              0,               0, -2/(far-near), tz},
         {              0,               0,             0,  1}};
      flush();
      scene.setProjectionMatrix( glm::transpose( projection ) );
   }

   void ortho(float left, float right, float bottom, float top) {
      ortho(left, right, bottom, top, bottom*2, top*2);
   }

   void ortho() {
      ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0);
   }

   void perspective(float fov, float a, float near, float far) {
      float f = 1 / tan(0.5 * fov);
      float rangeInv = 1.0 / (near - far);
      float A = (near + far) * rangeInv;
      float B = near * far * rangeInv * 2;
      glm::mat4 projection{
         {f/a,  0,  0,  0} ,
         {0,  f,  0,  0} ,
         {0,  0,  A,  B} ,
         {0,  0, -1,  0}
      };
      flush();
      scene.setProjectionMatrix( glm::transpose(projection) );
   }

   void perspective() {
      float fov = PI/3.0;
      float cameraZ = (height/2.0) / tan(fov/2.0);
      perspective( fov, (float)width/(float)height, cameraZ/10.0,  cameraZ*10.0);
   }

   void camera( float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ ) {

      PVector center{centerX, centerY, centerZ};
      PVector eye{eyeX,eyeY,eyeZ};
      PVector _up{upX,upY,upZ};

      PVector forward = (center - eye).normalize();
      PVector side = forward.cross(_up).normalize();
      PVector up = side.cross(forward).normalize();

      glm::mat4 view{
         {     side.x,     side.y,     side.z, 0.0f},
         {       up.x,       up.y,       up.z, 0.0f},
         { -forward.x, -forward.y, -forward.z, 0.0f},
         {       0.0f,       0.0f,       0.0f, 1.0f} };
      view = glm::transpose( view );

      glm::mat4 translate{
         {1.0,    0,     0,    -eyeX},
         {0,    1.0,     0,    -eyeY},
         {0,      0,   1.0,    -eyeZ},
         {0.0f, 0.0f,  0.0f,    1.0f} };
      translate = glm::transpose( translate );

      // Translate the camera to the origin
      flush();
      scene.setViewMatrix( view * translate );
   }

   void camera() {
      camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
             width / 2.0, height / 2.0, 0, 0, 1, 0);
   }

   glm::vec3 falloff = {1.0,0.0,0.0};

   void directionalLight(float r, float g, float b, float nx, float ny, float nz) {
      flush();
      glm::vec3 color = {r/255.0f, g/255.0f, b/255.0f};
      glm::vec3 worldDir = (_shape.getShapeMatrix() * PVector{nx,ny,nz}).normalize();
      scene.pushDirectionalLight( color, worldDir );
   }

   void pointLight(float r, float g, float b, float nx, float ny, float nz) {
      flush();
      glm::vec3 color = {r/255.0f, g/255.0f, b/255.0f};
      glm::vec3 worldPos = (_shape.getShapeMatrix() * PVector{nx,ny,nz});
      glm::vec4 position = { worldPos.x, worldPos.y, worldPos.z , 1};
      scene.pushPointLight( color, position, falloff );
   }

   void spotLight( float r, float g, float b, float x, float y, float z, float nx, float ny, float nz, float angle, float concentration) {
      flush();
      glm::vec3 color = {r/255.0f, g/255.0f, b/255.0f};
      glm::vec3 worldPos = (_shape.getShapeMatrix() * PVector{x,y,z});
      glm::vec4 position = { worldPos.x, worldPos.y, worldPos.z , 1};
      glm::vec3 worldDir = (_shape.getShapeMatrix() * PVector{nx,ny,nz}).normalize();
      scene.pushSpotLight( color, position, worldDir, falloff, {cosf(angle), concentration} );
   }

   void lightFalloff(float r, float g, float b) {
      falloff = { r, g, b };
   }

   void ambientLight(float r, float g, float b) {
      flush();
      glm::vec3 color = {r/255.0f, g/255.0f, b/255.0f};
      scene.pushAmbientLight( color );
   }

   void lights() {
      scene.clearLights();
      falloff = {1.0, 0.0,0.0};
      ambientLight( 0.5, 0.5, 0.5 );
      directionalLight( 0.5, 0.5, 0.5, 0.0, 0.0,-1.0 );
      //lightSpecular(0, 0, 0);
   };

   void noLights() {
      flush();
      scene.clearLights();
   }

   void textAlign(int x, int y) {
      xTextAlign = x;
      yTextAlign = y;
   }

   void textAlign(int x) {
      xTextAlign = x;
   }

   void blendMode(int b) {
      flush();
      glc.blendMode(b);
   }

   void hint(int type) {
      flush();
      glc.hint(type);
   }

   void text(const std::string &text, float x, float y, float twidth = -1, float theight = -1) {

      PImage text_image = currentFont.render_as_pimage(text);

      twidth = text_image.width;
      theight = text_image.height;

      float ascent = currentFont.textAscent();

      // this works well enough for the Letters.cc example but it's not really general
      if ( xTextAlign == CENTER ) {
         x = x - twidth / 2.0;
      }
      if ( yTextAlign == CENTER ) {
         y = y - ascent / 2.0;
      } else {
         y = y - ascent;
      }
      if ( xTextAlign == RIGHT ) {
         x = x - twidth;
      }
      if ( yTextAlign == RIGHT ) {
         y = y - ascent;
      }

      drawTexturedQuad({x,y},{x+twidth,y},{x+twidth,y+theight},{x,y+theight},
                       text_image, _shape.getFillColor() );
   }

   void text(char c, float x, float y, float twidth = -1, float theight = -1) {
      std::string s(&c,1);
      text(s,x,y,twidth,theight);
   }

   void background(float r, float g, float b, float a = color::scaleA) {
      auto color = gl::flatten_color_mode({r,g,b,a});
      localFrame.clear( color.r, color.g, color.b, color.a );
   }

   void background(float gray) {
      if (color::mode == HSB) {
         background(0,0,gray);
      } else {
         background(gray,gray,gray);
      }
   }

   void background(float gray, float alpha) {
      if (color::mode == HSB) {
         background(0,0,gray, alpha);
      } else {
         background(gray,gray,gray,alpha);
      }
   }

   void imageMode(int iMode) {
      image_mode = iMode;
   }

   PShape createBox(float w, float h, float d) {
      w = w / 2;
      h = h / 2;
      d = d / 2;

      PShape cube = createShape();
      cube.beginShape(TRIANGLES);
      cube.copyStyle( _shape );
      cube.textureMode(NORMAL);

      // Front face
      cube.normal(0.0,  0.0,  1.0);
      cube.vertex( -w, -h,  d, 0.0, 0.0 );
      cube.vertex(  w, -h,  d, 1.0, 0.0 );
      cube.vertex(  w,  h,  d, 1.0, 1.0 );
      cube.vertex( -w,  h,  d, 0.0, 1.0 );

      // Back face
      cube.normal(0.0,  0.0, -1.0);
      cube.vertex( -w, -h, -d, 0.0, 0.0 );
      cube.vertex( -w,  h, -d, 1.0, 0.0 );
      cube.vertex(  w,  h, -d, 1.0, 1.0 );
      cube.vertex(  w, -h, -d, 0.0, 1.0 );

      // Top face
      cube.normal(0.0,  1.0,  0.0);
      cube.vertex( -w,  h, -d, 0.0, 0.0 );
      cube.vertex( -w,  h,  d, 1.0, 0.0 );
      cube.vertex(  w,  h,  d, 1.0, 1.0 );
      cube.vertex(  w,  h, -d, 0.0, 1.0 );

      // Bottom face
      cube.normal(0.0, -1.0,  0.0);
      cube.vertex( -w, -h, -d, 0.0, 0.0 );
      cube.vertex(  w, -h, -d, 1.0, 0.0 );
      cube.vertex(  w, -h,  d, 1.0, 1.0 );
      cube.vertex( -w, -h,  d, 0.0, 1.0 );

      // Right face
      cube.normal(1.0,  0.0,  0.0);
      cube.vertex(  w, -h, -d, 0.0, 0.0 );
      cube.vertex(  w,  h, -d, 1.0, 0.0 );
      cube.vertex(  w,  h,  d, 1.0, 1.0 );
      cube.vertex(  w, -h,  d, 0.0, 1.0 );

      // Left face
      cube.normal(-1.0,  0.0,  0.0);
      cube.vertex( -w, -h, -d, 0.0, 0.0 );
      cube.vertex( -w, -h,  d, 1.0, 0.0 );
      cube.vertex( -w,  h,  d, 1.0, 1.0 );
      cube.vertex( -w,  h, -d, 0.0, 1.0 );

      cube.populateIndices( {
            0,1,2, 0,2,3, 4,5,6, 4,6,7,
            8,9,10, 8,10,11, 12,13,14, 12,14,15,
            16,17,18, 16,18,19, 20,21,22, 20,22,23
         } );

      cube.endShape();
      return cube;
   }

   void box(float w, float h, float d) {
      auto b =  createBox(w,h,d);
      shape( b );
   }

   void sphereDetail(float ures, float vres) {
      xsphere_ures = ures;
      xsphere_vres = vres;
   }

   PShape createSphere( float radius ) {
      PShape sphere = createShape();
      sphere.beginShape(TRIANGLES);
      sphere.copyStyle( _shape );
      sphere.textureMode(NORMAL);

      float latStep = M_PI / xsphere_ures;
      float lonStep = 2 * M_PI / xsphere_vres;

      for (int i = 0; i <= xsphere_ures; i++) {
         float lat = i * latStep;
         float cosLat = std::cos(lat);
         float sinLat = std::sin(lat);

         for (int j = 0; j <= xsphere_vres; j++) {
            float lon = j * lonStep;
            float cosLon = std::cos(lon);
            float sinLon = std::sin(lon);

            float x = sinLat * cosLon;
            float y = cosLat;
            float z = sinLat * sinLon;

            sphere.normal( {x,y,z} );
            sphere.vertex( x * radius, y * radius, z * radius,
                           map(j,0,xsphere_vres+1, 1.0, 0.0),
                           map(i,0,xsphere_ures+1, 1.0, 0.0));
         }
      }

      for (int i = 0; i < xsphere_ures; i++) {
         for (int j = 0; j < xsphere_vres; j++) {
            int idx0 = i * (xsphere_vres+1) + j;
            int idx1 = idx0 + 1;
            int idx2 = (i+1) * (xsphere_vres+1) + j;
            int idx3 = idx2 + 1;
            sphere.index(idx0);
            sphere.index(idx2);
            sphere.index(idx1);
            sphere.index(idx1);
            sphere.index(idx2);
            sphere.index(idx3);
         }
      }
      sphere.endShape();
      return sphere;
   }

   void sphere(float radius) {
      PShape sphere = createSphere(radius);
      shape( sphere );
      return;
   }

   // // Doesn't work becuase we need to flip the texture on Y axis
   // operator PImage() {
   //    PImage img = createImageFromTexture(glc.getColorBufferID());
   //    img.width = width;
   //    img.height = height;
   //    return img;
   // }

   void image(PGraphics gfx, float x, float y, float width=-1, float height=-1) {
      float left = x;
      if (width == -1) width = gfx.getWidth();
      if (height == -1) height = gfx.getHeight();
      float right = x + gfx.getWidth();
      float top = y;
      float bottom = y + gfx.getHeight();

      drawTexturedQuad( {left, bottom},
                        {right,bottom},
                        {right, top},
                        {left, top},
                        createImageFromTexture(gfx.getAsTexture()),
                        _shape.getTintColor() );
   }

   void image(PImage pimage, float left, float top, float right, float bottom) {
      if ( image_mode == CORNER ) {
         float iwidth = right;
         float iheight = bottom;
         right = left + iwidth;
         bottom = top + iheight;
      } else if ( image_mode == CENTER ) {
         float iwidth = right;
         float iheight = bottom;
         left = left - ( iwidth / 2.0 );
         top = top - ( iheight / 2.0 );
         right = left + iwidth;
         bottom = top + iheight;
      }
      drawTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom},
                       pimage, _shape.getTintColor()  );
   }

   void image(PImage pimage, float x, float y) {
      if ( image_mode == CORNER ) {
         image( pimage, x, y, pimage.width, pimage.height );;
    } else if ( image_mode == CORNERS ) {
         image( pimage, x, y, x + pimage.width, y + pimage.height );;
      } else   if (image_mode == CENTER) {
         image( pimage, x, y, pimage.width, pimage.height );
      } else {
         abort();
      }
   }

   void background(PImage bg) {
      image(bg,0,0);
   }

   void loadPixels() {
      flush();
      localFrame.blit( windowFrame );
      windowFrame.loadPixels( pixels );
      pixels_current = true;
   }

   void updatePixels() {
      flush();
      gl::framebuffer frame(width, height, SSAA, 1);
      frame.updatePixels( pixels );
      frame.blit( localFrame );
    }

   color get(int x, int y) {
      if (!pixels_current)
         loadPixels();
      return pixels[y*width+x];
   }

   void set(int x, int y, color c) {
      pixels[y*width+x] = c;
      updatePixels();
   }

   void set(int x, int y, PImage i) {
      image(i,x,y);
   }

   void rect(float x, float y, float _width, float _height) {
      PShape rect = createRect(x,y,_width,_height);
      shape( rect );
//      rect_opt = std::move( rect );
   }

   void ellipseMode(int mode) {
      ellipse_mode = mode;
   }

   void drawTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3,
                         PImage texture, color tint ) {
      PShape quad = createShape();
      quad.beginShape(TRIANGLES_NOSTROKE);
      quad.tint( tint );
      quad.textureMode(NORMAL);
      quad.texture(texture);
      quad.vertex( p0, {0.0, 0.0} );
      quad.vertex( p1, {1.0, 0.0} );
      quad.vertex( p2, {1.0, 1.0} );
      quad.vertex( p3, {0.0, 1.0} );
      quad.populateIndices( { 0,1,2, 0,2,3 } );
      quad.endShape();

      shape( quad );
   }

   void shape(PShape &pshape, float x, float y) {
      shape( pshape, x, y, pshape.width, pshape.height );
   }

   void shape(PShape &pshape, float x, float y, float width, float height) {
      pushMatrix();
      _shape.translate(x,y);
      _shape.scale(width / pshape.width, height / pshape.height);
      shape(pshape);
      popMatrix();
   }

   void shape(PShape &pshape) {
      pixels_current = false;
      if( pshape.isCompiled() ) {
         flush();
         auto &local = pshape.getBatch();
         if (pshape == _shape) {
            directDraw( local, PMatrix::Identity() );
         } else {
            directDraw( local, _shape.getShapeMatrix() );
         }
      } else {
         if (pshape == _shape) {
            pshape.flatten( batch, PMatrix::Identity() );
         } else {
            pshape.flatten( batch, _shape.getShapeMatrix() );
         }
      }
   }

   void ellipse(float x, float y, float width, float height) {
      PShape pshape = createEllipse(x, y, width, height);
      shape( pshape );
   }

   void arc(float x, float y, float width, float height, float start, float stop, int mode = DEFAULT) {
      PShape pshape = createArc(x, y, width, height, start, stop, ellipse_mode,  mode);
      shape( pshape );
   }

   void line(float x1, float y1, float z1, float x2, float y2, float z2) {
      PShape pshape = createLine( x1, y1, z1, x2, y2, z2);
      shape( pshape );
   }

   void point(float x, float y) {
      PShape pshape = createPoint(x, y);
      shape( pshape );
   }

   void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
      PShape pshape = createQuad(x1, y1, x2, y2, x3, y3, x4, y4);
      shape( pshape );
   }

   void triangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape pshape = createTriangle( x1, y1, x2, y2, x3, y3 );
      shape( pshape );
   }

   void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape pshape = createBezier(x1, y1, x2, y2, x3, y3, x4, y4);
      shape( pshape );
   }

   void endShape(int type = OPEN) {
      _shape.endShape(type);
      shape( _shape );
   }

   void rectMode(int mode){
      rect_mode = mode;
   }

   PShape createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape bezier = createShape();
      bezier.beginShape(POLYGON);
      bezier.copyStyle( _shape );
      bezier.vertex(x1, y1);
      bezier.bezierVertex(x2, y2, x3, y3, x4, y4);
      bezier.endShape(OPEN);
      return bezier;
   }

//   PShape rect_opt;
   PShape createRect(float x, float y, float width, float height) {
      if (rect_mode == CORNERS) {
         width = width - x;
         height = height - y;
      } else if (rect_mode == CENTER) {
         x = x - width / 2;
         y = y - height / 2;
      } else if (rect_mode == RADIUS) {
         width *= 2;
         height *= 2;
         x = x - width / 2;
         y = y - height / 2;
      }
      PShape shape = createShape();//( std::move(rect_opt) );
      shape.beginShape(CONVEX_POLYGON);
      shape.copyStyle( _shape );
      shape.normal(0,0,1);
      shape.vertex(x,y);
      shape.vertex(x+width,y);
      shape.vertex(x+width,y+height);
      shape.vertex(x,y+height);
      shape.populateIndices( { 0,1,2,0,2,3 } );
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
      PShape shape = createShape();
      shape.beginShape(POLYGON);
      shape.copyStyle( _shape );
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.vertex(x4, y4);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createLine(float x1, float y1, float z1, float x2, float y2, float z2) {
      PShape shape = createShape();
      shape.beginShape(POLYGON);
      shape.copyStyle( _shape );
      shape.vertex(x1,y1,z1);
      shape.vertex(x2,y2,z2);
      shape.endShape(OPEN);
      return shape;
   }

   PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape shape = createShape();
      shape.beginShape(TRIANGLES);
      shape.copyStyle( _shape );
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.populateIndices( { 0,1,2 } );
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createGroup() {
      PShape shape = createShape();
      shape.beginShape(GROUP);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createEllipse(float x, float y, float width, float height) {
      switch (ellipse_mode) {
      case CENTER:
         break;
      case RADIUS:
         width = width * 2.0;
         height = height * 2.0;
         break;
      case CORNER:
         x = x - width / 2.0;
         y = y - height / 2.0;
         break;
      case CORNERS:
         width = width - x;
         height = height - y;
         break;
      default:
         abort();
      }
      if (!_shape.isStroked() && !_shape.isTextureSet()) {
         // If there's no stroke and no texture use circle optimization here
         PShape shape = drawUntexturedFilledEllipse( x, y, width, height, _shape.getFillColor(), PMatrix::Identity() );
         return shape;
      } else if (_shape.getFillColor() == _shape.getStrokeColor() && !_shape.isTextureSet()) {
         PShape shape = drawUntexturedFilledEllipse( x, y, width + _shape.getStrokeWeight(), height + _shape.getStrokeWeight(),
                                                     _shape.getFillColor(), PMatrix::Identity() );
         return shape;
      } else {
         int NUMBER_OF_VERTICES=32;
         PShape shape = createShape();
         shape.beginShape(CONVEX_POLYGON);
         shape.copyStyle( _shape );
         shape.vertex( fast_ellipse_point( {x,y}, 0, width / 2.0, height /2.0) );
         for(int i = 1; i < NUMBER_OF_VERTICES-1; ++i) {
            shape.vertex( fast_ellipse_point( {x,y}, i, width / 2.0, height /2.0) );
         }
         shape.vertex( fast_ellipse_point( {x,y}, NUMBER_OF_VERTICES-1, width / 2.0, height /2.0) );
         shape.endShape(CLOSE);
         return shape;
      }
   }

   PShape createArc(float x, float y, float width, float height, float start,
                    float stop, int ellipse_mode, int mode) {

      if (ellipse_mode != RADIUS) {
         width /=2;
         height /=2;
      }
      int strokeMode, fillMode;
      switch( mode ) {
      case DEFAULT:
         strokeMode = OPEN_SKIP_FIRST_VERTEX_FOR_STROKE;
         fillMode = PIE;
         break;
      case PIE:
         strokeMode = CLOSE;
         fillMode = PIE;
         break;
      case CHORD:
         strokeMode = CLOSE;
         fillMode = CHORD;
         break;
      case OPEN:
         strokeMode = OPEN;
         fillMode = CHORD;
         break;
      default:
         abort();
      }

      if (!_shape.isStroked() && !_shape.isTextureSet()) {
         // If there's no stroke and no texture use circle optimization here
         PShape shape = createShape();
         if (fillMode == PIE) {
            // This isn't really a CONVEX_POLYGON but I know
            // it will fill ok with a traingle fan
            shape.beginShape(CONVEX_POLYGON);
            shape.copyStyle( _shape );
            shape.vertex(x,y,0.5,0.5);
         } else {
            // We could probably do better here to avoid the
            // triangulation pass but this works.
            shape.beginShape(POLYGON);
            shape.copyStyle( _shape );
         }
         shape.circleTexture();
         for(float i = start; i < stop; i = i + (TWO_PI / 8) ) {
            PVector pos = posOnUnitSquare( i );
            shape.vertex(
               x + pos.x * width,
               y + pos.y * height,
               0.5 + pos.x/2.0, 0.5 + pos.y/2.0 );
         }
         PVector pos = posOnUnitSquare( stop );
         shape.vertex(
            x + pos.x * width,
            y + pos.y * height,
            0.5 + pos.x/2.0, 0.5 + pos.y/2.0);
         if (fillMode == CHORD) {
            // For Chord mode we need to add two extra vertices at the radius at
            // begining and end.
            PVector pos = posOnUnitCircle( stop );
            shape.vertex(
               x + pos.x * width,
               y + pos.y * height,
               0.5 + pos.x/2.0, 0.5 + pos.y/2.0 );
            pos = posOnUnitCircle( start );
            shape.vertex(
               x + pos.x * width,
               y + pos.y * height,
               0.5 + pos.x/2.0, 0.5 + pos.y/2.0 );
         }
         shape.endShape(strokeMode);
         return shape;
      } else {
         PShape shape = createShape();
         shape.beginShape(CONVEX_POLYGON);
         shape.copyStyle( _shape );
         int NUMBER_OF_VERTICES=32;
         if ( fillMode == PIE ) {
            shape.vertex(x,y);
         }
         for(float i = start; i < stop; i = i + (TWO_PI / NUMBER_OF_VERTICES) ) {
            shape.vertex( { x + width * sinf(-i + HALF_PI), y + height * cosf(-i + HALF_PI) } );
         }
         shape.vertex( { x + width * sinf(-stop + HALF_PI), y + height * cosf(-stop + HALF_PI) } );
         shape.endShape(strokeMode);
         return shape;
      }
   }

   PShape createPoint(float x, float y) {
      PShape shape = createShape();
      shape.beginShape(POINTS);
      shape.copyStyle( _shape );
      shape.vertex(x,y);
      shape.endShape();
      return shape;
   }

   void smooth(int aaFactor=2, int aaMode=MSAA) {
      localFrame = gl::framebuffer(width, height, aaMode, aaFactor);
   }

   void noSmooth() {
      localFrame = gl::framebuffer(width, height, SSAA, 1);
   }

   void beginDraw() {}

   void endDraw() {
      flush();
   }

   int commit_draw() {
      endDraw();
      localFrame.blit( windowFrame );
      return std::exchange(flushes, 0);
   }

   void shader(PShader pshader, int kind = TRIANGLES) {
      flush();
      currentShader = pshader;
      glc.setShader( currentShader.getShader(), scene, batch );
      currentShader.set_uniforms();
      currentShader.bind();
   }

   void resetShader() {
      flush();
      defaultShader = currentShader;
   }

   void filter(PShader pshader) {
      PShader oldShader = currentShader;
      shader(pshader);
      // UPSIDE DOWN!:  image(createImageFromTexture(getAsTexture()),0,0);

      float left = 0;
      float right = 0 + getWidth();
      float top = 0;
      float bottom = 0 + getHeight();

      drawTexturedQuad( {left, bottom},
                        {right,bottom},
                        {right, top},
                        {left, top},
                        createImageFromTexture(getAsTexture()),
                        WHITE );
      shader(oldShader);
   }

   void filter(int kind) {
   }

   void filter(int kind, float param) {
   }

};

void PGraphics::filter(PShader shader) {
   return impl->filter(shader);
}

void PGraphics::filter(int kind) {
   return impl->filter(kind);
}

void PGraphics::filter(int kind, float param) {
   return impl->filter(kind,param);
}

int PGraphics::getWidth() const {
   return impl->getWidth();
}

int PGraphics::getHeight() const {
   return impl->getHeight();
}

unsigned int *PGraphics::getPixels() {
   return impl->getPixels();
}

PGraphics::PGraphics(int width, int height, int mode, int aaMode, int aaFactor)
   : impl(std::make_shared<PGraphicsImpl>(width, height, mode, aaMode, aaFactor)) {
   graphicsHandles().push_back(impl);
}

GLuint PGraphics::getAsTexture() {
   return impl->getAsTexture();
}

PImage PGraphics::getAsPImage() {
   return impl->getAsPImage();
}

void PGraphics::drawPImageWithCPU( PImage img, int x, int y ) {
   return impl->drawPImageWithCPU(img,x,y);
}

void PGraphics::save( const std::string &fileName ){
   return impl->save(fileName);
}

void PGraphics::saveFrame( std::string fileName ){
   return impl->saveFrame(fileName);
}

void PGraphics::pushMatrix(){
   return impl->pushMatrix();
}

void PGraphics::popMatrix(){
   return impl->popMatrix();
}

void PGraphics::translate(PVector t){
   return impl->_shape.translate(t);
}

void PGraphics::translate(float x, float y, float z){
   return impl->_shape.translate(x,y,z);
}

void PGraphics::transform(const PMatrix &transform_matrix){
   return impl->_shape.transform(transform_matrix);
}

void PGraphics::scale(float x, float y,float z){
   return impl->_shape.scale(x,y,z);
}

void PGraphics::scale(float x){
   return impl->_shape.scale(x);
}

void PGraphics::rotate(float angle, PVector axis){
   return impl->_shape.rotate(angle,axis);
}

void PGraphics::rotate(float angle){
   return impl->_shape.rotate(angle);
}

void PGraphics::rotateZ(float angle){
   return impl->_shape.rotateZ(angle);
}

void PGraphics::rotateY(float angle){
   return impl->_shape.rotateY(angle);
}

void PGraphics::rotateX(float angle){
   return impl->_shape.rotateX(angle);
}

float PGraphics::screenX(float x, float y, float z){
   return impl->scene.screenX(x,y,z);
}

float PGraphics::screenY(float x, float y, float z){
   return impl->scene.screenY(x,y,z);
}

void PGraphics::ortho(float left, float right, float bottom, float top, float near, float far){
   return impl->ortho(left,right,bottom,top,near,far);
}

void PGraphics::ortho(float left, float right, float bottom, float top){
   return impl->ortho(left,right,bottom,top);
}

void PGraphics::ortho(){
   return impl->ortho();
}

void PGraphics::perspective(float angle, float aspect, float minZ, float maxZ){
   return impl->perspective(angle,aspect,minZ,maxZ);
}

void PGraphics::perspective(){
   return impl->perspective();
}

void PGraphics::camera( float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ ){
   return impl->camera(eyeX,eyeY,eyeZ,centerX,centerY,centerZ,upX,upY,upZ);
}

void PGraphics::camera(){
   return impl->camera();
}

void PGraphics::directionalLight(float r, float g, float b, float nx, float ny, float nz){
   return impl->directionalLight(r,g,b,nx,ny,nz);
}

void PGraphics::pointLight(float r, float g, float b, float nx, float ny, float nz){
   return impl->pointLight(r,g,b,nx,ny,nz);
}

void PGraphics::spotLight(float r, float g, float b, float x, float y, float z,
                          float nx, float ny, float nz, float angle,
                          float concentration) {
   return impl->spotLight(r,g,b,x,y,z,nx,ny,nz,angle,concentration);
}

void PGraphics::lightFalloff(float r, float g, float b){
   return impl->lightFalloff(r,g,b);
}

void PGraphics::ambientLight(float r, float g, float b){
   return impl->ambientLight(r,g,b);
}

void PGraphics::lights(){
   return impl->lights();
}

void PGraphics::noLights(){
   return impl->noLights();
}

void PGraphics::textAlign(int a, int b) {
   impl->textAlign(a,b);
}

void PGraphics::textAlign(int a) {
   impl->textAlign(a);
}

void PGraphics::blendMode(int b){
   return impl->blendMode(b);
}

void PGraphics::hint(int type){
   return impl->hint(type);
}

void PGraphics::text(const std::string &text, float x, float y, float twidth, float theight) {
   return impl->text(text,x,y,twidth,theight);
}

void PGraphics::text(char c, float x, float y, float twidth, float theight) {
   return impl->text(c,x,y,twidth,theight);
}

void PGraphics::background(float r, float g, float b){
   return impl->background(r,g,b);
}

void PGraphics::background(color c){
   return impl->background(c.r,c.g,c.b);
}

void PGraphics::background(float gray){
   return impl->background(gray);
}

void PGraphics::background(float gray, float alpha){
   return impl->background(gray, alpha);
}

void PGraphics::imageMode(int iMode){
   return impl->imageMode(iMode);
}

PShape PGraphics::createBox(float w, float h, float d){
   return impl->createBox(w,h,d);
}

void PGraphics::box(float w, float h, float d){
   return impl->box(w,h,d);
}

void PGraphics::box(float size) {
   return impl->box(size,size,size);
}

void PGraphics::sphereDetail(float ures, float vres){
   return impl->sphereDetail(ures,vres);
}

void PGraphics::sphereDetail(float res){
   return impl->sphereDetail(res, res);
}

PShape PGraphics::createSphere( float radius ){
   return impl->createSphere(radius);
}

void PGraphics::sphere(float radius){
   return impl->sphere(radius);
}

void PGraphics::image(PGraphics gfx, float x, float y, float width, float height){
   return impl->image(gfx,x,y,width,height);
}

void PGraphics::image(PImage pimage, float left, float top, float right, float bottom){
   return impl->image(pimage,left,top,right,bottom);
}

void PGraphics::image(PImage pimage, float x, float y){
   return impl->image(pimage,x,y);
}

void PGraphics::background(PImage bg){
   return impl->background(bg);
}

void PGraphics::loadPixels(){
   return impl->loadPixels();
}

void PGraphics::updatePixels(){
   return impl->updatePixels();
}

color PGraphics::get(int x, int y){
   return impl->get(x,y);
}

void PGraphics::set(int x, int y, color c){
   return impl->set(x,y,c);
}

void PGraphics::set(int x, int y, PImage i){
   return impl->set(x,y,i);
}

void PGraphics::rect(float x, float y, float _width, float _height){
   return impl->rect(x,y,_width,_height);
}

void PGraphics::ellipseMode(int mode){
   return impl->ellipseMode(mode);
}

void PGraphics::shape(PShape &pshape, float x, float y){
   return impl->shape(pshape,x,y);
}

void PGraphics::shape(PShape &pshape, float x, float y, float width, float height){
   return impl->shape(pshape,x,y,width,height);
}

void PGraphics::shape(PShape &pshape){
   return impl->shape(pshape);
}

void PGraphics::ellipse(float x, float y, float width, float height){
   return impl->ellipse(x,y,width,height);
}

void PGraphics::ellipse(float x, float y, float radius){
   return impl->ellipse(x,y,radius,radius);
}

void PGraphics::arc(float x, float y, float width, float height, float start, float stop, int mode){
   return impl->arc(x,y,width,height,start,stop,mode);
}

void PGraphics::line(float x1, float y1, float z1, float x2, float y2, float z2){
   return impl->line(x1,y1,z1,x2,y2,z2);
}

void PGraphics::line(float x1, float y1, float x2, float y2){
   return impl->line(x1,y1,0,x2,y2,0);
}

void PGraphics::line(PVector start, PVector end){
   return impl->line(start.x,start.y, start.z, end.x,end.y, end.z);
}

void PGraphics::line(PLine l){
   return impl->line(l.start.x,l.start.y,l.start.z,
                     l.end.x,l.end.y,l.end.z);
}

void PGraphics::point(float x, float y){
   return impl->point(x,y);
}

void PGraphics::quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ){
   return impl->quad(x1,y1,x2,y2,x3,y3,x4,y4);
}

void PGraphics::triangle( float x1, float y1, float x2, float y2, float x3, float y3 ){
   return impl->triangle(x1,y1,x2,y2,x3,y3);
}

void PGraphics::bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4){
   return impl->bezier(x1,y1,x2,y2,x3,y3,x4,y4);
}

void PGraphics::beginShape(int style_) {
   return impl->_shape.beginShape(style_);
}

void PGraphics::tint(float r, float g, float b, float a){
   return impl->_shape.tint(r,g,b,a);
}

void PGraphics::tint(float r, float g, float b){
   return impl->_shape.tint(r,g,b);
}

void PGraphics::tint(float r, float a){
   return impl->_shape.tint(r,a);
}

void PGraphics::tint(float r){
   return impl->_shape.tint(r);
}

void PGraphics::tint(color c){
   return impl->_shape.tint(c);
}

void PGraphics::tint(color c, float a){
   return impl->_shape.tint(c, a);
}

void PGraphics::noTint() {
   return impl->_shape.noTint();
}

void PGraphics::fill(float r, float g, float b, float a){
   return impl->_shape.fill(r,g,b,a);
}

void PGraphics::fill(float r, float g, float b){
   return impl->_shape.fill(r,g,b);
}

void PGraphics::fill(float r, float a){
   return impl->_shape.fill(r,a);
}

void PGraphics::fill(float r){
   return impl->_shape.fill(r);
}

void PGraphics::fill(color c){
   return impl->_shape.fill(c);
}

void PGraphics::fill(color c, float a){
   return impl->_shape.fill(c, a);
}

void PGraphics::noFill() {
   return impl->_shape.noFill();
}

void PGraphics::stroke(float r, float g, float b, float a){
   return impl->_shape.stroke(r,g,b,a);
}

void PGraphics::stroke(float r, float g, float b){
   return impl->_shape.stroke(r,g,b);
}

void PGraphics::stroke(float r, float a){
   return impl->_shape.stroke(r,a);
}

void PGraphics::stroke(float r){
   return impl->_shape.stroke(r);
}

void PGraphics::stroke(color c){
   return impl->_shape.stroke(c);
}

void PGraphics::stroke(color c, float a){
   return impl->_shape.stroke(c, a);
}

void PGraphics::noStroke() {
   return impl->_shape.noStroke();
}

void PGraphics::strokeWeight(float x) {
   return impl->_shape.strokeWeight(x);
}

void PGraphics::strokeCap( int cap ) {
   return impl->_shape.strokeCap(cap);
}

void PGraphics::bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4){
   return impl->_shape.bezierVertex(x2,y2,x3,y3,x4,y4);
}

void PGraphics::bezierVertex(PVector v2, PVector v3, PVector v4){
   return impl->_shape.bezierVertex(v2,v2,v4);
}

void PGraphics::bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4){
   return impl->_shape.bezierVertex(x2,y2,z2,x3,y3,z3,x4,y4,z4);
}

void PGraphics::curveVertex( PVector c ) {
   return impl->_shape.curveVertex(c);
}

void PGraphics::curveVertex( float x, float y ) {
   return impl->_shape.curveVertex(x,y);
}

void PGraphics::curveTightness( float alpha ) {
   return impl->_shape.curveTightness(alpha);
}

void PGraphics::normal(PVector p){
   return impl->_shape.normal(p);
}

void PGraphics::normal(float x, float y, float z){
   return impl->_shape.normal(x,y,z);
}

void PGraphics::noNormal() {
   return impl->_shape.noNormal();
}

void PGraphics::vertex(float x, float y, float z) {
   return impl->_shape.vertex(x,y,z);
}

void PGraphics::vertex(float x, float y) {
   return impl->_shape.vertex(x,y);
}

void PGraphics::vertex(PVector p) {
   return impl->_shape.vertex(p);
}

void PGraphics::vertex(float x, float y, float z, float u, float v) {
   return impl->_shape.vertex(x,y,z,u,v);
}

void PGraphics::vertex(float x, float y, float u, float v) {
   return impl->_shape.vertex(x,y,u,v);
}

void PGraphics::vertex(PVector p, PVector2 t) {
   return impl->_shape.vertex(p,t);
}

void PGraphics::textureMode( int mode_ ){
   return impl->_shape.textureMode(mode_);
}

void PGraphics::texture(PImage img){
   return impl->_shape.texture(img);
}

void PGraphics::noTexture(){
   return impl->_shape.noTexture();
}

void PGraphics::endShape(int type) {
   return impl->endShape(type);
}

void PGraphics::rectMode(int mode){
   return impl->rectMode(mode);
}

PShape PGraphics::createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4){
   return impl->createBezier(x1,y1,x2,y2,x3,y3,x4,y4);
}

PShape PGraphics::createRect(float x, float y, float width, float height){
   return impl->createRect(x,y,width,height);
}

PShape PGraphics::createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ){
   return impl->createQuad(x1,y1,x2,y2,x3,y3,x4,y4);
}

PShape PGraphics::createLine(float x1, float y1, float z1, float x2, float y2, float z2){
   return impl->createLine(x1,y1,z1,x2,y2,z2);
}

PShape PGraphics::createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ){
   return impl->createTriangle(x1,y1,x2,y2,x3,y3);
}

PShape PGraphics::createGroup(){
   return impl->createGroup();
}

PShape PGraphics::createEllipse(float x, float y, float width, float height){
   return impl->createEllipse(x,y,width,height);
}

PShape PGraphics::createArc(float x, float y, float width, float height, float start,
                            float stop, int ellipse_mode, int mode){
   return impl->createArc(x,y,width,height,start,stop,ellipse_mode,mode);
}

PShape PGraphics::createPoint(float x, float y){
   return impl->createPoint(x,y);
}

void PGraphics::smooth(int aaFactor, int aaMode){
   return impl->smooth(aaFactor,aaMode);
}

void PGraphics::noSmooth(){
   return impl->noSmooth();
}

void PGraphics::beginDraw(){
   return impl->beginDraw();
}

void PGraphics::endDraw(){
   return impl->endDraw();
}

int PGraphics::commit_draw(){
   return impl->commit_draw();
}

void PGraphics::shader(PShader pshader, int kind){
   return impl->shader(pshader,kind);
}

void PGraphics::resetShader() { return impl->resetShader(); }

void PGraphics::resetMatrix(){
   return impl->_shape.resetMatrix();
}

static void PGraphics_releaseAllFrameBuffers() {
   for (auto i : graphicsHandles()) {
      if (auto p = i.lock()) {
         p->releaseResources();
      }
   }
}

void PGraphics::init() {
}

void PGraphics::close() {
   PGraphics_releaseAllFrameBuffers();
}

template <>
struct fmt::formatter<PGraphicsImpl> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const PGraphicsImpl& v, FormatContext& ctx) {
       return format_to(ctx.out(), "width={:<4} height={:<4}", v.width, v.height);
    }
};
