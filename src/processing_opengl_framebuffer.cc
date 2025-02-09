#include "glad/glad.h"

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <fmt/core.h>

#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_opengl_framebuffer.h"
#include "processing_debug.h"
#include "processing_pshader.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

namespace gl {

   framebuffer& framebuffer::operator=(framebuffer&&x) noexcept {
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

   framebuffer::framebuffer() {
      DEBUG_METHOD();
   }

   framebuffer::framebuffer(framebuffer &&x) noexcept : framebuffer() {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   framebuffer::framebuffer(int width_, int height_, int aaMode_, int aaFactor_)  {
      DEBUG_METHOD();

      aaFactor = aaFactor_;
      aaMode = aaMode_;

      if (aaFactor == 1)
         aaMode = SSAA;

      if (aaMode == SSAA) {
         width = aaFactor * width_;
         height = aaFactor * height_;
      } else if (aaMode == MSAA) {
         width = width_;
         height = height_;
      } else {
         abort();
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

      glActiveTexture(GL_TEXTURE0);

      glGenTextures(1, &colorBufferID);

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
         abort();
      }
   }

   GLuint framebuffer::getColorBufferID() {
      if (aaMode == MSAA) {
         if (textureBufferID)
            glDeleteTextures(1, &textureBufferID);
         framebuffer frame(width, height, SSAA, 1);
         blit( frame );
         textureBufferID = frame.colorBufferID;
         frame.colorBufferID = 0;
         return textureBufferID;
      } else {
         return colorBufferID;
      }
   }

   framebuffer::~framebuffer() {
      DEBUG_METHOD();
      if (textureBufferID)
         glDeleteTextures(1, &textureBufferID);
      if (depthBufferID)
         glDeleteRenderbuffers(1, &depthBufferID);
      if (colorBufferID)
         glDeleteTextures(1, &colorBufferID);
      if (id)
         glDeleteFramebuffers(1, &id);
   }

   void framebuffer::blit(framebuffer &dest) const {
      DEBUG_METHOD();
      if (id != dest.id) {
         glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id);
         glBlitFramebuffer(0,0,width,height,0,0,dest.width,dest.height,GL_COLOR_BUFFER_BIT,GL_LINEAR);
         glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      }
   }

   void framebuffer::updatePixels( const std::vector<unsigned int> &pixels ) {
      DEBUG_METHOD();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorBufferID);
      glTexSubImage2D(GL_TEXTURE_2D, 0,
                      0, 0,
                      width, height,
                      GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   void framebuffer::loadPixels( std::vector<unsigned int> &pixels ) {
      DEBUG_METHOD();
      pixels.resize(width*height);
      bind();
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
   }

   void framebuffer::saveFrame(void *surface) {
      DEBUG_METHOD();
      bind();
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, surface);
   }

   void framebuffer::invert( framebuffer &src ) {
      DEBUG_METHOD();

      bind();
      clear(0.0,0.0,0.0,1.0);

      PShader direct = directShader();
      GLuint shaderID = direct.getShader().programID;
      glUseProgram(shaderID);

      static float quadVertices[] = {
         // Position (NDC)   // TexCoord (flipped vertically)
         -1.0f, -1.0f,      0.0f, 1.0f,  // Bottom-left
          1.0f, -1.0f,      1.0f, 1.0f,  // Bottom-right
          1.0f,  1.0f,      1.0f, 0.0f,  // Top-right

         -1.0f, -1.0f,      0.0f, 1.0f,  // Bottom-left
          1.0f,  1.0f,      1.0f, 0.0f,  // Top-right
         -1.0f,  1.0f,      0.0f, 0.0f   // Top-left
      };
      GLuint VAO, VBO;
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);

      glBindVertexArray(VAO);

      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

      auto lpos = glGetAttribLocation(shaderID, "position");
      auto lcrd = glGetAttribLocation(shaderID, "texCoord" );
      auto ltex = glGetUniformLocation(shaderID, "texture1" );

      glEnableVertexAttribArray(lpos);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

      glEnableVertexAttribArray(lcrd);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

      glBindVertexArray(0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, src.getColorBufferID());
      glUniform1i(ltex, 0);

      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw fullscreen quad
      glBindVertexArray(0);
   }

   void framebuffer::bind() {
      DEBUG_METHOD();
      glBindFramebuffer(GL_FRAMEBUFFER, id);
      glViewport(0, 0, width, height);
   }

   void framebuffer::clear( float r, float g, float b, float a ) {
      DEBUG_METHOD();
      bind();
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void framebuffer::clearDepthBuffer() {
      DEBUG_METHOD();
      bind();
      glClear(GL_DEPTH_BUFFER_BIT);
   }

} // namespace gl
