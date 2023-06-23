#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <string>

#include "processing_math.h"
#include "processing_utils.h"
#include "processing_color.h"
#include "processing_pshape.h"
#include "processing_pimage.h"
#include "processing_pfont.h"
#include "processing_enum.h"
#include "processing_opengl.h"

class PGraphics : public TriangleDrawer {
public:
   static void init();

   static void close();

   PTexture currentTexture;
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

   bool xSmoothing = true;
   PShape _shape;
   std::vector<unsigned int> pixels;

   bool lights_ = false;
   std::array<float,3> ambientLightColor;
   std::array<float,3> directionLightColor;
   std::array<float,3> directionLightVector;
   std::array<float,3> pointLightColor;
   std::array<float,3> pointLightPosition;
   std::array<float,3> pointLightFalloff;

   PMatrix projection_matrix = PMatrix::Identity();
   PMatrix view_matrix = PMatrix::Identity();
   PMatrix move_matrix = PMatrix::Identity();

   std::vector<PMatrix> matrix_stack;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) noexcept {
      *this = std::move(x);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x) noexcept {
      std::swap(currentTexture, x.currentTexture);
      std::swap(textureMode_, x.textureMode_);
      std::swap(glc, x.glc);

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

      std::swap(lights_, x.lights_);
      std::swap(ambientLightColor, x.ambientLightColor);
      std::swap(directionLightColor, x.directionLightColor);
      std::swap(directionLightVector, x.directionLightVector);
      std::swap(pointLightColor, x.pointLightColor);
      std::swap(pointLightPosition, x.pointLightPosition);
      std::swap(pointLightFalloff, x.pointLightFalloff);

      std::swap(projection_matrix, x.projection_matrix);
      std::swap(view_matrix, x.view_matrix);
      std::swap(move_matrix, x.move_matrix);

      std::swap(matrix_stack, x.matrix_stack);

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

      move_matrix = PMatrix::Identity();
      view_matrix = PMatrix::Identity();
      projection_matrix = PMatrix::Identity();

      textFont( createFont("DejaVuSans.ttf",12));
      noLights();
      camera();
      perspective();

      background(DEFAULT_GRAY);
   }

   void save( const std::string &fileName ) {
      flush();
      glc.saveFrame( fileName );
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
      matrix_stack.push_back(move_matrix);
   }

   void popMatrix() {
      move_matrix = matrix_stack.back();
      matrix_stack.pop_back();
   }

   void translate(float x, float y, float z=0.0 ) {
      move_matrix = move_matrix * TranslateMatrix(PVector{x,y,z});
   }

   void transform(const PMatrix &transform_matrix) {
      move_matrix = move_matrix * transform_matrix;
   }

   void scale(float x, float y,float z = 1.0) {
      move_matrix = move_matrix * ScaleMatrix(PVector{x,y,z});
   }

   void scale(float x) {
      scale(x,x,x);
   }

   void rotate(float angle, PVector axis) {
      move_matrix = move_matrix * RotateMatrix(angle,axis);
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
      PVector4 in = { x, y, z, 1.0 };
      return (projection_matrix * view_matrix * in).data[0];
   }

   float screenY(float x, float y, float z = 0.0) {
      PVector4 in = { x, y, z, 1.0 };
      return (projection_matrix * view_matrix * in).data[1];
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

      projection_matrix = PMatrix{
         { 2/(right-left),               0,              0,  tx},
         {             0,  2/(top-bottom),              0,  ty},
         {           0,               0, -2/(far - near), tz},
         {0,               0,              0,   1}
      };
   }

   void ortho(float left, float right, float bottom, float top) {
      ortho(left, right, bottom, top, bottom*2, top*2);
   }

   void ortho() {
      ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0);
   }

   void perspective(float angle, float aspect, float minZ, float maxZ) {
      flush();
      projection_matrix = get_projection_matrix(angle, aspect, minZ, maxZ);
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

      flush();
      // Translate the camera to the origin
      view_matrix = view * translate;
   }

   void camera() {
      camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
             width / 2.0, height / 2.0, 0, 0, 1, 0);
   }

   void directionalLight(float r, float g, float b, float nx, float ny, float nz) {
      flush();
      lights_ = true;
      directionLightColor = {r/255.0f, g/255.0f, b/255.0f};
      directionLightVector = {nx, ny, nz};
   }

   void pointLight(float r, float g, float b, float nx, float ny, float nz) {
      flush();
      lights_ = true;
      pointLightColor = { r/255.0f, g/255.0f,  b/255.0f };
      pointLightPosition = {nx, ny, nz};
   }

   void lightFalloff(float r, float g, float b) {
      flush();
      pointLightFalloff = { r, g, b };
   }

   void ambientLight(float r, float g, float b) {
      flush();
      lights_ = true;
      ambientLightColor = { r/255.0f, g/255.0f, b/255.0f };
   }

   void lights() {
      flush();
      lights_ = true;
      ambientLightColor =    { 0.5, 0.5, 0.5 };
      directionLightColor =  { 0.5, 0.5, 0.5 };
      directionLightVector = { 0.0, 0.0,-1.0 };
      pointLightColor =      { 0.0, 0.0, 0.0 };
      pointLightPosition =   { 0.0, 0.0, 0.0 };
      pointLightFalloff =    { 1.0, 0.0, 0.0 };
      //lightSpecular(0, 0, 0);
   };

   void noLights() {
      flush();
      lights_ = false;
      ambientLightColor =    { 0.0, 0.0, 0.0};
      directionLightColor =  { 0.0, 0.0, 0.0};
      directionLightVector = { 0.0, 0.0,-1.0};
      pointLightColor =      { 0.0, 0.0, 0.0};
      pointLightPosition =   { 0.0, 0.0, 0.0};
      pointLightFalloff =    { 1.0, 0.0, 0.0};
   }

   void textFont(PFont font) {
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
      currentFont = createFont(currentFont.name, size);
   }


   void text(std::string text, float x, float y, float twidth = -1, float theight = -1);

   void text(char c, float x, float y, float twidth = -1, float theight = -1) {
      std::string s(&c,1);
      text(s,x,y,twidth,theight);
   }

   void background(float r, float g, float b) {
      auto color = flatten_color_mode(r,g,b,color::scaleA);
      glc.clear( color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0 );
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

   void drawTriangles( const std::vector<PVector> &vertices,
                       const std::vector<PVector> &normals,
                       const std::vector<PVector> &coords,
                       const std::vector<unsigned short> &indices,
                       const std::vector<color> &color,
                       const PMatrix &xxmove_matrix) {
     if (lights_) {
         glc.reserve( vertices.size(), xxmove_matrix,  projection_matrix.data,
                      view_matrix.data,
                      directionLightColor.data(),
                      directionLightVector.data(),
                      ambientLightColor.data(),
                      pointLightColor.data(),
                      pointLightPosition.data(),
                      pointLightFalloff.data());
    } else {
         std::array<float,3> white = {1.0f,1.0f,1.0f};
         std::array<float,3> black = {0.0f,0.0f,0.0f};
         glc.reserve( vertices.size(), xxmove_matrix, projection_matrix.data,
                      view_matrix.data,
                      black.data(),
                      directionLightVector.data(),
                      white.data(),
                      black.data(),
                      pointLightPosition.data(),
                      pointLightFalloff.data());
     }
     glc.drawTriangles( vertices, normals, coords, indices, color);
   }

   void flush() {
      if (lights_) {
         glc.flush( projection_matrix.data,
                    view_matrix.data,
                    directionLightColor.data(),
                    directionLightVector.data(),
                    ambientLightColor.data(),
                    pointLightColor.data(),
                    pointLightPosition.data(),
                    pointLightFalloff.data());
      } else {
         std::array<float,3> white = {1.0f,1.0f,1.0f};
         std::array<float,3> black = {0.0f,0.0f,0.0f};
         glc.flush( projection_matrix.data,
                    view_matrix.data,
                    black.data(),
                    directionLightVector.data(),
                    white.data(),
                    black.data(),
                    pointLightPosition.data(),
                    pointLightFalloff.data());
      }
   }

   void imageMode(int iMode) {
      image_mode = iMode;
   }

   void box(float w, float h, float d) {
      w = w / 2;
      h = h / 2;
      d = d / 2;

      PShape cube;
      cube.copyStyle( _shape );
      cube.beginShape(TRIANGLES);
      cube.texture(currentTexture);

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

      cube.indices = {
         0,1,2, 0,2,3, 4,5,6, 4,6,7,
         8,9,10, 8,10,11, 12,13,14, 12,14,15,
         16,17,18, 16,18,19, 20,21,22, 20,22,23
      };

      cube.endShape();

      shape( cube );
   };

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

   void sphere(float radius) {

      PShape sphere;
      sphere.copyStyle( _shape );
      sphere.beginShape(TRIANGLES);
      sphere.texture(currentTexture);

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

      shape( sphere );
   }

   void image(PGraphics &gfx, int x, int y) {
      float left = x;
      float right = x + gfx.width;
      float top = y;
      float bottom = y + gfx.height;
      PTexture texture = glc.getTexture( gfx.glc );
      drawTexturedQuad( {left, top},
                        {right,top},
                        {right, bottom},
                        {left, bottom},
                        texture );
   }

   void image(PImage &pimage, float left, float top, float right, float bottom) {
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
      PTexture texture = glc.getTexture( pimage.surface );
      drawTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom},
                       texture );
   }

   void image(PImage &pimage, float x, float y) {
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

   void background(PImage &bg) {
      image(bg,0,0);
   }

   void loadPixels() {
      flush();
      glc.loadPixels( pixels );
   }

   void updatePixels() {
      flush();
      glc.updatePixels( pixels );
   }

   color get(int x, int y) {
      loadPixels();
      return pixels[y*width+x];
   }

   void set(int x, int y, color c) {
      pixels[y*width+x] = c;
      updatePixels();
   }

   void rect(int x, int y, int _width, int _height) {
      PShape pshape = createRect(x,y,_width,_height);
      shape( pshape );
   }

   void ellipseMode(int mode) {
      ellipse_mode = mode;
   }

   void drawTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3,
                         PTexture texture ) {
      PShape quad;
      quad.textureMode(NORMAL);
      quad.texture(texture);
      quad.beginShape(TRIANGLES_NOSTROKE);
      quad.vertex( p0, {0.0, 0.0} );
      quad.vertex( p1, {1.0, 0.0} );
      quad.vertex( p2, {1.0, 1.0} );
      quad.vertex( p3, {0.0, 1.0} );
      quad.indices = { 0,1,2, 0,2,3 };
      quad.endShape();

      shape( quad );
   }


   void shape(PShape &pshape, float x, float y) {
      pushMatrix();
      translate(x,y);
      pshape.draw( *this, move_matrix );
      popMatrix();
   }

   void shape(PShape &pshape) {
      shape(pshape,0,0);
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
      PShape pshape = createLine( x1, y1, z1, x2, y2, z1);
      shape( pshape );
   }

   void line(PVector start, PVector end) {
      line(start.x,start.y, end.x,end.y);
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
   MAKE_GLOBAL(normal, _shape);
   MAKE_GLOBAL(noNormal, _shape);
   MAKE_GLOBAL(vertex, _shape);
   MAKE_GLOBAL(texture, _shape);
   MAKE_GLOBAL(noTexture, _shape);
   MAKE_GLOBAL(textureMode, _shape);

   void texture(PImage &img) {
      texture( glc.getTexture( img.surface ) );
   }


   void endShape(int type = OPEN) {
      _shape.endShape(type);
      shape(_shape, 0,0);
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
      PShape shape;
      shape.copyStyle( _shape );
      shape.beginShape(CONVEX_POLYGON);
      shape.vertex(x,y);
      shape.vertex(x+width,y);
      shape.vertex(x+width,y+height);
      shape.vertex(x,y+height);
      shape.indices = { 0,1,2,0,2,3 };
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
      shape.vertex(x1,y1,z2);
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
      shape.indices = { 0,1,2 };
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
      if (_shape.stroke_color.a == 0.0 && _shape.texture_.layer == 0) {
         // If there's no stroke and no texture use circle optimization here
         PShape shape = drawUntexturedFilledEllipse( x, y, width, height, _shape.fill_color );
         return shape;
      } else {
         int NUMBER_OF_VERTICES=32;
         PShape shape;
         shape.copyStyle( _shape );
         shape.beginShape(TRIANGLES);
         shape.vertex( fast_ellipse_point( {x,y}, 0, width / 2.0, height /2.0) );
         for(int i = 1; i < NUMBER_OF_VERTICES-1; ++i) {
            shape.vertex( fast_ellipse_point( {x,y}, i, width / 2.0, height /2.0) );
            shape.indices.push_back( 0 );
            shape.indices.push_back( i );
            shape.indices.push_back( i+1 );
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

      if (_shape.stroke_color.a == 0.0 && _shape.texture_.layer == 0) {
         // If there's no stroke and no texture use circle optimization here

         PShape shape;
         shape.copyStyle( _shape );
         // Hack to get the circle texture
         shape.texture( PTexture::circle() );
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



   void noSmooth() {
      // Doesn't yet apply to actual graphics
      xSmoothing = false;
   }

   void beginDraw() {}
   void endDraw() {}

   void commit_draw() {
      flush();
      glc.draw_main();
   }

   PGraphics createGraphics(int width, int height, int mode = P2D) {
      PGraphics pg{ width, height, mode, aaFactor };
      pg.view_matrix = view_matrix;
      pg.projection_matrix = projection_matrix;
      return pg;
   }

};



#endif
