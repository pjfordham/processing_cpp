#include "processing.h"

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
#include <memory>

// A simple Particle class

class Particle {
public:
  PVector position;
  PVector velocity;
  PVector acceleration;
  float lifespan;

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
  virtual void update() {
    velocity.add(acceleration);
    position.add(velocity);
    lifespan -= 2.0;
  }

  // Method to display
  virtual void display() {
    stroke(255, lifespan);
    fill(255, lifespan);
    ellipse(position.x, position.y, 8, 8);
  }

  // Is the particle still useful?
  boolean isDead() {
    return (lifespan < 0.0);
  }
};

// A subclass of Particle

class CrazyParticle : public Particle {

public:
  // Just adding one new variable to a CrazyParticle
  // It inherits all other fields from "Particle", and we don't have to retype them!
  float theta;

  // The CrazyParticle constructor can call the parent class (super class) constructor
  CrazyParticle(PVector l) : Particle(l) {
    // One more line of code to deal with the new variable, theta
    theta = 0.0;
  }

  // Notice we don't have the method run() here; it is inherited from Particle

  // This update() method overrides the parent class update() method
  void update() {
    Particle::update();
    // Increment rotation based on horizontal velocity
    float theta_vel = (velocity.x * velocity.mag()) / 10.0f;
    theta += theta_vel;
  }

  // This display() method overrides the parent class display() method
  void display() {
    // Render the ellipse just like in a regular particle
    Particle::display();
    // Then add a rotating line
    pushMatrix();
    translate(position.x, position.y);
    rotate(theta);
    stroke(255, lifespan);
    line(0, 0, 25, 0);
    popMatrix();
  }
};

// An ArrayList is used to manage the list of Particles

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
