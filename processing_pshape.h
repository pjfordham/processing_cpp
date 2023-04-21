#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_earclipping.h"
#include "processing_enum.h"

class PShape {
public:

   Eigen::Matrix4f shape_matrix = Eigen::Matrix4f::Identity();
   std::vector<PVector> vertices;
   std::vector<PShape> children;
   std::vector<unsigned short> indices;

   int style = POLYGON;
   int type = OPEN;


   PShape(const PShape& other) = delete;
   PShape& operator=(const PShape& other) = delete;

   PShape() {
   }

   PShape(PShape&& other) noexcept {
      std::swap(vertices, other.vertices);
      std::swap(indices, other.indices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(shape_matrix, other.shape_matrix);
   }

   PShape& operator=(PShape&& other) noexcept {
      std::swap(vertices, other.vertices);
      std::swap(indices, other.indices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(shape_matrix, other.shape_matrix);
      return *this;
   }

   ~PShape() {
   }

   void clear() {
      vertices.clear();
   }

   void translate(float x, float y, float z=0) {
      shape_matrix = shape_matrix * TranslateMatrix(PVector{x,y,z});
   }

   void scale(float x, float y,float z = 1) {
      shape_matrix = shape_matrix * ScaleMatrix(PVector{x,y,z});
   }

   void loadShape(const char *filename) {
   }

   void beginShape(int style_ = POLYGON) {
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      style = style_;
      clear();
   }

   void vertex(float x, float y, float z = 0.0) {
      vertices.push_back({x, y, z});
   }

   void vertex(PVector p) {
      vertices.push_back(p);
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      bezierVertex( x2, y2, 0, x3, y3, 0, x4, y4, 0);
   }

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
      float x1 = vertices.back().x;
      float y1 = vertices.back().y;
      for (float t = 0; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float t_ = 1 - t;
         float x = t_ * t_ * t_ * x1 + 3 * t_ * t_ * t * x2 + 3 * t_ * t * t * x3 + t * t * t * x4;
         float y = t_ * t_ * t_ * y1 + 3 * t_ * t_ * t * y2 + 3 * t_ * t * t * y3 + t * t * t * y4;
         vertex(x, y);
      }
   }

   void endShape(int type_ = OPEN) {
      // OPEN or CLOSE
      type = type_;
   }

   void addChild(PShape &&shape) {
      children.emplace_back( std::move(shape) );
   }
};




#endif
