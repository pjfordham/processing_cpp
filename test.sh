#!/bin/bash
rm -f *~ weak.o sketch.o sketch
file="${1:-sketch.cc}"
g++ -c -O3 weak.c -o weak.o
g++ -c -O3 -include processing.h $file  -o sketch.o
g++ sketch.o weak.o -lSDL2 -lSDL2_gfx -lSDL2_ttf -lfmt -lm -o ./sketch && ./sketch
