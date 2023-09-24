/**
 * PolygonPShapeOOP.
 *
 * Wrapping a PShape inside a custom class
 * and demonstrating how we can have a multiple objects each
 * using the same PShape.
 */

#include "Polygon.h"

// A list of objects
std::vector<Polygon> polygons;

// Three possible shapes
std::vector<PShape> shapes(3);

void setup() {
  size(640, 360, P2D);

  shapes[0] = createEllipse(0,0,100,100);
  shapes[0].setFill(color(255, 127));
  shapes[0].setStroke(false);
  shapes[1] = createRect(0,0,100,100);
  shapes[1].setFill(color(255, 127));
  shapes[1].setStroke(false);

  shapes[2].beginShape();
  shapes[2].fill(0, 127);
  shapes[2].noStroke();
  shapes[2].vertex(0, -50);
  shapes[2].vertex(14, -20);
  shapes[2].vertex(47, -15);
  shapes[2].vertex(23, 7);
  shapes[2].vertex(29, 40);
  shapes[2].vertex(0, 25);
  shapes[2].vertex(-29, 40);
  shapes[2].vertex(-23, 7);
  shapes[2].vertex(-47, -15);
  shapes[2].vertex(-14, -20);
  shapes[2].endShape(CLOSE);

  for (int i = 0; i < 25; i++) {
    int selection = int(random(shapes.size()));        // Pick a random index
    Polygon p(shapes[selection]);        // Use corresponding PShape to create Polygon
    polygons.push_back(p);
  }
}

void draw() {
  background(102);

  // Display and move them all
  for (Polygon &poly : polygons) {
    poly.display();
    poly.move();
  }
}

