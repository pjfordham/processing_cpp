#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <thread>
#include <chrono>
#include <fmt/core.h>
#include <filesystem>

#include "processing.h"
#include "processing_profile.h"
#include "processing_task_queue.h"

PGraphics g;
int setFrameRate = 60;
bool xloop = true;
int width = 0;
int height = 0;

unsigned int *pixels;


int mouseX = 0;
int mouseY = 0;
int pmouseX = 0;
int pmouseY = 0;

bool test_mode = false;
std::filesystem::path refDir;

char key = 0;
int keyCode = 0;

bool mousePressedb = false;
bool keyPressedb = false;
int frameRateb = 0;

void character_callback(GLFWwindow* window, unsigned int codepoint) {
   // Handle the Unicode character codepoint here
   // You can convert it to a character if needed
   char character = static_cast<char>(codepoint);
   key = character;
   keyCode = character;
   keyTyped();
}

void key_callback(GLFWwindow* window, int key_, int scancode, int action, int mods) {
   if (action == GLFW_PRESS || action == GLFW_REPEAT || action == GLFW_RELEASE) {
      switch(key_) {
      case GLFW_KEY_ESCAPE:
         exit();
         break;
      case GLFW_KEY_UP:
         key = CODED;
         keyCode = UP;
         break;
      case GLFW_KEY_DOWN:
         key = CODED;
         keyCode = DOWN;
         break;
      case GLFW_KEY_LEFT:
         key = CODED;
         keyCode = LEFT;
         break;
      case GLFW_KEY_RIGHT:
         key = CODED;
         keyCode = RIGHT;
         break;
      case GLFW_KEY_ENTER:
         key = CODED;
         keyCode = ENTER;
         break;
      case GLFW_KEY_SPACE:
         key = ' ';
         keyCode = 0;
         break;
      case GLFW_KEY_A:
      case GLFW_KEY_B:
      case GLFW_KEY_C:
      case GLFW_KEY_D:
      case GLFW_KEY_E:
      case GLFW_KEY_F:
      case GLFW_KEY_G:
      case GLFW_KEY_H:
      case GLFW_KEY_I:
      case GLFW_KEY_J:
      case GLFW_KEY_K:
      case GLFW_KEY_L:
      case GLFW_KEY_M:
      case GLFW_KEY_N:
      case GLFW_KEY_O:
      case GLFW_KEY_P:
      case GLFW_KEY_Q:
      case GLFW_KEY_R:
      case GLFW_KEY_S:
      case GLFW_KEY_T:
      case GLFW_KEY_U:
      case GLFW_KEY_V:
      case GLFW_KEY_W:
      case GLFW_KEY_X:
      case GLFW_KEY_Y:
      case GLFW_KEY_Z:
         keyCode = 0;
         key = (mods & GLFW_MOD_SHIFT) ? (key_ - GLFW_KEY_A + 'A') : (key_ - GLFW_KEY_A + 'a');
         break;
      case GLFW_KEY_0:
      case GLFW_KEY_1:
      case GLFW_KEY_2:
      case GLFW_KEY_3:
      case GLFW_KEY_4:
      case GLFW_KEY_5:
      case GLFW_KEY_6:
      case GLFW_KEY_7:
      case GLFW_KEY_8:
      case GLFW_KEY_9:
         keyCode = 0;
         key = key_ - GLFW_KEY_0 + '0';
         break;
      default:
         keyCode = CODED;
         key = 0;
         break;
      }
   }
   if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      keyPressedb = true;
      keyPressed();
   } else {
      keyPressedb = false;
      keyReleased();
   }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      mousePressed();
      mousePressedb = true;
   }
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      mouseReleased();
      mousePressedb = false;
   }
}

void scroll_callback(GLFWwindow* window, double offsetX, double offsetY) {
   mouseWheel({MouseEvent::WHEEL, 0,0,0,-(float)offsetY});
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
   mouseX = xpos;
   mouseY = ypos;
   if (mousePressedb) {
      mouseDragged();
   } else {
      mouseMoved();
   }
}

int drawFrame() {     // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame

   g.resetMatrix();
   g.noLights();
   // Call the sketch's draw()
   draw();
   int flushes = g.commit_draw();

   // Only update once per frame so we don't miss positions
   pmouseX = mouseX;
   pmouseY = mouseY;
   return flushes;
}

int frameCount = 0;

GLFWwindow *window = NULL;

PGraphics createGraphics(int width, int height, int mode) {
   return { width, height, mode };
}

void size(int _width, int _height, int mode) {

   // Create a GLFW window and OpenGL context
   if (test_mode)
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

   glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   window = glfwCreateWindow(_width, _height, "Proce++ing", nullptr, nullptr);
   if (!window) {
      fmt::print("Failed to create GLFW window\n");
      glfwTerminate();
      abort();
   }

   if (!test_mode) {
      // Set input callback functions
      glfwSetKeyCallback(window, key_callback);
      glfwSetCharCallback(window, character_callback);
      glfwSetCursorPosCallback(window, mouse_callback);
      glfwSetMouseButtonCallback(window, mouse_button_callback);
      glfwSetScrollCallback(window, scroll_callback);
   }


   glfwMakeContextCurrent(NULL);

   renderThread.enqueue([] {
      glfwMakeContextCurrent(window);
      // Initialize GLAD for OpenGL function loading
      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
         fmt::print("Failed to initialize GLAD\n");
         abort();
      }
   });

   // Create a window
   width = _width;
   height = _height;
   g = createGraphics(width, height, mode);
}

void exit() {
   glfwSetWindowShouldClose(window, GLFW_TRUE);
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

   // Initialize GLFW
   if (!glfwInit()) {
      fmt::print("Failed to initialize GLFW\n");
      abort();
   }

   PGraphics::init();
   PShader::init();
   PImage::init();
   PShape::init();

   textFont( createFont("DejaVuSans.ttf",12));

   {
      PROFILE_SCOPE("setup");
      setup();
      if (window) {
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
   while (window && !glfwWindowShouldClose(window)) {

      PROFILE_SCOPE("eventLoop");

      // Poll for and process events
      glfwPollEvents();

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
               glfwSwapBuffers(window);
            } );
         }
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
            if ( !g.testFrame( fileName + ext, refDir / (fileName + ext), fileName + dext ) ) {
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

   renderThread.wait_until_nothing_in_flight();
   renderThread.shutdown();

   glfwTerminate();

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

