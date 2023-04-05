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


void shape(const PShape &shape, float x, float y, float width, float height) {

}
#endif
