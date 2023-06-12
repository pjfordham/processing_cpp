/**
 * Penrose Tile L-System
 * by Geraldine Sarmiento.
 *
 * This code was based on Patrick Dwyer's L-System class.
 */

#include "PenroseLSystem.h"

PenroseLSystem ds;

void setup() {
  size(640, 360);
  ds.simulate(4);
}

void draw() {
  background(0);
  ds.render();
}






