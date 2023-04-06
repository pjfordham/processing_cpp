#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "processing_math.h"
#include "processing_opengl.h"

extern SDL_Renderer *renderer;
extern int xstrokeWeight;
extern SDL_Color stroke_color;
extern SDL_Color fill_color;

void line(float x1, float y1, float x2, float y2);


enum{
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
  RECT,
};

class PShape {
public:
   static int rect_mode;
   static int ellipse_mode;
   GLuint VAO = 0;
   
   std::vector<PVector> vertices;

   int style = LINES;
   int type = OPEN;

   void clear() {
      vertices.clear();
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

      GLuint indexbuffer;
      glGenBuffers(1, &indexbuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

      GLuint vertexbuffer;
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

   void vertex(float x, float y, float z = 0.0) {
      vertices.push_back({x, y, z});
   }

   void endShape(int type_ = OPEN) {
      type = type_;
   }

   void draw() {
      if (vertices.size() > 0) {
         if (style == POINTS) {
            for (auto z : vertices ) {
               void point(float x, float y);
               point(z.x, z.y);
            }
         } else if (style == TRIANGLE_STRIP) {
            glFilledTriangleStrip( vertices.size(), vertices.data(), fill_color );
            glTriangleStrip( vertices.size(), vertices.data(), stroke_color, xstrokeWeight);
         } else if (style == TRIANGLE_FAN) {
            glFilledTriangleFan( vertices.size(), vertices.data(), fill_color );
            glTriangleFan( vertices.size(), vertices.data(), stroke_color, xstrokeWeight);
         } else if (style == LINES) {
            if (type == CLOSE) {
               glFilledPoly( vertices.size(), vertices.data(), fill_color );
               glClosedLinePoly( vertices.size(), vertices.data(), stroke_color, xstrokeWeight);
            } else {
               glLinePoly( vertices.size(), vertices.data(), stroke_color, xstrokeWeight);
            }
         }
      }
   }
 
};

enum {
   CORNERS = 0,
   CORNER = 1,
   CENTER = 2,
   RADIUS = 3,
};

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

enum { /*OPEN == 0,*/ CHORD = 1, PIE=2, DEFAULT=3 };
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
   return shape;
}

GLuint unitCircleVAO(int NUMBER_OF_VERTICES=32) {
   PShape shape = createUnitCircle( NUMBER_OF_VERTICES );
   return shape.createVAO();
}


void shape(const PShape &shape, float x, float y, float width, float height) {

}
#endif
