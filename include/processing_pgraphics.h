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
#include "processing_opengl_framebuffer.h"
#include "processing_pshader.h"

class PGraphicsImpl;

class PGraphics {
   friend PGraphicsImpl;
   std::shared_ptr<PGraphicsImpl> impl;
public:
   static void init();

   static void close();

   PGraphics() {}

   ~PGraphics() {}

   operator PImage() {
      return getAsPImage();
   }

   PGraphics(int width, int height, int mode, int aaMode = MSAA, int aaFactor = 2);

   int getWidth() const;
   int getHeight() const;
   unsigned int *getPixels();
   gl::texture_ptr getAsTexture();
   PImage getAsPImage();
   void drawPImageWithCPU( PImage img, int x, int y );

   void save( const std::string &fileName );

   void saveFrame( std::string fileName = "frame-####.png" );

   void pushMatrix();
   void popMatrix();

   void translate(PVector t);

   void translate(float x, float y, float z=0.0 );
   void transform(const PMatrix &transform_matrix);

   void scale(float x, float y,float z = 1.0);

   void scale(float x);

   void rotate(float angle, PVector axis);

   void rotate(float angle);

   void rotateZ(float angle);

   void rotateY(float angle);

   void rotateX(float angle);

   float screenX(float x, float y, float z = 0.0);

   float screenY(float x, float y, float z = 0.0);

   void ortho(float left, float right, float bottom, float top, float near, float far);

   void ortho(float left, float right, float bottom, float top);
   void ortho();

   void perspective(float angle, float aspect, float minZ, float maxZ);
   void perspective();

   void camera( float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ );
   void camera();

   void directionalLight(float r, float g, float b, float nx, float ny, float nz);

   void pointLight(float r, float g, float b, float nx, float ny, float nz);

   void spotLight( float r, float g, float b, float x, float y, float z, float nx, float ny, float nz, float angle, float concentration);

   void lightFalloff(float r, float g, float b);

   void lightSpecular(float r, float g, float b);

   void ambientLight(float r, float g, float b);

   void lights();

   void noLights();

   void textFont(PFont font);

   void textAlign(int x, int y);

   void textAlign(int x);

   void textSize(int size);

   int blendMode(int b);

   void hint(int type);

   float textAscent();
   float textDescent();

   float textWidth(const std::string &text);

   void text(const std::string &text, float x, float y, float twidth = -1, float theight = -1);
   void text(char c, float x, float y, float twidth = -1, float theight = -1) ;

   void background(float r, float g, float b);

   void background(color c);

   void background(float gray);

   void background(float gray, float alpha);

   void imageMode(int iMode);

   PShape createBox(float w, float h, float d);

   void box(float w, float h, float d);

   void box(float size);

   void sphereDetail(float ures, float vres);

   void sphereDetail(float res);

   PShape createSphere( float radius );

   void sphere(float radius);

   void image(PGraphics gfx, float x, float y, float width=-1, float height=-1);

   void image(PImage pimage, float left, float top, float right, float bottom);

   void image(PImage pimage, float x, float y);

   void background(PImage bg);

   void loadPixels();

   void updatePixels();

   color get(int x, int y);

   void set(int x, int y, color c);

   void set(int x, int y, PImage i);

   void rect(float x, float y, float _width, float _height);
   void ellipseMode(int mode);

   void shape(PShape &pshape, float x, float y);

   void shape(PShape &pshape, float x, float y, float width, float height);

   void shape(PShape &pshape);

   void ellipse(PVector v, float width, float height);

   void ellipse(float x, float y, float width, float height);

   void ellipse(float x, float y, float radius);

   void arc(float x, float y, float width, float height, float start, float stop, int mode = DEFAULT);

   void line(float x1, float y1, float x2, float y2);

   void line(float x1, float y1, float z1, float x2, float y2, float z2);

   void line(PVector start, PVector end);

   void line(PLine l);

   void point(float x, float y);

   void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 );

   void triangle( float x1, float y1, float x2, float y2, float x3, float y3 );

   void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

   void beginShape(int style_ = POLYGON);

   void beginContour();

   void endContour();

   void tint(float r,float g, float b, float a);
   void tint(float r,float g, float b);
   void tint(float r,float a);
   void tint(float r);
   void tint(color c);
   void tint(color c, float a);
   void noTint();

   void specular(float r,float g, float b, float a = 1.0);
   void shininess(float r);

   void fill(float r,float g, float b, float a);
   void fill(float r,float g, float b);
   void fill(float r,float a);
   void fill(float r);
   void fill(color c);
   void fill(color c, float a);
   void noFill();

   void stroke(float r,float g, float b, float a);
   void stroke(float r,float g, float b);
   void stroke(float r,float a);
   void stroke(float r);
   void stroke(color c);
   void stroke(color c, float a);
   void noStroke();

   void strokeWeight(float x);
   void strokeCap( int cap );

   void vertex(float x, float y, float z);
   void vertex(float x, float y);
   void vertex(PVector p);
   void vertex(float x, float y, float z, float u, float v);
   void vertex(float x, float y, float u, float v);
   void vertex(PVector p, PVector2 t);

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4);
   void bezierVertex(PVector v2, PVector v3, PVector v4);
   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);

   void curveTightness(float alpha);
   void curveVertex(PVector c);
   void curveVertex(float x, float y);

   void textureMode( int mode_ );
   void texture(PImage img);
   void noTexture();

   void noNormal();
   void normal(PVector p);
   void normal(float x, float y, float z);

   void endShape(int type = OPEN);

   void rectMode(int mode);

   PShape createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

   PShape createRect(float x, float y, float width, float height);

   PShape createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 );

   PShape createLine(float x1, float y1, float z1, float x2, float y2, float z2);

   PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 );
   PShape createGroup();

   PShape createEllipse(float x, float y, float width, float height);

   PShape createArc(float x, float y, float width, float height, float start,
                    float stop, int ellipse_mode, int mode);

   PShape createPoint(float x, float y);

   void smooth(int aaFactor=2, int aaMode=MSAA);
   void noSmooth();

   void beginDraw();

   void endDraw();
   int commit_draw();

   void shader(PShader pshader, int kind = TRIANGLES);

   void resetShader();
   void resetMatrix();

   void filter(PShader shader);
   void filter(int kind);
   void filter(int kind, float param);

};



#endif
