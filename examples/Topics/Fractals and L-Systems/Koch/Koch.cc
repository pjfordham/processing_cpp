/**
 * Koch Curve
 * by Daniel Shiffman.
 *
 * Renders a simple fractal, the Koch snowflake.
 * Each recursive level is drawn in sequence.
 */

#include "KochFractal.hh"

KochFractal k;

void setup() {
  size(640, 360);
  frameRate(1);  // Animate slowly
  k = KochFractal(); // redo it here to pick up width and height.
}

void draw() {
  background(0);
  // Draws the snowflake!
  k.render();
  // Iterate
  k.nextLevel();
  // Let's not do it more than 5 times. . .
  if (k.getCount() > 5) {
    k.restart();
  }
}


