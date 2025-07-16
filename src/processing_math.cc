#define STB_PERLIN_IMPLEMENTATION

#include "processing_math.h"

#include <fmt/core.h>
#include <cmath>

int perlin_noise_seed = 0;
int perlin_octaves = 4 ;
float perlin_falloff = 0.5;

void noiseSeed(int seed) {
   perlin_noise_seed = seed;
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
      total += ((stb_perlin_noise3_seed(x * frequency, y * frequency, z * frequency, 0, 0, 0, perlin_noise_seed) + 1.0F) / 2.0F) * amplitude;

      maxValue += amplitude;

      amplitude *= perlin_falloff;
      frequency *= 2;
   }

   return total/maxValue;
}

void PMatrix::print() const {
   if ( identity ) {
      fmt::print("Identity\n");
   } else {
      const glm::mat4 &mat = *((glm::mat4*)this);
      fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
      fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
      fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat[2][0], mat[2][1], mat[2][2], mat[2][3] );
      fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat[3][0], mat[3][1], mat[3][2], mat[3][3] );
   }
}
