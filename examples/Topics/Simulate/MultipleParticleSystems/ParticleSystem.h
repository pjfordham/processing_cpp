// An ArrayList is used to manage the list of Particles
#include "CrazyParticle.h"
#include <memory>

class ParticleSystem {

public:
  std::vector<std::unique_ptr<Particle>> particles;    // An arraylist for all the particles
  PVector origin;                   // An origin point for where particles are birthed

  ParticleSystem(int num, PVector v) {
    origin = v;                        // Store the origin point
    for (int i = 0; i < num; i++) {
      particles.emplace_back(std::make_unique<Particle>(origin));    // Add "num" amount of particles to the arraylist
    }
  }


  void run() {
    // Cycle through the ArrayList backwards, because we are deleting while iterating
    for (int i = particles.size()-1; i >= 0; i--) {
      auto &p = particles[i];
      p->run();
      if (p->isDead()) {
        particles.erase(particles.begin() + i);
      }
    }
  }

  void addParticle() {
    std::unique_ptr<Particle> p;
    // Add either a Particle or CrazyParticle to the system
    if (int(random(0, 2)) == 0) {
      p = std::make_unique<Particle>(origin);
    }
    else {
      p = std::make_unique<CrazyParticle>(origin);
    }
    particles.push_back(std::move(p));
  }

  void addParticle(std::unique_ptr<Particle> p) {
    particles.push_back(std::move(p));
  }

  // A method to test if the particle system still has particles
  boolean dead() {
    return particles.empty();
  }
};

