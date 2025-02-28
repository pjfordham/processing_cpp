#include "glad/glad.h"

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <fmt/core.h>

#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_opengl_framebuffer.h"
#include "processing_debug.h"
#include "processing_opengl_shader.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

static const char *directVertexShader = R"glsl(
      #version 400
      in vec2 position;
      in vec2 texCoord;

      out vec2 vertTexCoord;

      void main() {
          gl_Position = vec4(position, -1.0, 1.0); // Directly use NDC
          vertTexCoord = texCoord;
      }
)glsl";

static const char *directFragmentShader = R"glsl(
      #version 400
      out vec4 fragColor;
      in vec2 vertTexCoord;
      uniform sampler2D texture1;

      void main() {
         fragColor = vec4(texture(texture1, vertTexCoord).xyz,1.0);
      }
)glsl";


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
      std::swap(did,x.did);
      std::swap(textureBufferID,x.textureBufferID);
      return *this;
   }

   framebuffer::framebuffer() noexcept {
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
      clear(0,0,0,1);

      if (aaMode == MSAA) {
         glGenFramebuffers(1, &did);
         glBindFramebuffer(GL_FRAMEBUFFER, did);

         glGenTextures(1, &textureBufferID);
         glBindTexture(GL_TEXTURE_2D, textureBufferID);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureBufferID, 0);

         auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if (err != GL_FRAMEBUFFER_COMPLETE) {
            fmt::print(stderr,"zFramebuffer not complete, OpenGL Error: {}\n",err);
            abort();
         }
         glClearColor(0, 0, 0, 1);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }
   }

   GLuint framebuffer::getColorBufferID() {
      if (aaMode == MSAA) {
         glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, did);
         glBlitFramebuffer(0,0,width,height,0,0,width,height,GL_COLOR_BUFFER_BIT,GL_LINEAR);
         glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
      if (did)
         glDeleteFramebuffers(1, &did);
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

   mainframe& mainframe::operator=(mainframe&&x) noexcept {
      DEBUG_METHOD();
      std::swap(width,x.width);
      std::swap(height,x.height);
      std::swap(direct,x.direct);
      std::swap(directVAO,x.directVAO);
      std::swap(directVBO,x.directVBO);
      return *this;
   }

   mainframe::mainframe() noexcept {
      DEBUG_METHOD();
   }

   mainframe::~mainframe() noexcept {
      DEBUG_METHOD();
      if (directVAO) {
         glBindVertexArray(directVAO);
         glBindBuffer(GL_ARRAY_BUFFER, 0);
         glBindVertexArray(0);
         glDeleteVertexArrays(1, &directVAO);
         glDeleteBuffers(1, &directVBO);
      }
   }

   mainframe::mainframe(mainframe &&x) noexcept : mainframe()  {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   mainframe::mainframe(int width_, int height_) : direct(directVertexShader, directFragmentShader)  {
      // Direct needs to be copy constrcted as move assignment doesn't seem to
      // work
      width = width_;
      height = height_;

      float quadVertices[] = {
         // Position (NDC)   // TexCoord (flipped vertically)
         -1.0f, -1.0f,      0.0f, 1.0f,  // Bottom-left
          1.0f, -1.0f,      1.0f, 1.0f,  // Bottom-right
          1.0f,  1.0f,      1.0f, 0.0f,  // Top-right

         -1.0f, -1.0f,      0.0f, 1.0f,  // Bottom-left
          1.0f,  1.0f,      1.0f, 0.0f,  // Top-right
         -1.0f,  1.0f,      0.0f, 0.0f   // Top-left
      };

      glGenVertexArrays(1, &directVAO);
      glGenBuffers(1, &directVBO);

      glBindVertexArray(directVAO);

      glBindBuffer(GL_ARRAY_BUFFER, directVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

      auto a_pos = direct.get_attribute("position");
      auto a_crd = direct.get_attribute("texCoord");
      a_pos.bind_vec2(4*sizeof(float), 0 );
      a_crd.bind_vec2(4*sizeof(float), (void*)(2*sizeof(float)));
      bind();
      glClearColor(1, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

   void mainframe::invert( framebuffer &src ) {
      bind();
      // Shouldn't need to do this
      clear(0.0,0.0,0.0,1.0);
      direct.bind();
      glBindVertexArray(directVAO);
      glBindBuffer(GL_ARRAY_BUFFER, directVBO);
      // Can probably remove this from fast path
      uniform texture1 = direct.get_uniform("texture1");
      texture1.set( 0 );
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_DEPTH_TEST);
      glBindTexture(GL_TEXTURE_2D, src.getColorBufferID());
      glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw fullscreen quad
      glEnable(GL_DEPTH_TEST);
      glBindVertexArray(0);
   }

   void mainframe::bind() {
      DEBUG_METHOD();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, width, height);
   }

   void mainframe::clear( float r, float g, float b, float a ) {
      DEBUG_METHOD();
      bind();
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

} // namespace gl
