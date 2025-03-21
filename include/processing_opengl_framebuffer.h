#ifndef PROCESSING_OPENGL_FRAMEBUFFER_H
#define PROCESSING_OPENGL_FRAMEBUFFER_H

#include <utility>
#include <vector>
#include <string>
#include <fmt/core.h>

#include "processing_enum.h"

typedef unsigned int GLuint;

namespace gl {
   class framebuffer;
}
template <> struct fmt::formatter<gl::framebuffer>;

namespace gl {
   class framebuffer {
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
         return width;
      }

      auto getHeight() const {
         return height;
      }

      bool isMainFrame() const {
         return id == 0;
      }

      static framebuffer constructMainFrame(int width, int height) {
         framebuffer frame;
         frame.width = width;
         frame.height = height;
         return frame;
      }

      framebuffer();

      framebuffer(int width_, int height_, int aaMode_ , int aaFactor);

      framebuffer(const framebuffer &x) = delete;

      framebuffer(framebuffer &&x) noexcept;

      ~framebuffer();

      framebuffer& operator=(const framebuffer&) = delete;

      framebuffer& operator=(framebuffer&&x) noexcept;

      void updatePixels( const std::vector<unsigned int> &pixels );

      void loadPixels( std::vector<unsigned int> &pixels );

      void invert( framebuffer &src );

      void bind();

      void clear( float r, float g, float b, float a );

      void blit(framebuffer &dest) const ;

      void saveFrame(void *surface);

      friend struct fmt::formatter<gl::framebuffer>;
   };
} // namespace gl

template <>
struct fmt::formatter<gl::framebuffer> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::framebuffer& v, FormatContext& ctx) {
      return format_to(ctx.out(), "aaFactor={:<2} aaMode={:<1} id={:4} width={:<4} height={:<4} depthBufferID={:<4} colorBufferID={:<4} textureBufferID={:<4}",
                       v.aaFactor, v.aaMode, v.id, v.width, v.height, v.depthBufferID, v.colorBufferID, v.textureBufferID);
   }
};

#endif
