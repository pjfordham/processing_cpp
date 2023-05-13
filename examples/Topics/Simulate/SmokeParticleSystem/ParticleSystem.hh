// A class to describe a group of Particles
// An ArrayList is used to manage the list of Particles

#include "Particle.hh"

class ParticleSystem {
public:
  std::vector<Particle> particles;    // An arraylist for all the particles
  PVector origin;                   // An origin point for where particles are birthed
  PImage img;

  ParticleSystem() {}

   ParticleSystem(int num, PVector v, PImage img_) : origin(v), img(img_) {
    for (int i = 0; i < num; i++) {
       particles.emplace_back(origin, img);         // Add "num" amount of particles to the arraylist
    }
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

  // Method to add a force vector to all particles currently in the system
  void applyForce(PVector dir) {
    // Enhanced loop!!!
    for (Particle &p : particles) {
      p.applyForce(dir);
    }
  }

  void addParticle() {
    particles.emplace_back(origin, img);
  }
};
