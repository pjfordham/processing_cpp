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
   std::vector<PVector> vertices;

   int style = LINES;
   int type = OPEN;

   void clear() {
      vertices.clear();
   }

   void loadShape(const char *filename) {
   }

   void createShape(int type, float x, float y, float width, float height) {
      switch(type) {
      case RECT:
         vertices.clear();
         vertices.push_back({x,y});
         vertices.push_back({x+width,y});
         vertices.push_back({x+width,y+height});
         vertices.push_back({x,y+height});
         break;
      default:
         break;
      }
   }

   void draw() {
      if (vertices.size() > 0) {
         if (style == POINTS) {
            for (auto z : vertices ) {
               glEllipse(z, xstrokeWeight, xstrokeWeight,stroke_color,stroke_color,0);
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

void shape(const PShape &shape, float x, float y, float width, float height) {

}
#endif
