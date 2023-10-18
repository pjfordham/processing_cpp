#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include "processing_color.h"

#include <vector>
#include <string_view>
#include <memory>

class PImageImpl;
typedef unsigned int GLuint;

// Use the PIMPL idiom to completely encapsulate the implementation
// but also to force PImage to behave with Java style reference
// semantics, i.e. copy only does a shallow reference counted copy.

class PImage {
   std::shared_ptr<PImageImpl> impl;
public:

   static void init();

   static void close();

   explicit operator bool() const;
   bool operator==(const PImage &x) const {
      return impl == x.impl && width == x.width && height == x.height && pixels == x.pixels;
   };
   bool operator!=(const PImage &x) const {
      return !(x==*this);
   };
   // int width() { return impl->width; }
   // int height() { return impl->height; }
   // unsigned int *pixels() { return impl->pixels; }
   int width = 0;
   int height = 0;
   unsigned int *pixels = nullptr;

   PImage() {}

   static PImage circle() {
      PImage a;
      a.width = -1;
      a.height = -1;
      return a;
   }

   PImage( std::shared_ptr<PImageImpl> impl_ );

   void mask(const PImage m);

   color get(int x, int y) const;

   GLuint getTextureID() const;

   void set(int x, int y, color c);

   void loadPixels() const;

   int pixels_length()const ;

   void updatePixels();

   bool isDirty() const;

   void setClean();

   void convolve (const std::vector<std::vector<float>> &kernel);

   void filter(int x, float level=1.0);

   void save_as( std::string_view filename ) const;
};

PImage createImage(int width, int height, int mode);
PImage createBlankImage();
PImage createImageFromTexture(GLuint textureID);
PImage loadImage(std::string_view URL);
PImage requestImage(std::string_view URL);

#endif
