#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include "processing_color.h"
#include "processing_texture_manager.h"

#include <vector>
#include <string_view>

class PImageImpl;

// Use the PIMPL idiom to completely encapsulate the implementation
// but also to force PImage to behave with Java style reference
// semantics, i.e. copy only does a shallow reference counted copy.

class PImage {
   std::shared_ptr<PImageImpl> impl;
public:

   static void init();

   static void close();

   operator bool() const;

   // int width() { return impl->width; }
   // int height() { return impl->height; }
   // unsigned int *pixels() { return impl->pixels; }
   int width = 0;
   int height = 0;
   unsigned int *pixels = nullptr;

   PImage() {}

   PImage( std::shared_ptr<PImageImpl> impl_ );

   void mask(const PImage m);

   PTexture getTexture(gl_context &glc);

   color get(int x, int y) const;

   void set(int x, int y, color c);

   void loadPixels() const;

   int pixels_length()const ;

   void updatePixels();

   void convolve (const std::vector<std::vector<float>> &kernel);

   void filter(int x, float level=1.0);

   void save_as( std::string_view filename ) const;

};

PImage createImage(int width, int height, int mode);
PImage loadImage(std::string_view URL);
PImage requestImage(std::string_view URL);

#endif
