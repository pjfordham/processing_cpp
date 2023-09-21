/**
 * Simple Particle System
 * by Daniel Shiffman.
 *
 * Particles are generated each cycle through draw(),
 * fall with gravity, and fade out over time.
 * A ParticleSystem object manages a variable size (ArrayList)
 * list of particles.
 */

#include "ParticleSystem.h"

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
