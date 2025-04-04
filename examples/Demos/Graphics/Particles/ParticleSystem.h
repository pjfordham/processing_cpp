#include "Particle.h"

class ParticleSystem {
public:
  std::vector<Particle> particles;

  PShape particleShape;

  ParticleSystem() {}

  ParticleSystem(int n, PImage sprite) {
    particleShape = createGroup();

    for (int i = 0; i < n; i++) {
      Particle &p = particles.emplace_back(sprite);
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
      if (p.isDead()) {
        p.rebirth(x, y);
      }
    }
  }

  void display() {

    shape(particleShape);
  }
};

