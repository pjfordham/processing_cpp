#include "processing_color.h"
#include "processing_math.h"
#include <fmt/core.h>

int color::mode = RGB;
float color::scaleR = 255.0f;
float color::scaleG = 255.0f;
float color::scaleB = 255.0f;
float color::scaleA = 255.0f;

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

color RANDOM_COLOR() {
   return color(random(255),random(255),random(255),255);
}

void color::print() {
   fmt::print(stderr,"R{0:#x}G{0:#x}B{0:#x}A{0:#x}\n",(int)r,(int)g,(int)b,(int)a);
}
