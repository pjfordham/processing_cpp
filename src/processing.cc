#include "processing.h"

#include <SDL2/SDL.h>

PGraphics g;
int setFrameRate = 60;
bool xloop = true;
int width = 0;
int height = 0;

unsigned int *pixels;


int frameCount = 0;
int zframeCount = 0;
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
   static unsigned int frameRateClock = SDL_GetTicks();
   static unsigned int ticks = SDL_GetTicks();
   // Print the frame rate every 10 seconds
   // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
   if (SDL_GetTicks() - ticks >= (1000 / setFrameRate))
   {
      unsigned int currentTicks = SDL_GetTicks();
      if (currentTicks - frameRateClock >= 10000) {
         float frameRate = 1000 * (float) zframeCount / (currentTicks - frameRateClock);
         printf("Frame rate: %f fps, %d flush rate\n", frameRate, g.glc.getFlushCount());
         zframeCount = 0;
         frameRateClock = currentTicks;
      }

      if (xloop || frameCount == 0) {
         g.glc.resetFlushCount();

         g._shape.shape_matrix = PMatrix::Identity();
         g.noLights();
         // Call the sketch's draw()
         draw();
         g.commit_draw();

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

__attribute__((weak)) int main(int argc, char* argv[]) {

   PGraphics::init();
   PFont::init();
   PImage::init();

   setup();

   bool quit = false;

   while (!quit) {
      quit = dispatchEvents();
      drawFrame();
   }

   PFont::close();
   PImage::close();
   PGraphics::close();

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
