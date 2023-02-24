#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <algorithm>
#include <chrono>
#include <vector>

struct Pos {
   int x;
   int y;
};

void setup();
void draw();

SDL_Window *window;
SDL_Renderer *renderer;

std::vector<Pos> shape;

int POINTS = 0;
void beginShape(int points) {
   shape.clear();
}

void vertex(int x, int y) {
   shape.push_back({x,y});
}

void endShape() {
   for (std::size_t i = 1; i < shape.size(); ++i) {
      SDL_RenderDrawLine(renderer, shape[i-1].x, shape[i-1].y, shape[i].x, shape[i].y);
   }
}

void stroke(int x) {}
void strokeWeight(int x) {}
void noStroke() {}

void ellipse(int x, int y, int width, int height) {
   filledEllipseRGBA(renderer, x, y, width, height, 255, 0, 0, 255);
   ellipseRGBA(renderer, x, y, width, height, 255, 255, 255,255);
}

void line(int x, int y, int xx, int yy) {
   SDL_RenderDrawLine(renderer, x, y, xx, yy);
}

double radians(double degrees) {
    return degrees * M_PI / 180.0;
}

double norm(double value, double start, double stop) {
    return (value - start) / (stop - start);
}

double norm(double value, double start1, double stop1, double start2, double stop2) {
    double adjustedValue = (value - start1) / (stop1 - start1);
    return start2 + (stop2 - start2) * adjustedValue;
}

int second() {
   // Get the current wall clock time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t and then to tm structure in local time
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&time);

    // Extract hours, minutes, and seconds from the tm structure
    int hours = local_time.tm_hour;
    int minutes = local_time.tm_min;
    int seconds = local_time.tm_sec;

    return seconds;
}

int minute() {
   // Get the current wall clock time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t and then to tm structure in local time
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&time);

    // Extract hours, minutes, and seconds from the tm structure
    int hours = local_time.tm_hour;
    int minutes = local_time.tm_min;
    int seconds = local_time.tm_sec;

    return minutes;
}

int hour() {
   // Get the current wall clock time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t and then to tm structure in local time
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&time);

    // Extract hours, minutes, and seconds from the tm structure
    int hours = local_time.tm_hour;
    int minutes = local_time.tm_min;
    int seconds = local_time.tm_sec;

    return hours;
}

void background(int gray) {
   // Clear window to Blue to do blue boarder.
   SDL_SetRenderDrawColor(renderer, gray,gray,gray,0xFF);
   SDL_RenderClear(renderer);
}

void background(int r, int g, int b) {
   // Clear window to Blue to do blue boarder.
   SDL_SetRenderDrawColor(renderer, r,g,b,0xFF);
   SDL_RenderClear(renderer);
}

#define M_PI 3.14159265358979323846
#define TWO_PI (M_PI * 2.0)
#define HALF_PI (M_PI / 2.0)

int width = 0;
int height = 0;

using std::min;

void size(int _width, int _height) {
   // Create a window
   width = _width;
   height = _height;
   window = SDL_CreateWindow("Proce++ing",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             width,
                             height,
                             SDL_WINDOW_SHOWN);

   if (window == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   // Create a renderer
   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   if (renderer == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

}

void fill(int r) {
   SDL_SetRenderDrawColor(renderer, r, r, r, 0xFF);
}

void fill(int r, int g, int b) {
   SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
}

void rect(int x, int y, int width, int height) {
   // draw black background for theatre of life
   SDL_Rect fillRect = { x, y, width, height };
   SDL_RenderFillRect(renderer, &fillRect);
}

float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

int main()
{
   // Initialize SDL
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return 1;
   }
  //  TTF_Font* font = NULL;
  //  if (TTF_Init() != 0) {
  //     printf("TTF_Init failed: %s\n", TTF_GetError());
  //     return 1;
  //  }
  // font = TTF_OpenFont("../Instruction.ttf", 40);
  //  if (font == NULL) {
  //     printf("TTF_OpenFont failed: %s\n", TTF_GetError());
  //     return 1;
  //  }


   setup();

   Uint32 clock = SDL_GetTicks();
   Uint32 frameRateClock = clock;
   bool quit = false;
   int frameCount = 0;

   while (!quit) {

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit = true;
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEWHEEL) {
         } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
               quit = true;
               break;
            case SDLK_i:
               break;
            case SDLK_p:
               break;
            case SDLK_c:
               break;
            case SDLK_SPACE:
               break;
            default:
               break;
            }
         }
      }

      frameCount++;
      // Print the frame rate every 10 seconds
      Uint32 currentTicks = SDL_GetTicks();
      if (currentTicks - frameRateClock >= 10000) {
         float frameRate = 1000 * (float) frameCount / (currentTicks - frameRateClock);
         printf("Frame rate: %f fps\n", frameRate);
         frameCount = 0;
         frameRateClock = currentTicks;
      }

      draw();
      
      // Update the screen
      SDL_RenderPresent(renderer);
   }

   // TTF_CloseFont(font);

   // Destroy the window and renderer
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);

   // Quit SDL
   SDL_Quit();
   return 0;
};
