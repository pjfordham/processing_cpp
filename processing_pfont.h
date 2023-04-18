#ifndef PROCESSING_PFONT_H
#define PROCESSING_PFONT_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>

#include "processing_color.h"


class PFont {
public:
   static std::map<PFont, TTF_Font *> fontMap;

   static PFont currentFont;

   static void init() {
      TTF_Font* font = NULL;
      if (TTF_Init() != 0) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init failed: %s\n", TTF_GetError());
         abort();
      }
   }

   static void close() {
      for (auto font : fontMap) {
         TTF_CloseFont(font.second);
      }
   }

   bool operator<(const PFont&a) const {
      if (name != a.name) {
         return name < a.name;
      } else {
         return  size < a.size;
      }
   }

   const char *name;
   int size;

   TTF_Font *open() {
      auto font = TTF_OpenFont(name, size);
      if (font == NULL) {
         printf("TTF_OpenFont failed: %s\n", TTF_GetError());
         abort();
      }
      return font;
   }

   PFont() : name(nullptr), size(0) {}

   PFont(const char *name_, int size_) {
      name = name_;
      size = size_;
   }

   GLuint render_text(std::string text, color color, float &width, float &height) {
      SDL_Surface* surface = TTF_RenderText_Blended(fontMap[currentFont], text.c_str(),
                                                    { (unsigned char)color.r,
                                                      (unsigned char)color.g,
                                                      (unsigned char)color.b,
                                                      (unsigned char)color.a });
      if (surface == NULL) {
         printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
         abort();
      }
      width = surface->w;
      height = surface->h;

      SDL_Surface* newSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
      if (newSurface == NULL) {
         abort();
      }

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

      SDL_FreeSurface(surface);
      SDL_FreeSurface(newSurface);
      return textureID;
   }

};

std::map<PFont, TTF_Font *> PFont::fontMap;

PFont createFont(const char *filename, int size)  {
   PFont key(filename,size);
   if (PFont::fontMap.count(key) == 0) {
      PFont font(filename,size);
      PFont::fontMap[key] = font.open();
   }
   return key;
}


#endif
