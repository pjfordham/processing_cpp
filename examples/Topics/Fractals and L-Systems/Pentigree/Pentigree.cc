/**
 * Pentigree L-System
 * by Geraldine Sarmiento.
 *
 * This code was based on Patrick Dwyer's L-System class.
 */
#include "PentigreeLSystem.h"

PentigreeLSystem ps;

void setup() {
  size(640, 360);
  ps.simulate(3);
}

void draw() {
  background(0);
  ps.render();
}

