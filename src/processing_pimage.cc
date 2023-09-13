#include "processing_pimage.h"
#include <sail-c++/sail-c++.h>
#include <curl/curl.h>

void PImage::init() {
   sail::log::set_barrier( SAIL_LOG_LEVEL_WARNING );
   // Initialize libcurl
   curl_global_init(CURL_GLOBAL_ALL);
}

void PImage::close() {
   curl_global_cleanup();
}

PImage::~PImage() {
   if (pixels) {
      delete [] pixels;
   }
}

PImage::PImage(const PImage &x){
   width = x.width;
   height = x.height;
   pixels = new uint32_t[width*height];
   std::copy(x.pixels, x.pixels+width*height, pixels);
}

PImage::PImage(int w, int h, int mode) : width(w), height(h){
   pixels = new uint32_t[width*height];
   std::fill(pixels, pixels+width*height, color(BLACK));
}

void PImage::loadPixels() {
}

void PImage::convolve(const std::vector<std::vector<float>> &kernel) {
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
}

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t realsize = size * nmemb;
    auto* response = static_cast<std::vector<unsigned char>*>(userdata);
    response->insert(response->end(), ptr, ptr + realsize);
    return realsize;
}

PImage loadImage(const char *URL)
{
   // Set up the libcurl easy handle
   CURL* curl = curl_easy_init();
   curl_easy_setopt(curl, CURLOPT_URL, URL);
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
         return sail::image(("data/"s + URL).c_str());
      } } ();

   if (!image.is_valid()) {
      abort();
   }
   image.convert(SAIL_PIXEL_FORMAT_BPP32_RGBA);

   PImage pimage = createImage(image.width(), image.height(),0);
   std::memcpy(pimage.pixels, image.pixels(), image.width() * image.height()*4);

   return pimage;
}

void PImage::save_as( const std::string &filename ) {
   sail::image_output output(filename);
   sail::image image = sail::image( (void*)pixels, SAIL_PIXEL_FORMAT_BPP32_RGBA, width, height );
   if (!image.is_valid())
      abort();
   output.next_frame( image );
   output.finish();
}
