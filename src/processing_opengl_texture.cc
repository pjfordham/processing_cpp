#include "glad/glad.h"
#include "processing_task_queue.h"
#include "processing_opengl_texture.h"

namespace gl {

   texture_t::~texture_t() {
      if(owning) {
         renderThread.enqueue( [id=id] {
            glDeleteTextures(1,&id);
         } );
      }
   }

   // Create a non owning texture wrapper
   texture_t::texture_t( GLuint textureID ) : id(textureID), owning(false) {
      // renderThread.enqueue( TaskQueue::Mode::Blocking, [&] {
      //    glBindTexture(GL_TEXTURE_2D, textureID);
      //    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
      //    glBindTexture(GL_TEXTURE_2D, 0);
      // } );
   }

   // Create and manage the texture
   texture_t::texture_t() : id(0), wrap(GL_CLAMP_TO_EDGE), owning(true) {
   }

   void texture_t::release() {
      if (id && owning) {
         renderThread.enqueue( [id=id] {
            glDeleteTextures(1,&id);
         } );
      }
      id = 0;
      wrap = GL_CLAMP_TO_EDGE;
      owning = false;
   }

   int texture_t::_get_width() const {
      int width;
      glBindTexture(GL_TEXTURE_2D, id);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
      glBindTexture(GL_TEXTURE_2D, 0);
      return width;
   }

   int texture_t::_get_height() const {
      int height;
      glBindTexture(GL_TEXTURE_2D, id);
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
      glBindTexture(GL_TEXTURE_2D, 0);
      return height;
   }

   int texture_t::get_width() const {
      int width;
      renderThread.enqueue( TaskQueue::Mode::Blocking, [&] {
         width = _get_width();
      } );
      return width;
   }

   int texture_t::get_height() const {
      int height;
      renderThread.enqueue( TaskQueue::Mode::Blocking, [&] {
         height = _get_height();
      } );
      return height;
   }

   GLuint texture_t::get_id() const {
      return id;
   }

   void texture_t::bind() const {
     // renderThread.enqueue( [id=id] {
        glBindTexture(GL_TEXTURE_2D, id);
     // } );
   }

   texture_t::operator bool() const {
      return id != 0;
   }

   void texture_t::set_pixels(const unsigned int *pixels, int width, int height) {
      renderThread.enqueue( TaskQueue::Mode::Blocking, [&] {
         if (!id) {
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
         }
         glBindTexture(GL_TEXTURE_2D, id);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
         glBindTexture(GL_TEXTURE_2D, 0);
      } );
   }

   void texture_t::get_pixels(unsigned int *pixels) const {
      renderThread.enqueue( TaskQueue::Mode::Blocking, [&] {
         glBindTexture(GL_TEXTURE_2D, id);
         glGetTexImage(GL_TEXTURE_2D, 0 , GL_RGBA, GL_UNSIGNED_BYTE, pixels );
         glBindTexture(GL_TEXTURE_2D, 0);
      } );
   }

}
