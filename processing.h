#ifndef PROCESSING_H
#define PROCESSING_H


#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <Eigen/Dense>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <map>
#include <fmt/core.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "weak.h"
#include "processing_math.h"
#include "processing_transforms.h"
#include "processing_color.h"
#include "processing_java_compatability.h"
#include "processing_pimage.h"
#include "processing_pshape.h"
#include "processing_pgraphics.h"
#include "processing_pfont.h"

FT_Library ft;
FT_Face face;

void init_freetype() {

   // Initialize FreeType
   FT_Error err = FT_Init_FreeType(&ft);
   if (err != 0) {
      printf("Failed to initialize FreeType\n");
      exit(EXIT_FAILURE);
   }
   FT_Int major, minor, patch;
   FT_Library_Version(ft, &major, &minor, &patch);
   // printf("FreeType's version is %d.%d.%d\n", major, minor, patch);

   // Load the TrueType font file
   FT_New_Face(ft, "./SourceCodePro-Regular.ttf", 0, &face);
   if (err != 0) {
      printf("Failed to load face\n");
      exit(EXIT_FAILURE);
   }

   // Set the character size
   FT_Set_Char_Size(face, 0, 16*64, 300, 300);
   if (err != 0) {
      printf("Failed to set char size\n");
      exit(EXIT_FAILURE);
   }

   // Load the glyph outline data
   FT_Load_Char(face, 'A', FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
   if (err != 0) {
      printf("Failed to load glyph outlie data\n");
      exit(EXIT_FAILURE);
   }

   std::vector<PVector> gvertices;
   std::vector<char> gtags;

   // Convert the glyph outline data into vertices
   for (int i = 0; i < face->glyph->outline.n_points; i++) {
      FT_Vector vec = face->glyph->outline.points[i];
      gvertices.emplace_back(PVector{vec.x / 64.0f , vec.y / 64.0f} );
      gtags.emplace_back(  face->glyph->outline.tags[i] );
   }

   // Render the vertices using OpenGL
   // glEnableClientState(GL_VERTEX_ARRAY);
   // glVertexPointer(2, GL_FLOAT, 0, &gvertices[0]);
   // glDrawArrays(GL_TRIANGLES, 0, gvertices.size() / 2);
   // glDisableClientState(GL_VERTEX_ARRAY);

   return;
}

void close_freetype() {
   // Cleanup
   FT_Done_Face(face);
   FT_Done_FreeType(ft);
   return;
};

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

GLuint Pmatrix;
GLuint Vmatrix;




// This is the global PGraphcs object that forms the top level canvas.
PGraphics g;

// This macro and it's following uses pull the API from PGraphcs into
// the global namespace dispatching to the PGraphics object g.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

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
   ortho(left, right, bottom, top, bottom*2, top*2);
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

   if (!glewIsSupported("GL_EXT_framebuffer_object")) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
      abort();
   }

   g = PGraphics(width, height, mode);

   // Get a handle for our "MVP" uniform
   Pmatrix = glGetUniformLocation(g.programID, "Pmatrix");
   Vmatrix = glGetUniformLocation(g.programID, "Vmatrix");
   Mmatrix = glGetUniformLocation(g.programID, "Mmatrix");

   AmbientLight = glGetUniformLocation(g.programID, "ambientLight");
   DirectionLightColor = glGetUniformLocation(g.programID, "directionLightColor");
   DirectionLightVector = glGetUniformLocation(g.programID, "directionLightVector");

   move_matrix = Eigen::Matrix4f::Identity();
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());

   noLights();
   camera();
   perspective();

   background(WHITE);
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

   // Disable freetype for now
   // init_freetype();

   PFont::init();
   PImage::init();

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

            glBindFramebuffer(GL_FRAMEBUFFER, g.localFboID);
            glClear(GL_DEPTH_BUFFER_BIT);

            draw();

	    if (g.localFboID != 0) {
	       // Reset to default view settings to draw next frame and
	       // draw the texture from the PGraphics flat.
	       noLights();
	       move_matrix = Eigen::Matrix4f::Identity();
	       glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());

	       Eigen::Matrix4f P = Eigen::Matrix4f::Identity();
	       Eigen::Matrix4f V = TranslateMatrix(PVector{-1,-1,0}) * ScaleMatrix(PVector{2.0f/width, 2.0f/height,1.0});

	       glUniformMatrix4fv(Pmatrix, 1,false, P.data());
	       glUniformMatrix4fv(Vmatrix, 1,false, V.data());

	       g.draw_main();

	       glUniformMatrix4fv(Pmatrix, 1,false, projection_matrix.data());
	       glUniformMatrix4fv(Vmatrix, 1,false, view_matrix.data());
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

   //close_freetype();

   // Clean up
   SDL_GL_DeleteContext(glContext);
   SDL_DestroyWindow(window);
   SDL_Quit();

   return 0;
}

#endif
