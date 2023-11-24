#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <string>
#include <unordered_map>

#include "processing_math.h"
#include "processing_utils.h"
#include "processing_color.h"
#include "processing_pshape.h"
#include "processing_pimage.h"
#include "processing_pfont.h"
#include "processing_enum.h"
#include "processing_opengl.h"

class PGraphics {
public:
   static void init();

   static void close();

   bool pixels_current = false;
   gl_framebuffer windowFrame;
   int textureMode_ = IMAGE;
   gl_context glc;

   int ellipse_mode = CENTER;
   int rect_mode = CORNER;
   int image_mode = CORNER;

   int width;
   int height;
   float aaFactor;

   PFont currentFont;
   int xTextAlign = LEFT;
   int yTextAlign = TOP;

   float xsphere_ures = 30;
   float xsphere_vres = 30;

   int xSmoothing = 1;
   PShape _shape;
   std::vector<unsigned int> pixels;

   std::vector<PMatrix> matrix_stack;

   std::unordered_map<std::string, PImage> words;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) noexcept : PGraphics() {
      *this = std::move(x);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x) noexcept {
      std::swap(textureMode_, x.textureMode_);
      std::swap(glc, x.glc);

      std::swap(pixels_current, x.pixels_current);
      std::swap(windowFrame, x.windowFrame);
      std::swap(ellipse_mode, x.ellipse_mode);
      std::swap(rect_mode, x.rect_mode);
      std::swap(image_mode, x.image_mode);

      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(aaFactor, x.aaFactor);

      std::swap(currentFont, x.currentFont);
      std::swap(xTextAlign, x.xTextAlign);
      std::swap(yTextAlign, x.yTextAlign);

      std::swap(xsphere_ures, x.xsphere_ures);
      std::swap(xsphere_vres, x.xsphere_vres);

      std::swap(xSmoothing, x.xSmoothing);
      std::swap(_shape, x._shape);
      std::swap(pixels, x.pixels);

      std::swap(matrix_stack, x.matrix_stack);
      std::swap(words, x.words);

      return *this;
   }

   PGraphics() {
   }

   ~PGraphics() {
   }

   PGraphics(int width, int height, int mode, float aaFactor) : glc( width, height, aaFactor ) {
      this->width = width;
      this->height = height;
      this->aaFactor = aaFactor;

      windowFrame = gl_framebuffer::constructMainFrame( width, height );

      glc.setProjectionMatrix( PMatrix::Identity() );
      glc.setViewMatrix( PMatrix::Identity() );

      textFont( createFont("DejaVuSans.ttf",12));
      noLights();
      camera();
      perspective();

      background(DEFAULT_GRAY);
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
      glc.flush();
      gl_framebuffer frame(width, height, 1, SSAA);
      glc.blit( frame );
      PImage image = createImage(width, height, 0);
      frame.saveFrame( image.pixels );
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

   void translate(PVector t) {
      _shape.transform( TranslateMatrix(t) );
   }

   void translate(float x, float y, float z=0.0 ) {
      translate(PVector{x,y,z});
   }

   void transform(const PMatrix &transform_matrix) {
      _shape.transform( transform_matrix );
   }

   void scale(float x, float y,float z = 1.0) {
      _shape.transform( ScaleMatrix(PVector{x,y,z}) );
   }

   void scale(float x) {
      scale(x,x,x);
   }

   void rotate(float angle, PVector axis) {
      _shape.transform( RotateMatrix(angle,axis) );
   }

   void rotate(float angle) {
      rotateZ(angle);
   }

   void rotateZ(float angle) {
      rotate(angle ,PVector{0,0,1});
   }

   void rotateY(float angle) {
      rotate(angle, PVector{0,1,0});
   }

   void rotateX(float angle) {
      rotate(angle, PVector{1,0,0});
   }

   float screenX(float x, float y, float z = 0.0) {
      return glc.screenX(x,y,z);
   }

   float screenY(float x, float y, float z = 0.0) {
      return glc.screenY(x,y,z);
   }

   PMatrix get_projection_matrix(float fov, float a, float near, float far) {
      float f = 1 / tan(0.5 * fov);
      float rangeInv = 1.0 / (near - far);
      float A = (near + far) * rangeInv;
      float B = near * far * rangeInv * 2;
      PMatrix ret = PMatrix{
         {f/a,  0,  0,  0} ,
         {0,  f,  0,  0} ,
         {0,  0,  A,  B} ,
         {0,  0, -1,  0}
      };
      return ret;
   }

   void ortho(float left, float right, float bottom, float top, float near, float far) {
      float tx = -(right + left) / (right - left);
      float ty = -(top + bottom) / (top - bottom);
      float tz = -(far + near) / (far - near);

      glc.flush();
      glc.setProjectionMatrix( {
            { 2/(right-left),               0,              0,  tx},
            {             0,  2/(top-bottom),              0,  ty},
            {           0,               0, -2/(far - near), tz},
            {0,               0,              0,   1}} );
   }

   void ortho(float left, float right, float bottom, float top) {
      ortho(left, right, bottom, top, bottom*2, top*2);
   }

   void ortho() {
      ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0);
   }

   void perspective(float angle, float aspect, float minZ, float maxZ) {
      glc.flush();
      glc.setProjectionMatrix( get_projection_matrix(angle, aspect, minZ, maxZ));
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

      PMatrix view{
         {     side.x,     side.y,     side.z, 0.0f},
         {      up.x,       up.y,       up.z, 0.0f},
         {-forward.x, -forward.y, -forward.z, 0.0f},
         {0.0f,       0.0f,       0.0f, 1.0f} };

      PMatrix translate{
         {1.0,    0,     0,    -eyeX},
         {0,    1.0,     0,    -eyeY},
         {0,      0,   1.0,    -eyeZ},
         {0.0f, 0.0f,  0.0f,    1.0f} };

      glc.flush();
      // Translate the camera to the origin
      glc.setViewMatrix( view * translate );
   }

   void camera() {
      camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
             width / 2.0, height / 2.0, 0, 0, 1, 0);
   }

   void directionalLight(float r, float g, float b, float nx, float ny, float nz) {
      glc.flush();
      glc.setLights( true );
      glc.setDirectionLightColor( {r/255.0f, g/255.0f, b/255.0f} );
      PVector worldDir = (_shape.getShapeMatrix() * PVector{nx,ny,nz}).normalize();
      glc.setDirectionLightVector( worldDir );
   }

   void pointLight(float r, float g, float b, float nx, float ny, float nz) {
      glc.flush();
      glc.setLights( true );
      PVector worldPos = (_shape.getShapeMatrix() * PVector{nx,ny,nz});
      glc.pushPointLightColor( { r/255.0f, g/255.0f,  b/255.0f } );
      glc.pushPointLightPosition( worldPos );
   }

   void lightFalloff(float r, float g, float b) {
      glc.flush();
      glc.setPointLightFalloff( { r, g, b } );
   }

   void ambientLight(float r, float g, float b) {
      glc.flush();
      glc.setLights( true );
      glc.setAmbientLight( { r/255.0f, g/255.0f, b/255.0f } );
   }

   void lights() {
      glc.flush();
      glc.setLights( true );
      glc.setAmbientLight(    { 0.5, 0.5, 0.5 } );
      glc.setDirectionLightColor(  { 0.5, 0.5, 0.5 } );
      glc.setDirectionLightVector( { 0.0, 0.0,-1.0 });
      glc.clearPointLights();
      glc.setPointLightFalloff(    { 1.0, 0.0, 0.0 });
      //lightSpecular(0, 0, 0);
   };

   void noLights() {
      glc.flush();
      glc.setLights( false );
      glc.clearPointLights();
   }

   void textFont(PFont font) {
      words.clear();
      currentFont = font;
   }

   void textAlign(int x, int y) {
      xTextAlign = x;
      yTextAlign = y;
   }

   void textAlign(int x) {
      xTextAlign = x;
   }

   void textSize(int size) {
      words.clear();
      currentFont = createFont(currentFont.name, size);
   }

   MAKE_GLOBAL(blendMode, glc);

   MAKE_GLOBAL(textAscent, currentFont);
   MAKE_GLOBAL(textDescent, currentFont);

   float textWidth(const std::string &text) {
      auto existing = words.find( text );
      if ( existing == words.end() ) {
         words[text] = currentFont.render_as_pimage(text);
      }
      return words[text].width;
   }

   void text(const std::string &text, float x, float y, float twidth = -1, float theight = -1) {
      auto existing = words.find( text );
      if ( existing == words.end() ) {
         words[text] = currentFont.render_as_pimage(text);
      }

      PImage text_image = words[text];

      twidth = text_image.width;
      theight = text_image.height;

      float ascent = textAscent();

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

   void background(float r, float g, float b) {
      auto color = flatten_color_mode({r,g,b,color::scaleA});
      glc.clear( color.r, color.g, color.b, color.a );
   }

   void background(color c) {
      background(c.r,c.g,c.b);
   }

   void background(float gray) {
      if (color::mode == HSB) {
         background(0,0,gray);
      } else {
         background(gray,gray,gray);
      }
   }

   void imageMode(int iMode) {
      image_mode = iMode;
   }

   PShape createBox(float w, float h, float d) {
      w = w / 2;
      h = h / 2;
      d = d / 2;

      PShape cube;
      cube.copyStyle( _shape );
      cube.beginShape(TRIANGLES);
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

   void box(float size) {
      box(size, size, size);
   }

   void sphereDetail(float ures, float vres) {
      xsphere_ures = ures;
      xsphere_vres = vres;
   }

   void sphereDetail(float res) {
      sphereDetail(res, res);
   }

   PShape createSphere( float radius ) {
      PShape sphere;
      sphere.copyStyle( _shape );
      sphere.beginShape(TRIANGLES);
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

   void image(PGraphics &gfx, float x, float y, float width=-1, float height=-1) {
      float left = x;
      if (width == -1) width = gfx.width;
      if (height == -1) height = gfx.height;
      float right = x + gfx.width;
      float top = y;
      float bottom = y + gfx.height;

      drawTexturedQuad( {left, bottom},
                        {right,bottom},
                        {right, top},
                        {left, top},
                        createImageFromTexture(gfx.glc.getColorBufferID()),
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
      glc.flush();
      glc.loadPixels( pixels );
      pixels_current = true;
   }

   void updatePixels() {
      glc.flush();
      glc.updatePixels( pixels );
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
      rect_opt = std::move( rect );
   }

   void ellipseMode(int mode) {
      ellipse_mode = mode;
   }

   void drawTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3,
                         PImage texture, color tint ) {
      PShape quad;
      quad.tint( tint );
      quad.textureMode(NORMAL);
      quad.texture(texture);
      quad.beginShape(TRIANGLES_NOSTROKE);
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
      translate(x,y);
      scale(width / pshape.width, height / pshape.height);
      shape(pshape);
      popMatrix();
   }

   void shape(PShape &pshape) {
      pixels_current = false;
      pshape.draw( glc, _shape.getShapeMatrix() );
   }

   void ellipse(float x, float y, float width, float height) {
      PShape pshape = createEllipse(x, y, width, height);
      shape( pshape );
   }

   void ellipse(float x, float y, float radius) {
      ellipse(x,y,radius,radius);
   }

   void arc(float x, float y, float width, float height, float start, float stop, int mode = DEFAULT) {
      PShape pshape = createArc(x, y, width, height, start, stop, ellipse_mode,  mode);
      shape( pshape );
   }

   void line(float x1, float y1, float x2, float y2) {
      line( x1, y1, 0.0f, x2, y2, 0.0f);
   }

   void line(float x1, float y1, float z1, float x2, float y2, float z2) {
      PShape pshape = createLine( x1, y1, z1, x2, y2, z2);
      shape( pshape );
   }

   void line(PVector start, PVector end) {
      line(start.x,start.y, start.z, end.x,end.y, end.z);
   }

   void line(PLine l) {
      line(l.start, l.end);
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

   MAKE_GLOBAL(beginShape, _shape);
   MAKE_GLOBAL(tint, _shape);
   MAKE_GLOBAL(fill, _shape);
   MAKE_GLOBAL(noStroke, _shape);
   MAKE_GLOBAL(stroke, _shape);
   MAKE_GLOBAL(strokeWeight, _shape);
   MAKE_GLOBAL(strokeCap, _shape);
   MAKE_GLOBAL(noFill, _shape);
   MAKE_GLOBAL(noTint, _shape);
   MAKE_GLOBAL(bezierVertex, _shape);
   MAKE_GLOBAL(curveVertex, _shape);
   MAKE_GLOBAL(curveTightness, _shape);
   MAKE_GLOBAL(normal, _shape);
   MAKE_GLOBAL(noNormal, _shape);
   MAKE_GLOBAL(vertex, _shape);
   MAKE_GLOBAL(texture, _shape);
   MAKE_GLOBAL(noTexture, _shape);
   MAKE_GLOBAL(textureMode, _shape);

   void endShape(int type = OPEN) {
      _shape.endShape(type);
      _shape.draw( glc, PMatrix::Identity() );
   }

   void rectMode(int mode){
      rect_mode = mode;
   }

   PShape createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape bezier;
      bezier.copyStyle( _shape );
      bezier.beginShape(POLYGON);
      bezier.vertex(x1, y1);
      bezier.bezierVertex(x2, y2, x3, y3, x4, y4);
      bezier.endShape(OPEN);
      return bezier;
   }

   PShape rect_opt;
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
      PShape shape( std::move(rect_opt) );
      shape.copyStyle( _shape );
      shape.beginShape(CONVEX_POLYGON);
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
      PShape shape;
      shape.copyStyle( _shape );
      shape.beginShape(POLYGON);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.vertex(x4, y4);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createLine(float x1, float y1, float z1, float x2, float y2, float z2) {
      PShape shape;
      shape.copyStyle( _shape );
      shape.beginShape(POLYGON);
      shape.vertex(x1,y1,z1);
      shape.vertex(x2,y2,z2);
      shape.endShape(OPEN);
      return shape;
   }

   PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape shape;
      shape.copyStyle( _shape );
      shape.beginShape(TRIANGLES);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.populateIndices( { 0,1,2 } );
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createGroup() {
      PShape shape;
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
      } else {
         int NUMBER_OF_VERTICES=32;
         PShape shape;
         shape.copyStyle( _shape );
         shape.beginShape(CONVEX_POLYGON);
         shape.vertex( fast_ellipse_point( {x,y}, 0, width / 2.0, height /2.0) );
         for(int i = 1; i < NUMBER_OF_VERTICES-1; ++i) {
            shape.vertex( fast_ellipse_point( {x,y}, i, width / 2.0, height /2.0) );
         }
         shape.vertex( fast_ellipse_point( {x,y}, NUMBER_OF_VERTICES-1, width / 2.0, height /2.0) );
         shape.endShape(CLOSE);
         return shape;
      }
   }

   PVector posOnUnitSquare( float angle ) {
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

   PVector posOnUnitCircle( float angle ) {
      return { sinf(-angle + HALF_PI), cosf(-angle + HALF_PI) };
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

         PShape shape;
         shape.copyStyle( _shape );
         shape.circleTexture();
         if (fillMode == PIE) {
            // This isn't really a CONVEX_POLYGON but I know
            // it will fill ok with a traingle fan
            shape.beginShape(CONVEX_POLYGON);
            shape.vertex(x,y,0.5,0.5);
         } else {
            // We could probably do better here to avoid the
            // triangulation pass but this works.
            shape.beginShape(POLYGON);
         }
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
         PShape shape;
         shape.copyStyle( _shape );
         shape.beginShape(CONVEX_POLYGON);
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
      PShape shape;
      shape.copyStyle( _shape );
      shape.beginShape(POINTS);
      shape.vertex(x,y);
      shape.endShape();
      return shape;
   }


// ----
// End shapes managed by Pshape.
// ----


   void smooth(int AA=2) {
      xSmoothing = AA;
   }
   void noSmooth() {
      // Doesn't yet apply to actual graphics
      xSmoothing = 1;
   }

   void beginDraw() {}
   void endDraw() {
      glc.flush();
   }

   void commit_draw() {
      endDraw();
      glc.blit( windowFrame );
   }

   PGraphics createGraphics(int width, int height, int mode = P2D) {
      return { width, height, mode, aaFactor };
   }

};



#endif
