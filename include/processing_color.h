#ifndef PROCESSING_COLOR_H
#define PROCESSING_COLOR_H

#include "processing_enum.h"
#include "processing_opengl.h"

#include <cmath>

// ----
// Begin color handling.
// ----
inline float red(unsigned int pixel) {
   return (pixel >>  0) & 0xFF;
}

inline float green(unsigned int pixel) {
   return (pixel >>  8) & 0xFF;
}

inline float blue(unsigned int pixel) {
   return (pixel >> 16) & 0xFF;
}

inline float alpha(unsigned int pixel) {
   return (pixel >> 24) & 0xFF;
}

class color {
public:
   static int mode;
   static float scaleR;
   static float scaleG;
   static float scaleB;
   static float scaleA;

   operator unsigned int() const {
      return
         ((unsigned char)r) << 0|
         ((unsigned char)g) << 8 |
         ((unsigned char)b) << 16 |
         ((unsigned char)a) << 24;
   }
   float r,g,b,a;
   color(float _r, float _g, float _b,float _a) : r(_r), g(_g), b(_b), a(_a) {
   }
   color(float _r, float _g, float _b) : color( _r, _g, _b, scaleA) {
   }
   color(float _r)  : color( _r, _r, _r, scaleA) {
   }
   color(unsigned int c, bool) : r( red(c) ), g(green(c)), b(blue(c)), a(alpha(c))  {
   }
   color()  {
   }
   void print() const;
   void printf() const;
};

const color DEFAULT_GRAY = color(240);
const color BLACK = color(0);
const color WHITE = color(255);
//const color GRAY = color(127);
const color LIGHT_GRAY = color(192);
const color DARK_GRAY = color(64);
const color RED = color(255, 0, 0);
const color GREEN = color(0, 255, 0);
const color BLUE = color(0, 0, 255);
const color YELLOW = color(255, 255, 0);
const color CYAN = color(0, 255, 255);
const color MAGENTA = color(255, 0, 255);

color RANDOM_COLOR();

inline void colorMode(int mode, float r, float g, float b, float a) {
   color::mode = mode;
   color::scaleR = r;
   color::scaleG = g;
   color::scaleB = b;
   color::scaleA = a;
}

inline void colorMode(int mode, float scale) {
   colorMode(mode, scale, scale, scale, scale);
}

inline void colorMode(int mode, float r, float g, float b) {
   colorMode(mode, r,g,b,255);
}

gl_context::color flatten_color_mode(color c);

inline color lerpColor(const color& c1, const color& c2, float amt) {
   return {
      c1.r + (c2.r - c1.r) * amt,
      c1.g + (c2.g - c1.g) * amt,
      c1.b + (c2.b - c1.b) * amt,
      c1.a + (c2.a - c1.a) * amt};
}

// ----
// End color handling.
// ----

#endif


