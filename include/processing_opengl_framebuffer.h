#ifndef PROCESSING_OPENGL_FRAMEBUFFER_H
#define PROCESSING_OPENGL_FRAMEBUFFER_H

#include <utility>
#include <vector>
#include <string>

#include "processing_enum.h"

typedef unsigned int GLuint;

class gl_framebuffer {
   int aaFactor = 1;
   int aaMode = MSAA;
   GLuint id = 0;
   int width = 0;
   int height = 0;
   GLuint depthBufferID = 0;
   GLuint colorBufferID = 0;
   GLuint textureBufferID = 0;

public:
   GLuint getColorBufferID() const;

   auto getWidth() const {
      return width;
   }

   auto getHeight() const {
      return height;
   }

   bool isMainFrame() const {
      return id == 0;
   }

   static gl_framebuffer constructMainFrame(int width, int height) {
      gl_framebuffer frame;
      frame.width = width;
      frame.height = height;
      return frame;
   }

   gl_framebuffer() {
   }

   gl_framebuffer(int width_, int height_, int aaFactor_, int aaMode_ );

   gl_framebuffer(const gl_framebuffer &x) = delete;

   gl_framebuffer(gl_framebuffer &&x) noexcept : gl_framebuffer() {
      *this = std::move(x);
   }

   ~gl_framebuffer();

   gl_framebuffer& operator=(const gl_framebuffer&) = delete;

   gl_framebuffer& operator=(gl_framebuffer&&x) noexcept {
      std::swap(id,x.id);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(colorBufferID,x.colorBufferID);
      std::swap(aaFactor,x.aaFactor);
      std::swap(width,x.width);
      std::swap(height,x.height);
      return *this;
   }

   void updatePixels( std::vector<unsigned int> &pixels );

   void loadPixels( std::vector<unsigned int> &pixels );

   void bind();

   void blit(gl_framebuffer &dest) const ;

   void saveFrame(void *surface);

};
#endif
