#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "processing_math.h"
#include "processing_math.h"

extern Matrix2D current_matrix;
extern SDL_Renderer *renderer;
extern int xstrokeWeight;
extern SDL_Color stroke_color;
extern SDL_Color fill_color;

void line(float x1, float y1, float x2, float y2);


enum{
   POINTS = 0,
   LINES = 1,
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
       if (style == POINTS) {
      for (auto z : vertices ) {
         auto p = current_matrix.multiply( z );
         if (xstrokeWeight == 1) {
            SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
            SDL_RenderDrawPoint(renderer, p.x, p.y);
         } else {
            filledEllipseRGBA(renderer, p.x, p.y, xstrokeWeight/2, xstrokeWeight/2,
                              stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
         }
      }
   } else if (type == CLOSE) {
      std::vector<Sint16> xs, ys;
      for (auto z : vertices ) {
         auto p = current_matrix.multiply( z );
         xs.push_back(p.x);
         ys.push_back(p.y);
      }

      filledPolygonRGBA(renderer,xs.data(),ys.data(),xs.size(),
                        fill_color.r,fill_color.g,fill_color.b,fill_color.a);
      polygonRGBA(renderer,xs.data(),ys.data(),xs.size(),
                  stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   } else if (style == LINES) {
      for (std::size_t i = 1; i < vertices.size(); ++i) {
         line(vertices[i-1].x, vertices[i-1].y, vertices[i].x, vertices[i].y);
      }
      if (type == CLOSE) {
         line(vertices[vertices.size()-1].x, vertices[vertices.size()-1].y, vertices[0].x, vertices[0].y);
      }
   }

   }
};

void shape(const PShape &shape, float x, float y, float width, float height) {

}
#endif
