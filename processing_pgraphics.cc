#include "processing_pgraphics.h"

#include <SDL2/SDL.h>

void PGraphics::init() {
   // Initialize SDL
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      abort();
   }
}

void PGraphics::close() {
   // Clean up
   SDL_Quit();
}

void PGraphics::text(std::string text, float x, float y, float twidth, float theight) {
   SDL_Surface *surface = currentFont.render_text(text);

   twidth = surface->w;
   theight = surface->h;

   PTexture texture = glc.getTexture( surface );

   SDL_FreeSurface(surface);

   // this works well enough for the Letters.cc example but it's not really general
   if ( xTextAlign == CENTER ) {
      x = x - twidth / 2;
   }
   if ( yTextAlign == CENTER ) {
      y = y - theight / 2;
   }
   if ( xTextAlign == RIGHT ) {
      x = x - twidth;
   }
   if ( yTextAlign == RIGHT ) {
      y = y - theight;
   }

   drawTexturedQuad({x,y},{x+twidth,y},{x+twidth,y+theight},{x,y+theight},
                    texture);
}
