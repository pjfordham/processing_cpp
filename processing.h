#ifndef PROCESSING_H
#define PROCESSING_H


#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_opengl_shaders.h"

#include <Eigen/Dense>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <map>
#include <fmt/core.h>

#include "weak.h"
#include "processing_math.h"
#include "processing_transforms.h"
#include "processing_color.h"
#include "processing_java_compatability.h"
#include "processing_opengl.h"
#include "processing_pimage.h"
#include "processing_pshape.h"
#include "processing_pgraphics.h"

SDL_Window *window;
SDL_Renderer *renderer;

Eigen::Matrix4f get_projection_matrix(float fov, float a, float near, float far) {
   float f = 1 / tan(0.5 * fov);
   float rangeInv = 1.0 / (near - far);
   float A = (near + far) * rangeInv;
   float B = near * far * rangeInv * 2;
   Eigen::Matrix4f ret = Eigen::Matrix4f{
      { f/a,  0,  0,  0 },
      {   0,  f,  0,  0 },
      {   0,  0,  A,  B },
      {   0,  0, -1,  0 }
   };
   return ret;
}

Eigen::Matrix4f projection_matrix; // Default is identity
Eigen::Matrix4f view_matrix; // Default is identity

GLuint Color;

GLuint programID;
GLuint Pmatrix;
GLuint Vmatrix;

void drawGeometry( const std::vector<float> &vertices,
                   const std::vector<float> &normals,
                   const std::vector<unsigned short> &triangles,
                   const std::vector<float> &colors) {

   GLuint VAO;
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   GLuint indexbuffer;
   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned short), triangles.data(), GL_STATIC_DRAW);

   GLuint normalbuffer;
   glGenBuffers(1, &normalbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
   glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

   GLuint colorbuffer;
   glGenBuffers(1, &colorbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
   glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);

   GLuint attribId = glGetAttribLocation(programID, "position");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(programID, "color");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(programID, "normal");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_SHORT, 0);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);
   glDeleteBuffers(1, &normalbuffer);
   glDeleteBuffers(1, &colorbuffer);

   // Unbind the buffer objects and VAO
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

}

void box(float w, float h, float d) {
   w = w / 2;
   h = h / 2;
   d = d / 2;
   std::vector<float> vertices = {
      // Front face
      -w, -h, d,
      w, -h, d,
      w, h, d,
      -w, h, d,

      // Back face
      -w, -h, -d,
      -w, h, -d,
      w, h, -d,
      w, -h, -d,

      // Top face
      -w, h, -d,
      -w, h, d,
      w, h, d,
      w, h, -d,

      // Bottom face
      -w, -h, -d,
      w, -h, -d,
      w, -h, d,
      -w, -h, d,

      // Right face
      w, -h, -d,
      w, h, -d,
      w, h, d,
      w, -h, d,

      // Left face
      -w, -h, -d,
      -w, -h, d,
      -w, h, d,
      -w, h, -d,
   };

   std::vector<float> normals = {
      // Front
      0.0,  0.0,  1.0,
      0.0,  0.0,  1.0,
      0.0,  0.0,  1.0,
      0.0,  0.0,  1.0,

      // Back
      0.0,  0.0, -1.0,
      0.0,  0.0, -1.0,
      0.0,  0.0, -1.0,
      0.0,  0.0, -1.0,

      // Top
      0.0,  1.0,  0.0,
      0.0,  1.0,  0.0,
      0.0,  1.0,  0.0,
      0.0,  1.0,  0.0,

      // Bottom
      0.0, -1.0,  0.0,
      0.0, -1.0,  0.0,
      0.0, -1.0,  0.0,
      0.0, -1.0,  0.0,

      // Right
      1.0,  0.0,  0.0,
      1.0,  0.0,  0.0,
      1.0,  0.0,  0.0,
      1.0,  0.0,  0.0,

      // Left
      -1.0,  0.0,  0.0,
      -1.0,  0.0,  0.0,
      -1.0,  0.0,  0.0,
      -1.0,  0.0,  0.0
   };

   std::vector<unsigned short>  triangles = {
      0,1,2, 0,2,3, 4,5,6, 4,6,7,
      8,9,10, 8,10,11, 12,13,14, 12,14,15,
      16,17,18, 16,18,19, 20,21,22, 20,22,23
   };

   std::vector<float> colors;

   for (int i = 0; i< vertices.size(); ++i ) {
      colors.push_back(PShape::fill_color.r / 255.0);
      colors.push_back(PShape::fill_color.g / 255.0);
      colors.push_back(PShape::fill_color.b / 255.0);
   }

   drawGeometry(vertices, normals, triangles, colors);
};

void box(float size) {
   box(size, size, size);
}

float xsphere_ures = 30;
float xsphere_vres = 30;

void sphereDetail(float ures, float vres) {
   xsphere_ures = ures;
   xsphere_vres = vres;
}

void sphereDetail(float res) {
   sphereDetail(res, res);
}

void sphere(float radius) {

   std::vector<float> vertices;
   std::vector<float> normals;
   std::vector<float> colors;

   float latStep = M_PI / xsphere_ures;
   float lonStep = 2 * M_PI / xsphere_vres;

   for (int i = 0; i <= xsphere_ures; i++) {
      float lat = i * latStep;
      float cosLat = std::cos(lat);
      float sinLat = std::sin(lat);

      for (int j = 0; j <= xsphere_vres; j++) {
         float lon = j * lonStep;
         float cosLon = std::cos(lon);
         float sinLon = std::sin(lon);

         float x = sinLat * cosLon;
         float y = sinLat * sinLon;
         float z = cosLat;

         normals.push_back( x );
         normals.push_back( y );
         normals.push_back( z );
         vertices.push_back( x * radius);
         vertices.push_back( y * radius);
         vertices.push_back( z * radius);
         colors.push_back(PShape::fill_color.r / 255.0);
         colors.push_back(PShape::fill_color.g / 255.0);
         colors.push_back(PShape::fill_color.b / 255.0);
      }
   }

   std::vector<unsigned short> indices;
   for (int i = 0; i < xsphere_ures; i++) {
      for (int j = 0; j < xsphere_vres; j++) {
         int idx0 = i * (xsphere_vres+1) + j;
         int idx1 = idx0 + 1;
         int idx2 = (i+1) * (xsphere_vres+1) + j;
         int idx3 = idx2 + 1;
         indices.push_back(idx0);
         indices.push_back(idx2);
         indices.push_back(idx1);
         indices.push_back(idx1);
         indices.push_back(idx2);
         indices.push_back(idx3);
      }
   }

   drawGeometry(vertices, normals, indices, colors);
}



// ----
// Begin shapes managed by Pshape.
// ----
PGraphics g;

#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args... args) { return instance.method(args...); }

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

MAKE_GLOBAL(fill, g);
MAKE_GLOBAL(noFill, g);
MAKE_GLOBAL(noStroke, g);
MAKE_GLOBAL(stroke, g);
MAKE_GLOBAL(strokeWeight, g);
MAKE_GLOBAL(strokeCap, g);
MAKE_GLOBAL(ellipseMode, g);
MAKE_GLOBAL(rectMode, g);
MAKE_GLOBAL(noSmooth,g);


PShape _shape;

void beginShape(int points = POLYGON) {
   _shape = PShape();
   _shape.beginShape(points);
}

void vertex(float x, float y, float z = 0.0) {
   _shape.vertex(x, y, z);
}

void endShape(int type = OPEN) {
   _shape.endShape(type);
   _shape.draw();
}
// ----
// End shapes managed by Pshape.
// ----


int setFrameRate = 60;
void frameRate(int rate) {
   setFrameRate = rate;
}


template <typename Container>
typename Container::value_type random(const Container& c) {
   auto random_it = std::next(std::begin(c), random(std::size(c)));
   return *random_it;
}

template <typename T>
T random(const std::initializer_list<T>& c) {
   auto random_it = std::next(std::begin(c), random(std::size(c)));
   return *random_it;
}

color lerpColor(const color& c1, const color& c2, float amt) {
   return {
      c1.r + (c2.r - c1.r) * amt,
      c1.g + (c2.g - c1.g) * amt,
      c1.b + (c2.b - c1.b) * amt,
      c1.a + (c2.a - c1.a) * amt};
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

int width = 0;
int height = 0;

using std::min;
SDL_GLContext glContext = NULL;

std::array<float,3> xambientLight;
std::array<float,3> xdirectionLightColor;
std::array<float,3> xdirectionLightVector;

GLuint AmbientLight;
GLuint DirectionLightColor;
GLuint DirectionLightVector;

void directionalLight(float r, float g, float b, float nx, float ny, float nz) {
   xdirectionLightColor = {r/255.0f, r/255.0f, r/255.0f};
   xdirectionLightVector = {nx, ny, nz};
   glUniform3fv(DirectionLightColor, 1, xdirectionLightColor.data() );
   glUniform3fv(DirectionLightVector, 1,xdirectionLightVector.data() );
}

void ambientLight(float r, float g, float b) {
   xambientLight = { r/255.0f, g/255.0f, b/255.0f };
   glUniform3fv(AmbientLight, 1, xambientLight.data() );
}

void lights() {
   ambientLight(128, 128, 128);
   directionalLight(128, 128, 128, 0, 0, -1);
   //lightFalloff(1, 0, 0);
   //lightSpecular(0, 0, 0);
};

void noLights() {
   ambientLight(255.0, 255.0, 255.0);
   directionalLight(0.0,0.0,0.0, 0.0,0.0,-1.0);
}

void ortho(float left, float right, float bottom, float top, float near, float far) {
   float tx = -(right + left) / (right - left);
   float ty = -(top + bottom) / (top - bottom);
   float tz = -(far + near) / (far - near);

   projection_matrix = Eigen::Matrix4f{
      { 2/(right-left),               0,              0,  tx },
      {              0,  2/(top-bottom),              0,  ty },
      {              0,               0, -2/(far - near), tz },
      {              0,               0,              0,   1 }
   };
   glUniformMatrix4fv(Pmatrix, 1,false, projection_matrix.data());
}

void ortho(float left, float right, float bottom, float top) {
   ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0,-10,10);
}

void ortho() {
   ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0);
}

void perspective(float angle, float aspect, float minZ, float maxZ) {
   projection_matrix = get_projection_matrix(angle, aspect, minZ, maxZ);
   glUniformMatrix4fv(Pmatrix, 1,false, projection_matrix.data());
}

void perspective() {
   float fov = PI/3.0;
   float cameraZ = (height/2.0) / tan(fov/2.0);
   perspective( fov, (float)width/(float)height, cameraZ/10.0,  cameraZ*10.0);
}

void camera( float eyeX, float eyeY, float eyeZ,
             float centerX, float centerY, float centerZ,
             float upX, float upY, float upZ ) {

   Eigen::Vector3f center = Eigen::Vector3f{centerX, centerY, centerZ};
   Eigen::Vector3f eye =  Eigen::Vector3f{eyeX,eyeY,eyeZ};
   Eigen::Vector3f _up = Eigen::Vector3f{upX,upY,upZ};

   Eigen::Vector3f forward = (center - eye).normalized();
   Eigen::Vector3f side = forward.cross(_up).normalized();
   Eigen::Vector3f up = side.cross(forward).normalized();

   Eigen::Matrix4f view{
      {    side[0],     side[1],     side[2], 0.0f},
      {      up[0],       up[1],       up[2], 0.0f},
      {-forward[0], -forward[1], -forward[2], 0.0f},
      {       0.0f,        0.0f,        0.0f, 1.0f} };

   Eigen::Matrix4f translate{
      {1.0,    0,     0,    -eyeX} ,
      {0,    1.0,     0,    -eyeY},
      {0,      0,   1.0,    -eyeZ},
      {0.0f, 0.0f,  0.0f,    1.0f} };

   // Translate the camera to the origin
   view_matrix = view * translate;
   glUniformMatrix4fv(Vmatrix, 1,false, view_matrix.data());
}

void camera() {
   camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
          width / 2.0, height / 2.0, 0, 0, 1, 0);
}

GLuint flatTextureShader;

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
   SDL_GLContext glContext = SDL_GL_CreateContext(window);
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

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   if (mode == P2D) {
      glDisable(GL_DEPTH_TEST);
      programID = LoadShaders(ShadersFlat());
   } else {
      glEnable(GL_DEPTH_TEST);
      programID = LoadShaders(Shaders3D());
   }
   glUseProgram(programID);

   // Get a handle for our "MVP" uniform
   Pmatrix = glGetUniformLocation(programID, "Pmatrix");
   Vmatrix = glGetUniformLocation(programID, "Vmatrix");
   Mmatrix = glGetUniformLocation(programID, "Mmatrix");
   Color = glGetUniformLocation(programID, "color");

   AmbientLight = glGetUniformLocation(programID, "ambientLight");
   DirectionLightColor = glGetUniformLocation(programID, "directionLightColor");
   DirectionLightVector = glGetUniformLocation(programID, "directionLightVector");

   flatTextureShader = LoadShaders(ShadersFlatTexture());

   if (!glewIsSupported("GL_EXT_framebuffer_object")) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
      abort();
   }

   g = PGraphics(width, height, mode);


   if (mode == P2D) {
      view_matrix = TranslateMatrix(PVector{-1,-1,0}) * ScaleMatrix(PVector{2.0f/width, 2.0f/height,1.0});
      projection_matrix = Eigen::Matrix4f::Identity();
      glUniformMatrix4fv(Vmatrix, 1,false, view_matrix.data());
      glUniformMatrix4fv(Pmatrix, 1, false, projection_matrix.data());
   } else {
      perspective();
      camera();
   }

   background(WHITE);
}


Uint32 *pixels; // pointer to the texture's pixel data in the desired format

void loadPixels() {
   g.loadPixels();
   pixels = g.pixels.data();
}

MAKE_GLOBAL(updatePixels, g);

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

int xTextAlign;
int yTextAlign;

void textAlign(int x, int y) {
   xTextAlign = x;
   yTextAlign = y;
}

void textAlign(int x) {
   xTextAlign = x;
}

void textSize(int size) {
   currentFont = createFont(currentFont.first, size);
}

void text(std::string text, float x, float y, float width=-1, float height=-1) {
   SDL_Surface* surface = TTF_RenderText_Blended(fontMap[currentFont], text.c_str(),
                                                 { (unsigned char)PShape::fill_color.r,
                                                   (unsigned char)PShape::fill_color.g,
                                                   (unsigned char)PShape::fill_color.b,
                                                   (unsigned char)PShape::fill_color.a });
   if (surface == NULL) {
      printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
      abort();
   }
   SDL_Surface* surface2 = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_BGRA8888, 0);
   if (surface2 == NULL) {
      abort();
   }

   width = surface2->w;
   height = surface2->h;

   // this works well enough for the Letters.cc example but it's not really general
   if ( xTextAlign == CENTER ) {
      x = x - width / 2;
   }
   if ( yTextAlign == CENTER ) {
      y = y - height / 2;
   }

   glTexturedQuad({x,y},{x+width,y},{x+width,y+height}, {x,y+height}, surface2);

   SDL_FreeSurface(surface);
   SDL_FreeSurface(surface2);
}

void text(char c, float x, float y, float width = -1, float height = -1) {
   std::string s(&c,1);
   text(s,x,y,width,height);
}

void imageMode(int iMode) {
   PImage::mode = iMode;
}

// We can add a tint color to the texture shader.
void tint(color tint) {}

void tint(color tint, float alpha) {}

void noTint() {
   PImage::tint = WHITE;
}

void image(const PImage &pimage, float left, float top, float right, float bottom) {
   if ( PImage::mode == CORNER ) {
      float width = right;
      float height = bottom;
      right = left + width;
      bottom = top + height;
   } else if ( PImage::mode == CENTER ) {
      float width = right;
      float height = bottom;
      left = left - ( width / 2.0 );
      top = top - ( height / 2.0 );
      right = left + width;
      bottom = top + height;
   }
   glTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom}, pimage.surface);
}

void image(const PImage &pimage, float x, float y) {
   if ( PImage::mode == CORNER ) {
      image( pimage, x, y, pimage.width, pimage.height );;
   } else if ( PImage::mode == CORNERS ) {
      image( pimage, x, y, x + pimage.width, y + pimage.height );;
   } else   if (PImage::mode == CENTER) {
      image( pimage, x, y, pimage.width, pimage.height );
   } else {
      abort();
   }
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

   // initialize SDL_image
   if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_Init JPG failed: %s\n", TTF_GetError());
      abort();
   }

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
            move_matrix = Eigen::Matrix4f::Identity();
            glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
            noLights();

            draw();

            // Only update once per frame so we don't miss positions
            pmouseX = mouseX;
            pmouseY = mouseY;

            // bind the real frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Clear the color and depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Draw the flipped PGraphics context
            g.draw(0,0,true);

            SDL_GL_SwapWindow(window);

            // Update the screen
            if (anything_drawn) {
               // Set the default render target
               // Swap buffers
               // SDL_SetRenderTarget(renderer, NULL);
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
