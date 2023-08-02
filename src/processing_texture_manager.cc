#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_texture_manager.h"

TextureManager::TextureManager( int w, int h ) : width(w), height(h) {
   if (width == 0 || height == 0)
      return;
   // create the texture array
   glGenTextures(1, &textureID);
   glBindTexture(GL_TEXTURE_2D, textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

   // set texture parameters
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
   GLubyte white[4] = { 255, 255, 255, 255 };
   glClearTexImage(textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
   clear();
};

TextureManager::~TextureManager() {
   if (textureID)
      glDeleteTextures(1, &textureID);
}
