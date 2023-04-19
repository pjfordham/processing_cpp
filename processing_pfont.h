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
      SDL_Surface* surface = TTF_RenderText_Blended(fontMap[*this], text.c_str(),
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

// #include <freetype2/ft2build.h>
// #include FT_FREETYPE_H

// FT_Library ft;
// FT_Face face;

// class PFont_FreeType {
// public:
//    static void init() {

//       // Initialize FreeType
//       FT_Error err = FT_Init_FreeType(&ft);
//       if (err != 0) {
//          printf("Failed to initialize FreeType\n");
//          exit(EXIT_FAILURE);
//       }
//       FT_Int major, minor, patch;
//       FT_Library_Version(ft, &major, &minor, &patch);
//       // printf("FreeType's version is %d.%d.%d\n", major, minor, patch);

//       // Load the TrueType font file
//       FT_New_Face(ft, "./SourceCodePro-Regular.ttf", 0, &face);
//       if (err != 0) {
//          printf("Failed to load face\n");
//          exit(EXIT_FAILURE);
//       }

//       // Set the character size
//       FT_Set_Char_Size(face, 0, 16*64, 300, 300);
//       if (err != 0) {
//          printf("Failed to set char size\n");
//          exit(EXIT_FAILURE);
//       }

//       // Load the glyph outline data
//       FT_Load_Char(face, 'A', FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
//       if (err != 0) {
//          printf("Failed to load glyph outlie data\n");
//          exit(EXIT_FAILURE);
//       }

//       std::vector<PVector> gvertices;
//       std::vector<char> gtags;

//       // Convert the glyph outline data into vertices
//       for (int i = 0; i < face->glyph->outline.n_points; i++) {
//          FT_Vector vec = face->glyph->outline.points[i];
//          gvertices.emplace_back(PVector{vec.x / 64.0f , vec.y / 64.0f} );
//          gtags.emplace_back(  face->glyph->outline.tags[i] );
//       }

//       // Render the vertices using OpenGL
//       // glEnableClientState(GL_VERTEX_ARRAY);
//       // glVertexPointer(2, GL_FLOAT, 0, &gvertices[0]);
//       // glDrawArrays(GL_TRIANGLES, 0, gvertices.size() / 2);
//       // glDisableClientState(GL_VERTEX_ARRAY);

//       return;
//    }

//    static void close() {
//       // Cleanup
//       FT_Done_Face(face);
//       FT_Done_FreeType(ft);
//       return;
//    };

// };

#endif
