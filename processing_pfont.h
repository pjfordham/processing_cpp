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
#include <filesystem>

#include "processing_color.h"


class PFont {
public:
   const char *name;
   int size;

   // Mappings from loaded font name and size to a TTF_Font*
   static std::map<std::pair<const char*,int>, TTF_Font *> fontMap;

   // Mappings from filename to full path for system fonts
   static std::map<std::string, std::string> fontFileMap;

   static void search_directory(const std::filesystem::path& path, const std::string& suffix) {
      for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
         if (!entry.is_directory() && entry.path().extension() == suffix) {
            fontFileMap[entry.path().filename().string()] = entry.path().string();
         }
      }
   }

   static std::vector<std::string> list() {
      std::filesystem::path path_to_search = "/usr/share/fonts/truetype";
      std::string suffix_to_find = ".ttf";
      search_directory(path_to_search, suffix_to_find);
      path_to_search = ".";
      search_directory(path_to_search, suffix_to_find);
      std::vector<std::string> keys;
      for (auto const& [key, value] : fontFileMap) {
         keys.push_back(key);
      }
      if (keys.size() == 0) {
         abort();
      };
      return keys;
   }

   static void init() {
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

   PFont() : name(nullptr), size(0) {}

   // We can safely copy and don't have to delete as the
   // map gets garbage collected at the end of the program
   PFont(const char *name_, int size_) : name(name_), size(size_) {
      if (PFont::fontFileMap.size() == 0) {
         PFont::list();
      }
      auto fontPath = fontFileMap[name].c_str();
      auto key = std::make_pair(name,size);
      if (PFont::fontMap.count(key) == 0) {
         auto font = TTF_OpenFont(fontPath, size);
         if (font == NULL) {
            printf("TTF_OpenFont failed: %s,%d %s\n", name, size, TTF_GetError());
            abort();
         }
         PFont::fontMap[key] = font;
      }
   }

   bool operator<(const PFont&a) const {
      if (name != a.name) {
         return name < a.name;
      } else {
         return  size < a.size;
      }
   }

   void render_text(GLuint textureID, int layer, std::string text, color color, float &width, float &height) {
      SDL_Surface* surface = TTF_RenderText_Blended(fontMap[std::make_pair(name,size)], text.c_str(),
                                                    { (unsigned char)color.r,
                                                      (unsigned char)color.g,
                                                      (unsigned char)color.b,
                                                      (unsigned char)color.a });
      if (surface == NULL) {
         printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
         surface = SDL_CreateRGBSurfaceWithFormat(0, 10, 10, 32, SDL_PIXELFORMAT_ABGR8888);
       }
      width = surface->w;
      height = surface->h;

      SDL_Surface* newSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
      if (newSurface == NULL) {
         abort();
      }

      glTextureSubImage3D(textureID, 0, 0, 0, layer, newSurface->w, newSurface->h, 1,
                          GL_RGBA, GL_UNSIGNED_BYTE, newSurface->pixels);

      SDL_FreeSurface(surface);
      SDL_FreeSurface(newSurface);
   }

};

std::map<std::string, std::string> PFont::fontFileMap;
std::map<std::pair<const char*,int>, TTF_Font *> PFont::fontMap;

PFont createFont(const char *name, int size)  {
  return {name, size};
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
