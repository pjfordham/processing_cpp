#ifndef PROCESSING_TEXTURE_MANAGER_H
#define PROCESSING_TEXTURE_MANAGER_H

#include <vector>
#include <algorithm>
#include <fmt/core.h>
#include <map>

#include "processing_math.h"

typedef unsigned int GLuint;

class PTexture {
public:
  int layer;
   int left;
   int top;
   int right;
   int bottom;
   int sheet_width;
   int sheet_height;
   friend class TextureManager;
   friend class gl_context;

public:
   PTexture() :
      layer(0), left(0), top(0), right(1), bottom(1),
      sheet_width(0), sheet_height(0) {}

   PTexture(int layer_, int left_, int top_, int right_, int bottom_, int sheet_width_, int sheet_height_) :
      layer( layer_), left( left_ ), top( top_ ), right( right_ ), bottom( bottom_),
      sheet_width( sheet_width_ ), sheet_height( sheet_height_) {}

   bool operator==(const PTexture &x) const {
      return
         layer == x.layer &&
         left == x.left &&
         top == x.top &&
         right == x.right &&
         bottom == x.bottom &&
         sheet_width == x.sheet_width &&
         sheet_height == x.sheet_height;
   }

   bool isValid() const {
      return sheet_width != 0;
   }

   PVector2 normalize(PVector2 t) const {
      // could check return values are within 0-1
      if (isValid()) {
            return {
               map(t.x,0,1.0,(1.0*left)/sheet_width,(1.0*(right-1))/sheet_width),
               map(t.y,0,1.0,(1.0*top)/sheet_height,(1.0*(bottom-1))/sheet_height) };
      } else {
         return t;
      }
   }

   static PTexture circle() {
      return { 8, 0,0, 1,1,1,1  };
   }

   int size() const {
      return width() * height();
   }

   int width() const {
      return right - left;
   }

   int height() const {
      return bottom - top;
   }

   void print() const {
      fmt::print(stderr,"{} {} {} {} {} ({} {})\n", layer, left, top, right, bottom, sheet_width, sheet_height);
   }

   bool operator<( const PTexture &other ) const {
      return size() < other.size();
   }
};

class TextureManager {
   int width, height;
   std::vector<PTexture> free;
   GLuint textureID = 0;

   void clear() {
      free.clear();
      // Leave 0,0,0 as a white pixel for untextured surfaces.
      free.push_back( {0, 0, 1, width, height, width, height} );
   }

public:
   TextureManager() : width(0), height(0) {
   }

   TextureManager( int w, int h );

   TextureManager(const TextureManager &x) = delete;

   TextureManager(TextureManager &&x) noexcept : TextureManager()  {
      *this = std::move( x );
   }

   auto getTextureID() const {
      return textureID;
   }

   TextureManager& operator=(const TextureManager&) = delete;

   TextureManager& operator=(TextureManager&&x) noexcept {
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(free, x.free);
      std::swap(textureID, x.textureID);
      return *this;
   }

   ~TextureManager();

   PTexture getFreeBlock(int w, int h) {
      std::sort( free.begin(), free.end() );
      free.reserve( free.size() + 2 );
      for( auto &block : free ) {
         if (block.width() >= w && block.height() >= h) {
            PTexture p{ block.layer, block.left, block.top,  block.left + w, block.top + h, width, height };
            free.push_back( {block.layer, block.left + w, block.top, block.right,  block.top + h, width, height } );
            block = {block.layer, block.left, block.top + h, block.right,  block.bottom, width, height };
            return p;
         }
      }
      return {}; // An invalid texture
   }

};
#endif
