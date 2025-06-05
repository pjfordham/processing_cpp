#include <thread>
#include <chrono>
#include <fmt/core.h>
#include <filesystem>

#include "processing.h"
#include "processing_profile.h"
#include "processing_task_queue.h"
#include "processing_psurface.h"

int setFrameRate = 60;
int frameRateb = 0;
int frameCount = 0;
bool xloop = true;
bool test_mode = false;
std::filesystem::path refDir;

PSurfaceMain surface;
PSurface *psurface = &surface;
void PSurface_setup();
void PSurface_draw();

// Elevate these to global scope
int &width = surface.width;
int &height = surface.height;
int &mouseX = surface.mouseX;
int &mouseY = surface.mouseY;
int &pmouseX = surface.pmouseX;
int &pmouseY = surface.pmouseY;
char &key = surface.key;
int &keyCode = surface.keyCode;
bool &mousePressedb = surface.mousePressedb;
bool &keyPressedb = surface.keyPressedb;
unsigned int *&pixels = surface.pixels;

void exit() {
   surface.shutdown();
}

int main(int argc, char* argv[]) {

   bool tests_failed = false;
   Profile::Instrumentor::Get().BeginSession("main");

   int frames = 0; // Default value

   for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];

      if (arg == "--test" && i + 1 < argc) {
         frames = std::stoi(argv[i + 1]); // Convert to integer
         test_mode = true;
      }

      if (arg == "--refDir" && i + 1 < argc) {
         refDir = argv[i + 1]; // Convert to integer
      }
   }

   PFont::init();
   PSurface::init();

   surface.construct();

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
      PSurface_setup();
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
         PSurface_draw();
         if (test_mode) {
            std::string ext = ".png";
            std::string dext = "-diff.png";
            std::filesystem::path baseName = std::filesystem::path(argv[0]).stem().string() + "-####";
            static int counter = 0;
            int c = counter++;
            std::string fileName = baseName.string();
            std::size_t pos = fileName.rfind('#');
            while (pos != std::string::npos) {
               fileName[pos] = '0' + (c % 10);
               c /= 10;
               pos = fileName.rfind('#', pos - 1);
            }
            if ( !surface.g.testFrame( fileName + ext, refDir / (fileName + ext), fileName + dext ) ) {
               tests_failed = true;
            }
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

   surface.shutdown();

   renderThread.wait_until_nothing_in_flight();
   renderThread.shutdown();

   PSurface::close();
   PFont::close();

   Profile::Instrumentor::Get().EndSession();
   return tests_failed ? 1 : 0;
}

#ifdef _MSC_VER
extern "C" void keyTyped_weak() {}
extern "C" void keyPressed_weak() {}
extern "C" void keyReleased_weak() {}
extern "C" void setup_weak() {}
extern "C" void draw_weak() {}
extern "C" void mousePressed_weak() {}
extern "C" void mouseDragged_weak() {}
extern "C" void mouseMoved_weak() {}
extern "C" void mouseReleased_weak() {}
extern "C" void mouseWheel_weak(const MouseEvent&) {}
#else
__attribute__((weak)) void keyTyped() {}
__attribute__((weak)) void keyPressed() {}
__attribute__((weak)) void keyReleased() {}
__attribute__((weak)) void setup() {}
__attribute__((weak)) void draw() {}
__attribute__((weak)) void mousePressed() {}
__attribute__((weak)) void mouseDragged() {}
__attribute__((weak)) void mouseMoved() {}
__attribute__((weak)) void mouseReleased() {}
__attribute__((weak)) void mouseWheel(const MouseEvent&) {}
#endif

