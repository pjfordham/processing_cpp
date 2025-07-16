#include "processing_color.h"
#include "processing_math.h"
#include <fmt/core.h>

int color::mode = RGB;
float color::scaleR = 255.0F;
float color::scaleG = 255.0F;
float color::scaleB = 255.0F;
float color::scaleA = 255.0F;

color RANDOM_COLOR() {
   return { random(255),random(255),random(255),255 };
}
