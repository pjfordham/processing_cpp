#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_earclipping.h"
#include "processing_enum.h"
#include "processing_texture_manager.h"

extern int width;
extern int height;

class PShape {
public:

   Eigen::Matrix4f shape_matrix = Eigen::Matrix4f::Identity();
   std::vector<PVector> vertices;
   std::vector<PVector> normals;
   std::vector<PShape> children;
   std::vector<unsigned short> indices;
   std::vector<PVector> coords;

   int style = POLYGON;
   int type = OPEN;

   PTexture texture_;

   PShape(const PShape& other) = delete;
   PShape& operator=(const PShape& other) = delete;

   PShape() {
   }

   PShape(PShape&& other) noexcept {
      std::swap(texture_, other.texture_);
      std::swap(normals, other.normals);
      std::swap(coords, other.coords);
      std::swap(vertices, other.vertices);
      std::swap(indices, other.indices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(shape_matrix, other.shape_matrix);
   }

   PShape& operator=(PShape&& other) noexcept {
      std::swap(texture_, other.texture_);
      std::swap(normals, other.normals);
      std::swap(coords, other.coords);
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
      coords.clear();
      indices.clear();
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

   void texture(PTexture texure) {
      texture_ = texure;
   }

   void noTexture() {
      texture_ = { 0,0,0,0,0 };
   }

   void normal(PVector p) {
      normals.push_back( p );
   }

   void normal(float x, float y, float z) {
      normals.push_back( {x, y, x} );
   }

   void vertex(float x, float y, float z) {
      vertices.push_back({x, y, z});
   }

   void vertex(float x, float y) {
      vertices.push_back({x, y, 0.0f});
   }

   void vertex(PVector p) {
      vertices.push_back(p);
   }

   void vertex(float x, float y, float z, float u, float v) {
      vertex({x, y, z}, { u , v });
   }

   void vertex(float x, float y, float u, float v) {
      vertex({x, y, 0.0f}, { u , v });
   }

   void vertex(PVector p, PVector t) {
      vertices.push_back(p);
      coords.push_back( {
            map(t.x,0,1.0,(1.0*texture_.left)/width,(1.0*texture_.right)/width),
            map(t.y,0,1.0,(1.0*texture_.top)/height,(1.0*texture_.bottom)/height),
        (float)texture_.layer});
      if ( coords.back().x > 1.0 || coords.back().y > 1.0)
         abort();
   }

   void index(unsigned short i) {
      indices.push_back(i);
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
      populateIndices();
   }

   unsigned short getCurrentIndex() {
      return vertices.size();
   }

   void populateIndices() {
      if (indices.size() != 0)
         return;

      if (vertices.size() == 0) abort();

      if (style == TRIANGLE_STRIP) {
         for (int i = 0; i < vertices.size() - 2; i++ ){
            indices.push_back(i);
            indices.push_back(i+1);
            indices.push_back(i+2);
         }
      } else if (style == CONVEX_POLYGON) {
         // Fill with triangle fan
         for (int i = 1; i < vertices.size() - 1 ; i++ ) {
            indices.push_back( 0 );
            indices.push_back( i );
            indices.push_back( i+1 );
         }
      } else if (style == POLYGON) {
         indices = triangulatePolygon({vertices.begin(),vertices.end()});
      } else if (style == TRIANGLES) {
         for (int i = 0; i < vertices.size(); i++ ) {
            indices.push_back( i );
         }
      }
   }

   void addChild(PShape &&shape) {
      children.emplace_back( std::move(shape) );
   }

};




#endif
