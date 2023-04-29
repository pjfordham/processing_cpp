#ifndef PROCESSING_PIMAGE_H
#define PROCESSING_PIMAGE_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <curl/curl.h>

#include "processing_color.h"
#include "processing_enum.h"

#include <vector>

class PImage {
public:
   int width;
   int height;
   unsigned int *pixels;
   SDL_Surface *surface;
   GLuint textureID = 0;

   operator bool() const {
      return surface;
   }

   static void init() {
      // initialize SDL_image
      if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Init JPG failed: %s\n", SDL_GetError());
         abort();
      }
      // Initialize libcurl
      curl_global_init(CURL_GLOBAL_ALL);
  }

   static void close() {
     curl_global_cleanup();
  }

   PImage() : width(0), height(0), pixels{NULL}, surface{NULL} {}

   ~PImage() {
      if (surface) {
         SDL_FreeSurface(surface);
      }
      if (textureID)
         glDeleteTextures(1, &textureID);
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
      std::swap(textureID, x.textureID);
   }

   PImage& operator=(const PImage&) = delete;
   PImage& operator=(PImage&&x){
      std::swap(surface, x.surface);
      std::swap(width, x.width);
      std::swap(height, x.height);
      std::swap(pixels, x.pixels);
      std::swap(textureID, x.textureID);
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

   PImage(int w, int h, int mode) : width(w), height(h){
      surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
      if (surface == NULL) {
         abort();
      }
      // Clear the surface to black
      SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
      pixels =  (Uint32*)surface->pixels;
   }

   PImage(SDL_Surface *surface_) {
      surface = SDL_ConvertSurfaceFormat(surface_, SDL_PIXELFORMAT_ABGR8888, 0);
      if (surface == NULL) {
         abort();
      }
      width = surface->w;
      height = surface->h;
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

   static void saveFrame(const GLuint framebufferID, int width, int height, const std::string& fileName) {

      // Bind the framebuffer and get its dimensions
      glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

      // Create SDL surface from framebuffer data
      SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
      glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

      // Flip the image vertically
      SDL_Surface* flippedSurface = SDL_CreateRGBSurface(0, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
      SDL_BlitSurface(surface, nullptr, flippedSurface, nullptr);
      SDL_FreeSurface(surface);

      // Save the image as PNG
      IMG_SavePNG(flippedSurface, fileName.c_str());

      // Cleanup
      SDL_FreeSurface(flippedSurface);
   }

};

PImage createImage(int width, int height, int mode) {
   return {width,height,mode};
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
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

   SDL_Surface *loaded;

   if (res == CURLE_OK) {
      // Load the image from the response data
      SDL_RWops* rw = SDL_RWFromConstMem(response_body.data(), response_body.size());
      loaded = IMG_Load_RW(rw, 1);
   } else {
      // If it didn't download check the local filesystem
      loaded = IMG_Load(URL);
   }
   if (loaded == NULL) {
      abort();
   }
   curl_easy_cleanup(curl);
   PImage image( loaded );
   SDL_FreeSurface(loaded);
   return image;
}

#endif
