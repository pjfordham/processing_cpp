#!/bin/bash
rm -f *~ weak.o sketch.o sketch
file="${1:-sketch.cc}"
g++ -c -g weak.c -o weak.o
g++ -c -g -include processing.h $file  -o sketch.o
g++ sketch.o weak.o -lSDL2 -lSDL2_gfx -lSDL2_ttf -O3 -lfmt -lm -o ./sketch && gdb ./sketch
