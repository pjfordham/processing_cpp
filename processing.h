#ifndef PROCESSING_H
#define PROCESSING_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <map>

#include "PerlinNoise.h"
#include "weak.h"
#include "processing_math.h"
#include "processing_java_compatability.h"

SDL_Texture* backBuffer;

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


std::vector<Matrix2D> matrix_stack;
Matrix2D current_matrix = Matrix2D::Identity();

PerlinNoise perlin_noise;
int perlin_octaves = 4 ;
float perlin_falloff = 0.5;

void noiseSeed(int seed) {
   perlin_noise = PerlinNoise(seed);
}

void noiseDetail(int lod, float falloff) {
   perlin_octaves = lod;
   perlin_falloff = falloff;
}

float noise(float x, float y = 0, float z = 0) {
   return perlin_noise.octave(x,y,z,perlin_octaves,perlin_falloff);
}

void pushMatrix() {
   matrix_stack.push_back(current_matrix);
}

void popMatrix() {
   current_matrix = matrix_stack.back();
   matrix_stack.pop_back();
}

void translate(float x, float y) {
   current_matrix = current_matrix.multiply( Matrix2D::translate(x,y) );
}

void scale(float x, float y) {
   current_matrix = current_matrix.multiply( Matrix2D::scale(x,y) );
}

void scale(float x) {
   scale(x,x);
}

void rotate(float angle) {
   current_matrix = current_matrix.multiply( Matrix2D::rotate(angle) );
}

bool anything_drawn = false;

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

SDL_Color stroke_color{255,255,255,255};
SDL_Color fill_color{255,255,255,255};

int xstrokeWeight = 1;

enum{
   POINTS = 0,
   LINES = 1,
};
enum{
   OPEN = 0,
   CLOSE = 1,
};


void DrawAntialiasedCircle(SDL_Renderer* renderer, int x, int y, int r) {
   anything_drawn = true;
   // Draw circle pixels using antialiasing
   for (float i = x - r; i < x + r; i += 0.5) {
      for (float j = y - r; j < y + r; j += 0.5) {
         float distance = dist(i,j,x,y);
         if (distance >= r - 1 && distance <= r) {
            float alpha = 1 - (r - distance);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<int>(alpha * 255));
            SDL_RenderDrawPoint(renderer, i, j);
         } else if (distance < r - 1) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, i, j);
         }
      }
   }
}
// Create a texture to render to
SDL_Texture* ellipse_texture;

void setup_ellipse_texture() {
   ellipse_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1000,1000);
   SDL_SetTextureBlendMode(ellipse_texture, SDL_BLENDMODE_BLEND);
   SDL_SetRenderTarget(renderer, ellipse_texture);
   SDL_SetRenderDrawColor(renderer,0,0,0,0);
   SDL_RenderFillRect(renderer, NULL);
   DrawAntialiasedCircle(renderer, 500,500,500);
}

void destroy_ellipse_texture() {
   SDL_DestroyTexture(ellipse_texture);
}

void ellipse(float x, float y, float width, float height) {
   anything_drawn = true;
   if (xellipse_mode != RADIUS ) {
      width /=2;
      height /=2;
   }

   PVector translation = current_matrix.get_translation();
   PVector scale = current_matrix.get_scale();
   float angle = current_matrix.get_angle();

   SDL_SetTextureColorMod(ellipse_texture, fill_color.r,fill_color.g,fill_color.b);
   SDL_SetTextureAlphaMod(ellipse_texture, fill_color.a);

   SDL_Rect  dstrect = {translation.x-width+x,translation.y+y-height,+width*2*scale.x,height*2*scale.y};

   SDL_Point pos{width-x,height-y};

   SDL_RenderCopyEx(renderer,ellipse_texture,NULL,&dstrect, -angle * 180 /M_PI, &pos,SDL_FLIP_NONE);

}

void line(float x1, float y1, float x2, float y2) {
   anything_drawn = true;
   PVector s = current_matrix.multiply(PVector{x1,y1});
   PVector f = current_matrix.multiply(PVector{x2,y2});
   if (xstrokeWeight == 1) {
      SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
      SDL_RenderDrawLine(renderer, s.x, s.y, f.x, f.y);
   } else {
      drawRoundedLine(s.x, s.y, f.x, f.y, xstrokeWeight, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   }
}

void point(float x, float y) {
   anything_drawn = true;
   PVector p = current_matrix.multiply(PVector{x,y});
   SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   SDL_RenderDrawPoint(renderer, p.x, p.y);
}

void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
   anything_drawn = true;
   PVector p1 = current_matrix.multiply(PVector{x1,y1});
   PVector p2 = current_matrix.multiply(PVector{x2,y2});
   PVector p3 = current_matrix.multiply(PVector{x3,y3});
   PVector p4 = current_matrix.multiply(PVector{x4,y4});
   Sint16 xs[] = {p1.x,p2.x,p3.x,p4.x};
   Sint16 ys[] = {p1.y,p2.y,p3.y,p4.y};
   filledPolygonRGBA(renderer,xs,ys,4,fill_color.r,fill_color.g,fill_color.b,fill_color.a);
}


std::vector<PVector> shape;

int shape_style = LINES;

void beginShape(int points = LINES) {
   shape_style = points;
   shape.clear();
}

void vertex(float x, float y) {
   shape.push_back({x, y});
}

void endShape(int type = OPEN) {
   anything_drawn = true;

   if (shape_style == POINTS) {
      for (auto z : shape ) {
         auto p = current_matrix.multiply( z );
         if (xstrokeWeight == 1) {
            SDL_SetRenderDrawColor(renderer, stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
            SDL_RenderDrawPoint(renderer, p.x, p.y);
         } else {
            filledEllipseRGBA(renderer, p.x, p.y, xstrokeWeight/2, xstrokeWeight/2,
                              stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
         }
      }
   } else if (type == CLOSE) {
      std::vector<Sint16> xs, ys;
      for (auto z : shape ) {
         auto p = current_matrix.multiply( z );
         xs.push_back(p.x);
         ys.push_back(p.y);
      }

      filledPolygonRGBA(renderer,xs.data(),ys.data(),xs.size(),
                        fill_color.r,fill_color.g,fill_color.b,fill_color.a);
      polygonRGBA(renderer,xs.data(),ys.data(),xs.size(),
                  stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a);
   } else if (shape_style == LINES) {
      for (std::size_t i = 1; i < shape.size(); ++i) {
         line(shape[i-1].x, shape[i-1].y, shape[i].x, shape[i].y);
      }
      if (type == CLOSE) {
         line(shape[shape.size()-1].x, shape[shape.size()-1].y, shape[0].x, shape[0].y);
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

class color {
 public:
   float r,g,b,a;
   color(float _r, float _g, float _b,float _a) : r(_r), g(_g), b(_b), a(_a) {
   }
   color(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(xcolorScaleA) {
   }
   color(float _r) : r(_r), g(_r), b(_r), a(xcolorScaleA) {
   }
   color()  {
   }
   operator unsigned int() {
      return
         ((unsigned char)a << 24) |
         ((unsigned char)r << 16) |
         ((unsigned char)g <<  8) |
         ((unsigned char)b <<  0);
   }
};


color lerpColor(const color& c1, const color& c2, float amt) {
    float r = c1.r + (c2.r - c1.r) * amt;
    float g = c1.g + (c2.g - c1.g) * amt;
    float b = c1.b + (c2.b - c1.b) * amt;
    float a = c1.a + (c2.a - c1.a) * amt;
    return color(r, g, b, a);
}

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

void stroke(float r,float g,  float b, float a) {
   stroke_color = flatten_color_mode(r,g,b,a);
}

void stroke(float r,float g, float b) {
   stroke(r,g,b,xcolorScaleA);
}

void stroke(float r,float a) {
   if (xcolorMode == HSB) {
      stroke(0,0,r,a);
   } else {
      stroke(r,r,r,a);
   }
}

void stroke(float r) {
   if (xcolorMode == HSB) {
      stroke(r,0,0,xcolorScaleA);
   } else {
      stroke(r,r,r,xcolorScaleA);
   }
}

void stroke(color c) {
   stroke(c.r,c.g,c.b,c.a);
}

void strokeWeight(int x) {
   xstrokeWeight = x;
}

void noStroke() {
   stroke_color = {0,0,0,0};
}

void noFill() {
   fill_color = {0,0,0,0};
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

void background(float r, float g, float b) {
   anything_drawn = true;
   auto color = flatten_color_mode(r,g,b,xcolorScaleA);
   SDL_SetRenderDrawColor(renderer, color.r,color.g,color.b, color.a);
   SDL_RenderClear(renderer);
}

void background(float gray) {
   if (xcolorMode == HSB) {
      background(0,0,gray);
   } else {
      background(gray,gray,gray);
   }
};


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

   void setup_rect_texture();
   setup_rect_texture();
   void setup_ellipse_texture();
   setup_ellipse_texture();

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
   if (xcolorMode == HSB) {
      fill(0,0,r,a);
   } else {
      fill(r,r,r,a);
   }
}

void fill(float r) {
   if (xcolorMode == HSB) {
      fill(0,0,r,xcolorScaleA);
   } else {
      fill(r,r,r,xcolorScaleA);
   }
}

void fill(class color color) {
   fill(color.r,color.g,color.b,color.a);
}

// Create a texture to render to
SDL_Texture* rect_texture;

void setup_rect_texture() {
   rect_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1,1);
   SDL_SetTextureBlendMode(rect_texture, SDL_BLENDMODE_BLEND);
   // Set the render target to the texture
   SDL_SetRenderTarget(renderer, rect_texture);
   SDL_SetRenderDrawColor(renderer,255,255,255,255);
   SDL_RenderFillRect(renderer, NULL);
}

void destroy_rect_texture() {
   SDL_DestroyTexture(rect_texture);
}

void rect(int x, int y, int width, int height) {
   anything_drawn = true;
   if (xrect_mode == CORNERS) {
      width = width -x;
      height = height - y;
   } else if (xrect_mode == CENTER) {
      x = x - width / 2;
      y = y - height / 2;
   } else if (xrect_mode == RADIUS) {
      width *= 2;
      height *= 2;
      x = x - width / 2;
      y = y - height / 2;
   }

   PVector translation = current_matrix.get_translation();
   PVector scale = current_matrix.get_scale();
   float angle = current_matrix.get_angle();

   SDL_SetTextureColorMod(rect_texture, fill_color.r,fill_color.g,fill_color.b);
   SDL_SetTextureAlphaMod(rect_texture, fill_color.a);

   SDL_Rect dstrect = {translation.x+scale.x*x,translation.y+scale.y*y,width*scale.x,height*scale.y};
   SDL_RenderCopyEx(renderer,rect_texture,NULL,&dstrect, -angle * 180 /M_PI, NULL,SDL_FLIP_NONE);

   rectangleRGBA(renderer,
                 translation.x+scale.x*x,
                 translation.y+scale.y*y,
                 translation.x+scale.x*x + width*scale.x,
                 translation.y+scale.y*y + height*scale.y,
                 stroke_color.r,stroke_color.g,stroke_color.b,stroke_color.a );
}


typedef std::pair<const char *, int> PFont;
PFont currentFont(NULL,0);

void textFont(PFont font) {
   currentFont = font;
}

std::map<PFont, TTF_Font *> fontMap;

PFont createFont(const char *filename, int size) {
   auto key = std::make_pair(filename,size);
   if (fontMap.count(key) == 0) {
      auto font = TTF_OpenFont(filename, size);
      if (font == NULL) {
         printf("TTF_OpenFont failed: %s\n", TTF_GetError());
         abort();
      }
      fontMap[key] = font;
   }
   return key;
}

void textSize(int size) {
   currentFont = createFont(currentFont.first, size);
}

void text(std::string text, int x, int y, int width=-1, int height=-1) {
   SDL_Surface* surface = TTF_RenderText_Blended(fontMap[currentFont], text.c_str(),
                                                 (SDL_Color){ stroke_color.r,
                                                    stroke_color.g, stroke_color.b,
                                                    stroke_color.a });
   if (surface == NULL) {
      printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
      abort();
   }

   SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
   if (texture == NULL) {
      printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
      abort();
   }

   // Get dimensions of text;
   int textWidth = surface->w;
   int textHeight = surface->h;

   SDL_Rect text_rect = {x, y, textWidth, textHeight};
   SDL_RenderCopy(renderer, texture, NULL, &text_rect);
   SDL_DestroyTexture(texture);
   SDL_FreeSurface(surface);
}

int frameCount = 0;
int zframeCount = 0;
int mouseX = 0;
int mouseY = 0;

char key = 0;

int main()
{
   // Initialize SDL
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
      return 1;
   }

   TTF_Font* font = NULL;
   if (TTF_Init() != 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init failed: %s\n", TTF_GetError());
      abort();
   }

   setup();

   Uint32 clock = SDL_GetTicks();
   Uint32 frameRateClock = clock;
   bool quit = false;

   // Set the initial tick count
   Uint32 ticks = SDL_GetTicks();

   bool dragging = false;
   while (!quit) {

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         if (event.type == SDL_QUIT) {
            quit = true;
         } else if (event.type == SDL_MOUSEMOTION ) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            if (dragging) {
               mouseDragged();
            }
         } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
               mousePressed();
               dragging = true;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
               mouseReleased();
               dragging = false;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
            }
         } else if (event.type == SDL_MOUSEWHEEL) {
         } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
               quit = true;
               break;
            }
            // Get the key code from the event
            SDL_Keycode keycode = event.key.keysym.sym;

            // Check if any of the modifier keys are pressed
            SDL_Keymod mod_state = SDL_GetModState();
            bool shift_pressed = mod_state & KMOD_SHIFT;

            // Convert the key code to a string
            const char* keyname = SDL_GetKeyName(keycode);

            key = 1;
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
               keyTyped();
            } else if (keyname[0] == 'S' && keyname[1] == 'p') {
               key = ' ';
               keyTyped();
            }
            keyPressed();
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
            current_matrix = Matrix2D::Identity();
            draw();
            // Update the screen
            if (anything_drawn) {
               // Set the default render target
               SDL_SetRenderTarget(renderer, NULL);
               SDL_RenderCopy(renderer, backBuffer, NULL, NULL);
               SDL_SetRenderTarget(renderer, backBuffer);
               SDL_RenderPresent(renderer);
               anything_drawn = false;
               frameCount++;
               zframeCount++;
            } else {
               SDL_Delay(5);
            }
         } else {
            SDL_Delay(5);
         }
         ticks = SDL_GetTicks();
      }
   }

   for (auto font : fontMap) {
      TTF_CloseFont(font.second);
   }

   destroy_rect_texture();
   destroy_ellipse_texture();
   // Destroy the window and renderer
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);

   // Quit SDL
   SDL_Quit();
   return 0;
};

#endif
