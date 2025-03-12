#include "glad/glad.h"
#include "processing_opengl_texture.h"
#include "processing_debug.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

namespace gl {

   texture_t::~texture_t() {
      DEBUG_METHOD();
      release();
   }

   // Create a non owning texture wrapper
   texture_t::texture_t( GLuint textureID ) : id(textureID), owning(false) {
      DEBUG_METHOD();
      glBindTexture(GL_TEXTURE_2D, textureID);
      glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   // Create and manage the texture
   texture_t::texture_t() : id(0), wrap(GL_CLAMP_TO_EDGE), owning(true) {
      DEBUG_METHOD();
   }

   void texture_t::release() {
      DEBUG_METHOD();
      if (id && owning) {
         glDeleteTextures(1,&id);
      }
      id = 0;
      wrap = GL_CLAMP_TO_EDGE;
      owning = true;
   }

   int texture_t::get_width() const {
      DEBUG_METHOD();
      int width;
      glBindTexture(GL_TEXTURE_2D, id);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
      glBindTexture(GL_TEXTURE_2D, 0);
      return width;
   }

   int texture_t::get_height() const {
      DEBUG_METHOD();
      int height;
      glBindTexture(GL_TEXTURE_2D, id);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
      glBindTexture(GL_TEXTURE_2D, 0);
      return height;
   }

   GLuint texture_t::get_id() const {
      DEBUG_METHOD();
      return id;
   }

   void texture_t::bind() const {
      DEBUG_METHOD();
      glBindTexture(GL_TEXTURE_2D, id);
   }

   texture_t::operator bool() const {
      DEBUG_METHOD();
      return id != 0;
   }

   void texture_t::set_pixels(const unsigned int *pixels, int width, int height, GLint wrap_) {
      DEBUG_METHOD();
      if (!id) {
         wrap = wrap_;
         glGenTextures(1, &id);
         glBindTexture(GL_TEXTURE_2D, id);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
         glBindTexture(GL_TEXTURE_2D, 0);
      }
      glBindTexture(GL_TEXTURE_2D, id);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   void texture_t::get_pixels(unsigned int *pixels) const {
      DEBUG_METHOD();
      glBindTexture(GL_TEXTURE_2D, id);
      glGetTexImage(GL_TEXTURE_2D, 0 , GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glBindTexture(GL_TEXTURE_2D, 0);
   }

}

static const char *textureWrapModeToText(GLint mode) {
   if (mode == GL_REPEAT) {
      return "GL_REPEAT       ";
   } else if (mode == GL_CLAMP_TO_EDGE) {
      return "GL_CLAMP_TO_EDGE";
   } else {
      return "INVALID MODE!!!!";
   }
}

template <>
struct fmt::formatter<gl::texture_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::texture_t& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "id={:<2} wrap={} owning={}",
			    v.id, textureWrapModeToText(v.wrap), v.owning ? "true " : "false" );
   }
};


