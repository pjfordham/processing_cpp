#ifndef PROCESSING_TEXTURE_MANAGER_H
#define PROCESSING_TEXTURE_MANAGER_H

#include <vector>
#include <stdio.h>
#include <algorithm>

class PTexture {
public:
   int layer = 0;
   int left = 0;
   int top = 0;
   int right  = 1;
   int bottom = 1;

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
      fprintf(stderr,"%d %d %d %d %d\n", layer, left, top, right, bottom);
   }

   bool operator<( const PTexture &other ) const {
      return size() < other.size();
   }
};

class TextureManager {
public:
   int width=0, height=0;
   std::vector<PTexture> free;
   TextureManager() : width(0), height(0) {
   }
   TextureManager( int w, int h ) : width(w), height(h) {
      clear();
   };

   TextureManager(const TextureManager &x) = delete;

   TextureManager(TextureManager &&x) {
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(free, x.free);
   }

   TextureManager& operator=(const TextureManager&) = delete;

   TextureManager& operator=(TextureManager&&x){
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(free, x.free);
      return *this;
   }

   void clear() {
      free.clear();
      free.push_back( {2, 0, 0, width, height} );
      free.push_back( {3, 0, 0, width, height} );
      free.push_back( {4, 0, 0, width, height} );
      free.push_back( {5, 0, 0, width, height} );
      free.push_back( {6, 0, 0, width, height} );
      free.push_back( {7, 0, 0, width, height} );
   }

   PTexture getFreeBlock(int w, int h) {
      std::sort( free.begin(), free.end() );
      free.reserve( free.size() + 2 );
      for( auto &block : free ) {
         if (block.width() >= w && block.height() >= h) {
            PTexture p{ block.layer, block.left, block.top,  block.left + w, block.top + h };
            free.push_back( {block.layer, block.left + w, block.top, block.right,  block.top + h } );
            block = {block.layer, block.left, block.top + h, block.right,  block.bottom };
            return p;
         }
      }
      abort();
      return {};
   }

};
#endif
