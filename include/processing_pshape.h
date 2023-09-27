#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_enum.h"
#include "processing_texture_manager.h"
#include "processing_opengl.h"
#include "processing_pimage.h"

class PShape {
   PVector n = { 0.0, 0.0, 1.0 };

public:

   PTexture texture_;
   PMatrix shape_matrix = PMatrix::Identity();

   struct vInfoExtra {
      color stroke;
      color tint;
      float weight;
      bool contour;
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
   float stroke_weight = 1.0f;
   int line_end_cap = ROUND;
   float tightness = 0.0f;
   bool contour = false;

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
      std::swap(tightness, other.tightness);
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

   void scale(float x) {
      scale(x,x,x);
   }

   void transform(const PMatrix &transform) {
      shape_matrix = transform;
   }

   void resetMatrix() {
      shape_matrix = PMatrix::Identity();
   }

   void loadShape(const char *filename) {
   }

   void beginShape(int style_ = POLYGON) {
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      style = style_;
      clear();
   }

   void beginContour() {
      contour = true;
   }

   void endContour() {
      contour = false;
   }

   void textureMode( int mode_ ) {
      mode = mode_;
   }

   void texture(PTexture texure) {
      texture_ = texure;
   }

   void texture(gl_context &glc, PImage &img) {
      texture( img.getTexture( glc ) );
   }

   void noTexture() {
      texture_ = {};
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
      vertex({x, y, 0}, {0.0f,0.0f});
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

   void vertex(PVector p, PVector2 t) {

      if (mode == IMAGE) {
         t.x /= texture_.width();
         t.y /= texture_.height();
      }

      vertices.push_back( { p, n, texture_.normalize( t ), texture_.layer, fill_color } );
      extras.push_back( {stroke_color, tint_color, stroke_weight, contour } );
   }

   void index(unsigned short i) {
      indices.push_back(i);
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      bezierVertex( x2, y2, 0, x3, y3, 0, x4, y4, 0);
   }

   void bezierVertex(PVector v2, PVector v3, PVector v4) {
      bezierVertex( v2.x, v2.y, v2.z, v3.x, v3.y, v3.z, v4.x, v4.y, v4.z);
   }

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
      float x1 = vertices.back().position.x;
      float y1 = vertices.back().position.y;
      for (float t = 0.01; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPointCubic( x1, x2, x3, x4, t );
         float y = bezierPointCubic( y1, y2, y3, y4, t );
         vertex(x, y);
      }
   }

   void bezierVertexQuadratic(PVector control, PVector anchor2) {
      float anchor1_x = vertices.back().position.x;
      float anchor1_y = vertices.back().position.y;
      for (float t = 0.01; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPointQuadratic( anchor1_x, control.x, anchor2.x, t );
         float y = bezierPointQuadratic( anchor1_y, control.y, anchor2.y, t );
         vertex(x, y);
      }
   }

   void curveTightness(float alpha) {
      tightness = alpha;
   }

   std::vector<PVector> curve_vertices;

   void curveVertex(PVector c) {
      curve_vertices.push_back(c);
   }

   void drawCurve() {
      constexpr int segments = 50;
      constexpr float dt = (1.0 / segments);
      constexpr float hdt = 0.5f * dt;
      constexpr float hdt2x2 = 0.5f * 2.0f * dt * dt;
      constexpr float hdt3x6 = 0.5f * 6.0f * dt * dt * dt;

      size_t size = curve_vertices.size();
      float s = tightness;

      for (int i = 0; i < size; i++) {
         auto p0 = curve_vertices[i];
         auto p1 = curve_vertices[(i+1)%size];
         auto p2 = curve_vertices[(i+2)%size];
         auto p3 = curve_vertices[(i+3)%size];

         float xt3 = ( p0.x * (s-1)   + p1.x * (s+3)  + p2.x * (-3-s)  + p3.x * (1-s));
         float xt2 = ( p0.x * (1-s)*2 + p1.x * (-5-s) + p2.x * (s+2)*2 + p3.x * (s-1));
         float xt1 = ( p0.x * (s-1) /*+ p1.x * 0*/    + p2.x * (1-s) /*+ p3.x * 0*/);
         float xt0 = ( p0.x * 0       + p1.x * 2      + p2.x * 0       + p3.x * 0);

         float yt3 = ( p0.y * (s-1)   + p1.y * (s+3)  + p2.y * (-3-s)  + p3.y * (1-s));
         float yt2 = ( p0.y * (1-s)*2 + p1.y * (-5-s) + p2.y * (s+2)*2 + p3.y * (s-1));
         float yt1 = ( p0.y * (s-1) + /*p1.y * 0*/    + p2.y * (1-s) /*+ p3.y * 0*/);
         float yt0 = ( p0.y * 0       + p1.y * 2      + p2.y * 0       + p3.y * 0);

         float zt3 = ( p0.z * (s-1)   + p1.z * (s+3)  + p2.z * (-3-s)  + p3.z * (1-s));
         float zt2 = ( p0.z * (1-s)*2 + p1.z * (-5-s) + p2.z * (s+2)*2 + p3.z * (s-1));
         float zt1 = ( p0.z * (s-1) /*+ p1.z * 0 */   + p2.z * (1-s) /*+ p3.z * 0*/);
         float zt0 = ( p0.z * 0       + p1.z * 2      + p2.z * 0       + p3.z * 0);

         PVector pos = p1; // {xt0, yt0, zt0};
         PVector vel = PVector{xt1, yt1, zt1} * hdt;
         PVector acc = PVector{xt2, yt2, zt2} * hdt2x2;
         PVector jer = PVector{xt3, yt3, zt3} * hdt3x6;

         // Just draw the control points
         // vertex(p1);

         for (int i = 0; i < segments - 1; ++i) {
            // Use the full quadtraic queation
            float t1 = i * dt;
            float t2 = t1 * t1;
            float t3 = t2 * t1;
            vertex( 0.5f * PVector{
                  xt3 * t3 + xt2 * t2 + xt1 * t1 + xt0,
                  yt3 * t3 + yt2 * t2 + yt1 * t1 + yt0,
                  zt3 * t3 + zt2 * t2 + zt1 * t1 + zt0
               });
            // Use add differential in discrete steps
            // vertex(pos);
            // pos += vel;
            // vel += acc;
            // acc += jer;
         }
      }
   }

   void curveVertex(float x, float y, float z) {
      curveVertex({x,y,z});
   }

   void curveVertex(float x, float y) {
      curveVertex(x,y,0);
   }

   bool isClockwise() const;

   void endShape(int type_ = OPEN) {
      // OPEN or CLOSE
      if (curve_vertices.size() > 0) {
         drawCurve();
         curve_vertices.clear();
      }

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

   void fill(class gl_context::color color) {
      fill_color = color;
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

   void strokeWeight(float x) {
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

   void setStrokeWeight(float w) {
      for ( auto&&v : extras ) {
         v.weight = w;
      }
   }

   void setTexture( gl_context &glc, PImage &img ) {
     // need to recalc texture coordintaes.
     texture( glc, img );
     for ( auto&&v : vertices ) {
        v.coord = texture_.normalize( v.coord );
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
            child.draw(glc, transform * shape_matrix);
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

   void setVertex(int i, PVector v) {
      vertices[i].position = v;
   }

   void setVertex(int i, float x, float y , float z = 0) {
      vertices[i].position = {x,y,z};
   }

   PVector getVertex(int i, PVector &x) const {
      return x = getVertex(i);
   }
};

PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius);
PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, gl_context::color color, const PMatrix &transform);

#endif
