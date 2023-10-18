#include "glad/glad.h"

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <fmt/core.h>

#include "processing_enum.h"
#include "processing_opengl_framebuffer.h"

gl_framebuffer::gl_framebuffer(int width_, int height_, int aaFactor_, int aaMode_)  {
   aaFactor = aaFactor_;
   aaMode = aaMode_;

   if (aaMode == SSAA) {
      width = aaFactor * width_;
      height = aaFactor * height_;
   } else {
      width = width_;
      height = height_;
   }

   glGenFramebuffers(1, &id);
   bind();

   if (aaMode == MSAA && !GL_EXT_framebuffer_multisample) {
      // Multisample extension is not supported
      abort();
   }

   glGenRenderbuffers(1, &depthBufferID);
   glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
   if (aaMode == MSAA) {
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, aaFactor, GL_DEPTH_COMPONENT, width, height);
   } else {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
   }

   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

   glGenTextures(1, &colorBufferID);

   glActiveTexture(GL_TEXTURE0);
   if (aaMode == MSAA) {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBufferID);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, aaFactor, GL_RGBA, width, height, GL_TRUE);
      glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorBufferID, 0);
   } else {
      glBindTexture(GL_TEXTURE_2D, colorBufferID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
   }

   auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (err != GL_FRAMEBUFFER_COMPLETE) {
      fmt::print(stderr,"Framebuffer not complete, OpenGL Error: {}\n",err);
   }
}

GLuint gl_framebuffer::getColorBufferID() const {
   if (aaMode == MSAA) {
      if (textureBufferID)
         glDeleteTextures(1, &textureBufferID);
      gl_framebuffer frame(width, height, 1, SSAA);
      blit( frame );
      GLuint textureBufferID = frame.colorBufferID;
      frame.colorBufferID = 0;
      return textureBufferID;
   } else {
      return colorBufferID;
   }
}

gl_framebuffer::~gl_framebuffer() {
   if (depthBufferID)
      glDeleteRenderbuffers(1, &depthBufferID);
   if (colorBufferID)
      glDeleteTextures(1, &colorBufferID);
   if (textureBufferID)
      glDeleteTextures(1, &textureBufferID);
   if (id)
      glDeleteFramebuffers(1, &id);
}

void gl_framebuffer::blit(gl_framebuffer &dest) const {
   glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id);
   glBlitFramebuffer(0,0,width,height,0,0,dest.width,dest.height,GL_COLOR_BUFFER_BIT,GL_LINEAR);
}

void gl_framebuffer::updatePixels( const std::vector<unsigned int> &pixels ) {
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, colorBufferID);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
                   0, 0,
                   width, height,
                   GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void gl_framebuffer::loadPixels( std::vector<unsigned int> &pixels ) {
   pixels.resize(width*height);
   bind();
   glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void gl_framebuffer::saveFrame(void *surface) {
   bind();
   glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, surface);
}

void gl_framebuffer::bind() {
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   glViewport(0, 0, width, height);
}
