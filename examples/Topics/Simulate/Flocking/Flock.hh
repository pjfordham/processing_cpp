// The Flock (a list of Boid objects)

#include "Boid.hh"

class Flock {
  std::vector<Boid> boids; // An ArrayList for all the boids
public:

  Flock() {
  }

  void run() {
    for (Boid &b : boids) {
      b.run(boids);  // Passing the entire list of boids to each boid individually
    }
  }

  void addBoid(Boid b) {
    boids.push_back(b);
  }

};

