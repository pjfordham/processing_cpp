#!/bin/bash
file="${1:-sketch.cc}"
g++ $file -lSDL2 -lSDL2_gfx -lm && ./a.out
