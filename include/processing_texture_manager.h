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
   int layer = 0;
   int left = 0;
   int top = 0;
   int right  = 1;
   int bottom = 1;
   int sheet_width = 0;
   int sheet_height = 0;

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

   bool isValid() {
      return sheet_width != 0;
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
   struct key {
      int width, height;
      void *p;
      bool operator<( const key &other ) const {
         if ( width != other.width) return width < other.width;
         if ( height != other.height) return height < other.height;
         return p < other.p;
      }
   };
   std::map<key, PTexture> tmap;

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

   void clear() {
      free.clear();
      tmap.clear();
      // Leave 0,0,0 as a white pixel for untextured surfaces.
      free.push_back( {0, 0, 1, width, height, width, height} );
   }

   PTexture getFreeBlock(int w, int h, void *z) {
      if (tmap.count({w,h,z}) == 1) {
         return tmap[{w,h,z}];
      }

      std::sort( free.begin(), free.end() );
      free.reserve( free.size() + 2 );
      for( auto &block : free ) {
         if (block.width() >= w && block.height() >= h) {
            PTexture p{ block.layer, block.left, block.top,  block.left + w, block.top + h, width, height };
            free.push_back( {block.layer, block.left + w, block.top, block.right,  block.top + h, width, height } );
            block = {block.layer, block.left, block.top + h, block.right,  block.bottom, width, height };
            tmap[{w,h,z}] = p;
            return p;
         }
      }
      return {}; // An invalid texture
   }

};
#endif
