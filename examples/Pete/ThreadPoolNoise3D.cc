/**
 * Noise3D.
 *
 * Using 3D noise to create simple animated texture.
 * Here, the third dimension ('z') is treated as time.
 */

#include <thread>
#include "ThreadPool.h"

float increment = 0.01;
// The noise function's 3rd argument, a global variable that increments once per cycle
float zoff = 0.0;
// We will increment zoff differently than xoff and yoff
float zincrement = 0.02;

std::vector<std::jthread> threads;

progschj::ThreadPool pool;

void setup() {
   size(640, 360);
}

void draw() {

   // Optional: adjust noise detail here
   // noiseDetail(8,0.65f);

   loadPixels();

   float xoff = 0.0; // Start xoff at 0

   int chunk_size = height / pool.get_pool_size();

   // For every x,y coordinate in a 2D space, calculate a noise value and produce a brightness value
   for (int y = 0; y < height; y+=chunk_size) {
      int ystart = y;
      int yend = std::min(height,ystart + chunk_size);
      auto result = pool.enqueue( [ystart, yend] {
         for (int y = ystart; y < yend; y++) {
            float yoff = y * increment;   // Increment xoff
            for (int x = 0; x < width; x++) {
               float xoff = x * increment; // Increment yoff

               // Calculate noise and scale by 255
               float bright = noise(xoff,yoff,zoff)*255;

               // Try using this line instead
               // float bright = random(0,255);

               // Set each pixel onscreen to a grayscale value
               pixels[y*width + x] = color(bright,bright,bright);
            }
         }
         return 0;
      } );
   }
   pool.wait_until_empty();
   updatePixels();

   zoff += zincrement; // Increment zoff


}
