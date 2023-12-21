#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <thread>
#include <chrono>
#include <fmt/core.h>

#include "processing.h"
#include "processing_profile.h"

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


char key = 0;
int keyCode = 0;

bool mousePressedb = false;
bool keyPressedb = false;

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

   g._shape.resetMatrix();
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

void size(int _width, int _height, int mode) {

   // Create a GLFW window and OpenGL context
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

   // Set input callback functions
   glfwSetKeyCallback(window, key_callback);
   glfwSetCharCallback(window, character_callback);
   glfwSetCursorPosCallback(window, mouse_callback);
   glfwSetMouseButtonCallback(window, mouse_button_callback);

   glfwMakeContextCurrent(window);

   // Initialize GLAD for OpenGL function loading
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      fmt::print("Failed to initialize GLAD\n");
      abort();
   }

   // Create a window
   width = _width;
   height = _height;
   g = PGraphics(width, height, mode);
}

void exit() {
   glfwSetWindowShouldClose(window, GLFW_TRUE);
}

__attribute__((weak)) int main(int argc, char* argv[]) {

   Profile::Instrumentor::Get().BeginSession("main");

   // Initialize GLFW
   if (!glfwInit()) {
      fmt::print("Failed to initialize GLFW\n");
      abort();
   }

   PGraphics::init();
   PShader::init();
   PFont::init();
   PImage::init();
   PShape::init();

   {
      PROFILE_SCOPE("setup");
      setup();
   }

   int zframeCount = 0;

   unsigned int targetFrameTime = 1000 / setFrameRate;

   auto frameRateClock = std::chrono::high_resolution_clock::now();
   int flushes = 0;

   // Main rendering loop
   while (window && !glfwWindowShouldClose(window)) {

      PROFILE_SCOPE("eventLoop");

      // Poll for and process events
      if (window)
         glfwPollEvents();

      auto startTicks = std::chrono::high_resolution_clock::now();

      // Print the frame rate every 10 seconds
      unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(startTicks - frameRateClock).count();
      if (millis >= 10000) {
         float frameRate = 1000 * (float) zframeCount / millis;
         printf("Frame rate: %f fps, %d flush rate\n", frameRate, flushes);
         zframeCount = 0;
         frameRateClock = startTicks;
      }

      if (window && (xloop || frameCount == 0)) {
         {
            PROFILE_SCOPE("drawFrame");
            flushes = drawFrame();
         }
         {
            PROFILE_SCOPE("glSwapWindow");
            glfwSwapBuffers(window);
         }
         frameCount++;
         zframeCount++;
      }
      auto endTicks = std::chrono::high_resolution_clock::now();
      unsigned int actualFrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTicks - startTicks).count();

      // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
      if ( actualFrameTime < targetFrameTime ) {
         PROFILE_SCOPE("vSync");
         std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTime - actualFrameTime));
      }
   }

   PShape::close();
   PFont::close();
   PImage::close();
   PShader::close();
   PGraphics::close();

   // Clean up resources, force g to be destructed before we close down GLFW
   g = {};

   glfwTerminate();

   Profile::Instrumentor::Get().EndSession();
   return 0;
}

__attribute__((weak)) void keyTyped() {}
__attribute__((weak)) void keyPressed() {}
__attribute__((weak)) void keyReleased() {}
__attribute__((weak)) void setup() {}
__attribute__((weak)) void draw() {}
__attribute__((weak)) void mousePressed() {}
__attribute__((weak)) void mouseDragged() {}
__attribute__((weak)) void mouseMoved() {}
__attribute__((weak)) void mouseReleased() {}
