#ifndef PROCESSING_H
#define PROCESSING_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>

struct Pos {
   int x;
   int y;
};

void setup();
void draw();

SDL_Window *window;
SDL_Renderer *renderer;

enum {
  RADIUS = 0,
  DIAMETER = 1,
};

enum {
  CORNERS = 0,
  WIDTH = 1,
};

float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

int xellipse_mode = DIAMETER;
int xrect_mode = WIDTH;

void ellipseMode(int mode) {
   xellipse_mode = mode;
}

void rectMode(int mode){
   xrect_mode = mode;
}

void drawRoundedLine( int x1, int y1, int x2, int y2, int thickness, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    // Draw thick line
    thickLineRGBA(renderer, x1, y1, x2, y2, thickness, r, g, b, a);
    
    // // Draw rounded ends
     filledCircleRGBA(renderer, x1, y1, thickness / 2, r, g, b, a);
     filledCircleRGBA(renderer, x2, y2, thickness / 2, r, g, b, a);
}

SDL_Color stroke_color{255,255,255,255};
SDL_Color fill_color{255,255,255,255};

std::vector<Pos> shape;
int xstrokeWeight = 1;

enum{
   POINTS = 0,
   LINES = 1,
};

int shape_style = LINES;

void beginShape(int points) {
   shape_style = points;
   shape.clear();
}

void vertex(int x, int y) {
   shape.push_back({x,y});
}

void ellipse(int x, int y, int width, int height) {
   if (xellipse_mode == RADIUS ) {
      width *=2;
      height *=2;
   }
   filledEllipseRGBA(renderer, x, y, width/2, height/2, fill_color.r,fill_color.g,fill_color.b,fill_color.a);
   ellipseRGBA(renderer, x, y, width/2, height/2, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
}

void line(int x, int y, int xx, int yy) {
   if (xstrokeWeight == 1) {
      SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
      SDL_RenderDrawLine(renderer, x, y, xx, yy);
   } else {
      drawRoundedLine(x, y, xx, yy, xstrokeWeight, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   }
}
void endShape() {
   SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a); // set drawing color to red
   if (shape_style == LINES) {
      for (std::size_t i = 1; i < shape.size(); ++i) {
         line(shape[i-1].x, shape[i-1].y, shape[i].x, shape[i].y);
      }
      line(shape[shape.size()-1].x, shape[shape.size()-1].y, shape[0].x, shape[0].y);
   } else if (shape_style == POINTS) {
      for (std::size_t i = 0; i < shape.size(); ++i) {
         if (xstrokeWeight == 1) {
            SDL_RenderDrawPoint(renderer, shape[i].x, shape[i].y); // draw point at (10, 10)
         } else {
            filledEllipseRGBA(renderer, shape[i].x, shape[i].y, xstrokeWeight/2, xstrokeWeight/2, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
         }
      }
   }
}

enum {
  RGB = 0,
  HSB = 1,
};

int xcolorMode = RGB;
int xcolorScale = 255;

void colorMode(int mode, float scale) {
  xcolorMode = mode;
  xcolorScale = scale;
}

 void stroke(unsigned char x) {
   stroke_color = SDL_Color{x,x,x,255};
}

void strokeWeight(int x) {
   xstrokeWeight = x;
}

void noStroke() {
   stroke_color = {0,0,0,0};
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

auto start_time = std::chrono::high_resolution_clock::now();

unsigned long millis() {
   auto current_time = std::chrono::high_resolution_clock::now();
   auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
   return elapsed_time;
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

float constrain(float value, float lower, float upper) {
    if (value < lower) {
        return lower;
    } else if (value > upper) {
        return upper;
    } else {
        return value;
    }
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

void fill(float _r) {
   unsigned char r = map(_r,0,xcolorScale,0,255);
   fill_color = { r,r,r,255};
}

void fill(float _r,float _a) {
   unsigned char r = map(_r,0,xcolorScale,0,255);
   unsigned char a = map(_a,0,xcolorScale,0,255);
   fill_color = { r,r,r,a };
}

void fill(float _r,float _g,  float _b) {
   unsigned char r = map(_r,0,xcolorScale,0,255);
   unsigned char g = map(_g,0,xcolorScale,0,255);
   unsigned char b = map(_b,0,xcolorScale,0,255);
   fill_color = { r,g,b,255};
}

void rect(int x, int y, int width, int height) {
   if (xrect_mode == CORNERS) {
      width = width -x;
      height = height - y;
   }
   SDL_SetRenderDrawColor(renderer, fill_color.r,fill_color.g,fill_color.b,fill_color.a); // set drawing color to red
   SDL_Rect fillRect = { x, y, width, height };
   SDL_RenderFillRect(renderer, &fillRect);
}



int frameCount = 0;
int mouseX = 0;
int mouseY = 0;

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

   while (!quit) {

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit = true;
         } else if (event.type == SDL_MOUSEMOTION ) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
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

#endif
