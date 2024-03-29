/**
 * Forces (Gravity and Fluid Resistence) with Vectors
 * by Daniel Shiffman.
 *
 * Demonstration of multiple force acting on bodies (Mover class)
 * Bodies experience gravity continuously
 * Bodies experience fluid resistance when in "water"
 */


class Mover {

public:
  // position, velocity, and acceleration
  PVector position;
  PVector velocity;
  PVector acceleration;

  // Mass is tied to size
  float mass;

  Mover(float m, float x, float y) {
    mass = m;
    position = PVector(x, y);
    velocity = PVector(0, 0);
    acceleration = PVector(0, 0);
  }

  // Newton's 2nd law: F = M * A
  // or A = F / M
  void applyForce(PVector force) {
    // Divide by mass
    PVector f = PVector::div(force, mass);
    // Accumulate all forces in acceleration
    acceleration.add(f);
  }

  void update() {
    // Velocity changes according to acceleration
    velocity.add(acceleration);
    // position changes by velocity
    position.add(velocity);
    // We must clear acceleration each frame
    acceleration.mult(0);
  }

  // Draw Mover
  void display() {
    stroke(255);
    strokeWeight(2);
    fill(255, 200);
    ellipse(position.x, position.y, mass*16, mass*16);
  }

  // Bounce off bottom of window
  void checkEdges() {
    if (position.y > height) {
      velocity.y *= -0.9;  // A little dampening when hitting the bottom
      position.y = height;
    }
  }
};
