#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_earclipping.h"
#include "processing_transforms.h"

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

enum {
   POINTS,
   POLYGON,
   LINES,
   GROUP,
   TRIANGLE_STRIP,
   TRIANGLE_FAN,
   TRIANGLES,
   OPEN,
   CLOSE,
   CORNERS,
   CORNER,
   CENTER,
   RADIUS,
   ROUND,
   SQUARE,
   PROJECT,
   CHORD,
   PIE,
   DEFAULT,
   DIAMETER,
};

extern GLuint programID;
extern GLuint Color;

class PShape;
PShape createPoint(float x, float y);

class PShape {
public:
   static color stroke_color;
   static color fill_color;
   static int rect_mode;
   static int ellipse_mode;
   static int stroke_weight;
   static int line_end_cap;

private:
   GLuint VAO = 0;
   GLuint vertexbuffer = 0;
   int vertexbuffer_size = 0;
   Eigen::Matrix4f shape_matrix = Eigen::Matrix4f::Identity();
   std::vector<PVector> vertices;
   std::vector<PShape> children;

public:
   int style = POLYGON;

private:
   int type = OPEN;

public:
   bool stroke_only = false;

   PShape(const PShape& other) = delete;
   PShape& operator=(const PShape& other) = delete;

   PShape() {
   }

   PShape(PShape&& other) noexcept {
      std::swap(VAO, other.VAO);
      std::swap(vertexbuffer_size, other.vertexbuffer_size);
      std::swap(vertexbuffer, other.vertexbuffer);
      std::swap(vertices, other.vertices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(stroke_only, other.stroke_only);
      std::swap(shape_matrix, other.shape_matrix);
   }

   PShape& operator=(PShape&& other) noexcept {
      std::swap(VAO, other.VAO);
      std::swap(vertexbuffer_size, other.vertexbuffer_size);
      std::swap(vertexbuffer, other.vertexbuffer);
      std::swap(vertices, other.vertices);
      std::swap(children, other.children);
      std::swap(style, other.style);
      std::swap(type, other.type);
      std::swap(stroke_only, other.stroke_only);
      std::swap(shape_matrix, other.shape_matrix);
      return *this;
   }

   ~PShape() {
      releaseVAO();
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

   void beginShape(int points = POLYGON) {
      style = points;
      clear();
   }

   void createVAO() {
      createVAO( vertices );
   }

   void createVAO( std::vector<PVector> &vertices ) {
      // Create a vertex array object (VAO)
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      vertexbuffer_size = vertices.size();

      glGenBuffers(1, &vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);

      GLuint attribId = glGetAttribLocation(programID, "position");
      glEnableVertexAttribArray(attribId);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glVertexAttribPointer(
         attribId,                         // attribute
         3,                                // size
         GL_FLOAT,                         // type
         GL_FALSE,                         // normalized?
         sizeof(PVector),                  // stride
         (void*)offsetof(PVector,x)        // array buffer offset
         );
   }

   static bool stroke_on() {
      return stroke_color.a != 0;
   }

   void borrowVAO( const PShape &shape ) {
      VAO = shape.VAO;
      vertexbuffer_size = shape.vertexbuffer_size;
   }

   void releaseVAO() {
      // sometimes we borrow a VAO, but we'd never have the vertexbuffer if we did
      if (vertexbuffer) {
         glDeleteBuffers(1, &vertexbuffer);
         glDeleteVertexArrays(1, &VAO);
         vertexbuffer = 0;
      }
      VAO = 0;
   }

   void vertex(float x, float y, float z = 0.0) {
      vertices.push_back({x, y, z});
   }

   void vertex(PVector p) {
      vertices.push_back(p);
   }

   void endShape(int type_ = OPEN) {
      type = type_;

      if (vertices.size() > 0) {
         if (style == POINTS) {
            for (auto z : vertices ) {
               children.emplace_back(createPoint( z.x, z.y ));
            }
         } else if (style == TRIANGLE_STRIP) {
            createVAO();
            if (!stroke_only)
               glTriangleStrip( vertices.size(), vertices.data(), stroke_weight);
         } else if (style == TRIANGLE_FAN) {
            createVAO();
            if (!stroke_only)
               glTriangleFan( vertices.size(), vertices.data(), stroke_weight);
         } else if (style == TRIANGLES) {
            createVAO();
            if (!stroke_only) {
               glTriangleStrip( vertices.size(), vertices.data(), stroke_weight);
            }
         } else if (style == POLYGON) {
            if (type == CLOSE) {
               std::vector<PVector> triangles = triangulatePolygon({vertices.begin(),vertices.end()});
               style = TRIANGLES;
               createVAO( triangles );
            }
            if (!stroke_only)
               glLinePoly( vertices.size(), vertices.data(), stroke_weight, type == CLOSE);
         } else if (style == LINES) {
            glLinePoly( vertices.size(), vertices.data(), stroke_weight, type == CLOSE);
         } else {
            abort();
         }
      }
   }

   // void _glRoundLine(PVector p1, PVector p2, color color, int weight) const {

   //    // Compute direction vector of line
   //    PVector direction = p2 - p1;
   //    direction.normalize();

   //    // Compute first orthogonal vector
   //    PVector z_axis(0.0, 0.0, 1.0);
   //    PVector orthogonal1 = direction.cross(z_axis);
   //    orthogonal1.normalize();

   //    // Compute second orthogonal vector
   //    PVector orthogonal2 = direction.cross(orthogonal1);
   //    orthogonal2.normalize();

   //    if (orthogonal1 == PVector{0.0,0.0,0.0} ) {
   //       orthogonal1 = PVector{1.0, 0.0, 0.0};
   //       orthogonal2 = PVector{0.0, 1.0, 0.0};
   //    }

   //    // Compute dimensions of cuboid
   //    float length = weight * 1;

   //    // Construct vertices of cuboid
   //    std::vector<PVector> vertices(8);
   //    vertices[0] = p1 - orthogonal1 * length - orthogonal2 * length;
   //    vertices[1] = p1 + orthogonal1 * length - orthogonal2 * length;
   //    vertices[2] = p1 + orthogonal1 * length + orthogonal2 * length;
   //    vertices[3] = p1 - orthogonal1 * length + orthogonal2 * length;
   //    vertices[4] = p2 - orthogonal1 * length - orthogonal2 * length;
   //    vertices[5] = p2 + orthogonal1 * length - orthogonal2 * length;
   //    vertices[6] = p2 + orthogonal1 * length + orthogonal2 * length;
   //    vertices[7] = p2 - orthogonal1 * length + orthogonal2 * length;


   //    std::vector<PVector> vertexBuffer;

   //    vertexBuffer = { vertices[0], vertices[1],vertices[2],vertices[3] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   //    vertexBuffer = { vertices[4], vertices[5],vertices[6],vertices[7] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   //    vertexBuffer = { vertices[0], vertices[1],vertices[4],vertices[5] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   //    vertexBuffer = { vertices[2], vertices[3],vertices[6],vertices[7] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   //    vertexBuffer = { vertices[1], vertices[5],vertices[6],vertices[2] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   //    vertexBuffer = { vertices[0], vertices[4],vertices[3],vertices[7] };
   //    glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );


   // }


   // only used by glTriangleStrip and glTriangleFan, as mitred line probably
   // wouldn't work.
   void glLine(PShape &triangles, PVector p1, PVector p2, int weight) const {

      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      triangles.vertex(p1 + normal);
      triangles.vertex(p1 - normal);
      triangles.vertex(p2 + normal);

      triangles.vertex(p2 + normal);
      triangles.vertex(p2 - normal);
      triangles.vertex(p1 - normal);

   }

   // MAke this make a new phsape and shove it in children
   void glTriangleStrip(int points, const PVector *p,int weight) {
      PShape triangles;
      triangles.beginShape(TRIANGLES);
      glLine(triangles, p[0], p[1], weight);
      for (int i=2;i<points;++i) {
         glLine(triangles, p[i-1], p[i], weight);
         glLine(triangles, p[i], p[i-2], weight);
      }
      triangles.stroke_only = true;
      triangles.endShape();
      children.push_back(std::move(triangles));
   }

   void glTriangleFan(int points, const PVector *p, int weight) {
      PShape triangles;
      triangles.beginShape(TRIANGLES);
      glLine(triangles, p[0], p[1], weight );
      for (int i=2;i<points;++i) {
         glLine(triangles, p[i-1], p[i], weight);
         glLine(triangles, p[i],   p[0], weight);
      }
      triangles.stroke_only = true;
      triangles.endShape();
      children.push_back(std::move(triangles));
   }

   PLine glLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) const {
      PLine l1{ p1, p2 };
      PLine l2{ p2, p3 };
      PLine low_l1 = l1.offset(-half_weight);
      PLine high_l1 = l1.offset(half_weight);
      PLine low_l2 = l2.offset(-half_weight);
      PLine high_l2 = l2.offset(half_weight);
      return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
   }

   void glLinePoly(int points, const PVector *p, int weight, bool closed)  {
      PLine start;
      PLine end;

      PShape triangle_strip;
      triangle_strip.beginShape(TRIANGLE_STRIP);

      float half_weight = weight / 2.0;
      if (closed) {
         start = glLineMitred(p[points-1], p[0], p[1], half_weight );
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
         PLine next = glLineMitred(p[i], p[i+1], p[i+2], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }
      if (closed) {
         PLine next = glLineMitred(p[points-2], p[points-1], p[0], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }

      triangle_strip.vertex( end.start );
      triangle_strip.vertex( end.end );

      triangle_strip.stroke_only = true;
      triangle_strip.endShape(closed ? CLOSE : OPEN);
      children.push_back(std::move(triangle_strip));
   }

   void drawVAO(color color, GLuint element_type) {
      float color_vec[] = {
         color.r / 255.0f,
         color.g / 255.0f,
         color.b / 255.0f,
         color.a / 255.0f };
      glUniform4fv(Color, 1, color_vec);
      glBindVertexArray(VAO);
      glDrawArrays(element_type, 0, vertexbuffer_size);
      glBindVertexArray(0);
      return;
   }

   void draw() {
      pushMatrix();
      transform( shape_matrix );
      if ( VAO ) {
         switch( style ) {
         case TRIANGLE_FAN:
            drawVAO( stroke_only ? stroke_color : fill_color , GL_TRIANGLE_FAN );
            break;
         case TRIANGLE_STRIP:
            drawVAO( stroke_only ? stroke_color : fill_color , GL_TRIANGLE_STRIP );
            break;
         case TRIANGLES:
            drawVAO( stroke_only ? stroke_color : fill_color , GL_TRIANGLES );
            break;
         default:
            abort();
         }
      }
      for (auto &&child : children) {
         child.draw();
      }
      popMatrix();
   }

   void addChild(PShape &&shape) {
      children.emplace_back( std::move(shape) );
   }
};

color PShape::stroke_color{255,255,255,255};
color PShape::fill_color{255,255,255,255};
int PShape::stroke_weight = 1;
int PShape::line_end_cap = ROUND;
int PShape::ellipse_mode = DIAMETER;
int PShape::rect_mode = CORNER;




PShape createRect(float x, float y, float width, float height) {
   if (PShape::rect_mode == CORNERS) {
      width = width - x;
      height = height - y;
   } else if (PShape::rect_mode == CENTER) {
      x = x - width / 2;
      y = y - height / 2;
   } else if (PShape::rect_mode == RADIUS) {
      width *= 2;
      height *= 2;
      x = x - width / 2;
      y = y - height / 2;
   }
   PShape shape;
   shape.beginShape(POLYGON);
   shape.vertex(x,y);
   shape.vertex(x+width,y);
   shape.vertex(x+width,y+height);
   shape.vertex(x,y+height);
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
   PVector p[] = {{x1,y1},{x1,y1},{x2,y2},{x2,y2}};

   float half_stroke = PShape::stroke_weight/2.0;

   PShape shape;
   shape.stroke_only = true;
   shape.beginShape(POLYGON);

   PVector direction = PVector{x2-x1,y2-y1};
   PVector normal = direction.normal();
   normal.normalize();
   normal.mult(half_stroke);

   if (PShape::line_end_cap == ROUND ) {
      int NUMBER_OF_VERTICES=16;

      float start_angle = direction.heading() + HALF_PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(x1 + cos(i + start_angle) * half_stroke,
                      y1 + sin(i + start_angle) * half_stroke);
      }

      start_angle += PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(x2 + cos(i + start_angle) * half_stroke,
                      y2 + sin(i + start_angle) * half_stroke);
      }
   } else {
      p[0].add(normal);
      p[1].sub(normal);
      p[2].sub(normal);
      p[3].add(normal);

      if (PShape::line_end_cap == PROJECT) {
         direction.normalize();
         direction.mult(half_stroke);
         p[0].sub(direction);
         p[1].sub(direction);
         p[2].add(direction);
         p[3].add(direction);
      }

      shape.vertex( p[0] );
      shape.vertex( p[1] );
      shape.vertex( p[2] );
      shape.vertex( p[3] );
   }
   shape.endShape(CLOSE);
   return shape;
}

PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
   PShape shape;
   shape.beginShape(TRIANGLE_STRIP);
   shape.vertex(x1, y1);
   shape.vertex(x2, y2);
   shape.vertex(x3, y3);
   shape.endShape(CLOSE);
   return shape;
}


PVector ellipse_point(const PVector &center, int index, float start, float end, float xradius, float yradius) {
   float angle = map( index, 0, 32, start, end);
   return PVector( center.x + xradius * sin(-angle + HALF_PI),
                   center.y + yradius * cos(-angle + HALF_PI),
                   center.z);
}

PShape createArc(float x, float y, float width, float height, float start,
                 float stop, int mode = DEFAULT) {

   if (PShape::ellipse_mode != RADIUS) {
      width /=2;
      height /=2;
   }
   PShape shape;
   shape.beginShape(POLYGON);
   int NUMBER_OF_VERTICES=32;
   if ( mode == DEFAULT || mode == PIE ) {
      shape.vertex(x,y);
   }
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      shape.vertex( ellipse_point( {x,y}, i, start, stop, width, height ) );
   }
   shape.vertex( ellipse_point( {x,y}, 32, start, stop, width, height ) );
   shape.endShape(CLOSE);
   return shape;
   // NEED to tweak outline see Arc.cc
   // int NUMBER_OF_VERTICES=32;
   // std::vector<PVector> vertexBuffer;
   // if ( mode == PIE ) {
   //    vertexBuffer.push_back(center);
   // }
   // for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
   //    vertexBuffer.push_back( ellipse_point( center, i, start, end, xradius, yradius ) );
   // }
   // vertexBuffer.push_back( ellipse_point( center, 32, start, end, xradius, yradius ) );
   // if ( mode == CHORD || mode == PIE ) {
   //    vertexBuffer.push_back( vertexBuffer[0] );
   // }
   // glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}


PShape createUnitCircle(int NUMBER_OF_VERTICES = 32) {
   PShape shape;
   shape.beginShape(POLYGON);
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      shape.vertex( ellipse_point( {0,0,0}, i, 0, TWO_PI, 1.0, 1.0 ) );
   }
   shape.endShape(CLOSE);
   return shape;
}

PShape createEllipse(float x, float y, float width, float height) {
   static PShape unitCircle = createUnitCircle();
   PShape ellipse;
   ellipse.borrowVAO( unitCircle );
   if (PShape::ellipse_mode != RADIUS) {
      width /=2;
      height /=2;
   }
   ellipse.style = TRIANGLE_FAN;
   ellipse.translate(x,y);
   ellipse.scale(width,height);

   if ( PShape::stroke_on() ) {
      PShape group;
      group.beginShape(GROUP);

      PShape shape;
      shape.beginShape(LINES);
      shape.stroke_only = true;
      for(int i = 0; i < 32; ++i) {
         shape.vertex( ellipse_point( {x,y,0}, i, 0, TWO_PI, width, height ) );
      }
      shape.endShape(CLOSE);

      group.addChild( std::move(ellipse) );
      group.addChild( std::move(shape) );
      group.endShape(OPEN);
      return group;
   }
   return ellipse;
}

PShape createPoint(float x, float y) {
   static PShape unitCircle = createUnitCircle();
   PShape shape;
   shape.borrowVAO( unitCircle );
   shape.style = TRIANGLE_FAN;
   shape.translate(x,y);
   shape.scale(PShape::stroke_weight,PShape::stroke_weight);
   shape.stroke_only = true;
   return shape;
}



#endif
