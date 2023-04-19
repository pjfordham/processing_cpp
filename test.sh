#!/bin/bash
rm -f *~ weak.o sketch.o sketch
file="${1:-sketch.cc}"
g++ -c -O3 PerlinNoise.cc -o PerlinNoise.o
g++ -c -O3 weak.c -o weak.o
g++ -c -O3 -I/usr/include/freetype2 -include processing.h $file  -o sketch.o
g++ sketch.o weak.o PerlinNoise.o -lSDL2 -lSDL2_ttf -lfreetype  -lSDL2_image -lglfw -lGLEW -lGL -lcurl -lSDL2main -lfmt -lm -o ./sketch && timeout 120s ./sketch
