#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <fmt/core.h>
#include <string>
#include <vector>

#include "processing_psurface.h"
#include "processing_enum.h"
#include "processing_task_queue.h"

static std::vector<PSurface*> &surfaceHandles() {
   static std::vector<PSurface*> handles;
   return handles;
}

extern PSurface *psurface;

void PSurface_setup() {
   renderThread.enqueue( [i=psurface] {
      i->makeContextCurrent();
   } );
   psurface->setupFrame();
   for (auto *i : surfaceHandles()) {
      if (i == psurface) continue;
      i->construct(psurface);
      renderThread.enqueue( [i] {
         i->makeContextCurrent();
      } );
      i->setupFrame();
   }
}

void PSurface_draw() {
   for (auto *i : surfaceHandles()) {
      if (i->width) {
         renderThread.enqueue( [i] {
            i->makeContextCurrent();
         } );
         i->drawFrame();
         renderThread.enqueue( [i] {
            i->swapBuffers();
         } );
      }
   }
}

PSurface::PSurface() {
   surfaceHandles().push_back(this);
}

PSurface::~PSurface() {
   auto &vec = surfaceHandles();
   vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
}

static void character_callback(GLFWwindow* window, unsigned int codepoint) {
   // Handle the Unicode character codepoint here
   // You can convert it to a character if needed
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   char character = static_cast<char>(codepoint);
   surface->key = character;
   surface->keyCode = character;
   surface->keyTyped();
}

static void key_callback(GLFWwindow* window, int key_, int scancode, int action, int mods) {
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   if (action == GLFW_PRESS || action == GLFW_REPEAT || action == GLFW_RELEASE) {
      switch(key_) {
      case GLFW_KEY_ESCAPE:
         surface->shutdown();
         break;
      case GLFW_KEY_UP:
         surface->key = CODED;
         surface->keyCode = UP;
         break;
      case GLFW_KEY_DOWN:
         surface->key = CODED;
         surface->keyCode = DOWN;
         break;
      case GLFW_KEY_LEFT:
         surface->key = CODED;
         surface->keyCode = LEFT;
         break;
      case GLFW_KEY_RIGHT:
         surface->key = CODED;
         surface->keyCode = RIGHT;
         break;
      case GLFW_KEY_ENTER:
         surface->key = CODED;
         surface->keyCode = ENTER;
         break;
      case GLFW_KEY_SPACE:
         surface->key = ' ';
         surface->keyCode = 0;
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
         surface->keyCode = 0;
         surface->key = (mods & GLFW_MOD_SHIFT) ? (key_ - GLFW_KEY_A + 'A') : (key_ - GLFW_KEY_A + 'a');
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
         surface->keyCode = 0;
         surface->key = key_ - GLFW_KEY_0 + '0';
         break;
      default:
         surface->keyCode = CODED;
         surface->key = 0;
         break;
      }
   }
   if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      surface->keyPressedb = true;
      surface->keyPressed();
   } else {
      surface->keyPressedb = false;
      surface->keyReleased();
   }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      surface->mousePressed();
      surface->mousePressedb = true;
   }
   else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      surface->mouseReleased();
      surface->mousePressedb = false;
   }
}

static void scroll_callback(GLFWwindow* window, double offsetX, double offsetY) {
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   surface->mouseWheel({MouseEvent::WHEEL, 0,0,0,-(float)offsetY});
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   surface->mouseX = xpos;
   surface->mouseY = ypos;
   if (surface->mousePressedb) {
      surface->mouseDragged();
   } else {
      surface->mouseMoved();
   }
}

static void framebuffer_size_callback(GLFWwindow* window, int _width, int _height) {
   auto *surface = static_cast<PSurface*>(glfwGetWindowUserPointer(window));
   surface->dispatch_size_callback(_width,_height);
}

void PSurface::init() {
   if (!glfwInit()) {
      fmt::print("Failed to initialize GLFW\n");
      abort();
   }
}

void PSurface::close() {
   glfwTerminate();
}

void PSurface::setResizable( bool r ) {
   resizable = r;
   if (resizable) {
      glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE);
   } else {
      glfwSetWindowSizeLimits(window, width, height, width, height);
   }
}

void PSurface::dispatch_size_callback(int _width, int _height) {
   width = _width;
   height = _height;
   g.resize(width,height);
}

bool PSurface::getResizable() const {
   return resizable;
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

void PSurface::shutdown() {
   gl::texture_t::blank()->release();
   glfwSetWindowShouldClose(window, GLFW_TRUE);
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

void PSurface::construct(PSurface *main) {

   glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
   glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   window = glfwCreateWindow(640, 480, "Proce++ing", nullptr, main ? main->window : nullptr);
   if (!test_mode) {
      // Set input callback functions
      glfwSetKeyCallback(window, key_callback);
      glfwSetCharCallback(window, character_callback);
      glfwSetCursorPosCallback(window, mouse_callback);
      glfwSetMouseButtonCallback(window, mouse_button_callback);
      glfwSetScrollCallback(window, scroll_callback);
      glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
   }
   glfwMakeContextCurrent(nullptr);
   glfwSetWindowUserPointer(window, this);
}

void PSurface::makeContextCurrent() {
   glfwMakeContextCurrent(window);
   static bool GLAD = true;
   if (GLAD) {
      //  Initialize GLAD for OpenGL function loading
      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
         fmt::print("Failed to initialize GLAD\n");
         abort();
      }
      GLAD = false;
   }
}

int PSurface::drawFrame() {
   g.beginDraw();
   draw();
   g.endDraw();
   return g.commit_draw();
}

void PSurface::size(int _width, int _height) {
   size(_width,_height, P2D );
}

void PSurface::size(int _width, int _height, int mode) {
   setSize(_width, _height);
   if (!test_mode)
      setVisible( true );
   width = _width;
   height = _height;
   g = PGraphics(width, height, mode);
   g.beginDraw();
}

void PSurface::setupFrame() {
   textFont( createFont("DejaVuSans.ttf",12) );
   setup();
   if (width != 0) {
      // Draw anything from setup.
      g.endDraw();
      g.commit_draw();
   }
   PShape::optimize();
}

void PSurface::loadPixels() {
   g.loadPixels();
   pixels = g.getPixels();
}

PGraphics PSurface::createGraphics(int w, int h, int mode) {
   return {w,h,mode};
}
PGraphics PSurface::createGraphics(int w, int h) {
   return {w,h,P3D};
}
