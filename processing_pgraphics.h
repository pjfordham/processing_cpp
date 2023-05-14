#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <SDL2/SDL.h>

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream
#include <string>

#include "processing_pshader.h"
#include "processing_math.h"
#include "processing_color.h"
#include "processing_pshape.h"
#include "processing_pimage.h"
#include "processing_pfont.h"
#include "processing_enum.h"
#include "processing_opengl.h"

class PGraphics {
public:
   PTexture currentTexture;
   gl_context glc;

   color stroke_color = WHITE;
   color fill_color = WHITE;
   color tint_color = WHITE;

   int stroke_weight = 1;
   int line_end_cap = ROUND;
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
   std::vector<Uint32> pixels;

   bool lights_ = false;
   std::array<float,3> ambientLightColor;
   std::array<float,3> directionLightColor;
   std::array<float,3> directionLightVector;
   std::array<float,3> pointLightColor;
   std::array<float,3> pointLightPosition;
   std::array<float,3> pointLightFalloff;

   Eigen::Matrix4f projection_matrix; // Default is identity
   Eigen::Matrix4f view_matrix; // Default is identity
   Eigen::Matrix4f move_matrix; // Default is identity

   std::vector<Eigen::Matrix4f> matrix_stack;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) noexcept {
      *this = std::move(x);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x) noexcept {
      std::swap(currentTexture, x.currentTexture);
      std::swap(glc, x.glc);

      std::swap(stroke_color, x.stroke_color);
      std::swap(fill_color, x.fill_color);
      std::swap(tint_color, x.tint_color);

      std::swap(stroke_weight, x.stroke_weight);
      std::swap(line_end_cap, x.line_end_cap);
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

      move_matrix = Eigen::Matrix4f::Identity();
      view_matrix = Eigen::Matrix4f::Identity();
      projection_matrix = Eigen::Matrix4f::Identity();

      textFont( createFont("DejaVuSans.ttf",12));
      noLights();
      camera();
      perspective();
      noTexture();

      background(DEFAULT_GRAY);
      stroke(BLACK);
      fill(WHITE);
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
      flush();
      glc.saveFrame( fileName );
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

   void transform(Eigen::Matrix4f transform_matrix) {
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
      rotate(angle ,PVector{0,0,1});
   }

   void rotateY(float angle) {
      rotate(angle, PVector{0,1,0});
   }

   void rotateX(float angle) {
      rotate(angle, PVector{1,0,0});
   }

   Eigen::Matrix4f get_projection_matrix(float fov, float a, float near, float far) {
      float f = 1 / tan(0.5 * fov);
      float rangeInv = 1.0 / (near - far);
      float A = (near + far) * rangeInv;
      float B = near * far * rangeInv * 2;
      Eigen::Matrix4f ret = Eigen::Matrix4f{
         { f/a,  0,  0,  0 },
         {   0,  f,  0,  0 },
         {   0,  0,  A,  B },
         {   0,  0, -1,  0 }
      };
      return ret;
   }

   void ortho(float left, float right, float bottom, float top, float near, float far) {
      float tx = -(right + left) / (right - left);
      float ty = -(top + bottom) / (top - bottom);
      float tz = -(far + near) / (far - near);

      projection_matrix = Eigen::Matrix4f{
         { 2/(right-left),               0,              0,  tx },
         {              0,  2/(top-bottom),              0,  ty },
         {              0,               0, -2/(far - near), tz },
         {              0,               0,              0,   1 }
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

      Eigen::Vector3f center = Eigen::Vector3f{centerX, centerY, centerZ};
      Eigen::Vector3f eye =  Eigen::Vector3f{eyeX,eyeY,eyeZ};
      Eigen::Vector3f _up = Eigen::Vector3f{upX,upY,upZ};

      Eigen::Vector3f forward = (center - eye).normalized();
      Eigen::Vector3f side = forward.cross(_up).normalized();
      Eigen::Vector3f up = side.cross(forward).normalized();

      Eigen::Matrix4f view{
         {    side[0],     side[1],     side[2], 0.0f},
         {      up[0],       up[1],       up[2], 0.0f},
         {-forward[0], -forward[1], -forward[2], 0.0f},
         {       0.0f,        0.0f,        0.0f, 1.0f} };

      Eigen::Matrix4f translate{
         {1.0,    0,     0,    -eyeX} ,
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


   void text(std::string text, float x, float y, float twidth = -1, float theight = -1) {
      SDL_Surface *surface = currentFont.render_text(text, fill_color);

      twidth = surface->w;
      theight = surface->h;

      PTexture texture = glc.getTexture( surface );

      SDL_FreeSurface(surface);

      // this works well enough for the Letters.cc example but it's not really general
      if ( xTextAlign == CENTER ) {
         x = x - twidth / 2;
      }
      if ( yTextAlign == CENTER ) {
         y = y - theight / 2;
      }

      drawTexturedQuad({x,y},{x+twidth,y},{x+twidth,y+theight},{x,y+theight},
                       texture, WHITE);
   }

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
                       color color) {
      if (lights_) {
        glc.reserve( vertices.size(), move_matrix,  projection_matrix.data(),
                      view_matrix.data(),
                      directionLightColor.data(),
                      directionLightVector.data(),
                      ambientLightColor.data(),
                      pointLightColor.data(),
                      pointLightPosition.data(),
                      pointLightFalloff.data());
      } else {
         std::array<float,3> white = {1.0f,1.0f,1.0f};
         std::array<float,3> black = {0.0f,0.0f,0.0f};
         glc.reserve( vertices.size(), move_matrix, projection_matrix.data(),
                    view_matrix.data(),
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
          glc.flush( projection_matrix.data(),
                 view_matrix.data(),
                 directionLightColor.data(),
                 directionLightVector.data(),
                 ambientLightColor.data(),
                 pointLightColor.data(),
                 pointLightPosition.data(),
                 pointLightFalloff.data());
      } else {
         std::array<float,3> white = {1.0f,1.0f,1.0f};
         std::array<float,3> black = {0.0f,0.0f,0.0f};
         glc.flush( projection_matrix.data(),
                    view_matrix.data(),
                    black.data(),
                    directionLightVector.data(),
                    white.data(),
                    black.data(),
                    pointLightPosition.data(),
                    pointLightFalloff.data());
      }
   }

   void noTexture() {
      currentTexture = {};
   }

   void texture(PImage &img) {
      currentTexture = glc.getTexture( img );
   }

   void imageMode(int iMode) {
      image_mode = iMode;
   }

   void tint(float r,float g,  float b, float a) {
      tint_color = flatten_color_mode(r,g,b,a);
   }

   void tint(float r,float g, float b) {
      tint(r,g,b,color::scaleA);
   }

   void tint(float r,float a) {
      if (color::mode == HSB) {
         tint(0,0,r,a);
      } else {
         tint(r,r,r,a);
      }
   }
   void tint(float r) {
      if (color::mode == HSB) {
         tint(r,0,0,color::scaleA);
      } else {
         tint(r,r,r,color::scaleA);
      }
   }

   void tint(color c) {
      tint(c.r,c.g,c.b,c.a);
   }

   void noTint() {
      tint_color = WHITE;
   }

   void box(float w, float h, float d) {
      w = w / 2;
      h = h / 2;
      d = d / 2;

      PShape cube;
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
                        texture, tint_color);
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
      PTexture texture = glc.getTexture( pimage );
      drawTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom},
                       texture, tint_color);
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

   // ----
   // Begin shapes managed by Pshape.
   // ----
   void fill(float r,float g,  float b, float a) {
      fill_color = flatten_color_mode(r,g,b,a);
   }

   void fill(float r,float g, float b) {
      fill(r,g,b,color::scaleA);
   }

   void fill(float r,float a) {
      if (color::mode == HSB) {
         fill(0,0,r,a);
      } else {
         fill(r,r,r,a);
      }
   }

   void fill(float r) {
      if (color::mode == HSB) {
         fill(0,0,r,color::scaleA);
      } else {
         fill(r,r,r,color::scaleA);
      }
   }

   void fill(class color color) {
      fill(color.r,color.g,color.b,color.a);
   }

   void fill(class color color, float a) {
      fill(color.r,color.g,color.b,a);
   }

   void stroke(float r,float g,  float b, float a) {
      stroke_color = flatten_color_mode(r,g,b,a);
   }

   void stroke(float r,float g, float b) {
      stroke(r,g,b,color::scaleA);
   }

   void stroke(float r,float a) {
      if (color::mode == HSB) {
         stroke(0,0,r,a);
      } else {
         stroke(r,r,r,a);
      }
   }

   void rect(int x, int y, int _width, int _height) {
      PShape pshape = createRect(x,y,_width,_height);
      shape( pshape );
   }

   void stroke(float r) {
      if (color::mode == HSB) {
         stroke(r,0,0,color::scaleA);
      } else {
         stroke(r,r,r,color::scaleA);
      }
   }

   void stroke(color c) {
      stroke(c.r,c.g,c.b,c.a);
   }

   void strokeWeight(int x) {
      stroke_weight = x;
   }

   void noStroke() {
      stroke_color = {0,0,0,0};
   }

   void noFill() {
      fill_color = {0,0,0,0};
   }

   void ellipseMode(int mode) {
      ellipse_mode = mode;
   }

   void printMatrix4f(const Eigen::Matrix4f& mat) {
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
   }

   void drawTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3,
                         PTexture texture, color tint) {
      PShape quad;
      quad.texture(texture);
      quad.beginShape(TRIANGLES);
      quad.vertex( p0, {0.0, 0.0} );
      quad.vertex( p1, {1.0, 0.0} );
      quad.vertex( p2, {1.0, 1.0} );
      quad.vertex( p3, {0.0, 1.0} );
      quad.indices = { 0,1,2, 0,2,3 };
      quad.endShape();

      shape( quad );
   }

   PLine drawLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) const {
      PLine l1{ p1, p2 };
      PLine l2{ p2, p3 };
      PLine low_l1 = l1.offset(-half_weight);
      PLine high_l1 = l1.offset(half_weight);
      PLine low_l2 = l2.offset(-half_weight);
      PLine high_l2 = l2.offset(half_weight);
      return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
   }

   PShape drawLinePoly(int points, const PVector *p, int weight, bool closed)  {
      PLine start;
      PLine end;

      PShape triangle_strip;
      triangle_strip.beginShape(TRIANGLE_STRIP);

      float half_weight = weight / 2.0;
      if (closed) {
         start = drawLineMitred(p[points-1], p[0], p[1], half_weight );
         end = start;
      } else {
         PVector normal = (p[1] - p[0]).normal();
         normal.normalize();
         normal.mult(half_weight);
         start = {  p[0] + normal, p[0] - normal };
         normal = (p[points-1] - p[points-2]).normal();
         normal.normalize();
         normal.mult(half_weight);
         end = { p[points-1] + normal, p[points-1] - normal };
      }

      triangle_strip.vertex( start.start );
      triangle_strip.vertex( start.end );

      for (int i =0; i<points-2;++i) {
         PLine next = drawLineMitred(p[i], p[i+1], p[i+2], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }
      if (closed) {
         PLine next = drawLineMitred(p[points-2], p[points-1], p[0], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }

      triangle_strip.vertex( end.start );
      triangle_strip.vertex( end.end );

      triangle_strip.endShape(CLOSE);
      return triangle_strip;
   }

   PShape drawRoundLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(CONVEX_POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      int NUMBER_OF_VERTICES=16;

      float start_angle = PVector{p2.x-p1.x,p2.y-p1.y}.heading() + HALF_PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(p1.x + cos(i + start_angle) * weight/2, p1.y + sin(i+start_angle) * weight/2);
      }

      start_angle += PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(p2.x + cos(i+start_angle) * weight/2, p2.y + sin(i+start_angle) * weight/2);
      }
      shape.endShape(CLOSE);
      return shape;
   }

   PShape drawLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(CONVEX_POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      shape.vertex(p1 + normal);
      shape.vertex(p1 - normal);
      shape.vertex(p2 - normal);
      shape.vertex(p2 + normal);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape drawCappedLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(CONVEX_POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      PVector end_offset = PVector{p2.x-p1.x,p2.y-p1.y};
      end_offset.normalize();
      end_offset.mult(weight/2.0);

      shape.vertex(p1 + normal - end_offset);
      shape.vertex(p1 - normal - end_offset);
      shape.vertex(p2 - normal + end_offset);
      shape.vertex(p2 + normal + end_offset);

      shape.endShape(CLOSE);
      return shape;
   }

   void _line(PShape &triangles, PVector p1, PVector p2, int weight) const {

      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      unsigned short i = triangles.getCurrentIndex();
      triangles.vertex( p1 + normal );
      triangles.vertex( p1 - normal );
      triangles.vertex( p2 - normal );
      triangles.vertex( p2 + normal );

      triangles.index( i + 0 );
      triangles.index( i + 1 );
      triangles.index( i + 2 );

      triangles.index( i + 0 );
      triangles.index( i + 2 );
      triangles.index( i + 3 );
   }

   PShape drawTriangleStrip(int points, const PVector *p,int weight) {
      PShape triangles;
      triangles.beginShape(TRIANGLES);
      _line(triangles, p[0], p[1], weight);
      for (int i=2;i<points;++i) {
         _line(triangles, p[i-1], p[i], weight);
         _line(triangles, p[i], p[i-2], weight);
      }
      triangles.endShape();
      return triangles;
   }

   void shape_stroke(PShape &pshape, color color) {
      switch( pshape.style ) {
      case POINTS:
      {
         for (auto z : pshape.vertices ) {
            shape_fill( _createEllipse(z.x, z.y, stroke_weight, stroke_weight, CENTER),color );
         }
         break;
      }
      case POLYGON:
      case CONVEX_POLYGON:
      {
         if (pshape.vertices.size() > 2 ) {
            if (pshape.type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
               shape_fill( drawLinePoly( pshape.vertices.size() - 1, pshape.vertices.data() + 1, stroke_weight, false),
                           color );
            } else {
               shape_fill( drawLinePoly( pshape.vertices.size(), pshape.vertices.data(), stroke_weight, pshape.type == CLOSE),
                           color );
            }
         } else if (pshape.vertices.size() == 2) {
            switch(line_end_cap) {
            case ROUND:
               shape_fill( drawRoundLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), color );
               break;
            case PROJECT:
               shape_fill( drawCappedLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), color );
               break;
            case SQUARE:
               shape_fill( drawLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), color );
               break;
            default:
               abort();
            }
         } else if (pshape.vertices.size() == 1) {
            shape_fill( _createEllipse(pshape.vertices[0].x, pshape.vertices[0].y, stroke_weight, stroke_weight, CENTER),color );
         }
         break;
      }
      case TRIANGLE_STRIP:
      {
         shape_fill( drawTriangleStrip( pshape.vertices.size(), pshape.vertices.data(), stroke_weight),color );
         break;
      }
      case TRIANGLES:
         break;
      default:
         abort();
         break;
      }
   }

   void shape_fill(const PShape &pshape, color color) {
      if (pshape.vertices.size() > 2 && pshape.style != POINTS) {
         if (pshape.normals.size() == 0) {
            std::vector<PVector> normals( pshape.vertices.size(), {0.0f,0.0f,0.0f });
            // Iterate over all triangles
            for (int i = 0; i < pshape.indices.size()/3; i++) {
               // Get the vertices of the current triangle
               PVector v1 = pshape.vertices[pshape.indices[i * 3]];
               PVector v2 = pshape.vertices[pshape.indices[i * 3 + 1]];
               PVector v3 = pshape.vertices[pshape.indices[i * 3 + 2]];

               // Calculate the normal vector of the current triangle
               PVector edge1 = v2 - v1;
               PVector edge2 = v3 - v1;
               PVector normal = (edge1.cross(edge2)).normalize();

               // Add the normal to the normals list for each vertex of the triangle
               normals[pshape.indices[i * 3]] = normals[pshape.indices[i * 3]] + normal;
               normals[pshape.indices[i * 3 + 1]] = normals[pshape.indices[i * 3 + 1]] + normal;
               normals[pshape.indices[i * 3 + 2]] = normals[pshape.indices[i * 3 + 2]] + normal;
            }

            // Normalize all the normals
            for (int i = 0; i < normals.size(); i++) {
               normals[i].normalize();
            }
            drawTriangles(  pshape.vertices, normals, pshape.coords, pshape.indices, color );
         } else {
            drawTriangles(  pshape.vertices, pshape.normals, pshape.coords, pshape.indices, color );
         }
      }
   }

   void shape(PShape &pshape, float x, float y) {
      pushMatrix();
      translate(x,y);
      transform( pshape.shape_matrix );
      if ( pshape.style == GROUP ) {
         for (auto &&child : pshape.children) {
            shape(child,0,0);
         }
      } else {
         if (pshape.texture_.layer != 0) {
            if (tint_color.a != 0) {
               shape_fill(pshape,tint_color);
            }
         } else {
            if (fill_color.a != 0) {
               shape_fill(pshape,fill_color);
            }
         }
         if (stroke_color.a != 0) {
            shape_stroke(pshape, stroke_color);
         }
      }
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
      PShape pshape = createArc(x, y, width, height, start, stop, mode);
      shape( pshape );
   }

   void strokeCap(int cap) {
      line_end_cap = cap;
   }

   void line(float x1, float y1, float x2, float y2) {
      PShape pshape = createLine( x1, y1, x2, y2);
      shape( pshape );
   }

   void line(float x1, float y1, float z1, float x2, float y2, float z2) {
      abort();
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

   void beginShape(int points = POLYGON) {
      _shape = PShape();
      _shape.beginShape(points);
   }

   void vertex(float x, float y, float z = 0.0) {
      _shape.vertex(x, y, z);
   }

   void normal(float x, float y, float z) {
      _shape.normal(x, y, z);
   }

   void noNormal() {
      _shape.noNormal();
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      _shape.bezierVertex(x2,y2, x3,y3,x4,y4);
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
      shape.beginShape(POLYGON);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.vertex(x4, y4);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createLine(float x1, float y1, float x2, float y2) {
      PShape shape;
      shape.beginShape(POLYGON);
      shape.vertex(x1,y1);
      shape.vertex(x2,y2);
      shape.endShape(OPEN);
      return shape;
   }

   PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape shape;
      shape.beginShape(TRIANGLES);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.indices = { 0,1,2 };
      shape.endShape(CLOSE);
      return shape;
   }

   float sincos[32][2] = {
      { 1.000000, -0.000000 },
      { 0.980785, 0.195090 },
      { 0.923880, 0.382683 },
      { 0.831470, 0.555570 },
      { 0.707107, 0.707107 },
      { 0.555570, 0.831470 },
      { 0.382683, 0.923880 },
      { 0.195090, 0.980785 },
      { 0.000000, 1.000000 },
      { -0.195090, 0.980785 },
      { -0.382683, 0.923880 },
      { -0.555570, 0.831470 },
      { -0.707107, 0.707107 },
      { -0.831470, 0.555570 },
      { -0.923880, 0.382683 },
      { -0.980785, 0.195090 },
      { -1.000000, -0.000000 },
      { -0.980785, -0.195090 },
      { -0.923880, -0.382683 },
      { -0.831470, -0.555570 },
      { -0.707107, -0.707107 },
      { -0.555570, -0.831470 },
      { -0.382683, -0.923880 },
      { -0.195091, -0.980785 },
      { -0.000000, -1.000000 },
      { 0.195090, -0.980785 },
      { 0.382683, -0.923880 },
      { 0.555570, -0.831470 },
      { 0.707107, -0.707107 },
      { 0.831469, -0.555570 },
      { 0.923880, -0.382684 },
      { 0.980785, -0.195090 } };

   PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius) {
      return PVector( center.x + xradius * sincos[index][0],
                      center.y + yradius * sincos[index][1],
                      center.z);
   }

   PVector ellipse_point(const PVector &center, int index, float start, float end, float xradius, float yradius) {
      float angle = map( index, 0, 32, start, end);
      return PVector( center.x + xradius * sin(-angle + HALF_PI),
                      center.y + yradius * cos(-angle + HALF_PI),
                      center.z);
   }

   PShape createUnitCircle(int NUMBER_OF_VERTICES = 32) {
      PShape shape;
      shape.beginShape(CONVEX_POLYGON);
      for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
         shape.vertex( ellipse_point( {0,0,0}, i, 0, TWO_PI, 1.0, 1.0 ) );
      }
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createEllipse(float x, float y, float width, float height) {
      return _createEllipse(x,y,width,height,ellipse_mode);
   }

   PShape _createEllipse(float x, float y, float width, float height, int ellipse_mode) {
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
      int NUMBER_OF_VERTICES=32;
      PShape shape;
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

   PShape createArc(float x, float y, float width, float height, float start,
                    float stop, int mode) {

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

      PShape shape;
      shape.beginShape(CONVEX_POLYGON);
      int NUMBER_OF_VERTICES=32;
      if ( fillMode == PIE ) {
         shape.vertex(x,y);
      }
      for(float i = start; i < stop; i = i + (TWO_PI / NUMBER_OF_VERTICES) ) {
         shape.vertex( { x + width * sin(-i + HALF_PI), y + height * cos(-i + HALF_PI) } );
      }
      shape.vertex( { x + width * sin(-stop + HALF_PI), y + height * cos(-stop + HALF_PI) } );
      shape.endShape(strokeMode);
      return shape;
   }

   PShape createPoint(float x, float y) {
      PShape shape;
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
