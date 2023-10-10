#include "processing_pimage.h"
#include <sail-c++/sail-c++.h>
#include <curl/curl.h>

class PImageImpl {
public:
   int width;
   int height;
   unsigned int *pixels;
   PTexture texture;

   operator bool() const {
      return pixels;
   }

   PImageImpl() : width(0), height(0), pixels{NULL} {}

   ~PImageImpl();

   PImageImpl(const PImageImpl &x);

   PImageImpl(PImageImpl &&x) noexcept : PImageImpl() {
      *this = std::move(x);
   }

   PImageImpl(int w, int h, uint32_t *pixels_);

   PImageImpl& operator=(const PImageImpl&) = delete;
   PImageImpl& operator=(PImageImpl&&x) noexcept {
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(pixels, x.pixels);
      std::swap(texture, x.texture);
      return *this;
   }

   PTexture getTexture(gl_context &glc) {
      if (!texture.isValid()) {
          texture = glc.getTexture( width, height, pixels );
      }
      return texture;
   }

   void mask(const PImageImpl &mask) {
      if ( width != mask.width || height != mask.height )
         abort();
      for(int i = 0; i < (width * height); ++i) {
         unsigned int p = pixels[i];
         unsigned int q = mask.pixels[i];
         pixels[i] = (p & 0x00FFFFFF) | ( (0xFF - ( q & 0xFF)) << 24 );
      }
      texture = {};
   }

   color get(int x, int y) const {
      if ( 0 <= x && x < width && 0 <= y && y < height)
         return pixels[y * width + x];
      else
         return BLACK;
   }

   void set(int x, int y, color c) {
      if ( 0 <= x && x < width && 0 <= y && y < height)
         pixels[y * width + x] = c;
      texture = {};
   }

   PImageImpl(int w, int h, int mode);

   void loadPixels() const;

   int pixels_length() const {
      return width * height;
   }

   void updatePixels() {
      texture = {};
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
      texture = {};
   }

   void save_as( const std::string &filename ) const;
};

PImage::PImage( std::shared_ptr<PImageImpl> impl_ ) {
   impl = impl_;
   width = impl_->width;
   height = impl_->height;
   pixels = impl_->pixels;
}

PImage::operator bool() const {
   return (bool)*impl;
}

void PImage::mask(const PImage m) {
   impl->mask(*(m.impl));
}

PTexture PImage::getTexture(gl_context &glc) {
   return impl->getTexture(glc);
}

color PImage::get(int x, int y) const {
   return impl->get(x,y);
}

void PImage::set(int x, int y, color c) {
   impl->set(x,y,c);
}

void PImage::loadPixels() const {
   impl->loadPixels();
}

int PImage::pixels_length() const {
   return impl->pixels_length();
}

void PImage::updatePixels() {
   impl->updatePixels();
}

void PImage::convolve (const std::vector<std::vector<float>> &kernel) {
   impl->convolve(kernel);
}

void PImage::filter(int x, float level) {
   impl->filter(x,level);
}

void PImage::save_as( std::string_view filename ) const {
   impl->save_as(std::string(filename));
}

void PImage::init() {
   sail::log::set_barrier( SAIL_LOG_LEVEL_WARNING );
   // Initialize libcurl
   curl_global_init(CURL_GLOBAL_ALL);
}

void PImage::close() {
   curl_global_cleanup();
}

PImageImpl::~PImageImpl() {
   if (pixels) {
      delete [] pixels;
   }
}

PImageImpl::PImageImpl(const PImageImpl &x) : PImageImpl(x.width, x.height, x.pixels) {
}

PImageImpl::PImageImpl(int w, int h, int mode) : width(w), height(h){
   pixels = new uint32_t[width*height];
   std::fill(pixels, pixels+width*height, color(BLACK));
}

PImageImpl::PImageImpl(int w, int h, uint32_t *pixels_) : width(w), height(h){
   pixels = new uint32_t[width*height];
   std::copy(pixels_, pixels_+width*height, pixels);
}

void PImageImpl::loadPixels() const {
}

void PImageImpl::convolve(const std::vector<std::vector<float>> &kernel) {
   // Create a new image of the same size as the original surface
   auto blurred = new uint32_t[width*height];

   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         float r = 0, g = 0, b = 0, a = 0;
         float weight_sum = 0;

         for (int j = 0; j < kernel.size(); j++) {
            for (int i = 0; i < kernel[0].size(); i++) {
               int neighbor_x = x + i - ( (kernel.size() - 1 ) / 2 );
               int neighbor_y = y + j - ( (kernel[i].size() - 1 ) / 2 );;
               if (neighbor_x >= 0 && neighbor_x < width && neighbor_y >= 0 && neighbor_y < height) {
                  uint32_t neighbor_pixel = pixels[neighbor_y * width + neighbor_x];
                  color color(neighbor_pixel);
                  float weight = kernel[i][j];
                  r += weight * color.r;
                  g += weight * color.g;
                  b += weight * color.b;
                  a += weight * color.a;
                  weight_sum += weight;
               }
            }
         }

         // Set the value of the pixel in the new surface to the computed average
         r /= weight_sum;
         g /= weight_sum;
         b /= weight_sum;
         a /= weight_sum;
         blurred[y * width + x] = color(r, g, b, a);
      }
   }

   // Replace the original surface with the blurred surface
   delete [] pixels;
   pixels = blurred;
   texture = {};
}

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::vector<unsigned char>*>(userdata);
    response->insert(response->end(), ptr, ptr + realsize);
    return realsize;
}

PImageImpl loadImageImpl(std::string_view URL)
{
   // Set up the libcurl easy handle
   std::string sURL{URL};
   CURL* curl = curl_easy_init();
   curl_easy_setopt(curl, CURLOPT_URL, sURL.c_str());
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
   std::vector<unsigned char> response_body;
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

   // Perform the request
   CURLcode res = curl_easy_perform(curl);

   sail::image image = [&] {
      if (res == CURLE_OK) {
         // Load the image from the response data
         sail::image_input image_input(response_body.data(), response_body.size());
         curl_easy_cleanup(curl);
         return image_input.next_frame();
      } else {
         // If it didn't download check the local filesystem
         using namespace std::literals;
         return sail::image(("data/"s + sURL).c_str());
      } } ();

   if (!image.is_valid()) {
      abort();
   }
   image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA);

   return PImageImpl( image.width(), image.height(), (uint32_t*)image.pixels() );
}

void PImageImpl::save_as( const std::string &filename ) const {
   sail::image_output output(filename);
   sail::image image = sail::image( (void*)pixels, SAIL_PIXEL_FORMAT_BPP32_RGBA, width, height );
   if (!image.is_valid())
      abort();
   output.next_frame( image );
   output.finish();
}

PImage createImage(int width, int height, int mode) {
   return PImage( std::make_shared<PImageImpl>(width,height,mode) );
}

PImage loadImage(std::string_view URL) {
   PImageImpl img = loadImageImpl(URL);
   return PImage( std::make_shared<PImageImpl>(img) );
}

PImage requestImage(std::string_view URL) {
   // This should be an async load
   return loadImage(URL);
}
