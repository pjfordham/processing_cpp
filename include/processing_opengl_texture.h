#ifndef PROCESSING_OPENGL_TEXTURE_H
#define PROCESSING_OPENGL_TEXTURE_H

#include "glad/glad.h"
#include "processing_task_queue.h"

namespace gl {

   class texture_t {
      GLuint id = 0;
      GLint wrap = GL_CLAMP_TO_EDGE;
      bool owning = false;

   public:
      ~texture_t() {
         if(owning) {
            renderThread.dispatch( [&] {
               glDeleteTextures(1,&id);
            } );
         }
      }

      // Create a non owning texture wrapper
      texture_t( GLuint textureID ) : id(textureID) {
         renderThread.dispatch( [&] {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap);
            glBindTexture(GL_TEXTURE_2D, 0);
         } );
      }

      // Create and manage the texture
      texture_t() {
         owning = true;
      }

      void release() {
         if (id && owning) {
            renderThread.dispatch( [&] {
               glDeleteTextures(1,&id);
            } );
         }
         id = 0;
         wrap = GL_CLAMP_TO_EDGE;
         owning = false;
      }

      int get_width() const {
         int width;
         renderThread.dispatch( [&] {
            glBindTexture(GL_TEXTURE_2D, id);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            glBindTexture(GL_TEXTURE_2D, 0);
         } );
         return width;
      }

      int get_height() const {
         int height;
         renderThread.dispatch( [&] {
            glBindTexture(GL_TEXTURE_2D, id);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            glBindTexture(GL_TEXTURE_2D, 0);
         } );
         return height;
      }

      GLuint get_id() const {
         return id;
      }

      operator bool() const {
         return id != 0;
      }

      void set_pixels(const unsigned int *pixels, int width, int height) {
         renderThread.dispatch( [&] {
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

      void get_pixels(unsigned int *pixels) const {
         renderThread.dispatch( [&] {
            glBindTexture(GL_TEXTURE_2D, id);
            glGetTexImage(GL_TEXTURE_2D, 0 , GL_RGBA, GL_UNSIGNED_BYTE, pixels );
            glBindTexture(GL_TEXTURE_2D, 0);
         } );
      }

   };

}

#endif
