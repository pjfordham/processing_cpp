#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_pimage.h"
#include "processing_java_compatability.h"

#include <vector>
#include <string_view>
#include <memory>

struct PMaterial;
class PShapeImpl;

class PShape {
   std::shared_ptr<PShapeImpl> impl;
public:
   static void init();

   static void close();

   float width=0, height=0;
   static const PImage getBlankTexture() {
      static PImage blankTexture = createBlankImage();
      return blankTexture;
   }

   PShape();
   PShape( std::shared_ptr<PShapeImpl> impl_ );

   bool operator==(const PShape &x) const {
      return impl == x.impl;
   };

   bool operator!=(const PShape &x) const {
      return !(x==*this);
   };

   struct vInfoExtra {
      color stroke;
      float weight;
   };

   const PMatrix& getShapeMatrix();

   void addChild( const PShape shape );

   PShape getChild( int i );

   color getFillColor() const;

   color getTintColor() const;

   bool isGroup() const;

   void copyStyle( const PShape other );

   void clear();

   void rotate(float angle);

   void rotateZ(float angle);

   void rotateY(float angle);

   void rotateX(float angle);

   void rotate(float angle, PVector axis);

   void translate(float x, float y, float z=0);

   void scale(float x, float y,float z = 1);

   void scale(float x);

   void transform(const PMatrix &transform);

   void resetMatrix();

   void beginShape(int style_ = POLYGON);

   void beginContour();

   void endContour();

   void textureMode( int mode_ );

   bool isTextureSet() const;

   void material(PMaterial &mat);

   void texture(PImage img);

   void circleTexture();

   void noTexture();

   void noNormal();

   void normal(PVector p);

   void normal(float x, float y, float z);

   void vertex(float x, float y, float z);

   void vertex(float x, float y);

   void vertex(PVector p);

   void vertex(float x, float y, float z, float u, float v);

   void vertex(float x, float y, float u, float v);

   void vertex(PVector p, PVector2 t);

   void index(unsigned short i);

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4);

   void bezierVertex(PVector v2, PVector v3, PVector v4);

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);

   void bezierVertexQuadratic(PVector control, PVector anchor2);

   void curveTightness(float alpha);

   void curveVertex(PVector c);

   void drawCurve();

   void curveVertex(float x, float y, float z);
   void curveVertex(float x, float y);

   bool isClockwise() const;

   void endShape(int type_ = OPEN);

   unsigned short getCurrentIndex();

   void populateIndices();

   void populateIndices( std::vector<unsigned short> &&i );

   void fill(float r,float g,  float b, float a);

   void fill(float r,float g, float b);

   void fill(float r,float a);

   void fill(float r);

   void fill(class color color);

   void fill(class color color, float a);

   void stroke(float r,float g,  float b, float a);

   void stroke(float r,float g, float b);

   void stroke(float r,float a);

   void stroke(float r);

   void stroke(color c);

   void strokeWeight(float x);

   void noStroke();

   void noFill();

   bool isStroked() const;

   bool isFilled() const;

   void tint(float r,float g,  float b, float a);

   void tint(float r,float g, float b);

   void tint(float r,float a);

   void tint(float r);

   void tint(color c);

   void noTint();

   void strokeCap(int cap);

   void setStroke(bool c);

   void setStroke(color c);

   void setStrokeWeight(float w);

   void setTexture( PImage img );

   void setFill(bool z);

   void setFill(color c);

   void setTint(color c);

   void flatten(std::vector<gl_context::VAO> &parent_vao, const PMatrix& transform) const;

   void flattenTransforms(const PMatrix& transform);

   void draw(gl_context &glc, const PMatrix& transform);

   void draw_normals(std::vector<gl_context::VAO> &parent_vao, const PMatrix& transform) const;

   void draw_stroke(std::vector<gl_context::VAO> &parent_vao, const PMatrix& transform) const;

   void draw_fill(std::vector<gl_context::VAO> &parent_vao, const PMatrix& transform) const;

   int getChildCount() const;

   int getVertexCount() const;

   PVector getVertex(int i) const;

   void setVertex(int i, PVector v);

   void setVertex(int i, float x, float y , float z = 0);

   PVector getVertex(int i, PVector &x) const;
};

PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius);
PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, color color, const PMatrix &transform);

PShape loadShapeOBJ(std::string_view objPath);
PShape loadShape(std::string_view objPath);

#endif
