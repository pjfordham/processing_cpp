#include "processing_pgraphics.h"

void PGraphics::init() {
}

void PGraphics::close() {
}

void PGraphics::text(const std::string &text, float x, float y, float twidth, float theight) {
   PTexture texture;
   auto existing = words.find( text );
   if ( existing != words.end() ) {
      texture = existing->second;
   } else {
      PImage image = currentFont.render_as_pimage(text);
      texture = words[text] = glc.getTexture( image.width, image.height, image.pixels );
   }

   twidth = texture.width();
   theight = texture.height();

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
                    texture, _shape.fill_color);
}
