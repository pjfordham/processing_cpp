// Gravitational Attraction (3D)
// Daniel Shiffman <http://www.shiffman.net>

// A class for an orbiting Planet

class Planet {
public:
  // Basic physics model (position, velocity, acceleration, mass)
  PVector position;
  PVector velocity;
  PVector acceleration;
  float mass;

  Planet() {}
  Planet(float m, float x, float y, float z) {
    mass = m;
    position = PVector(x, y, z);
    velocity = PVector(1, 0);   // Arbitrary starting velocity
    acceleration = PVector(0, 0);
  }

  // Newton's 2nd Law (F = M*A) applied
  void applyForce(PVector force) {
    PVector f = PVector::div(force, mass);
    acceleration.add(f);
  }

  // Our motion algorithm (aka Euler Integration)
  void update() {
    velocity.add(acceleration); // Velocity changes according to acceleration
    position.add(velocity);     // position changes according to velocity
    acceleration.mult(0);
  }

  // Draw the Planet
  void display() {
    noStroke();
    fill(255);
    pushMatrix();
    translate(position.x, position.y, position.z);
    sphere(mass*8);
    popMatrix();
  }
};
