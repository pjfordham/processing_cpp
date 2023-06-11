/**
 * Multiple Particle Systems
 * by Daniel Shiffman.
 *
 * Click the mouse to generate a burst of particles
 * at mouse position.
 *
 * Each burst is one instance of a particle system
 * with Particles and CrazyParticles (a subclass of Particle).
 * Note use of Inheritance and Polymorphism.
 */

#include "ParticleSystem.h"

std::vector<ParticleSystem> systems;

PFont f;

void setup() {
  f = createFont("SourceCodePro-Regular.ttf", 18);
  textFont(f);
  size(640, 360);
}

void draw() {
  background(0);
  for (ParticleSystem &ps : systems) {
    ps.run();
    ps.addParticle();
  }
  if (systems.empty()) {
    fill(255);
    textAlign(CENTER);
    text("click mouse to add particle systems", width/2, height/2);
  }
}

void mousePressed() {
  systems.emplace_back(1, PVector(mouseX, mouseY));
}
