#ifndef PROCESSING_H
#define PROCESSING_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <algorithm>
#include <cmath>
#include <fmt/core.h>
#include <fstream>
#include <vector>
#include <string>

#include "weak.h"
#include "processing_math.h"
#include "processing_color.h"
#include "processing_java_compatability.h"
#include "processing_pimage.h"
#include "processing_pshape.h"
#include "processing_pgraphics.h"
#include "processing_pfont.h"
#include "processing_time.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_GLContext glContext = NULL;

// This is the global PGraphcs object that forms the top level canvas.
PGraphics g;

// This macro and it's following uses pull the API from PGraphcs into
// the global namespace dispatching to the PGraphics object g.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

MAKE_GLOBAL(createGraphics, g);
MAKE_GLOBAL(saveFrame, g);
MAKE_GLOBAL(background, g);
MAKE_GLOBAL(ellipse, g);
MAKE_GLOBAL(rect, g);
MAKE_GLOBAL(line, g);
MAKE_GLOBAL(point, g);
MAKE_GLOBAL(quad, g);
MAKE_GLOBAL(triangle, g);
MAKE_GLOBAL(arc, g);
MAKE_GLOBAL(shape, g);
MAKE_GLOBAL(bezier, g);
MAKE_GLOBAL(beginShape, g);
MAKE_GLOBAL(vertex, g);
MAKE_GLOBAL(bezierVertex, g);
MAKE_GLOBAL(endShape, g);
MAKE_GLOBAL(image, g);
MAKE_GLOBAL(imageMode, g);
MAKE_GLOBAL(tint, g);
MAKE_GLOBAL(texture, g);
MAKE_GLOBAL(noTexture, g);
MAKE_GLOBAL(box, g);
MAKE_GLOBAL(sphere, g);
MAKE_GLOBAL(sphereDetail, g);
MAKE_GLOBAL(createRect, g);
MAKE_GLOBAL(createQuad, g);
MAKE_GLOBAL(createLine, g);
MAKE_GLOBAL(createTriangle, g);
MAKE_GLOBAL(createArc, g);
MAKE_GLOBAL(createEllipse, g);
MAKE_GLOBAL(createPoint, g);
MAKE_GLOBAL(fill, g);
MAKE_GLOBAL(noFill, g);
MAKE_GLOBAL(noStroke, g);
MAKE_GLOBAL(stroke, g);
MAKE_GLOBAL(strokeWeight, g);
MAKE_GLOBAL(strokeCap, g);
MAKE_GLOBAL(ellipseMode, g);
MAKE_GLOBAL(rectMode, g);
MAKE_GLOBAL(noSmooth,g);
MAKE_GLOBAL(updatePixels, g);
MAKE_GLOBAL(textFont, g);
MAKE_GLOBAL(textAlign, g);
MAKE_GLOBAL(textSize, g);
MAKE_GLOBAL(text, g);
MAKE_GLOBAL(directionalLight, g);
MAKE_GLOBAL(ambientLight, g);
MAKE_GLOBAL(lights, g);
MAKE_GLOBAL(noLights, g);
MAKE_GLOBAL(ortho, g);
MAKE_GLOBAL(perspective, g);
MAKE_GLOBAL(camera, g);
MAKE_GLOBAL(pushMatrix, g);
MAKE_GLOBAL(popMatrix, g);
MAKE_GLOBAL(translate, g);
MAKE_GLOBAL(transform, g);
MAKE_GLOBAL(scale, g);
MAKE_GLOBAL(rotate, g);
MAKE_GLOBAL(rotateY, g);
MAKE_GLOBAL(rotateX, g);

template <typename T>
std::vector<T> subset(const std::vector<T> &c, int start, int length) {
   return { c.begin() + start, c.begin() + start + length };
}

std::vector<std::string> split(const std::string& str, char delimiter) {
  std::vector<std::string> result;
  std::string token;

  for (const char& c : str) {
    if (c == delimiter) {
      result.push_back(token);
      token.clear();
    } else {
      token += c;
    }
  }

  // Push back the last token, if any
  if (!token.empty()) {
    result.push_back(token);
  }

  return result;
}

std::vector<std::string> loadStrings(const std::string& fileName) {
    std::vector<std::string> lines;
    std::ifstream inputFile(fileName);

    if (!inputFile.is_open()) {
       abort();
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    inputFile.close();
    return lines;
}

int setFrameRate = 60;
void frameRate(int rate) {
   setFrameRate = rate;
}

bool xloop = true;

void noLoop() {
   xloop = false;
}

void loop() {
   xloop = true;
}

int width = 0;
int height = 0;

void size(int _width, int _height, int mode = P2D) {
   // Create a window
   width = _width;
   height = _height;

   window = SDL_CreateWindow("Proce++ing",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             width,
                             height,
                             SDL_WINDOW_OPENGL);

   if (window == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   // Set OpenGL attributes
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

   // Create OpenGL context
   glContext = SDL_GL_CreateContext(window);
   if (glContext == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   g = PGraphics(width, height, mode);
}


Uint32 *pixels; // pointer to the texture's pixel data in the desired format

void loadPixels() {
   g.loadPixels();
   pixels = g.pixels.data();
}

int frameCount = 0;
int zframeCount = 0;
int mouseX = 0;
int mouseY = 0;
int pmouseX = 0;
int pmouseY = 0;

void redraw() {
   frameCount = 0;
}

char key = 0;
int keyCode = 0;

bool mousePressedb = false;

int main(int argc, char* argv[]) {
   // Initialize SDL
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return 1;
   }

   PFont::init();
   PImage::init();

   // Call the sketch's setup.
   setup();

   Uint32 clock = SDL_GetTicks();
   Uint32 frameRateClock = clock;
   bool quit = false;

   // Set the initial tick count
   Uint32 ticks = SDL_GetTicks();

   while (!quit) {
      // Handle events
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit = true;
         } else if (event.type == SDL_MOUSEMOTION ) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            if (mousePressedb) {
               mouseDragged();
            }
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
               mousePressed();
               mousePressedb = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
               mouseReleased();
               mousePressedb = false;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEWHEEL) {
         } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
               quit = true;
               break;
            case SDLK_UP:
               key = CODED;
               keyCode = UP;
               break;
            case SDLK_DOWN:
               key = CODED;
               keyCode = DOWN;
               break;
            case SDLK_RIGHT:
               key = CODED;
               keyCode = RIGHT;
               break;
            case SDLK_LEFT:
               key = CODED;
               keyCode = LEFT;
               break;
            default:
            {
               // Get the key code from the event
               SDL_Keycode sdl_keycode = event.key.keysym.sym;

               // Check if any of the modifier keys are pressed
               SDL_Keymod mod_state = SDL_GetModState();
               bool shift_pressed = mod_state & KMOD_SHIFT;

               // Convert the key code to a string
               const char* keyname = SDL_GetKeyName(sdl_keycode);

               key = 1;
               keyCode = 0;
               // Get the first character of the string and convert to lowercase if shift is not pressed
               if ( keyname[1] == 0) {
                  char zkey = keyname[0];
                  if (shift_pressed) {
                     // Leave the key uppercase if shift is pressed
                  } else if (zkey >= 'A' && zkey <= 'Z') {
                     // Convert to lowercase if the key is a letter
                     zkey += 'a' - 'A';
                  }
                  key = zkey;
                  keyCode = key;
                  if (event.type == SDL_KEYDOWN)
                     keyTyped();
               } else if (keyname[0] == 'S' && keyname[1] == 'p') {
                  key = ' ';
                  keyCode = key;
                  if (event.type == SDL_KEYDOWN)
                     keyTyped();
               }
            }
            }
            if (event.type == SDL_KEYDOWN)
               keyPressed();
            else
               keyReleased();
         }
      }

      // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
      if (SDL_GetTicks() - ticks >= (1000 / setFrameRate))
      {
         // Print the frame rate every 10 seconds
         Uint32 currentTicks = SDL_GetTicks();
         if (currentTicks - frameRateClock >= 10000) {
            float frameRate = 1000 * (float) zframeCount / (currentTicks - frameRateClock);
            printf("Frame rate: %f fps, %d flush rate\n", frameRate, g.flushes);
            zframeCount = 0;
            frameRateClock = currentTicks;
         }

         if (xloop || frameCount == 0) {
            g.flushes = 0;

            // Call the sketch's draw()
            draw();
            g.glc.flush( g );
            if (g.localFboID != 0) {
               g.draw_main();
               g.glc.flush( g );
            }

            SDL_GL_SwapWindow(window);

            // Only update once per frame so we don't miss positions
            pmouseX = mouseX;
            pmouseY = mouseY;

            // Update the screen
            frameCount++;
            zframeCount++;
         } else {
            SDL_Delay(5);
         }
         ticks = SDL_GetTicks();
      }

   }

   PFont::close();
   PImage::close();

   // Clean up
   SDL_GL_DeleteContext(glContext);
   SDL_DestroyWindow(window);
   SDL_Quit();

   return 0;
}

#endif
