/**
 * WigglePShape.
 *
 * How to move the individual vertices of a PShape
 */

#include "Wiggler.h"

// A "Wiggler" object
Wiggler w;

void setup() {
  size(640, 360, P2D);
  w = Wiggler();
}

void draw() {
  background(255);
  w.display();
  w.wiggle();
}


