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

   glGenRenderbuffers(1, &colorBufferID);
   glBindRenderbuffer(GL_RENDERBUFFER, colorBufferID);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, this->width, this->height);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferID);

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

gl_framebuffer::~gl_framebuffer() {
   if (id)
      glDeleteFramebuffers(1, &id);
   if (depthBufferID)
      glDeleteRenderbuffers(1, &depthBufferID);
   if (colorBufferID)
      glDeleteRenderbuffers(1, &colorBufferID);
}

void gl_framebuffer::bind() {
   // Bind the framebuffer and get its dimensions
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   glViewport(0, 0, width, height);
}
