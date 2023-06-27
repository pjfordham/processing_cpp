#include "processing_math.h"

#include <fmt/core.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <vector>

class PerlinNoise {
   static float fade(float t) {
      return t * t * t * (t * (t * 6 - 15) + 10);
   }

   static float lerp(float t, float a, float b) {
      return ::lerp(a , b, t);
   }

   static float grad(int hash, float x, float y, float z) {
      int h = hash & 15;
      // Convert lower 4 bits of hash into 12 gradient directions
      float u = h < 8 ? x : y,
         v = h < 4 ? y : h == 12 || h == 14 ? x : z;
      return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
   }

   // The permutation vector
   static constexpr int original_permutation[] = {
       151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233,
       7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,
       23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252,
       219, 203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,
       174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,
       27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230,
       220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,  65,  25,
       63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
       200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186,
       3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126,
       255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
       28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,
       221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,
       108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228,
       251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,
       145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
       184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236,
       205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,
       215, 61,  156, 180};

   std::vector<int> p;
public:

   PerlinNoise() {
      // Initialize the permutation vector with the original permutation values
      p.insert(p.end(), std::begin( original_permutation ), std::end( original_permutation ) );
      p.insert(p.end(), std::begin( original_permutation ), std::end( original_permutation ) );
   }

   PerlinNoise(unsigned int seed) {
      // Reorder original permutation vector based on the value of seed
      p.insert(p.end(), std::begin( original_permutation ), std::end( original_permutation ) );
      std::shuffle(p.begin(), p.end(), std::default_random_engine(seed));
      p.insert(p.end(), p.begin(), p.end());
   }

   float noise(float x, float y, float z) {
      int X = (int) floor(x) & 255;
      int Y = (int) floor(y) & 255;
      int Z = (int) floor(z) & 255;

      x -= floor(x);
      y -= floor(y);
      z -= floor(z);

      float u = fade(x);
      float v = fade(y);
      float w = fade(z);

      int A = p[X] + Y;
      int AA = p[A] + Z;
      int AB = p[A + 1] + Z;
      int B = p[X + 1] + Y;
      int BA = p[B] + Z;
      int BB = p[B + 1] + Z;

      // Add blended results from 8 corners of cube
      return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x,   y,   z),
                                     grad(p[BA  ], x-1, y,   z)),
                             lerp(u, grad(p[AB  ], x,   y-1, z),
                                     grad(p[BB  ], x-1, y-1, z))),
                     lerp(v, lerp(u, grad(p[AA+1], x,   y,   z-1),
                                     grad(p[BA+1], x-1, y,   z-1)),
                             lerp(u, grad(p[AB+1], x,   y-1, z-1),
                                     grad(p[BB+1], x-1, y-1, z-1))));
   }
};


PerlinNoise perlin_noise;
int perlin_octaves = 4 ;
float perlin_falloff = 0.5;


void noiseSeed(int seed) {
   perlin_noise = PerlinNoise(seed);
}

void noiseDetail(int lod, float falloff) {
   perlin_octaves = lod;
   perlin_falloff = falloff;
}

float noise(float x, float y, float z) {
   float total = 0;
   float frequency = 1;
   float amplitude = 1;
   float maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
   for(int i=0;i<perlin_octaves;i++) {
      total += ((perlin_noise.noise(x * frequency, y * frequency, z * frequency) + 1.0) / 2.0) * amplitude;

      maxValue += amplitude;

      amplitude *= perlin_falloff;
      frequency *= 2;
   }

   return total/maxValue;
}

void PVector::print() const {
   fmt::print(" {:>8.2} {:>8.2} {:>8.2}\n",x,y,z);
}

void PMatrix::print() const {
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(0,0), mat(0,1), mat(0,2), mat(0,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(1,0), mat(1,1), mat(1,2), mat(1,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(2,0), mat(2,1), mat(2,2), mat(2,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(3,0), mat(3,1), mat(3,2), mat(3,3) );
}
