#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Just for enum
#include "processing_pshape.h"
#include "processing_color.h"

#include <vector>

enum {
  THRESHOLD,
  GRAY,
  OPAQUE,
  INVERT,
  POSTERIZE,
  BLUR,
  ERODE,
  DILATE,
};

enum { /*RGB=0,*/ ARGB = 1,  ALPHA=2 };

class PImage {
public:
   int width;
   int height;
   unsigned int *pixels;
   SDL_Surface *surface;

   PImage() : width(0), height(0), pixels{NULL}, surface{NULL} {}

   ~PImage() {
      if (surface) {
         SDL_FreeSurface(surface);
      }
   }

   PImage(const PImage &x){
      width = x.width;
      height = x.height;
      surface = SDL_ConvertSurfaceFormat(x.surface, SDL_PIXELFORMAT_ABGR8888, 0);
      pixels = (Uint32 *)surface->pixels;
   }

   PImage(PImage &&x) {
      std::swap(surface, x.surface);
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(pixels, x.pixels);
   }

   PImage& operator=(const PImage&) = delete;
   PImage& operator=(PImage&&x){
      std::swap(surface, x.surface);
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(pixels, x.pixels);
      return *this;
   }

   void mask(const PImage &mask) {
      for(int i = 0; i< width * height; ++i) {
         Uint32 p = pixels[i];
         Uint32 q = mask.pixels[i];
         pixels[i] = (p & 0x00FFFFFF) | ( (0xFF - ( q & 0xFF)) << 24 );
      }
   }

   color get(int x, int y) {
      return { pixels[y * width + x], false };
   }

   PImage(const char *filename) {
      SDL_Surface *loaded = IMG_Load(filename);
      if (loaded == NULL) {
         abort();
      }
      surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ABGR8888, 0);
      if (surface == NULL) {
         abort();
      }
      SDL_FreeSurface(loaded);
      width = surface->w;
      height = surface->h;
      pixels =  (Uint32*)surface->pixels;
   }

   PImage(int w, int h, int mode) : width(w), height(h){
      surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
      if (surface == NULL) {
         abort();
      }
      // Clear the surface to black
      SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
      pixels =  (Uint32*)surface->pixels;
   }

   void loadPixels() {
      pixels =  (Uint32*)surface->pixels;
   }

   int pixels_length() {
      return width * height;
   }

   void updatePixels() {
   }

   void filter(int x) {
      if (x == GRAY) {
         for(int i = 0; i< width * height; ++i) {
            Uint32 p = pixels[i];
            Uint32 x = (red(p) + green(p) + blue(p)) / 3;
            Uint32 y =  ((Uint32)alpha(p) << 24) | (x << 16) | (x << 8) | x;
            pixels[i] = y;
         }
      }
   }
};

PImage loadImage(const char *filename) {
   return {filename};
}

PImage createImage(int width, int height, int mode) {
   return {width,height,mode};
}

#endif
