// The Particle System
#include "Particle.h"

class ParticleSystem {
public:
  // It's just an ArrayList of particle objects
  std::vector<Particle> particles;

  // The PShape to group all the particle PShapes
  PShape particleShape;

  ParticleSystem() {}

  ParticleSystem(int n, PImage &sprite) {
    // The PShape is a group
    particleShape = createGroup();

    // Make all the Particles
    for (int i = 0; i < n; i++) {
       Particle p(sprite);
       particles.push_back(p);
      // Each particle's PShape gets added to the System PShape
      particleShape.addChild(p.getShape());
    }
  }

  void update() {
    for (Particle &p : particles) {
      p.update();
    }
  }

  void setEmitter(float x, float y) {
    for (Particle &p : particles) {
      // Each particle gets reborn at the emitter location
      if (p.isDead()) {
        p.rebirth(x, y);
      }
    }
  }

  void display() {
    for (Particle &p : particles) {
       shape(p.getShape());
    }
    }
};

