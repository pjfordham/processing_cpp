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

using FloatArrayList = std::vector<float>;

SDL_Window *window;
SDL_Renderer *renderer;

enum {
  DIAMETER = 1,
//  RADIUS = 3,
  };

enum {
  CORNERS = 0,
  CORNER = 1,
  CENTER = 2,
  RADIUS = 3,
};

bool anything_drawn;

float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

int xellipse_mode = DIAMETER;
int xrect_mode = CORNER;

void ellipseMode(int mode) {
   xellipse_mode = mode;
}

void rectMode(int mode){
   xrect_mode = mode;
}

void drawRoundedLine( int x1, int y1, int x2, int y2, int thickness, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
   anything_drawn = true;
   // Draw thick line
    thickLineRGBA(renderer, x1, y1, x2, y2, thickness, r, g, b, a);
    
    // // Draw rounded ends
     filledCircleRGBA(renderer, x1, y1, thickness / 2, r, g, b, a);
     filledCircleRGBA(renderer, x2, y2, thickness / 2, r, g, b, a);
}

float lerp(float start, float stop, float amt) {
    return start + (stop - start) * amt;
}

SDL_Color stroke_color{255,255,255,255};
SDL_Color fill_color{255,255,255,255};

std::vector<Pos> shape;
int xstrokeWeight = 1;
typedef bool boolean;

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
   anything_drawn = true;
   if (xellipse_mode == RADIUS ) {
      width *=2;
      height *=2;
   }
   filledEllipseRGBA(renderer, x, y, width/2, height/2, fill_color.r,fill_color.g,fill_color.b,fill_color.a);
   ellipseRGBA(renderer, x, y, width/2, height/2, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
}

void line(int x, int y, int xx, int yy) {
   anything_drawn = true;
   if (xstrokeWeight == 1) {
      SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
      SDL_RenderDrawLine(renderer, x, y, xx, yy);
   } else {
      drawRoundedLine(x, y, xx, yy, xstrokeWeight, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   }
}

float dist(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

float random(float min, float max) {
    float range = max - min;
    return static_cast<float>(std::rand()) / RAND_MAX * range + min;
}

void point(int x, int y ){
   anything_drawn = true;
   SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   SDL_RenderDrawPoint(renderer, x, y);
}

void endShape() {
   anything_drawn = true;
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

int setFrameRate = 60;
void frameRate(int rate) {
   setFrameRate = rate;
}

enum {
  RGB = 0,
  HSB = 1,
};

int xcolorMode = RGB;
int xcolorScaleR = 255;
int xcolorScaleG = 255;
int xcolorScaleB = 255;
int xcolorScaleA = 255;


void colorMode(int mode, float r, float g, float b, float a) {
  xcolorMode = mode;
  xcolorScaleR = r;
  xcolorScaleG = g;
  xcolorScaleB = b;
  xcolorScaleA = a;
}

void colorMode(int mode, float scale) {
   colorMode(mode, scale, scale, scale, scale);
}

void colorMode(int mode, float r, float g, float b) {
   colorMode(mode, r,g,b,255);
}

SDL_Color HSBtoRGB(float h, float s, float v, float a)
{
    int i = floorf(h * 6);
    auto f = h * 6.0 - i;
    auto p = v * (1.0 - s);
    auto q = v * (1.0 - f * s);
    auto t = v * (1.0 - (1.0 - f) * s);

    float r,g,b;
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return {
       (unsigned char)roundf(r * 255),
       (unsigned char)roundf(g * 255),
       (unsigned char)roundf(b * 255) ,
       (unsigned char)a
    };
}

SDL_Color flatten_color_mode(float r, float g, float b, float a) {
   r = map(r,0,xcolorScaleR,0,255);
   g = map(g,0,xcolorScaleG,0,255);
   b = map(b,0,xcolorScaleB,0,255);
   a = map(a,0,xcolorScaleA,0,255);
   if (xcolorMode == HSB) {
      return HSBtoRGB(r/255.0,g/255.0,b/255.0,a);
   }
    return {
       (unsigned char)r,
       (unsigned char)g,
       (unsigned char)b,
       (unsigned char)a
    };
}

Uint32 color(int r, int g, int b, int a) {
   SDL_Color color = flatten_color_mode(r,g,b,a);
   return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

Uint32 color(int r, int g, int b) {
  return color(r, g, b, 255);
}

Uint32 color(int gray) {
  return color(gray, gray, gray);
}

void stroke(float r,float g,  float b, float a) {
   stroke_color = flatten_color_mode(r,g,b,a);
}

void stroke(float r,float g, float b) {
   stroke(r,g,b,xcolorScaleA);
}

void stroke(float r,float a) {
   stroke(r,r,r,a);
}

void stroke(float r) {
   stroke(r,r,r,xcolorScaleA);
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

bool xloop = true;

void noLoop() {
   xloop = false;
}

void loop() {
   xloop = true;
}

void background(int r, int g, int b) {
   anything_drawn = true;
   SDL_SetRenderDrawColor(renderer, r,g,b,0xFF);
   SDL_RenderClear(renderer);
}

void background(int gray) {
   background(gray, gray,gray);
}

#define PI 3.14159265358979323846
#define TWO_PI (PI * 2.0)
#define HALF_PI (PI / 2.0)

int width = 0;
int height = 0;

using std::min;

SDL_Texture* backBuffer;

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

   backBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
   if (!backBuffer)
   {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Texture could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   SDL_SetTextureBlendMode(backBuffer, SDL_BLENDMODE_BLEND);
   // Set the back buffer as the render target
   SDL_SetRenderTarget(renderer, backBuffer);
   SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

   background(255);

}


std::vector<Uint32> _pixels;
Uint32 *pixels; // pointer to the texture's pixel data in the desired format

void loadPixels() {
   _pixels.resize(width*height);
   pixels =  _pixels.data();
   int pitch = width * sizeof(Uint32);
   SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, pitch);
}

void updatePixels() {
   anything_drawn = true;
   int pitch = width * sizeof(Uint32);
   SDL_UpdateTexture(backBuffer, NULL, (void*) pixels, pitch);
   _pixels.clear();
   pixels = NULL;
}

void fill(float r,float g,  float b, float a) {
   fill_color = flatten_color_mode(r,g,b,a);
}

void fill(float r,float g, float b) {
   fill(r,g,b,xcolorScaleA);
}

void fill(float r,float a) {
   fill(r,r,r,a);
}

void fill(float r) {
   fill(r,r,r,xcolorScaleA);
}



void rect(int x, int y, int width, int height) {
   anything_drawn = true;
   if (xrect_mode == CORNERS) {
      width = width -x;
      height = height - y;
   } else if (xrect_mode == CENTER) {
      x = x - width / 2;
      y = y - height / 2;
   }
   SDL_SetRenderDrawColor(renderer, fill_color.r,fill_color.g,fill_color.b,fill_color.a);
   SDL_Rect fillRect = { x, y, width, height };
   SDL_RenderFillRect(renderer, &fillRect);
}



int frameCount = 0;
int zframeCount = 0;
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

   // Set the initial tick count
   Uint32 ticks = SDL_GetTicks();

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

      // Update the screen if 16.6667ms (60 FPS) have elapsed since the last frame
      if (SDL_GetTicks() - ticks >= (1000 / setFrameRate))
      {
         // Print the frame rate every 10 seconds
         Uint32 currentTicks = SDL_GetTicks();
         if (currentTicks - frameRateClock >= 10000) {
            float frameRate = 1000 * (float) zframeCount / (currentTicks - frameRateClock);
            printf("Frame rate: %f fps\n", frameRate);
            zframeCount = 0;
            frameRateClock = currentTicks;
         }

         if (xloop || frameCount == 0) {
            anything_drawn = false;
            draw();
            // Update the screen
            if (anything_drawn) {
               // Set the default render target
               SDL_SetRenderTarget(renderer, NULL);
               SDL_RenderCopy(renderer, backBuffer, NULL, NULL);
               SDL_SetRenderTarget(renderer, backBuffer);
               SDL_RenderPresent(renderer);
               frameCount++;
               zframeCount++;
            } else {
               SDL_Delay(5);
            }
         } else {
            SDL_Delay(5);
         }
         ticks = SDL_GetTicks();
}   }

   // TTF_CloseFont(font);

   // Destroy the window and renderer
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);

   // Quit SDL
   SDL_Quit();
   return 0;
};

#endif
