#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_earclipping.h"
#include "processing_transforms.h"
#include "processing_enum.h"

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

class PShape {
public:

   Eigen::Matrix4f shape_matrix = Eigen::Matrix4f::Identity();
   std::vector<PVector> vertices;
   std::vector<PShape> children;

   int style = POLYGON;
   int type = OPEN;


   PShape(const PShape& other) = delete;
   PShape& operator=(const PShape& other) = delete;

   PShape() {
   }

   PShape(PShape&& other) noexcept {
      std::swap(vertices, other.vertices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(shape_matrix, other.shape_matrix);
   }

   PShape& operator=(PShape&& other) noexcept {
      std::swap(vertices, other.vertices);
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

   void endShape(int type_ = OPEN) {
      // OPEN or CLOSE
      type = type_;
   }

   void addChild(PShape &&shape) {
      children.emplace_back( std::move(shape) );
   }
};




#endif
