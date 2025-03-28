#include <thread>
#include <chrono>
#include <fmt/core.h>

#include "processing.h"
#include "processing_profile.h"
#include "processing_task_queue.h"
#include "processing_psurface.h"

PGraphics g;
int setFrameRate = 60;
bool xloop = true;
int width = 0;
int height = 0;

unsigned int *pixels;


bool test_mode = false;

int frameRateb = 0;

int drawFrame() {     // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame

   g.resetMatrix();
   g.noLights();
   // Call the sketch's draw()
   draw();
   int flushes = g.commit_draw();

   return flushes;
}

int frameCount = 0;


PGraphics createGraphics(int width, int height, int mode) {
   return { width, height, mode };
}

// Callback function to handle window resizing
void framebuffer_size_callback(int _width, int _height) {
   width = _width;
   height = _height;
   g.resize(width,height);
}

void size(int _width, int _height, int mode) {

   surface.setSize(_width, _height);
   if (!test_mode)
      surface.setVisible( true );

   // Create a window
   width = _width;
   height = _height;
   g = createGraphics(width, height, mode);
}

__attribute__((weak)) int main(int argc, char* argv[]) {

   Profile::Instrumentor::Get().BeginSession("main");

   int frames = 0; // Default value

   for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];

      if (arg == "--test" && i + 1 < argc) {
         frames = std::stoi(argv[i + 1]); // Convert to integer
         test_mode = true;
      }
   }

   PFont::init();

   surface.init(test_mode);
   surface.setSizeCallback(framebuffer_size_callback);
   renderThread.enqueue( [&] {
      surface.makeContextCurrent();
   } );

   PGraphics::init();
   PShader::init();
   PImage::init();
   PShape::init();

   textFont( createFont("DejaVuSans.ttf",12));

   {
      PROFILE_SCOPE("setup");
      setup();
      if (width != 0) {
         // Draw anything from setup.
         g.commit_draw();
      }
      PShape::optimize();
   }

   int zframeCount = 0;

   unsigned int targetFrameTime = 1000 / setFrameRate;

   auto frameRateClock = std::chrono::high_resolution_clock::now();
   int flushes = 0;

   // Main rendering loop, window is always true inside this loop
   while (surface.runLoop()) {

      PROFILE_SCOPE("eventLoop");

      // Poll for and process events
      surface.pollEvents();

      auto startTicks = std::chrono::high_resolution_clock::now();

      // Print the frame rate every 10 seconds
      unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(startTicks - frameRateClock).count();
      frameRateb = test_mode ? setFrameRate : (1000 * (float) zframeCount / millis);
      if (millis >= 10000) {
         fmt::print("Frame rate: {} fps, {} flush rate\n", frameRateb, flushes);
         zframeCount = 0;
         frameRateClock = startTicks;
      }

      if (xloop || frameCount == 0 || test_mode) {
         {
            PROFILE_SCOPE("drawFrame");
            flushes = drawFrame();
         }
         {
            PROFILE_SCOPE("glSwapWindow");
            renderThread.enqueue( [] {
               surface.swapBuffers();
            } );
         }
         if (test_mode) {
            g.saveFrame( std::string(argv[0]) + "-####.png" );
         }
         frameCount++;
         zframeCount++;
      }
      PShape::gc();
      auto endTicks = std::chrono::high_resolution_clock::now();
      unsigned int actualFrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTicks - startTicks).count();

      // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
      if ( !test_mode && actualFrameTime < targetFrameTime ) {
         PROFILE_SCOPE("vSync");
         std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTime - actualFrameTime));
      }
      if (frames == 1) break;
      else {
         if (frames) frames--;
      }
   }

   // ensure all renderThread work is complete before we shutdown
   renderThread.wait_until_nothing_in_flight();

   PShape::close();
   PImage::close();
   PShader::close();
   PGraphics::close();

   renderThread.wait_until_nothing_in_flight();
   renderThread.shutdown();

   surface.shutdown();

   PFont::close();

   Profile::Instrumentor::Get().EndSession();
   return 0;
}
