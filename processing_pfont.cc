#include "processing_pfont.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <filesystem>
#include <map>

std::map<std::string, std::string> fontFileMap;
std::map<std::pair<const char*,int>, TTF_Font *> fontMap;

bool operator<(const PFont&a, const PFont &b) {
   if (a.name != b.name) {
      return a.name < b.name;
   } else {
      return a.size < b.size;
   }
}

void PFont::init() {
   if (TTF_Init() != 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init failed: %s\n", TTF_GetError());
      abort();
   }
}

void PFont::close() {
   for (auto font : fontMap) {
      TTF_CloseFont(font.second);
   }
}

void search_directory(const std::string& path, const std::string& suffix) {
   for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
      if (!entry.is_directory() && entry.path().extension() == suffix) {
         fontFileMap[entry.path().filename().string()] = entry.path().string();
      }
   }
}

std::vector<std::string>  PFont::list() {
   std::filesystem::path path_to_search = "/usr/share/fonts/truetype";
   std::string suffix_to_find = ".ttf";
   search_directory(path_to_search, suffix_to_find);
   path_to_search = "data";
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


// We can safely copy and don't have to delete as the
// map gets garbage collected at the end of the program
PFont::PFont(const char *name_, int size_) : name(name_), size(size_) {
   if (fontFileMap.size() == 0) {
      PFont::list();
   }
   auto fontPath = fontFileMap[name].c_str();
   auto key = std::make_pair(name,size);
   if (fontMap.count(key) == 0) {
      auto font = TTF_OpenFont(fontPath, size);
      if (font == NULL) {
         printf("TTF_OpenFont failed: %s,%d %s\n", name, size, TTF_GetError());
         abort();
      }
      fontMap[key] = font;
   }
}

SDL_Surface *PFont::render_text(const std::string &text) {
   SDL_Surface* surface = TTF_RenderText_Blended(fontMap[std::make_pair(name,size)], text.c_str(),
                                                 { 255, 255, 255, 255 });
   if (surface == NULL) {
      printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
      surface = SDL_CreateRGBSurfaceWithFormat(0, 10, 10, 32, SDL_PIXELFORMAT_ABGR8888);
   }
   SDL_Surface* newSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
   if (newSurface == NULL) {
      abort();
   }

   SDL_FreeSurface(surface);

   return newSurface;
}
