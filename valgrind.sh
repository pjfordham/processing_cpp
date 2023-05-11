#!/bin/bash
rm -f *~ weak.o sketch.o sketch
file="${1:-sketch.cc}"
clang++ -std=c++17 -c -g PerlinNoise.cc -o PerlinNoise.o
clang++ -std=c++17 -c -g weak.cc -o weak.o
clang++ -std=c++17 -c -I/usr/include/freetype2 -g -include processing.h $file  -o sketch.o
clang++ -std=c++17 sketch.o weak.o PerlinNoise.o -lSDL2 -lSDL2_ttf -lSDL2_image -lfreetype -lglfw -lGLEW -lGL -lSDL2main -lcurl -lfmt -lm -o ./sketch && valgrind --num-callers=30 ./sketch
