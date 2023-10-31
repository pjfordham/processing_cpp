#ifndef PROCESSING_OPENGL_FRAMEBUFFER_H
#define PROCESSING_OPENGL_FRAMEBUFFER_H

#include <utility>
#include <vector>
#include <string>
#include <fmt/core.h>

#include "processing_enum.h"
#include "processing_debug.h"
#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

typedef unsigned int GLuint;

class gl_framebuffer;
template <> struct fmt::formatter<gl_framebuffer>;

class gl_framebuffer {
public:
   int aaFactor = 1;
   int aaMode = SSAA;
   GLuint id = 0;
   int width = 0;
   int height = 0;
   GLuint depthBufferID = 0;
   GLuint colorBufferID = 0;
   GLuint textureBufferID = 0;

public:
   GLuint getColorBufferID();

   auto getWidth() const {
      DEBUG_METHOD();
      return width;
   }

   auto getHeight() const {
      DEBUG_METHOD();
      return height;
   }

   bool isMainFrame() const {
      DEBUG_METHOD();
      return id == 0;
   }

   static gl_framebuffer constructMainFrame(int width, int height) {
      gl_framebuffer frame;
      frame.width = width;
      frame.height = height;
      return frame;
   }

   gl_framebuffer() {
      DEBUG_METHOD();
   }

   gl_framebuffer(int width_, int height_, int aaFactor_, int aaMode_ );

   gl_framebuffer(const gl_framebuffer &x) = delete;

   gl_framebuffer(gl_framebuffer &&x) noexcept : gl_framebuffer() {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   ~gl_framebuffer();

   gl_framebuffer& operator=(const gl_framebuffer&) = delete;

   gl_framebuffer& operator=(gl_framebuffer&&x) noexcept {
      DEBUG_METHOD();
      std::swap(aaFactor,x.aaFactor);
      std::swap(aaMode,x.aaMode);
      std::swap(id,x.id);
      std::swap(width,x.width);
      std::swap(height,x.height);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(colorBufferID,x.colorBufferID);
      std::swap(textureBufferID,x.textureBufferID);
      return *this;
   }

   void updatePixels( const std::vector<unsigned int> &pixels );

   void loadPixels( std::vector<unsigned int> &pixels );

   void bind();

   void blit(gl_framebuffer &dest) const ;

   void saveFrame(void *surface);

};

template <>
struct fmt::formatter<gl_framebuffer> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl_framebuffer& v, FormatContext& ctx) {
      return format_to(ctx.out(), "aaFactor={:<2} aaMode={:<1} id={:4} width={:<4} height={:<4} depthBufferID={:<4} colorBufferID={:<4} textureBufferID={:<4}",
                       v.aaFactor, v.aaMode, v.id, v.width, v.height, v.depthBufferID, v.colorBufferID, v.textureBufferID);
   }
};

#endif
