/**
 * ParticleSystemPShape
 *
 * A particle system optimized for drawing using PShape
 */
#include "ParticleSystem.h"

// Particle System object
ParticleSystem ps;
// A PImage for particle's texture
PImage sprite;

void setup() {
  size(640, 360, P2D);
  // Load the image
  sprite = loadImage("sprite.png");
  // A new particle system with 10,000 particles
  ps = ParticleSystem(10000,sprite);

  // Writing to the depth buffer is disabled to avoid rendering
  // artifacts due to the fact that the particles are semi-transparent
  // but not z-sorted.
  hint(DISABLE_DEPTH_MASK);

}

void draw () {
  background(0);
  // Update and display system
  ps.update();
  ps.display();

  // Set the particle system's emitter location to the mouse
  ps.setEmitter(mouseX,mouseY);

  // // Display frame rate
  fill(255);
  textSize(16);
  text(fmt::format("Frame rate: {}",int(frameRateb)),10,20);

}

