/**
 * Simple Particle System
 * by Daniel Shiffman.
 *
 * Particles are generated each cycle through draw(),
 * fall with gravity, and fade out over time.
 * A ParticleSystem object manages a variable size (ArrayList)
 * list of particles.
 */

// A simple Particle class

class Particle {
  PVector position;
  PVector velocity;
  PVector acceleration;
  float lifespan;

public:
  Particle(PVector l) {
    acceleration = PVector(0, 0.05);
    velocity = PVector(random(-1, 1), random(-2, 0));
    position = l;
    lifespan = 255.0;
  }

  void run() {
    update();
    display();
  }

  // Method to update position
  void update() {
    velocity.add(acceleration);
    position.add(velocity);
    lifespan -= 1.0;
  }

  // Method to display
  void display() {
    stroke(255, lifespan);
    fill(255, lifespan);
    ellipse(position.x, position.y, 8, 8);
  }

  // Is the particle still useful?
  boolean isDead() {
    if (lifespan < 0.0) {
      return true;
    } else {
      return false;
    }
  }
};

// A class to describe a group of Particles
// An ArrayList is used to manage the list of Particles

class ParticleSystem {
  std::vector<Particle> particles;
  PVector origin;

public:
  ParticleSystem() {}

  ParticleSystem(PVector position) {
    origin = position;
  }

  void addParticle() {
    particles.emplace_back(origin);
  }

  void run() {
    for (int i = particles.size()-1; i >= 0; i--) {
      Particle &p = particles[i];
      p.run();
      if (p.isDead()) {
        particles.erase(particles.begin() + i);
      }
    }
  }
};

ParticleSystem ps;

void setup() {
  size(640, 360);
  ps = ParticleSystem(PVector(width/2, 50));
}

void draw() {
  background(0);
  ps.addParticle();
  ps.run();
}
