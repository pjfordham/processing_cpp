#ifndef PROCESSING_COLOR_H
#define PROCESSING_COLOR_H

#include "processing_math.h"
#include "processing_enum.h"
#include "processing_opengl_color.h"

#include <fmt/core.h>

#include <cmath>
#include <glm/vec4.hpp>

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
   color(float _r, float _a) : color( _r, _r, _r, _a ) {
   }
   color(color c, float _a) : color( c.r, c.g, c.b, _a ) {
   }
   color(float _r)  : color( _r, _r, _r, scaleA) {
   }
   color(unsigned int c) : r( red(c) ), g(green(c)), b(blue(c)), a(alpha(c))  {
   }
   color(bool b) = delete;
   color(int c) = delete;
   color()  {
   }
   color blend(const color& a) {
        float alpha_a = a.a;
        float alpha_bg = this->a;

        // Calculate the blended color components
        float result_r = (a.r * alpha_a + this->r * alpha_bg * (1.0 - alpha_a)) / (alpha_a + alpha_bg * (1.0 - alpha_a));
        float result_g = (a.g * alpha_a + this->g * alpha_bg * (1.0 - alpha_a)) / (alpha_a + alpha_bg * (1.0 - alpha_a));
        float result_b = (a.b * alpha_a + this->b * alpha_bg * (1.0 - alpha_a)) / (alpha_a + alpha_bg * (1.0 - alpha_a));
        float result_alpha = alpha_a + alpha_bg * (1.0 - alpha_a);

        return color(result_r, result_g, result_b, result_alpha);
   }
   color tint(const color &a) {
      return color(a.r*r,a.g*g, a.b*b, a.a*a);
   }
   glm::vec4 gl_color() {
      return glm::vec4{r / color::scaleR,  g / color::scaleR, b / color::scaleB, a / color::scaleA};
   }
};

const color DEFAULT_GRAY = color(240.0f);
const color BLACK = color(0.0f);
const color WHITE = color(255.0f);
//const color GRAY = color(127.0f);
const color LIGHT_GRAY = color(192.0f);
const color DARK_GRAY = color(64.0f);
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

inline color lerpColor(const color& c1, const color& c2, float amt) {
   return {
      c1.r + (c2.r - c1.r) * amt,
      c1.g + (c2.g - c1.g) * amt,
      c1.b + (c2.b - c1.b) * amt,
      c1.a + (c2.a - c1.a) * amt};
}

inline float brightness(color rgb) {
  if (color::mode == HSB) {
     return rgb.r;
   }
   return 0.299f * rgb.r + 0.587f * rgb.g + 0.114f * rgb.b;
}

inline gl::color HSBtoRGB(float h, float s, float v, float a) {
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

inline gl::color flatten_color_mode(color c) {
   float r = map(c.r,0,::color::scaleR,0,1);
   float g = map(c.g,0,::color::scaleG,0,1);
   float b = map(c.b,0,::color::scaleB,0,1);
   float a = map(c.a,0,::color::scaleA,0,1);
   if (color::mode == HSB) {
      return HSBtoRGB(r,g,b,a);
   }
   return { r, g, b, a };
}

template <>
struct fmt::formatter<color> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const color& v, FormatContext& ctx) {
       return format_to(ctx.out(), "R{:3},G{:3},B{:3},A{:3}",(int)v.r,(int)v.g,(int)v.b,(int)v.a);
    }
};

// ----
// End color handling.
// ----

#endif


