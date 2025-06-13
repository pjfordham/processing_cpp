#include <iostream>
#include <filesystem>

#include <curl/curl.h>
#include <fmt/core.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include "processing_pimage.h"
#include "processing_debug.h"
#include "processing_opengl_texture.h"

#include "glad/glad.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

void createDirectoriesForFile(const std::string& filename) {
   namespace fs = std::filesystem;

   fs::path filePath(filename);

   // Remove the filename itself to ensure only directories remain
   fs::path directoryPath = filePath.parent_path();

   if (!directoryPath.empty()) {
     // Create directories along the path if they don't exist
      if (!fs::exists(directoryPath)) {
         if (!fs::create_directories(directoryPath)) {
            std::cerr << "Failed to create directories: " << directoryPath.string() << std::endl;
            abort();
         }
      }
   }
}

int textureWrapMode = CLAMP;

template <> struct fmt::formatter<PImageImpl>;

class PImageImpl {
public:
   int width = 0;
   int height = 0;
   unsigned int *pixels = nullptr;
   gl::texture_ptr texture;
   int textureWrap;
   bool dirty = true;

   ~PImageImpl() {
      DEBUG_METHOD();
      if (pixels) {
         delete [] pixels;
      }
   }

   PImageImpl(int w, int h, int mode) : width(w), height(h) {
      DEBUG_METHOD();
      textureWrap = mode;
      pixels = new uint32_t[width*height];
      std::fill(pixels, pixels+width*height, color(BLACK));
   }

   PImageImpl(gl::texture_ptr textureID_) : texture(textureID_) {
      DEBUG_METHOD();
      width = texture->get_width();
      height = texture->get_height();
   }

   PImageImpl(int w, int h, uint32_t *pixels_, int) : width(w), height(h) {
      DEBUG_METHOD();
      textureWrap = textureWrapMode;
      pixels = new uint32_t[width*height];
      std::copy(pixels_, pixels_+width*height, pixels);
   }

   PImageImpl() = delete;
   PImageImpl(PImageImpl &&x) = delete;
   PImageImpl(const PImageImpl &x) = delete;
   PImageImpl& operator=(const PImageImpl&) = delete;
   PImageImpl& operator=(PImageImpl&&x) = delete;

   void mask(const PImageImpl &mask) {
      DEBUG_METHOD();
      if ( width != mask.width || height != mask.height )
         abort();
      if (!pixels)
         loadPixels();
      for(int i = 0; i < (width * height); ++i) {
         unsigned int p = pixels[i];
         unsigned int q = mask.pixels[i];
         pixels[i] = (p & 0x00FFFFFF) | ( (q & 0xFF) << 24 );
      }
      dirty = true;
   }

   color get(int x, int y) {
      DEBUG_METHOD();
      if (!pixels)
         loadPixels();
      if ( 0 <= x && x < width && 0 <= y && y < height)
         return pixels[y * width + x];
      else
         return BLACK;
   }

   void set(int x, int y, color c) {
      DEBUG_METHOD();
      if (!pixels)
         loadPixels();
      if ( 0 <= x && x < width && 0 <= y && y < height) {
         pixels[y * width + x] = c;
         dirty = true;
      }
   }

   gl::texture_ptr getTextureID() {
      DEBUG_METHOD();
      if ( dirty ) {
         updatePixels();
      }
      return texture;
   }

   bool isDirty() {
      DEBUG_METHOD();
      return dirty;
   }

   void setClean() {
      DEBUG_METHOD();
      dirty = false;
   }

   void wrapMode( int a ) {
      DEBUG_METHOD();
      textureWrap = a;
      dirty = true;
   }

   int pixels_length() const {
      DEBUG_METHOD();
      return width * height;
   }

   void updatePixels() {
      DEBUG_METHOD();
      if (dirty) {
         if (!texture) {
            texture = std::make_shared<gl::texture_t>();
         }
         texture->set_pixels( pixels, width, height, textureWrap == CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT );
         dirty = false;
      }
   }

   void releaseTexture() {
      DEBUG_METHOD();
      if (texture) {
         texture->release();
         dirty = true;
      }
   }

   void filter(int x, float level=1.0) {
      DEBUG_METHOD();
      if (!pixels)
         loadPixels();
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
      dirty = true;
   }

   void loadPixels() {
       DEBUG_METHOD();
       if (pixels) return;
       if (!texture)
          abort();
       pixels = new uint32_t[width*height];
       texture->bind();
       glGetTexImage( GL_TEXTURE_2D, 0 , GL_RGBA, GL_UNSIGNED_BYTE, pixels );
   }

   void convolve(const std::vector<std::vector<float>> &kernel) {
      DEBUG_METHOD();
      loadPixels();
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
      dirty = true;
   }

   void save_as( const std::string &filename ) {
      DEBUG_METHOD();
      loadPixels();
      createDirectoriesForFile(filename);
      stbi_write_png(filename.c_str(), width, height, 4, pixels, width * 4);
   }

};

template <>
struct fmt::formatter<PImageImpl> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const PImageImpl& v, FormatContext& ctx) {
       return format_to(ctx.out(), "width={:<4} height={:<4} pixels={:<16} textureID={:<4} dirty={:<6}", v.width, v.height, (void*)v.pixels, v.texture ? v.texture->get_id() : -2 ,v.dirty);
    }
};

bool PImage::isDirty() const {
   return impl->isDirty();
}

void PImage::setClean() {
   impl->setClean();
}

static std::vector<std::weak_ptr<PImageImpl>> &imageHandles() {
   static std::vector<std::weak_ptr<PImageImpl>> handles;
   return handles;
}

static void PImage_releaseAllTextures() {
   for (auto i : imageHandles()) {
      if (auto p = i.lock()) {
         p->releaseTexture();
      }
   }
}

PImage::PImage( std::shared_ptr<PImageImpl> impl_ ) {
   impl = impl_;
   imageHandles().push_back( impl_ );
}

PImage::operator bool() const {
   return impl->pixels != nullptr;
}

void PImage::mask(const PImage m) {
   impl->mask(*(m.impl));
}

color PImage::get(int x, int y) const {
   return impl->get(x,y);
}

gl::texture_ptr PImage::getTextureID() const {
   return impl->getTextureID();
}

void PImage::set(int x, int y, color c) {
   impl->set(x,y,c);
}

void PImage::loadPixels() const {
   impl->loadPixels();
}

void PImage::wrapMode(int w) {
   impl->wrapMode(w);
}

int PImage::pixels_length() const {
   return impl->pixels_length();
}

void PImage::updatePixels() {
   impl->updatePixels();
}

void PImage::releaseTexture() {
   impl->releaseTexture();
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
   curl_global_init(CURL_GLOBAL_ALL);
}

void PImage::close() {
   PImage_releaseAllTextures();
   curl_global_cleanup();
}

PImage PImage::circle() {
   static PImage a = createImageFromTexture( gl::texture_t::circle() );
   return a;
}

PImage createBlankImage() {
   auto p = PImage( std::make_shared<PImageImpl>(1,1,CLAMP) );
   p.set(0,0,color(255.0f));
   return p;
}

PImage createImage(int width, int height, int mode) {
   return PImage( std::make_shared<PImageImpl>(width,height,mode) );
}

PImage createImageFromTexture(gl::texture_ptr textureID) {
   return PImage( std::make_shared<PImageImpl>(textureID) );
}

size_t write_callback(char* contents, size_t size, size_t nmemb, void* userp) {
    std::vector<unsigned char>* buffer = static_cast<std::vector<unsigned char>*>(userp);
    size_t total_size = size * nmemb;
    buffer->insert(buffer->end(), (unsigned char*)contents, (unsigned char*)contents + total_size);
    return total_size;
}

PImage loadImage(std::string_view URL) {
   std::string sURL{URL};
   int width, height, channels;
   unsigned char* data = stbi_load(("data/" + sURL).c_str(), &width, &height, &channels, 4);

   if (!data) {
      // If local load fails, try downloading with CURL
      CURL* curl = curl_easy_init();
      if (!curl) return PImage(nullptr);

      curl_easy_setopt(curl, CURLOPT_URL, sURL.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
      std::vector<unsigned char> response_body;
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
      // Perform the request
      CURLcode res = curl_easy_perform(curl);
      if (res == CURLE_OK) {
         // Load the image from the response data
         data = stbi_load_from_memory(response_body.data(), response_body.size(), &width, &height, &channels, 4);
         curl_easy_cleanup(curl);
      }
   }
   if (!data) {
      abort();
   }

   PImage image(std::make_shared<PImageImpl>(width, height, (uint32_t*)data, 0));
   stbi_image_free(data);
   return image;
}

PImage _loadImage(std::filesystem::path path ) {
   int width, height, channels;
   unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

   if (!data) {
      abort();
   }

   PImage image(std::make_shared<PImageImpl>(width, height, (uint32_t*)data, 0));
   stbi_image_free(data);
   return image;
}

PImage requestImage(std::string_view URL) {
   // This should be an async load
   return loadImage(URL);
}

//
// Supply width, height and pixels as properties for compatability.
//

int &PImage::_width() {return impl->width; }
int &PImage::_height() {return impl->height; }
unsigned int *&PImage::_pixels() {return impl->pixels; }
const int &PImage::_width() const{return impl->width; }
const int &PImage::_height() const {return impl->height; }
unsigned int *const &PImage::_pixels() const{return impl->pixels; }

std::size_t PImage::_width_offset() { return offsetof(PImage,width); }
std::size_t PImage::_height_offset() { return offsetof(PImage,height); }
std::size_t PImage::_pixels_offset() { return offsetof(PImage,pixels); }

//
// End of properties.
//
