#!/bin/bash
file="${1:-sketch.cc}"
g++ -include processing.h $file -lSDL2 -lSDL2_gfx -O3 -lm -o sketch && ./sketch
