#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include "processing_color.h"
#include "processing_enum.h"

#include <vector>
#include <string>

struct SDL_Surface;

class PImage {
public:
   int width;
   int height;
   unsigned int *pixels;
   SDL_Surface *surface;

   operator bool() const {
      return surface;
   }

   static void init();

   static void close();

   PImage() : width(0), height(0), pixels{NULL}, surface{NULL} {}

   ~PImage();

   PImage(const PImage &x);

   PImage(PImage &&x) noexcept : PImage() {
      *this = std::move(x);
   }

   PImage& operator=(const PImage&) = delete;
   PImage& operator=(PImage&&x) noexcept {
      std::swap(surface, x.surface);
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(pixels, x.pixels);
      return *this;
   }

   void mask(const PImage &mask) {
      if ( width != mask.width || height != mask.height )
         abort();
      for(int i = 0; i < (width * height); ++i) {
         unsigned int p = pixels[i];
         unsigned int q = mask.pixels[i];
         pixels[i] = (p & 0x00FFFFFF) | ( (0xFF - ( q & 0xFF)) << 24 );
      }
   }

   color get(int x, int y) const {
      if ( 0 <= x && x < width && 0 <= y && y < height)
         return { pixels[y * width + x], false };
      else
         return BLACK;
   }

   void set(int x, int y, color c) {
      if ( 0 <= x && x < width && 0 <= y && y < height)
         pixels[y * width + x] = c;
   }

   PImage(int w, int h, int mode);

   PImage(SDL_Surface *surface_);

   void loadPixels();

   int pixels_length() const {
      return width * height;
   }

   void updatePixels() {
   }

   void convolve (const std::vector<std::vector<float>> &kernel);

   void filter(int x, float level=1.0) {
      switch (x) {
      case GRAY:
         for(int i = 0; i< width * height; ++i) {
            unsigned int p = pixels[i];
            unsigned int x = (red(p) + green(p) + blue(p)) / 3;
            pixels[i] = color( x,x,x, alpha(p) );
         }
         break;
      case THRESHOLD:
         for(int i = 0; i< width * height; ++i) {
            unsigned int p = pixels[i];
            unsigned int x = (red(p) + green(p) + blue(p)) / 3;
            if ( x > level )
               pixels[i] = color( WHITE, alpha(p) );
            else
               pixels[i] = color( BLACK, alpha(p) );
         }
         break;
      case BLUR:
         convolve( { { 1, 2, 1},
                     { 2, 4, 2},
                     { 1, 2, 1}} );
         break;
      case OPAQUE:
         for(int i = 0; i< width * height; ++i) {
            unsigned int p = pixels[i];
            pixels[i] = color( red(p), green(p), blue(p), 255);
         }
         break;
      case INVERT:
         for(int i = 0; i< width * height; ++i) {
            unsigned int p = pixels[i];
            pixels[i] = color( 255-red(p), 255-green(p), 255-blue(p), alpha(p));;
         }
         break;
      default:
         abort();
      }
   }

};

inline PImage createImage(int width, int height, int mode) {
   return {width,height,mode};
}

inline size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::vector<unsigned char>*>(userdata);
    response->insert(response->end(), ptr, ptr + realsize);
    return realsize;
}

PImage loadImage(const char *URL);

inline PImage requestImage(const std::string &URL) {
   // This should be an async load
   return loadImage(URL.c_str());
}

#endif
