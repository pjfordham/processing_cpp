#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_math.h"
#include "processing_opengl.h"
#include "processing_color.h"

void ellipse(float x, float y, float width, float height);
void background(float gray);

class PGraphics {
public:
   GLuint bufferID;
   GLuint localFboID;

   color stroke_color;
   color fill_color;
   int gfx_width, gfx_height;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) {
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(fill_color, x.fill_color);
      std::swap(stroke_color, x.stroke_color);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x){
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(fill_color, x.fill_color);
      std::swap(stroke_color, x.stroke_color);
      return *this;
   }

   PGraphics() {
      localFboID = 0;
      bufferID = 0;
   }
   ~PGraphics() {
      if (localFboID)
         glDeleteFramebuffers(1, &localFboID);
      if (bufferID)
         glDeleteTextures(1, &bufferID);
   }
   PGraphics(int width, int height) {
      // Create a framebuffer object
      glGenFramebuffers(1, &localFboID);
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);

      gfx_width = width;
      gfx_height = height;

      // Create a texture to render to
      glGenTextures(1, &bufferID);
      glBindTexture(GL_TEXTURE_2D, bufferID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferID, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   }

   void background(float c) {
      extern GLuint fboID;
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      ::background(c);
      glBindFramebuffer(GL_FRAMEBUFFER, fboID);
  }
   void noFill() {
      fill_color = {0,0,0,0};
   }
   void stroke(float c) {
      stroke_color = {c,c,c,255};
   }
   void ellipse(float x, float y, float width, float height) {
      extern GLuint fboID;
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      ::ellipse(x,y,width,height);
      glBindFramebuffer(GL_FRAMEBUFFER, fboID);
   }
   void beginDraw() {}
   void endDraw() {}

   void draw(float x, float y) {
      glTexturedQuad( {x, y},
                      {x+gfx_width,y},
                      {x+gfx_width,y+gfx_height},
                      {x,y+gfx_height},
                      1.0,1.0, bufferID );
      return;
   }
};


PGraphics createGraphics(int width, int height) {
   return { width, height };
}

void image(PGraphics &gfx, int x, int y) {
   gfx.draw(x,y);
}


#endif
