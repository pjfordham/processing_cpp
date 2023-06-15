/**
 * PolygonPShapeOOP.
 *
 * Wrapping a PShape inside a custom class
 */

#include "Star.h"

// A Star object
Star s1, s2;

void setup() {
  size(640, 360, P2D);

  // Make a new Star
  s1 = Star();
  s2 = Star();

}

void draw() {
  background(51);

  s1.display();  // Display the first star
  s1.move();  // Move the first star

  s2.display();  // Display the second star
  s2.move();  // Move the second star

}

