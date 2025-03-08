#ifndef PROCESSING_OPENGL_TEXTURE_H
#define PROCESSING_OPENGL_TEXTURE_H

#include <memory>

typedef unsigned int GLuint;
typedef signed int GLint;

namespace gl {

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

      void release();

      int get_width() const;

      int get_height() const;

      GLuint get_id() const;

      void bind() const;

      operator bool() const;

      void set_pixels(const unsigned int *pixels, int width, int height);

      void get_pixels(unsigned int *pixels) const;
   };

   typedef std::shared_ptr<texture_t> texture_ptr;
}

#endif
