#ifndef PROCESSING_OPENGL_FRAMEBUFFER_H
#define PROCESSING_OPENGL_FRAMEBUFFER_H

#include <utility>

typedef unsigned int GLuint;

class gl_framebuffer {
   GLuint id = 0;
   int width = 0;
   int height = 0;
   GLuint depthBufferID = 0;
   GLuint colorBufferID = 0;
public:

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
      frame.id = 0;
      frame.width = width;
      frame.height = height;
      return frame;
   }

   gl_framebuffer() {
   }

   gl_framebuffer(int width_, int height_);

   gl_framebuffer(const gl_framebuffer &x) = delete;

   gl_framebuffer(gl_framebuffer &&x) noexcept {
      *this = std::move(x);
   }

   gl_framebuffer& operator=(const gl_framebuffer&) = delete;

   gl_framebuffer& operator=(gl_framebuffer&&x) noexcept {
      std::swap(id,x.id);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(colorBufferID,x.colorBufferID);
      std::swap(width,x.width);
      std::swap(height,x.height);
      return *this;
   }

   ~gl_framebuffer();

   void bind();

   void blit(gl_framebuffer &dest);
};
#endif
