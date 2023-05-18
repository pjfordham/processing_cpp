/**
 * Circle Collision with Swapping Velocities
 * by Ira Greenberg.
 *
 * Based on Keith Peter's Solution in
 * Foundation Actionscript Animation: Making Things Move!
 */

#include "Ball.hh"

std::vector<Ball> balls =  {
  Ball(100, 400, 20),
  Ball(700, 400, 80)
};

void setup() {
  size(640, 360);
}

void draw() {
  background(51);

  for (Ball &b : balls) {
    b.update();
    b.display();
    b.checkBoundaryCollision();
  }

  balls[0].checkCollision(balls[1]);
}




