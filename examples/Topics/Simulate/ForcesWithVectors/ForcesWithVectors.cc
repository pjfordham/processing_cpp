/**
 * Forces (Gravity and Fluid Resistence) with Vectors
 * by Daniel Shiffman.
 *
 * Demonstration of multiple force acting on bodies (Mover class)
 * Bodies experience gravity continuously
 * Bodies experience fluid resistance when in "water"
 */

#include "Liquid.hh"


// Five moving bodies
std::vector<Mover> movers;

// Liquid
Liquid liquid;

// Restart all the Mover objects randomly
void reset() {
  movers.clear();
  for (int i = 0; i < 10; i++) {
    movers.emplace_back(random(0.5, 3), 40+i*70, 0);
  }
}

PFont f;

void setup() {
  size(640, 360);
  //f = createFont("SourceCodePro-Regular.ttf", 18);
  //textFont(f);
  reset();
  // Create liquid object
  liquid = Liquid(0, height/2, width, height/2, 0.1);
}

void draw() {
  background(0);

  // Draw water
  liquid.display();

  for (Mover &mover : movers) {

    // Is the Mover in the liquid?
    if (liquid.contains(mover)) {
      // Calculate drag force
      PVector drag = liquid.drag(mover);
      // Apply drag force to Mover
      mover.applyForce(drag);
    }

    // Gravity is scaled by mass here!
    PVector gravity(0, 0.1*mover.mass);
    // Apply gravity
    mover.applyForce(gravity);

    // Update and display
    mover.update();
    mover.display();
    mover.checkEdges();
  }

  fill(255);
  text("click mouse to reset", 10, 30);
}

void mousePressed() {
  reset();
}

