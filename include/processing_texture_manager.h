#ifndef PROCESSING_TEXTURE_MANAGER_H
#define PROCESSING_TEXTURE_MANAGER_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include <stdio.h>
#include <algorithm>
#include "processing_math.h"

class PTexture {
public:
   int layer = 0;
   int left = 0;
   int top = 0;
   int right  = 1;
   int bottom = 1;
   int sheet_width = 0;
   int sheet_height = 0;

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
      fprintf(stderr,"%d %d %d %d %d (%d %d)\n", layer, left, top, right, bottom, sheet_width, sheet_height);
   }

   float ntop() const {
      return map(top,0,sheet_height, 0, 1.0);
   }

   float nbottom() const {
      return map(bottom,0,sheet_height, 0, 1.0);
   }

   float nleft() const {
      return map(left,0,sheet_width, 0, 1.0);
   }

   float nright() const {
      return map(right,0,sheet_width, 0, 1.0);
   }

   bool operator<( const PTexture &other ) const {
      return size() < other.size();
   }
};

class TextureManager {
   int width, height;
   std::vector<PTexture> free;

public:
   GLuint textureID = 0;
   TextureManager() : width(0), height(0) {
   }

   TextureManager( int w, int h ) : width(w), height(h) {
      if (width == 0 || height == 0)
         return;
      // create the texture array
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      // set texture parameters
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
      GLubyte white[4] = { 255, 255, 255, 255 };
      glClearTexImage(textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
      clear();
   };

   TextureManager(const TextureManager &x) = delete;

   TextureManager(TextureManager &&x) noexcept : TextureManager()  {
      *this = std::move( x );
   }

   TextureManager& operator=(const TextureManager&) = delete;

   TextureManager& operator=(TextureManager&&x) noexcept {
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(free, x.free);
      std::swap(textureID, x.textureID);
      return *this;
   }

   ~TextureManager() {
      if (textureID)
         glDeleteTextures(1, &textureID);
   }

   void clear() {
      free.clear();
      // Leave 0,0,0 as a white pixel for untextured surfaces.
      free.push_back( {0, 0, 1, width, height, width, height} );
   }

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
      abort();
      return {};
   }

};
#endif
