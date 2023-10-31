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
