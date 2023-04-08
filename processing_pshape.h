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


class PShape {
public:
   static color stroke_color;
   static color fill_color;
   static int rect_mode;
   static int ellipse_mode;
   static int stroke_weight;
   static int line_end_cap;
   GLuint VAO = 0;
   GLuint indexbuffer = 0;
   GLuint vertexbuffer = 0;
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
      indexbuffer = other.indexbuffer;
      other.indexbuffer = 0;
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
      indexbuffer = other.indexbuffer;
      other.indexbuffer = 0;
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

      std::vector<unsigned short> indices;
      std::vector<float> vertex;
      for(int i = 0; i < vertices.size(); ++i) {
         vertex.push_back( vertices[i].x );
         vertex.push_back( vertices[i].y );
         vertex.push_back( vertices[i].z );
         indices.push_back(indices.size());
      }

      glGenBuffers(1, &indexbuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

      glGenBuffers(1, &vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(), GL_STATIC_DRAW);

      GLuint attribId = glGetAttribLocation(programID, "position");
      glEnableVertexAttribArray(attribId);
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
      glVertexAttribPointer(
         attribId,                         // attribute
         3,                                // size
         GL_FLOAT,                         // type
         GL_FALSE,                         // normalized?
         0,                                // stride
         (void*)0                          // array buffer offset
         );

      return VAO;
   }

   void releaseVAO() {
      // sometimes we borrow a VAO, but we'd never have the vertexbuffer if we did
      if (vertexbuffer) {
         glDeleteBuffers(1, &vertexbuffer);
         glDeleteBuffers(1, &indexbuffer);
         glDeleteVertexArrays(1, &VAO);
         VAO = 0;
         vertexbuffer = 0;
         indexbuffer = 0;
      }
   }

   void vertex(float x, float y, float z = 0.0) {
      vertices.push_back({x, y, z});
   }

   void endShape(int type_ = OPEN) {
      type = type_;
   }

   void draw( ) {
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
         glDrawElements(GL_TRIANGLE_FAN, 32, GL_UNSIGNED_SHORT, 0);
         glBindVertexArray(0);
         glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
         return;
      }

      if (vertices.size() > 0) {
         if (style == POINTS) {
            for (auto z : vertices ) {
               void point(float x, float y);
               point(z.x, z.y);
            }
         } else if (style == TRIANGLE_STRIP) {
            glFilledTriangleStrip( vertices.size(), vertices.data(), fill_color );
            glTriangleStrip( vertices.size(), vertices.data(), stroke_color, stroke_weight);
         } else if (style == TRIANGLE_FAN) {
            glFilledTriangleFan( vertices.size(), vertices.data(), fill_color );
            glTriangleFan( vertices.size(), vertices.data(), stroke_color, stroke_weight);
         } else if (style == LINES) {
            if (type == CLOSE) {
               if (stroke_only) {
                  glFilledPoly( vertices.size(), vertices.data(), stroke_color );
               } else {
                  glFilledPoly( vertices.size(), vertices.data(), fill_color );
                  glLinePoly( vertices.size(), vertices.data(), stroke_color, stroke_weight, true);
               }
            } else {
               glLinePoly( vertices.size(), vertices.data(), stroke_color, stroke_weight, false);
            }
         }
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

   PVector direction = PVector{x2-x1,y2-y1};
   PVector normal = direction.normal();
   normal.normalize();
   normal.mult(half_stroke);

   if (PShape::line_end_cap == ROUND ) {
      PShape shape;
      shape.stroke_only = true;
      shape.style = LINES;
      shape.type = CLOSE;
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
      return shape;
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

    auto shape =  createQuad( p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
    shape.stroke_only = true;
    return shape;
   }
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
   ellipse.VAO = unitCircle.VAO;
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
   shape.VAO = unitCircle.VAO;
   shape.translate(x,y);
   shape.scale(PShape::stroke_weight,PShape::stroke_weight);
   shape.stroke_only = true;
   return shape;
}



void shape(const PShape &shape, float x, float y, float width, float height) {

}
#endif
