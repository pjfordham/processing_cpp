#include "processing.h"

#include <SDL2/SDL.h>

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

bool dispatchEvents() {     // Handle events
   bool quit = false;
   SDL_Event event;
   while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
         quit = true;
      } else if (event.type == SDL_MOUSEMOTION ) {
         mouseX = event.motion.x;
         mouseY = event.motion.y;
         if (mousePressedb) {
            mouseDragged();
         } else {
            mouseMoved();
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
         case SDLK_RETURN:
         case SDLK_KP_ENTER:
            key = CODED;
            keyCode = ENTER;
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
   return quit;
}

void drawFrame() {     // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
   g.glc.resetFlushCount();

   g._shape.shape_matrix = PMatrix::Identity();
   g.noLights();
   // Call the sketch's draw()
   draw();
   g.commit_draw();

   // Only update once per frame so we don't miss positions
   pmouseX = mouseX;
   pmouseY = mouseY;

}

int frameCount = 0;

SDL_Window *window = NULL;
SDL_Renderer *renderer =NULL;
void *glContext = NULL;

void size(int _width, int _height, int mode) {
   window = SDL_CreateWindow("Proce++ing",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             _width,
                             _height,
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

   // Initialize GLEW
   glewExperimental = true; // Needed for core profile
   if (glewInit() != GLEW_OK) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glew init error\n");
      abort();
   }

   if (!glewIsSupported("GL_EXT_framebuffer_object")) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
      abort();
   }

   // Create a window
   width = _width;
   height = _height;
   g = PGraphics(width, height, mode, 2.0);
}

__attribute__((weak)) int main(int argc, char* argv[]) {


   PGraphics::init();
   PFont::init();
   PImage::init();

   setup();

   int zframeCount = 0;

   unsigned int targetFrameTime = 1000 / setFrameRate;

   unsigned int frameRateClock = SDL_GetTicks();

   bool quit = false;

   while (!quit) {
      unsigned int startTicks = SDL_GetTicks();

      // Print the frame rate every 10 seconds
      if (startTicks - frameRateClock >= 10000) {
         float frameRate = 1000 * (float) zframeCount / (startTicks - frameRateClock);
         printf("Frame rate: %f fps, %d flush rate\n", frameRate, g.glc.getFlushCount());
         zframeCount = 0;
         frameRateClock = startTicks;
      }

      quit = dispatchEvents();
      if (xloop || frameCount == 0) {
         drawFrame();
         SDL_GL_SwapWindow(window);
         frameCount++;
         zframeCount++;
      }
      unsigned int endTicks = SDL_GetTicks();
      unsigned int actualFrameTime = endTicks - startTicks;

      // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
      if ( actualFrameTime < targetFrameTime ) {
         SDL_Delay( targetFrameTime - actualFrameTime);
      }
   }

   PFont::close();
   PImage::close();
   PGraphics::close();

   if (glContext)
      SDL_GL_DeleteContext(glContext);
   if (window)
      SDL_DestroyWindow(window);

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
