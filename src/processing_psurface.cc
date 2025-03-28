#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <fmt/core.h>
#include <string>

#include "processing.h"
#include "processing_psurface.h"
#include "processing_enum.h"


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
         surface.close();
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

PSurface surface;

// Callback function to handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height) {
   surface.dispatch_size_callback(_width,_height);
}


void PSurface::setResizable( bool r ) {
   resizable = r;
   if (resizable) {
      glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE);
   } else {
      glfwSetWindowSizeLimits(window, width, height, width, height);
   }
}

void PSurface::setSizeCallback(SizeCallbackType c) {
   callback = c;
}

void PSurface::dispatch_size_callback(int _width, int _height) {
   width = _width;
   height = _height;
   callback(width,height);
}

bool PSurface::getResizable() const {
   return resizable;
}

void PSurface::shutdown() {
   glfwTerminate();
}

void PSurface::setSize(int _width, int _height) {
   width = _width;
   height = _height;
   glfwSetWindowSize(window, _width, _height);
   setResizable(resizable);
}

void PSurface::setTitle( std::string_view t ) {
   std::string s(t);
   glfwSetWindowTitle(window, s.c_str());
}

void PSurface::setLocation( int x, int y ) {
   glfwSetWindowPos(window, x, y);
}

void PSurface::pollEvents() {
   glfwPollEvents();
}

void PSurface::swapBuffers() {
   glfwSwapBuffers(window);
   // Only update once per frame so we don't miss positions
   pmouseX = mouseX;
   pmouseY = mouseY;
}

void PSurface::close() {
   glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void exit() {
   surface.close();
}

void PSurface::setVisible( bool r) {
   if (r) {
      glfwShowWindow(window);
   } else {
      glfwHideWindow(window);
   }
}

bool PSurface::runLoop() {
   return width != 0 && !glfwWindowShouldClose(window);
}


void PSurface::init(bool test_mode) {
   // Initialize GLFW
   if (!glfwInit()) {
      fmt::print("Failed to initialize GLFW\n");
      abort();
   } else {
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      window = glfwCreateWindow(640, 480, "Proce++ing", nullptr, nullptr);
      if (!test_mode) {
         // Set input callback functions
         glfwSetKeyCallback(window, key_callback);
         glfwSetCharCallback(window, character_callback);
         glfwSetCursorPosCallback(window, mouse_callback);
         glfwSetMouseButtonCallback(window, mouse_button_callback);
         glfwSetScrollCallback(window, scroll_callback);
         glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
      }
      glfwMakeContextCurrent(NULL);
   }
}

void PSurface::makeContextCurrent() {
   glfwMakeContextCurrent(window);
   // Initialize GLAD for OpenGL function loading
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      fmt::print("Failed to initialize GLAD\n");
      abort();
   }
}
