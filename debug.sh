#!/bin/bash
rm -f *~ weak.o sketch.o sketch
file="${1:-sketch.cc}"
g++ -c -g PerlinNoise.cc -o PerlinNoise.o
g++ -c -g weak.c -o weak.o
g++ -c -I/usr/include/freetype2 -g -include processing.h $file  -o sketch.o
g++ sketch.o weak.o PerlinNoise.o -lSDL2 -lSDL2_ttf -lSDL2_image -lfreetype -lglfw -lGLEW -lGL -lSDL2main -lcurl -lfmt -lm -o ./sketch && gdb ./sketch
