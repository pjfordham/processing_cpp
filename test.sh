#!/bin/bash
file="${1:-sketch.cc}"
g++ -include processing.h $file -lSDL2 -lSDL2_gfx -lm && ./a.out
