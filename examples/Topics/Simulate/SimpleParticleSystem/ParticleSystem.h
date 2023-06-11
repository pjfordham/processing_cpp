// A class to describe a group of Particles
// An ArrayList is used to manage the list of Particles

#include "Particle.h"

class ParticleSystem {
public:
  std::vector<Particle> particles;
  PVector origin;

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
