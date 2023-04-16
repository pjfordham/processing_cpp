#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "processing_math.h"

bool anything_drawn = false;


unsigned int next_power_of_2(unsigned int v)
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;
   return v;
}

extern GLuint programID;
extern Eigen::Matrix4f move_matrix; // Default is identity
extern Eigen::Matrix4f projection_matrix; // Default is identity
extern Eigen::Matrix4f view_matrix; // Default is identity

void printMatrix4f(const Eigen::Matrix4f& mat) {
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
}

void glTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, float xrange, float yrange, GLuint textureID, bool flip=false) {
   GLuint uSampler = glGetUniformLocation(programID, "uSampler");

   extern GLuint Color;
   float color_vec[] = { 0,0,0,0 };
   glUniform4fv(Color, 1, color_vec);

   int textureUnitIndex = 0;
   glBindTexture(GL_TEXTURE_2D, textureID);
   glUniform1i(uSampler,0);
   glActiveTexture(GL_TEXTURE0 + textureUnitIndex);


   Eigen::Vector4f vert0 = Eigen::Vector4f{p0.x,p0.y,0,1};
   Eigen::Vector4f vert1 = Eigen::Vector4f{p1.x,p1.y,0,1};
   Eigen::Vector4f vert2 = Eigen::Vector4f{p2.x,p2.y,0,1};
   Eigen::Vector4f vert3 = Eigen::Vector4f{p3.x,p3.y,0,1};

   std::vector<float> vertices{
      vert0[0],  vert0[1], 0.0f,
      vert1[0],  vert1[1], 0.0f,
      vert2[0],  vert2[1], 0.0f,
      vert3[0],  vert3[1], 0.0f,
   };

   std::vector<float> coords;
   if (flip) {
      coords = {
         0.0f, yrange,
         xrange, yrange,
         xrange, 0.0f,
         0.0f, 0.0f,
      };
   } else {
      coords = {
         0.0f, 0.0f,
         xrange, 0.0f,
         xrange, yrange,
         0.0f, yrange,
      };
   }

   std::vector<unsigned short>  indices = {
      0,1,2, 0,2,3,
   };

   GLuint localVAO;
   // Create a vertex array object (VAO)
   glGenVertexArrays(1, &localVAO);
   glBindVertexArray(localVAO);

   GLuint indexbuffer;
   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   GLuint coordsbuffer;
   glGenBuffers(1, &coordsbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, coordsbuffer);
   glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(float), coords.data(), GL_STATIC_DRAW);


   GLuint attribId = glGetAttribLocation(programID, "position");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(programID, "coords");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, coordsbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      2,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glBindVertexArray(localVAO);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
   glBindVertexArray(0);

   glDeleteVertexArrays(1, &localVAO);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);
   glDeleteBuffers(1, &coordsbuffer);

   // Unbind the buffer objects and VAO
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   glBindTexture(GL_TEXTURE_2D, 0);
}

void glTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, SDL_Surface *surface) {
   anything_drawn = true;

   int newWidth = next_power_of_2(surface->w);
   int newHeight = next_power_of_2(surface->h);

   SDL_Surface* newSurface = SDL_CreateRGBSurface(surface->flags, newWidth, newHeight,
                                                  surface->format->BitsPerPixel,
                                                  surface->format->Rmask,
                                                  surface->format->Gmask,
                                                  surface->format->Bmask,
                                                  surface->format->Amask);
   if (newSurface == NULL) {
      abort();
   }

   // clear new surface with a transparent color and blit existing surface to it
   SDL_FillRect(newSurface, NULL, SDL_MapRGBA(newSurface->format, 0, 0, 0, 0));
   SDL_BlitSurface(surface, NULL, newSurface, NULL);

   // Calculate extends of texture to use
   float xrange = (1.0 * surface->w) / newWidth;
   float yrange = (1.0 * surface->h) / newHeight;

   // Create an OpenGL texture from the SDL_Surface
   GLuint textureID;
   glGenTextures(1, &textureID);
   glBindTexture(GL_TEXTURE_2D, textureID);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newSurface->w, newSurface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, newSurface->pixels);

   glBindTexture(GL_TEXTURE_2D, 0);
   glTexturedQuad(p0,p1,p2,p3,xrange, yrange, textureID);

   glDeleteTextures(1, &textureID);
   SDL_FreeSurface(newSurface);


}


#endif
