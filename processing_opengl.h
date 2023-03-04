#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "processing_math.h"

bool anything_drawn = false;

std::vector<Matrix2D> matrix_stack;
Matrix2D current_matrix = Matrix2D::Identity();

void glLoadMatrix() {
   PVector t = current_matrix.get_translation();
   PVector s = current_matrix.get_scale();
   float r = current_matrix.get_angle();

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(t.x, t.y, 0);
   glScalef(s.x, s.y, 0);
   glRotatef(r*(180.0/PI),0, 0,1);
}


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

   glLoadMatrix();

   glEnable(GL_TEXTURE_2D);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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

   glBegin(GL_QUADS);
   glColor4f(1,1,1,1);
   glTexCoord2f(0.0f, 0.0f);
   glVertex2f(p0.x,p0.y);
   glTexCoord2f(xrange, 0.0f);
   glVertex2f(p1.x,p1.y);
   glTexCoord2f(xrange, yrange);
   glVertex2f(p2.x,p2.y);
   glTexCoord2f(0.0f, yrange);
   glVertex2f(p3.x,p3.y);
   glEnd();

   // Unbind the texture and free handle
   glBindTexture(GL_TEXTURE_2D, 0);
   glDeleteTextures(1, &textureID);

   glDisable(GL_TEXTURE_2D);
   SDL_FreeSurface(newSurface);
}

void glFilledPoly(int points, PVector *p, SDL_Color color) {
   anything_drawn = true;
   glLoadMatrix();
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor4f(color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);
   glBegin(GL_TRIANGLE_FAN);
   for (int i =0; i<points;++i) {
      glVertex2f(p[i].x, p[i].y);
   };
   glEnd();
}

void glLine(PVector p1, PVector p2, SDL_Color color, int weight) {

   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   PVector p[] = {p1,p1,p2,p2};
   p[0].add(normal);
   p[1].sub(normal);
   p[2].sub(normal);
   p[3].add(normal);

   glFilledPoly(4, p, color);

}

void glLines(int points, PVector *p, SDL_Color color, int weight) {
   for (int i =1; i<points;++i) {
      glLine(p[i-1], p[i], color, weight);
   }
}

void glLinePoly(int points, PVector *p, SDL_Color color, int weight) {
   glLines(points, p, color, weight);
   glLine(p[points-1], p[0], color, weight);
}

void glFilledEllipse( PVector center, float xradius, float yradius, SDL_Color color ) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   for(float i = 0; i < 2 * M_PI; i += 2 * M_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   glFilledPoly(vertexBuffer.size(), vertexBuffer.data(), color );
}

void glLineEllipse( PVector center, float xradius, float yradius, SDL_Color color, int weight) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   for(float i = 0; i < 2 * M_PI; i += 2 * M_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}

#endif
