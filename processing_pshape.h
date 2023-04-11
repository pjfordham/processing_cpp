#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_opengl.h"
#include "processing_color.h"

enum {
   POINTS = 0,
   LINES = 1,
   TRIANGLE_STRIP,
   TRIANGLE_FAN,
};

enum{
   OPEN = 0,
   CLOSE = 1,
};
enum {
   CORNERS = 0,
   CORNER = 1,
   CENTER = 2,
   RADIUS = 3,
};

enum {
   ROUND = 0,
   SQUARE,
   PROJECT,
};

enum {
   /*OPEN == 0,*/
   CHORD = 1,
   PIE=2,
   DEFAULT=3
};
enum {
   DIAMETER = 1,
//  RADIUS = 3,
};

// EVERYTHIING BELOW HERE IS ONLY USED BY PSHAPE
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
   GLuint VAO = 0;
   GLuint vertexbuffer = 0;
   int vertexbuffer_size = 0;
   Eigen::Matrix4f shape_matrix = Eigen::Matrix4f::Identity();
   std::vector<PVector> vertices;

   int style = LINES;
   int type = OPEN;
   bool stroke_only = false;

   PShape(const PShape& other) = delete;
   PShape& operator=(const PShape& other) = delete;

   PShape() {
   }

   PShape(PShape&& other) noexcept {
      VAO = other.VAO;
      other.VAO = 0;
      vertexbuffer_size = other.vertexbuffer_size;
      other.vertexbuffer_size = 0;
      vertexbuffer = other.vertexbuffer;
      other.vertexbuffer = 0;
      vertices = std::move(other.vertices);
      other.vertices.clear();
      style = other.style;
      type = other.type;
      stroke_only = other.stroke_only;
   }

   PShape& operator=(PShape&& other) noexcept {
      VAO = other.VAO;
      other.VAO = 0;
      vertexbuffer_size = other.vertexbuffer_size;
      other.vertexbuffer_size = 0;
      vertexbuffer = other.vertexbuffer;
      other.vertexbuffer = 0;
      vertices = std::move(other.vertices);
      other.vertices.clear();
      style = other.style;
      type = other.type;
      stroke_only = other.stroke_only;
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

   void beginShape(int points = LINES) {
      style = points;
      clear();
   }

   GLuint createVAO() {
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

      return VAO;
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
         VAO = 0;
         vertexbuffer = 0;
      }
   }

   void vertex(float x, float y, float z = 0.0) {
      vertices.push_back({x, y, z});
   }

   void endShape(int type_ = OPEN) {
      type = type_;
   }

   void glFilledElement(GLuint element_type, int points, const PVector *p, color color) const {
      anything_drawn = true;

      GLuint VAO;
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      GLuint vertexbuffer;
      glGenBuffers(1, &vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, points * 3 * sizeof(float), p, GL_STATIC_DRAW);

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

      float color_vec[] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
      glUniform4fv(Color, 1, color_vec);

      glDrawArrays(element_type, 0, points);

      glDeleteBuffers(1, &vertexbuffer);

      // Unbind the buffer objects and VAO
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glDeleteVertexArrays(1, &VAO);
   }

   void glFilledPoly(int points, const PVector *p, color color)  {
      std::vector<PVector> triangles = triangulatePolygon({p,p+points});
      glFilledElement(GL_TRIANGLES,triangles.size(), triangles.data(),BLUE);

      // This would works ut we overwrite the original data before we draw the outline
      // vertices = triangles;
      // stroke_only = true;
      // createVAO();
      // draw( GL_TRIANGLES );
      // releaseVAO();
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
   void glLine(std::vector<PVector> &triangles, PVector p1, PVector p2, color color, int weight)  {

      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      triangles.push_back(p1 + normal);
      triangles.push_back(p1 - normal);
      triangles.push_back(p2 + normal);

      triangles.push_back(p2 + normal);
      triangles.push_back(p2 - normal);
      triangles.push_back(p1 - normal);

   }

   void glTriangleStrip(int points, const PVector *p, color color,int weight)  {
      std::vector<PVector> triangles;
      for (int i =2; i<points;++i) {
         glLine(triangles, p[i-2], p[i-1], color, weight);
         glLine(triangles, p[i-1], p[i], color, weight);
         glLine(triangles, p[i], p[i-2], color, weight);
      }
      vertices = triangles;
      stroke_only = true;
      createVAO();
      draw( GL_TRIANGLES );
      releaseVAO();
   }

   void glTriangleFan(int points, const PVector *p, color color,int weight)  {
      std::vector<PVector> triangles;
      glLine(triangles, p[0], p[1], color, weight );
      for (int i =2; i<points;++i) {
         glLine(triangles, p[i-1], p[i], color, weight);
         glLine(triangles, p[0], p[i], color, weight);
      }
      vertices = triangles;
      stroke_only = true;
      createVAO();
      draw( GL_TRIANGLES );
      releaseVAO();
   }

   PLine glLineMitred(PVector p1, PVector p2, PVector p3, float half_weight)  {
      PLine l1{ p1, p2 };
      PLine l2{ p2, p3 };
      PLine low_l1 = l1.offset(-half_weight);
      PLine high_l1 = l1.offset(half_weight);
      PLine low_l2 = l2.offset(-half_weight);
      PLine high_l2 = l2.offset(half_weight);
      return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
   }

   void glLinePoly(int points, const PVector *p, color color, int weight, bool closed)  {
      PLine start;
      PLine end;

      std::vector<PVector> triangle_strip;

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

      triangle_strip.push_back( start.start );
      triangle_strip.push_back( start.end );

      for (int i =0; i<points-2;++i) {
         PLine next = glLineMitred(p[i], p[i+1], p[i+2], half_weight);
         triangle_strip.push_back( next.start );
         triangle_strip.push_back( next.end );
      }
      if (closed) {
         PLine next = glLineMitred(p[points-2], p[points-1], p[0], half_weight);
         triangle_strip.push_back( next.start );
         triangle_strip.push_back( next.end );
      }

      triangle_strip.push_back( end.start );
      triangle_strip.push_back( end.end );

      vertices = triangle_strip;
      stroke_only = true;
      createVAO();
      draw( GL_TRIANGLE_STRIP );
      releaseVAO();
   }

   void draw(GLuint element_type = GL_TRIANGLE_FAN ) {
      extern GLuint Mmatrix;
      if ( VAO ) {
         if (stroke_only) {
            float color_vec[] = {
               stroke_color.r / 255.0f,
               stroke_color.g / 255.0f,
               stroke_color.b / 255.0f,
               stroke_color.a / 255.0f };
            glUniform4fv(Color, 1, color_vec);
         } else {
            float color_vec[] = {
               fill_color.r / 255.0f,
               fill_color.g / 255.0f,
               fill_color.b / 255.0f,
               fill_color.a / 255.0f };
            glUniform4fv(Color, 1, color_vec);
         }

         Eigen::Matrix4f new_matrix = move_matrix * shape_matrix;
         glUniformMatrix4fv(Mmatrix, 1,false, new_matrix.data());
         glBindVertexArray(VAO);
         glDrawArrays(element_type, 0, vertexbuffer_size);
         glBindVertexArray(0);
         glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
         return;
      }

      if (vertices.size() > 0) {
         // Eigen::Matrix4f new_matrix = move_matrix * shape_matrix;
         // glUniformMatrix4fv(Mmatrix, 1,false, new_matrix.data());
         if (style == POINTS) {
            for (auto z : vertices ) {
               createPoint( z.x, z.y ).draw();
            }
         } else if (style == TRIANGLE_STRIP) {
            createVAO();
            draw( GL_TRIANGLE_STRIP );
            releaseVAO();
            glTriangleStrip( vertices.size(), vertices.data(), stroke_color, stroke_weight);
         } else if (style == TRIANGLE_FAN) {
            createVAO();
            draw( GL_TRIANGLE_FAN );
            releaseVAO();
            glTriangleFan( vertices.size(), vertices.data(), stroke_color, stroke_weight);
         } else if (style == LINES) {
            if (type == CLOSE) {
               if (stroke_only) {
                  // It's one of our own shapes so we known it's convex and
                  // we don't need to triangulate.
                  createVAO();
                  draw();
                  releaseVAO();
               } else {
                  glFilledPoly( vertices.size(), vertices.data(), fill_color );
                  glLinePoly( vertices.size(), vertices.data(), stroke_color, stroke_weight, true);
               }
            } else {
               glLinePoly( vertices.size(), vertices.data(), stroke_color, stroke_weight, false);
            }
         }
         // glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
      }
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
      width = width -x;
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
   shape.style = LINES;
   shape.type = CLOSE;
   shape.vertices = {
      {x,y},
      {x+width,y},
      {x+width,y+height},
      {x,y+height}
   };
   return shape;
}

PShape createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
   PShape shape;
   shape.style = LINES;
   shape.type = CLOSE;
   shape.vertices = { PVector{x1,y1},PVector{x2,y2},PVector{x3,y3},PVector{x4,y4} };
   return shape;
}


PShape createLine(float x1, float y1, float x2, float y2) {
   PVector p[] = {{x1,y1},{x1,y1},{x2,y2},{x2,y2}};

   float half_stroke = PShape::stroke_weight/2.0;

   PShape shape;
   shape.stroke_only = true;
   shape.type = CLOSE;
   shape.style = LINES;

   PVector direction = PVector{x2-x1,y2-y1};
   PVector normal = direction.normal();
   normal.normalize();
   normal.mult(half_stroke);

   if (PShape::line_end_cap == ROUND ) {
      int NUMBER_OF_VERTICES=16;

      float start_angle = direction.get_angle() + HALF_PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertices.emplace_back(x1 + cos(i + start_angle) * half_stroke,
                                     y1 + sin(i + start_angle) * half_stroke);
      }

      start_angle += PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertices.emplace_back(x2 + cos(i + start_angle) * half_stroke,
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

      shape.vertices = { p[0], p[1], p[2], p[3] };
   }
   return shape;
}

PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
   PShape shape;
   shape.style = TRIANGLE_FAN;
   shape.type = CLOSE;
   shape.vertices ={ PVector{x1,y1},PVector{x2,y2},PVector{x3,y3} };
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
   shape.style = LINES;
   shape.type = CLOSE;
   int NUMBER_OF_VERTICES=32;
   if ( mode == DEFAULT || mode == PIE ) {
      shape.vertices.push_back({x,y});
   }
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      shape.vertices.push_back( ellipse_point( {x,y}, i, start, stop, width, height ) );
   }
   shape.vertices.push_back( ellipse_point( {x,y}, 32, start, stop, width, height ) );
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
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      shape.vertices.push_back( ellipse_point( {0,0,0}, i, 0, TWO_PI, 1.0, 1.0 ) );
   }
   shape.createVAO();
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
   ellipse.translate(x,y);
   ellipse.scale(width,height);
   return ellipse;
}

PShape createPoint(float x, float y) {
   static PShape unitCircle = createUnitCircle();
   PShape shape;
   shape.borrowVAO( unitCircle );
   shape.translate(x,y);
   shape.scale(PShape::stroke_weight,PShape::stroke_weight);
   shape.stroke_only = true;
   return shape;
}



#endif
