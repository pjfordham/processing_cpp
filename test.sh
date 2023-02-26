#!/bin/bash
rm -f *~
file="${1:-sketch.cc}"
g++ -include processing.h $file -lSDL2 -lSDL2_gfx -O3 -lm -o sketch && ./sketch
rm ./sketch

