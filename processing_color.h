#ifndef PROCESSING_COLOR_H
#define PROCESSING_COLOR_H

#include "processing_math.h"
#include "processing_enum.h"

// ----
// Begin color handling.
// ----
float red(unsigned int pixel) {
   return (pixel >>  0) & 0xFF;
}

float green(unsigned int pixel) {
   return (pixel >>  8) & 0xFF;
}

float blue(unsigned int pixel) {
   return (pixel >> 16) & 0xFF;
}

float alpha(unsigned int pixel) {
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
   color(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(scaleA) {
   }
   color(float _r) : r(_r), g(_r), b(_r), a(scaleA) {
   }
   color(unsigned int c, bool) : r( red(c) ), g(green(c)), b(blue(c)), a(alpha(c))  {
   }
   color()  {
   }
   void print() {
      fprintf(stderr,"R%xG%xB%xA%x\n",(int)r,(int)g,(int)b,(int)a);
   }
};

int color::mode = RGB;
float color::scaleR = 255.0f;
float color::scaleG = 255.0f;
float color::scaleB = 255.0f;
float color::scaleA = 255.0f;

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
color RANDOM_COLOR() {
   return color(random(255),random(255),random(255),255);
}
void colorMode(int mode, float r, float g, float b, float a) {
   color::mode = mode;
   color::scaleR = r;
   color::scaleG = g;
   color::scaleB = b;
   color::scaleA = a;
}

void colorMode(int mode, float scale) {
   colorMode(mode, scale, scale, scale, scale);
}

void colorMode(int mode, float r, float g, float b) {
   colorMode(mode, r,g,b,255);
}

color HSBtoRGB(float h, float s, float v, float a)
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
   return { r * 255, g * 255, b * 255, a };
}

color flatten_color_mode(float r, float g, float b, float a) {
   r = map(r,0,color::scaleR,0,255);
   g = map(g,0,color::scaleG,0,255);
   b = map(b,0,color::scaleB,0,255);
   a = map(a,0,color::scaleA,0,255);
   if (color::mode == HSB) {
      return HSBtoRGB(r/255.0,g/255.0,b/255.0,a);
   }
   return { r, g, b, a };
}
// ----
// End color handling.
// ----

#endif


