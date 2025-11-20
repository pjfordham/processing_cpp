#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include "processing_properties.h"
#include "processing_opengl_texture.h"
#include "processing_color.h"

#include <vector>
#include <string_view>
#include <memory>
#include <filesystem>

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
   bool operator<(const PImage& x) {
      return impl < x.impl;
   }
   bool operator==(const PImage &x) const {
      return impl == x.impl;
   };
   bool operator!=(const PImage &x) const {
      return !(x==*this);
   };
   auto operator<=>(const PImage &other) const noexcept = default;

   //
   // Supply width, height and pixels as properties for compatability.
   //

   int &_width();
   int &_height();
   unsigned int *&_pixels();

   const int &_width() const;
   const int &_height() const;
   unsigned int *const &_pixels() const;

   static std::size_t _width_offset();
   static std::size_t _height_offset();
   static std::size_t _pixels_offset();

   [[no_unique_address]] property_t<PImage,int, &PImage::_width,  &PImage::_width,  &PImage::_width_offset>  width;
   [[no_unique_address]] property_t<PImage,int, &PImage::_height, &PImage::_height, &PImage::_height_offset> height;
   [[no_unique_address]] property_t<PImage,unsigned int*, &PImage::_pixels, &PImage::_pixels, &PImage::_pixels_offset> pixels;

   //
   // End of properties.
   //

   PImage() {}

   static PImage circle();

   PImage( std::shared_ptr<PImageImpl> impl_ );

   void wrapMode( int wrap );

   void mask(const PImage m);

   color get(int x, int y) const;

   gl::texture_t_ptr getTextureID() const;

   void set(int x, int y, color c);

   void loadPixels() const;

   int pixels_length()const ;

   void releaseTexture();

   void updatePixels();

   bool isDirty() const;

   void setClean();

   void convolve (const std::vector<std::vector<float>> &kernel);

   void filter(int x, float level=1.0);

   void save_as( std::string_view filename ) const;
};

PImage createImage(int width, int height, int mode);
PImage createImageFromTexture(gl::texture_t_ptr textureID);
PImage loadImage(std::string_view URL);
PImage _loadImage(const std::filesystem::path &path);
PImage requestImage(std::string_view URL);

#endif
