#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_enum.h"
#include "processing_texture_manager.h"
#include "processing_opengl.h"

class PShape {
   PVector n = { 0.0, 0.0, 1.0 };

public:

   PTexture texture_;
   PMatrix shape_matrix = PMatrix::Identity();

   struct vInfoExtra {
      color stroke;
      color tint;
      int weight;
   };

   std::vector<gl_context::vertex> vertices;
   std::vector<vInfoExtra> extras;

   bool setNormals = false;
   std::vector<unsigned short> indices;

   std::vector<PShape> children;

   int style = POLYGON;
   int type = OPEN;
   int mode = IMAGE;
   float width = 1.0;
   float height = 1.0;

   color stroke_color = BLACK;
   gl_context::color fill_color = flatten_color_mode(WHITE);
   color tint_color = WHITE;
   int stroke_weight = 1;
   int line_end_cap = ROUND;

   PShape(const PShape& other) = default;
   PShape& operator=(const PShape& other) = default;

   PShape() {
      vertices.reserve(4);
      extras.reserve(4);
      indices.reserve(6);
   }

   PShape(PShape&& other) noexcept {
      *this = std::move(other);
   }

   PShape& operator=(PShape&& other) noexcept {
      std::swap(texture_, other.texture_);
      std::swap(n, other.n);
      std::swap(setNormals, other.setNormals);
      std::swap(vertices, other.vertices);
      std::swap(extras, other.extras);
      std::swap(indices, other.indices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(mode, other.mode);
      std::swap(shape_matrix, other.shape_matrix);
      std::swap(stroke_color, other.stroke_color);
      std::swap(fill_color, other.fill_color);
      std::swap(tint_color, other.tint_color);
      std::swap(stroke_weight, other.stroke_weight);
      std::swap(line_end_cap, other.line_end_cap);
      std::swap(width, other.width);
      std::swap(height, other.height);
      return *this;
   }

   ~PShape() {
   }

   void addChild( const PShape &shape ) {
      children.push_back( shape );
   }

   PShape &getChild( int i ) {
      return children[i];
   }

   void copyStyle( const PShape &other ) {
      texture_= other.texture_;
      n = other.n;
      stroke_color = other.stroke_color;
      fill_color = other.fill_color;
      tint_color = other.tint_color;
      stroke_weight = other.stroke_weight;
      line_end_cap = other.line_end_cap;
   }

   void clear() {
      vertices.clear();
      extras.clear();
      indices.clear();
      children.clear();
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

   void rotate(float angle, PVector axis) {
      shape_matrix = shape_matrix * RotateMatrix(angle,axis);
   }

   void translate(float x, float y, float z=0) {
      shape_matrix = shape_matrix * TranslateMatrix(PVector{x,y,z});
   }

   void scale(float x, float y,float z = 1) {
      shape_matrix = shape_matrix * ScaleMatrix(PVector{x,y,z});
   }

   void transform(const PMatrix &transform) {
      shape_matrix = transform;
   }

   void loadShape(const char *filename) {
   }

   void beginShape(int style_ = POLYGON) {
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      style = style_;
      clear();
   }

   void textureMode( int mode_ ) {
      mode = mode_;
   }

   void texture(PTexture texure) {
      texture_ = texure;
   }

   void noTexture() {
      texture_ = { 0,0,0,0,0 };
   }

   void noNormal() {
      setNormals = false;
   }

   void normal(PVector p) {
      setNormals = true;
      n = p;
   }

   void normal(float x, float y, float z) {
      setNormals = true;
      n.x = x;
      n.y = y;
      n.z = z;
   }

   void vertex(float x, float y, float z) {
      vertex({x, y, z}, {0.0f,0.0f});
   }

   void vertex(float x, float y) {
      vertices.push_back( { {x,y}, n, {0,0}, fill_color } );
      extras.push_back( { stroke_color, tint_color, stroke_weight } );
   }

   void vertex(PVector p) {
      vertex(p, {0.0f,0.0f});
   }

   void vertex(float x, float y, float z, float u, float v) {
      vertex({x, y, z}, {u, v});
   }

   void vertex(float x, float y, float u, float v) {
      vertex({x, y, 0.0f}, { u , v });
   }

   void vertex(PVector p, PVector t) {

      if (mode != NORMAL) {
         t.x /= texture_.width();
         t.y /= texture_.height();
      }

      PVector tprime{
         map(t.x,0,1.0,(1.0*texture_.left)/texture_.sheet_width,(1.0*texture_.right)/texture_.sheet_width),
         map(t.y,0,1.0,(1.0*texture_.top)/texture_.sheet_height,(1.0*texture_.bottom)/texture_.sheet_height),
         (float)texture_.layer};

      // if ( tprime.x > 1.0 || tprime.y > 1.0)
      //    abort();

      vertices.push_back( { p, n, tprime, fill_color } );
      extras.push_back( {stroke_color, tint_color, stroke_weight } );
   }

   void index(unsigned short i) {
      indices.push_back(i);
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      bezierVertex( x2, y2, 0, x3, y3, 0, x4, y4, 0);
   }

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
      float x1 = vertices.back().position.x;
      float y1 = vertices.back().position.y;
      for (float t = 0; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPoint( x1, x2, x3, x4, t );
         float y = bezierPoint( y1, y2, y3, y4, t );
         vertex(x, y);
      }
   }

   void endShape(int type_ = OPEN) {
      // OPEN or CLOSE
      if (style == POLYGON || style == LINES)
         type = type_;
      else
         type = CLOSE;
      populateIndices();
      if (!setNormals) {
         // Iterate over all triangles
         for (int i = 0; i < indices.size()/3; i++) {
            // Get the vertices of the current triangle
            PVector v1 = vertices[indices[i * 3]].position;
            PVector v2 = vertices[indices[i * 3 + 1]].position;
            PVector v3 = vertices[indices[i * 3 + 2]].position;

            // Calculate the normal vector of the current triangle
            PVector edge1 = v2 - v1;
            PVector edge2 = v3 - v1;
            PVector normal = (edge1.cross(edge2)).normalize();

            // Add the normal to the normals list for each vertex of the triangle
            vertices[indices[i * 3]].normal = vertices[indices[i * 3]].normal + normal;
            vertices[indices[i * 3 + 1]].normal = vertices[indices[i * 3 + 1]].normal + normal;
            vertices[indices[i * 3 + 2]].normal = vertices[indices[i * 3 + 2]].normal + normal;
         }
         // The shader will normalize all the normals
      }
   }

   unsigned short getCurrentIndex() {
      return vertices.size();
   }

   void populateIndices();

   void addChild(PShape &&shape) {
      children.emplace_back( std::move(shape) );
   }

   void fill(float r,float g,  float b, float a) {
      fill_color = flatten_color_mode( {r,g,b,a} );
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
      stroke_color = {r,g,b,a};
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

   void tint(float r,float g,  float b, float a) {
      tint_color = {r,g,b,a};
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

   void strokeCap(int cap) {
      line_end_cap = cap;
   }

   void setStroke(color c) {
      for ( auto&&v : extras ) {
         v.stroke = c;
      }
   }

   void setStrokeWeight(int w) {
      for ( auto&&v : extras ) {
         v.weight = w;
      }
   }

   void setFill(bool z) {
      if (!z )
         for ( auto&&v : vertices ) {
            v.fill = flatten_color_mode({0.0,0.0,0.0,0.0});
         }
   }

   void setFill(color c) {
      for ( auto&&v : vertices ) {
         v.fill = flatten_color_mode(c);
      }
   }

   void setTint(color c) {
      for ( auto&&v : extras ) {
         v.tint = c;
      }
   }

   void draw(gl_context &glc, const PMatrix& transform) const {
      if ( style == GROUP ) {
         for (auto &&child : children) {
            child.draw(glc, transform);
         }
      } else {
         draw_fill(glc, transform);
         if ( stroke_color.a != 0 )
            draw_stroke(glc, transform);
      }
   }

   void draw_stroke(gl_context &glc, const PMatrix& transform) const;
   void draw_fill(gl_context &gl, const PMatrix& transform) const;

   int getChildCount() const {
      return children.size();
   }

   int getVertexCount() const {
      return vertices.size();
   }

   PVector getVertex(int i) const {
      return vertices[i].position;
   }
};

PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius);
PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, gl_context::color color, const PMatrix &transform);

#endif
