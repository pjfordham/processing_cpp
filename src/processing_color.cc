#include "processing_color.h"
#include "processing_math.h"
#include <fmt/core.h>

int color::mode = RGB;
float color::scaleR = 255.0f;
float color::scaleG = 255.0f;
float color::scaleB = 255.0f;
float color::scaleA = 255.0f;

color RANDOM_COLOR() {
   return color(random(255),random(255),random(255),255);
}

void color::print() const {
   fmt::print("R{0:#x} G{0:#x} B{0:#x} A{0:#x}\n",(int)r,(int)g,(int)b,(int)a);
}

void color::printf() const {
   fmt::print("R{} G{} B{} A{}\n",r,g,b,a);
}
