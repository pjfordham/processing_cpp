#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <fmt/core.h>

#include "processing_opengl_framebuffer.h"

gl_framebuffer::gl_framebuffer(int width_, int height_)  : width(width_), height(height_) {
   glGenFramebuffers(1, &id);
   bind();

   glGenRenderbuffers(1, &depthBufferID);
   glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);

   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

   glGenTextures(1, &colorBufferID);
   glBindTexture(GL_TEXTURE_2D, colorBufferID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);

   auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (err != GL_FRAMEBUFFER_COMPLETE) {
      fmt::print(stderr,"OepnGL Error: %d\n",err);
   }
}

void gl_framebuffer::blit(gl_framebuffer &dest) {
   glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id);
   glBlitFramebuffer(0,0,width,height,0,0,dest.width,dest.height,GL_COLOR_BUFFER_BIT,GL_LINEAR);
}

void gl_framebuffer::updatePixels( std::vector<unsigned int> &pixels, int window_width, int window_height ) {
   gl_framebuffer temp(window_width, window_height);
   temp.bind();
   glBindTexture(GL_TEXTURE_2D, temp.colorBufferID);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
                   0, 0,
                   window_width, window_height,
                   GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
   temp.blit( *this );
}

gl_framebuffer::~gl_framebuffer() {
   if (id)
      glDeleteFramebuffers(1, &id);
   if (depthBufferID)
      glDeleteRenderbuffers(1, &depthBufferID);
   if (colorBufferID)
      glDeleteTextures(1, &colorBufferID);
}

void gl_framebuffer::bind() {
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   glViewport(0, 0, width, height);
}
