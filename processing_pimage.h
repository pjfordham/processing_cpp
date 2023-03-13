#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

float red(unsigned int pixel) {
   return (pixel >>  0) & 0xFF;
}

float green(unsigned int pixel) {
   return (pixel >>  8) & 0xFF;
}

float blue(unsigned int pixel) {
   return (pixel >> 16) & 0xFF;
}

float alpha(unsigned int pixel) {
   return (pixel >> 24) & 0xFF;
}

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
      surface = SDL_ConvertSurfaceFormat(x.surface, SDL_PIXELFORMAT_ARGB8888, 0);
      pixels = (Uint32 *)surface->pixels;
   }

   PImage(PImage &&other) = delete;
   constexpr PImage& operator=(const PImage&) = delete;
   constexpr PImage& operator=(PImage&&x){
      if (surface) {
         SDL_FreeSurface(surface);
      }
      width = x.width;
      height = x.height;
      pixels = x.pixels;
      surface = x.surface;
      x.width = 0;
      x.height = 0;
      x.pixels = NULL;
      x.surface = NULL;
      return *this;
   }

   PImage(const char *filename) {
      SDL_Surface *loaded = IMG_Load(filename);
      if (loaded == NULL) {
         abort();
      }
      surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
      if (surface == NULL) {
         abort();
      }
      SDL_FreeSurface(loaded);
      width = surface->w;
      height = surface->h;
      pixels =  (Uint32*)surface->pixels;
   }

   PImage(int w, int h, int mode) : width(w), height(h){
      surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
      if (surface == NULL) {
         abort();
      }
      // Clear the surface to black
      SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
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
