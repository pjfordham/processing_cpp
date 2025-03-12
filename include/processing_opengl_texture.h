#ifndef PROCESSING_OPENGL_TEXTURE_H
#define PROCESSING_OPENGL_TEXTURE_H

#include <memory>

typedef unsigned int GLuint;
typedef signed int GLint;

#include <fmt/core.h>

namespace gl {
   class texture_t;
}

template <> struct fmt::formatter<gl::texture_t>;

namespace gl {

   class texture_t;
   typedef std::shared_ptr<texture_t> texture_ptr;

   class texture_t {
      GLuint id;
      GLint wrap;
      bool owning;

   public:
      ~texture_t();

      // Create a non owning texture wrapper
      texture_t( GLuint textureID );

      // Create and manage the texture
      texture_t();

      texture_t(const texture_t&) = delete;
      texture_t(texture_t&&) = delete;
      texture_t& operator=(const texture_t&) = delete;
      texture_t& operator=(texture_t&&) = delete;

      static texture_ptr circle() {
         static texture_ptr t = std::make_shared<texture_t>(-1);
         return t;
      }

      void release();

      int get_width() const;

      int get_height() const;

      int _get_width() const;

      int _get_height() const;

      GLuint get_id() const;

      void bind() const;

      operator bool() const;

      void set_pixels(const unsigned int *pixels, int width, int height, GLint wrap);

      void get_pixels(unsigned int *pixels) const;

      friend struct fmt::formatter<gl::texture_t>;
  };

}
#endif
