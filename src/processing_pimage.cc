#include "processing_pimage.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <curl/curl.h>

void PImage::init() {
   // initialize SDL_image
   if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Init JPG failed: %s\n", SDL_GetError());
      abort();
   }
   // Initialize libcurl
   curl_global_init(CURL_GLOBAL_ALL);
}

void PImage::close() {
   curl_global_cleanup();
}

PImage::~PImage() {
   if (surface) {
      SDL_FreeSurface(surface);
   }
}

PImage::PImage(const PImage &x){
   width = x.width;
   height = x.height;
   surface = SDL_ConvertSurfaceFormat(x.surface, SDL_PIXELFORMAT_ABGR8888, 0);
   pixels = (Uint32 *)surface->pixels;
}
PImage::PImage(int w, int h, int mode) : width(w), height(h){
   surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
   if (surface == NULL) {
      abort();
   }
   // Clear the surface to black
   SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
   pixels = (Uint32*)surface->pixels;
}

PImage::PImage(SDL_Surface *surface_) {
   surface = SDL_ConvertSurfaceFormat(surface_, SDL_PIXELFORMAT_ABGR8888, 0);
   if (surface == NULL) {
      abort();
   }
   width = surface->w;
   height = surface->h;
   pixels = (Uint32*)surface->pixels;
}

void PImage::loadPixels() {
   pixels = (unsigned int*)surface->pixels;
}

void PImage::convolve (const std::vector<std::vector<float>> &kernel) {
   // Create a new surface of the same size as the original surface
   SDL_Surface* blurred_surface = SDL_CreateRGBSurfaceWithFormat(0, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->format);

   for (int y = 0; y < surface->h; y++) {
      for (int x = 0; x < surface->w; x++) {
         float r = 0, g = 0, b = 0, a = 0;
         float weight_sum = 0;

         for (int j = 0; j < kernel.size(); j++) {
            for (int i = 0; i < kernel[0].size(); i++) {
               int neighbor_x = x + i - ( (kernel.size() - 1 ) / 2 );
               int neighbor_y = y + j - ( (kernel[i].size() - 1 ) / 2 );;
               if (neighbor_x >= 0 && neighbor_x < surface->w && neighbor_y >= 0 && neighbor_y < surface->h) {
                  Uint32 neighbor_pixel = ((Uint32*)surface->pixels)[neighbor_y * width + neighbor_x];
                  Uint8 neighbor_r, neighbor_g, neighbor_b, neighbor_a;
                  SDL_GetRGBA(neighbor_pixel, surface->format, &neighbor_r, &neighbor_g, &neighbor_b, &neighbor_a);
                  float weight = kernel[i][j];
                  r += weight * neighbor_r;
                  g += weight * neighbor_g;
                  b += weight * neighbor_b;
                  a += weight * neighbor_a;
                  weight_sum += weight;
               }
            }
         }

         // Set the value of the pixel in the new surface to the computed average
         r /= weight_sum;
         g /= weight_sum;
         b /= weight_sum;
         a /= weight_sum;
         Uint32 blurred_pixel = SDL_MapRGBA(blurred_surface->format, r, g, b, a);
         ((Uint32*)blurred_surface->pixels)[y * width + x] = blurred_pixel;
      }
   }

   // Replace the original surface with the blurred surface
   SDL_FreeSurface(surface);
   surface = blurred_surface;
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

   SDL_Surface *loaded;

   if (res == CURLE_OK) {
      // Load the image from the response data
      SDL_RWops* rw = SDL_RWFromConstMem(response_body.data(), response_body.size());
      loaded = IMG_Load_RW(rw, 1);
   } else {
      // If it didn't download check the local filesystem
      using namespace std::literals;
      loaded = IMG_Load(("data/"s + URL).c_str());
   }
   if (loaded == NULL) {
      abort();
   }
   curl_easy_cleanup(curl);
   PImage image( loaded );
   SDL_FreeSurface(loaded);
   return image;
}
