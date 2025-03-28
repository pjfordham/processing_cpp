#include "glad/glad.h"

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <fmt/core.h>
#include <GLFW/glfw3.h>
#include <thread>

#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_opengl_framebuffer.h"
#include "processing_debug.h"
#include "processing_opengl_shader.h"
#include "processing_task_queue.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

static const char *directVertexShader = R"glsl(
      #version 400

      out vec2 vertTexCoord;

      void main() {
          // Define the hardcoded vertices and texture coordinates
          const vec4 vertices[6] = vec4[6](
              vec4(-1.0, -1.0, 0.0, 1.0),  // Bottom-left
              vec4( 1.0, -1.0, 1.0, 1.0),  // Bottom-right
              vec4( 1.0,  1.0, 1.0, 0.0),  // Top-right
              vec4(-1.0, -1.0, 0.0, 1.0),  // Bottom-left
              vec4( 1.0,  1.0, 1.0, 0.0),  // Top-right
              vec4(-1.0,  1.0, 0.0, 0.0)   // Top-left
          );

          // Hardcoded vertices are indexed, so use gl_VertexID to fetch
          vec4 vertex = vertices[gl_VertexID];

          // Set the position in NDC space
          gl_Position = vec4(vertex.xy, -1.0, 1.0);

          // Pass the texture coordinates
          vertTexCoord = vertex.zw;
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
      std::swap(colorBuffer,x.colorBuffer);
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

      renderThread.enqueue( [&] {

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
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      glActiveTexture(GL_TEXTURE0);

      glGenTextures(1, &colorBufferID);

      if (aaMode == MSAA) {
         glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBufferID);
         glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, aaFactor, GL_RGBA, width, height, GL_TRUE);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorBufferID, 0);
         glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
      } else {
         glBindTexture(GL_TEXTURE_2D, colorBufferID);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
         glBindTexture(GL_TEXTURE_2D, 0);
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
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureBufferID, 0);
         glBindTexture(GL_TEXTURE_2D, 0);

         auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if (err != GL_FRAMEBUFFER_COMPLETE) {
            fmt::print(stderr,"Framebuffer not complete, OpenGL Error: {}\n",err);
            abort();
         }
         glClearColor(0, 0, 0, 1);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         colorBuffer = std::make_shared<texture_t>(textureBufferID);
      } else {
         colorBuffer = std::make_shared<texture_t>(colorBufferID);
      }
      } );
      renderThread.wait_until_nothing_in_flight();
   }

   texture_ptr framebuffer::getColorBufferID() {
      if (aaMode == MSAA) {
         renderThread.enqueue([width=width, height=height, id=id, did=did] {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, did);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
         } );
      }
      return colorBuffer;
   }

   framebuffer::~framebuffer() {
      DEBUG_METHOD();

      if (id) {
         renderThread.enqueue([&] {
            if (id) {
               glBindFramebuffer(GL_FRAMEBUFFER, id);
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
               if (aaMode == MSAA) {
                  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
               } else {
                  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
               }
               glBindFramebuffer(GL_FRAMEBUFFER, 0);
               glDeleteFramebuffers(1, &id);
            }

            if (did) {
               glBindFramebuffer(GL_FRAMEBUFFER, did);
               glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
               glBindFramebuffer(GL_FRAMEBUFFER, 0);
               glDeleteFramebuffers(1, &did);
            }

            if (textureBufferID)
               glDeleteTextures(1, &textureBufferID);
            if (depthBufferID)
               glDeleteRenderbuffers(1, &depthBufferID);
            if (colorBufferID)
               glDeleteTextures(1, &colorBufferID);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
         });
         renderThread.wait_until_nothing_in_flight();
      }
   }

   void framebuffer::blit(framebuffer &dest) const {
      DEBUG_METHOD();
      if (id != dest.id) {
         renderThread.enqueue([width=width,height=height,id=id,dwidth=dest.width, dheight=dest.height, did=dest.id] {
          glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
          glBindFramebuffer(GL_DRAW_FRAMEBUFFER, did);
          glBlitFramebuffer(0, 0, width, height, 0, 0, dwidth, dheight,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR);
          glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
          glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        });
        }
   }

   void framebuffer::updatePixels( const std::vector<unsigned int> &pixels ) {
      DEBUG_METHOD();
      renderThread.enqueue( [&] {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, colorBufferID);
         glTexSubImage2D(GL_TEXTURE_2D, 0,
                         0, 0,
                         width, height,
                         GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
         glBindTexture(GL_TEXTURE_2D, 0);
      });
      renderThread.wait_until_nothing_in_flight();
   }

   void framebuffer::loadPixels( std::vector<unsigned int> &pixels ) {
      DEBUG_METHOD();
      renderThread.enqueue( [&] {
      pixels.resize(width*height);
      bind();
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      } );
      renderThread.wait_until_nothing_in_flight();
   }

   void framebuffer::saveFrame(void *surface) {
      DEBUG_METHOD();
      renderThread.enqueue( [&] {
         bind();
         glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, surface);
      } );
      renderThread.wait_until_nothing_in_flight();
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
      std::swap(texture1,x.texture1);
      std::swap(directVAO, x.directVAO);
      return *this;
   }

   mainframe::mainframe() noexcept {
      DEBUG_METHOD();
   }

   mainframe::~mainframe() noexcept {
      DEBUG_METHOD();
   }

   void mainframe::release_shader() noexcept {
      direct = {};
      renderThread.enqueue( [&] {
         if ( directVAO ) {
            glDeleteVertexArrays(1, &directVAO);
            directVAO = 0;
         } } );
      renderThread.wait_until_nothing_in_flight();
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
      renderThread.enqueue( [&] {
         texture1 = direct.get_uniform("texture1");
         glGenVertexArrays(1, &directVAO);
      } );
      renderThread.wait_until_nothing_in_flight();
   }

   void mainframe::invert( texture_ptr textureID ) {
      renderThread.enqueue( [directVAO=directVAO, width=width, height=height, textureID,
                             &direct = direct, &texture1 = texture1 ] {
         glBindVertexArray(directVAO);
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         glViewport(0, 0, width, height);
         glClearColor(0, 0, 0, 1);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         direct.bind();
         texture1.set( 15 );
         glActiveTexture(GL_TEXTURE0+15);
         glDisable(GL_DEPTH_TEST);
         glBindTexture(GL_TEXTURE_2D, textureID->get_id());
         glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw fullscreen quad
         glEnable(GL_DEPTH_TEST);
         glBindVertexArray(0);
      } );
   }

} // namespace gl
