#ifndef PROCESSING_H
#define PROCESSING_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_opengl.h>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <map>
#include <fmt/core.h>

#include "PerlinNoise.h"
#include "weak.h"
#include "processing_math.h"
#include "processing_java_compatability.h"
#include "processing_opengl.h"

bool render_to_backbuffer = true;

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

Matrix3D get_projection_matrix(float angle, float a, float zMin, float zMax) {
   float ang = tan(angle*.5);
   return {
        0.5f/ang, 0 , 0, 0,
        0, 0.5f*a/ang, 0, 0,
        0, 0, -(zMax+zMin)/(zMax-zMin), -1,
        0, 0, (-2*zMax*zMin)/(zMax-zMin), 0
   };
}

std::vector<Matrix3D> matrix_stack;
Matrix3D move_matrix; // Default is identity
Matrix3D projection_matrix; // Default is identity
Matrix3D view_matrix; // Default is identity

void glTransform() {
   auto transform = projection_matrix * view_matrix * move_matrix;
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(transform.data());
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
}

void glTransformClear() {
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
}

void pushMatrix() {
   matrix_stack.push_back(move_matrix);
}

void popMatrix() {
   move_matrix = matrix_stack.back();
   matrix_stack.pop_back();
   glTransform();
}

void translate(float x, float y, float z=0) {
   move_matrix = move_matrix * Matrix3D::translate(PVector{x,y,z});
   glTransform();
}

void scale(float x, float y,float z = 1) {
   move_matrix = move_matrix * Matrix3D::scale(PVector{x,y,z});
   glTransform();
}

void scale(float x) {
   scale(x,x,x);
}

void rotate(float angle, PVector axis) {
   move_matrix = move_matrix * Matrix3D::rotate(angle,axis);
   glTransform();
}


void rotate(float angle) {
   move_matrix = move_matrix * Matrix3D::rotate(angle,PVector{0,0,1});
   glTransform();
}

void rotateY(float angle) {
   move_matrix = move_matrix * Matrix3D::rotate(angle,PVector{0,1,0});
   glTransform();
}

void rotateX(float angle) {
   move_matrix = move_matrix * Matrix3D::rotate(angle,PVector{1,0,0});
   glTransform();
}

int xellipse_mode = DIAMETER;
int xrect_mode = CORNER;

void ellipseMode(int mode) {
   xellipse_mode = mode;
}

void rectMode(int mode){
   xrect_mode = mode;
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

enum {
   ROUND = 0,
   SQUARE,
   PROJECT,
};

int xendCap = ROUND;
bool xSmoothing = true;

void noSmooth() {
   // Doesn't yet apply to actual graphics
   xSmoothing = false;
}

void ellipse(float x, float y, float width, float height) {
   if (xellipse_mode != RADIUS ) {
      width /=2;
      height /=2;
   }
   glFilledEllipse(PVector{x,y}, width, width, 0,TWO_PI,fill_color);
   glLineEllipse(PVector{x,y}, width, width, 0,TWO_PI,stroke_color, xstrokeWeight);
}

void ellipse(float x, float y, float radius) {
   ellipse(x, y, radius, radius);
}

void arc(float x, float y, float width, float height, float start, float stop) {
   if (xellipse_mode != RADIUS ) {
      width /=2;
      height /=2;
   }
   glFilledEllipse(PVector{x,y}, width, width, start,stop,fill_color);
   glLineEllipse(PVector{x,y}, width, width, start,stop,stroke_color, xstrokeWeight);
}

void strokeCap(int cap) {
   xendCap = cap;
}

void line(float x1, float y1, float x2, float y2) {
   if (xendCap == ROUND) {
      glRoundLine( PVector{x1,y1,0}, PVector{x2,y2,0}, stroke_color, xstrokeWeight );
   } else if (xendCap == SQUARE) {
      glLine( PVector{x1,y1,0}, PVector{x2,y2,0}, stroke_color, xstrokeWeight );
   } else if (xendCap == PROJECT) {
      // Untested implementation
      glCappedLine( PVector{x1,y1,0}, PVector{x2,y2,0}, stroke_color, xstrokeWeight );
   } else {
      abort();
   }
}

void line(float x1, float y1, float z1, float x2, float y2, float z2) {
   if (xendCap == ROUND) {
      glRoundLine( PVector{x1,y1,z1}, PVector{x2,y2,z2}, stroke_color, xstrokeWeight );
   } else if (xendCap == SQUARE) {
      glLine( PVector{x1,y1,z1}, PVector{x2,y2,z1}, stroke_color, xstrokeWeight );
   } else if (xendCap == PROJECT) {
      // Untested implementation
      glCappedLine( PVector{x1,y1,z1}, PVector{x2,y2,z1}, stroke_color, xstrokeWeight );
   } else {
      abort();
   }
}

void box(float w, float h, float d) {
  w = w / 2;
  h = h / 2;
  d = d / 2;
  PVector vertices[] = {
      // Front face
      {-w, -h, d},
      {w, -h, d},
      {w, h, d},
      {-w, h, d},

      // Back face
      {-w, -h, -d},
      {-w, h, -d},
      {w, h, -d},
      {w, -h, -d},

      // Top face
      {-w, h, -d},
      {-w, h, d},
      {w, h, d},
      {w, h, -d},

      // Bottom face
      {-w, -h, -d},
      {w, -h, -d},
      {w, -h, d},
      {-w, -h, d},

      // Right face
      {w, -h, -d},
      {w, h, -d},
      {w, h, d},
      {w, -h, d},

      // Left face
      {-w, -h, -d},
      {-w, -h, d},
      {-w, h, d},
      {-w, h, -d},
   };

   // Define the indices for each face of the box
   unsigned int indices[][3] = { // front face
     {0, 1, 2},
     {0, 2, 3},

     {4, 5, 6},
     {4, 6, 7},

     {8, 9, 10},
     {8, 10, 11},

     {12, 13, 14},
     {12, 14, 15},

     {16,17,18},
     {16,18,19},

     {20,21,22},
     {20,22,23}
   };

   SDL_Color colors[]= {
      SDL_Color{  0,  0,255,255},
      SDL_Color{  0,255,  0,255},
      SDL_Color{  0,255,255,255},
      SDL_Color{255,  0,  0,255},
      SDL_Color{255,  0,255,255},
      SDL_Color{255,255,  0,255},
      SDL_Color{255,255,255,255} };
      int i = 0;
   for( auto triangle : indices ) {
      PVector points[] = { vertices[triangle[0]],vertices[triangle[1]], vertices[triangle[2]] };
      glFilledPoly(3, points, colors[i] );
      i = (i + 1) % 7;
   }
}

void box(float size) {
   box(size, size, size);
}

void point(float x, float y) {
   glFilledEllipse(PVector{x,y},xstrokeWeight,xstrokeWeight, 0,TWO_PI,stroke_color);
}

void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
   PVector points[] = { PVector{x1,y1},PVector{x2,y2},PVector{x3,y3},PVector{x4,y4} };
   glFilledPoly(4,points, fill_color);
   glLinePoly(4, points, stroke_color, xstrokeWeight );
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

   if (shape_style == POINTS) {
      for (auto z : shape ) {
         glFilledEllipse(z, xstrokeWeight, xstrokeWeight, 0,TWO_PI,stroke_color);
      }
   } else if (type == CLOSE) {
      glFilledPoly( shape.size(), shape.data(), fill_color );
      glLinePoly( shape.size(), shape.data(), stroke_color, xstrokeWeight);
   } else if (shape_style == LINES) {
      glLinePoly( shape.size(), shape.data(), stroke_color, xstrokeWeight);
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

const color BLACK = color(0);
const color WHITE = color(255);
const color GRAY = color(127);
const color LIGHT_GRAY = color(192);
const color DARK_GRAY = color(64);
const color RED = color(255, 0, 0);
const color GREEN = color(0, 255, 0);
const color BLUE = color(0, 0, 255);
const color YELLOW = color(255, 255, 0);
const color CYAN = color(0, 255, 255);
const color MAGENTA = color(255, 0, 255);
color RANDOM_COLOR() {
   return color(random(255),random(255),random(255),255);
}

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

void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
   // Compute the Bezier curve points
   std::vector<PVector> curve;
   for (float t = 0; t <= 1; t += 0.01) {
      float t_ = 1 - t;
      float x = t_ * t_ * t_ * x1 + 3 * t_ * t_ * t * x2 + 3 * t_ * t * t * x3 + t * t * t * x4;
      float y = t_ * t_ * t_ * y1 + 3 * t_ * t_ * t * y2 + 3 * t_ * t * t * y3 + t * t * t * y4;
      curve.emplace_back(x, y);
   }
   glLines(curve.size(), curve.data(), stroke_color, xstrokeWeight);
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
   // Set clear color
   glClearColor(color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);
   // Clear screen
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void background(color c) {
   background(c.r,c.g,c.b);
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
SDL_GLContext glContext = NULL;

GLuint backBufferID;
GLuint fboID;

enum {
   P2D, P3D
};

void lights() {

  };

void perspective(float angle, float aspect, float minZ, float maxZ) {
   projection_matrix = get_projection_matrix(angle, aspect, minZ, maxZ);
   glTransform();
}

void perspective() {
   float fov = PI/3.0;
   float cameraZ = (height/2.0) / tan(fov/2.0);
   perspective( fov, (float)width/(float)height, 1.0f,100.0f);
}

void camera( float eyeX, float eyeY, float eyeZ,
             float centerX, float centerY, float centerZ,
             float upX, float upY, float upZ ) {

   PVector center = PVector{centerX, centerY, centerZ};
   PVector eye =  PVector{eyeX,eyeY,eyeZ};
   PVector up = PVector{upX,upY,upZ};

   PVector forward = center - eye;
   forward.normalize();

   auto right = forward.cross(up);
   right.normalize();

   up = right.cross(forward);
   up.normalize();

   view_matrix = Matrix3D(    right.x,    right.y,    right.z, 0 ,
                                 up.x,       up.y,       up.z, 0,
                           -forward.x, -forward.y, -forward.z, 0,
                                    0,         0 ,          0, 1 );

   // Translate the camera to the origin
   view_matrix = view_matrix * Matrix3D::translate( PVector{-eyeX,-eyeY, -eyeZ} );

   glTransform();
}


void size(int _width, int _height, int MODE = P2D) {
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

   // Create OpenGL context
   SDL_GLContext glContext = SDL_GL_CreateContext(window);
   if (glContext == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
      abort();
   }

   if (render_to_backbuffer) {
      // Initialize GLEW to load OpenGL extensions
      glewExperimental = GL_TRUE;
      GLenum glewError = glewInit();
      if (glewError != GLEW_OK)
      {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glew init error\n");
         abort();
      }

      if (!glewIsSupported("GL_EXT_framebuffer_object")) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
         abort();
      }

      // Create a framebuffer object
      glGenFramebuffers(1, &fboID);
      glBindFramebuffer(GL_FRAMEBUFFER, fboID);

      // Create a texture to render to
      glGenTextures(1, &backBufferID);
      glBindTexture(GL_TEXTURE_2D, backBufferID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backBufferID, 0);

      // Create a renderbuffer for the depth buffer
      GLuint depthBufferID;
      glGenRenderbuffers(1, &depthBufferID);
      glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

      // Attach the depth buffer to the framebuffer object
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   }

   // Set the clear color and depth values
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_BLEND);

   if (MODE == P2D) {
      view_matrix = Matrix3D();
      view_matrix = view_matrix * Matrix3D::translate(PVector{-1,-1,0});
      view_matrix = view_matrix * Matrix3D::scale(PVector{2.0f/width, 2.0f/height,1.0});
      projection_matrix = Matrix3D();
   } else {
      view_matrix = Matrix3D();
      perspective();
   }

   background(255);
}


std::vector<Uint32> _pixels;
Uint32 *pixels; // pointer to the texture's pixel data in the desired format

void loadPixels() {
   _pixels.resize(width*height);
   pixels =  _pixels.data();
   // Read the pixel data from the framebuffer into the array
   glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void updatePixels() {
   anything_drawn = true;
   // Write the pixel data to the framebuffer
   glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
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

void rect(int x, int y, int _width, int _height) {
   if (xrect_mode == CORNERS) {
      _width = _width -x;
      _height = _height - y;
   } else if (xrect_mode == CENTER) {
      x = x - _width / 2;
      y = y - _height / 2;
   } else if (xrect_mode == RADIUS) {
      _width *= 2;
      _height *= 2;
      x = x - _width / 2;
      y = y - _height / 2;
   }
   quad(x,y, x+_width,y,  x+_width,y+_height,  x,y+_height);
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

void text(std::string text, float x, float y, int width=-1, int height=-1) {
   SDL_Surface* surface = TTF_RenderText_Blended(fontMap[currentFont], text.c_str(),
                                                 (SDL_Color){ stroke_color.r,
                                                    stroke_color.g, stroke_color.b,
                                                    stroke_color.a });
   if (surface == NULL) {
      printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
      abort();
   }

   glTexturedQuad(PVector{x,y},
                  PVector{x +surface->w,y},
                  PVector{x+surface->w,y+surface->h},
                  PVector{x, y+ surface->h},
                  surface );

   SDL_FreeSurface(surface);
}

int frameCount = 0;
int zframeCount = 0;
int mouseX = 0;
int mouseY = 0;

char key = 0;
int keyCode = 0;

enum {
   LEFT = 37,
   RIGHT = 39,
   UP = 38,
   DOWN = 40,
};

bool mousePressedb = false;

int main(int argc, char* argv[]) {
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


   // Set OpenGL attributes
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
         } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
               quit = true;
               break;
            }
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
               keyTyped();
            } else if (keyname[0] == 'S' && keyname[1] == 'p') {
               key = ' ';
               keyCode = key;
               keyTyped();
            } else if (keyname[0] == 'L' && keyname[1] == 'e') {
               keyCode = LEFT;
            } else if (keyname[0] == 'R' && keyname[1] == 'i') {
               keyCode = RIGHT;
            } else if (keyname[0] == 'U' && keyname[1] == 'p') {
               keyCode = UP;
            } else if (keyname[0] == 'D' && keyname[1] == 'o') {
               keyCode = DOWN;
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

            glClear(GL_DEPTH_BUFFER_BIT);
            move_matrix = Matrix3D::identity();
            glTransform();

            draw();

            if (render_to_backbuffer) {
               glBindFramebuffer(GL_FRAMEBUFFER, 0);

               // Load the identity matrices
               glTransformClear();

               glEnable(GL_TEXTURE_2D);
               // Clear the color and depth buffers
               glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
               glBindTexture(GL_TEXTURE_2D, backBufferID);
               glDisable(GL_BLEND);

               GLfloat vertices[][2] = {
                  { -1.0f, -1.0f},
                  {  1.0f, -1.0f},
                  {  1.0f,  1.0f},
                  { -1.0f,  1.0f},
               };

               // Invert texture on y-axis to correct
               GLfloat texCoords[][2] = {
                  {0.0f, 1.0f},
                  {1.0f, 1.0f},
                  {1.0f, 0.0f},
                  {0.0f, 0.0f},
               };

               glBegin(GL_QUADS);
               glColor4f(1,1,1,1);
               for (int i = 0 ; i< 4; ++i ){
                  glTexCoord2f(texCoords[i][0], texCoords[i][1]);
                  glVertex2f(vertices[i][0],vertices[i][1]);
               }
               glEnd();
               glDisable(GL_TEXTURE_2D);
               glEnable(GL_BLEND);
            }

            SDL_GL_SwapWindow(window);

            if (render_to_backbuffer) {
               glBindFramebuffer(GL_FRAMEBUFFER, fboID);
               glBindTexture(GL_TEXTURE_2D, backBufferID);
            }

            // Update the screen
            if (anything_drawn) {
               // Set the default render target
               // Swap buffers
               // SDL_SetRenderTarget(renderer, NULL);
               // SDL_RenderCopy(renderer, backBuffer, NULL, NULL);
               // SDL_SetRenderTarget(renderer, backBuffer);
               // SDL_RenderPresent(renderer);
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

   // Clean up
   SDL_GL_DeleteContext(glContext);
   SDL_DestroyWindow(window);
   SDL_Quit();

   return 0;
}

#endif
