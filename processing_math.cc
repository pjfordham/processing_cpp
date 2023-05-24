#include "processing_math.h"

#include "PerlinNoise.h"
#include <fmt/core.h>

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
   return perlin_noise.octave(x,y,z,perlin_octaves,perlin_falloff);
}

void PVector::print() {
   fmt::print(" {:>8.2} {:>8.2} {:>8.2}\n",x,y,z);
}

void PMatrix::print() {
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(0,0), mat(0,1), mat(0,2), mat(0,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(1,0), mat(1,1), mat(1,2), mat(1,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(2,0), mat(2,1), mat(2,2), mat(2,3) );
   fmt::print(" {:>8.2} {:>8.2} {:>8.2} {:>8.2}\n",mat(3,0), mat(3,1), mat(3,2), mat(3,3) );
}
