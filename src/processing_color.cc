#include "processing_color.h"
#include "processing_math.h"
#include <fmt/core.h>

int color::mode = RGB;
float color::scaleR = 255.0f;
float color::scaleG = 255.0f;
float color::scaleB = 255.0f;
float color::scaleA = 255.0f;

static gl_context::color HSBtoRGB(float h, float s, float v, float a)
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
   return { r, g, b, a };
}

gl_context::color flatten_color_mode(color c) {
   float r = map(c.r,0,color::scaleR,0,1);
   float g = map(c.g,0,color::scaleG,0,1);
   float b = map(c.b,0,color::scaleB,0,1);
   float a = map(c.a,0,color::scaleA,0,1);
   if (color::mode == HSB) {
      return HSBtoRGB(r,g,b,a);
   }
   return { r, g, b, a };
}

color RANDOM_COLOR() {
   return color(random(255),random(255),random(255),255);
}

void color::print() {
   fmt::print("R{0:#x} G{0:#x} B{0:#x} A{0:#x}\n",(int)r,(int)g,(int)b,(int)a);
}

void color::printf() {
   fmt::print("R{} G{} B{} A{}\n",r,g,b,a);
}
