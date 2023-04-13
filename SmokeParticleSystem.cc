/**
 * Smoke Particle System
 * by Daniel Shiffman.
 *
 * A basic smoke effect using a particle system. Each particle
 * is rendered as an alpha masked image.
 */

// A simple Particle class, renders the particle as an image

class Particle {
public:
   PVector loc;
   PVector vel;
   PVector acc;
   float lifespan;
   PImage img;

   Particle(PVector l, PImage img_) :
      acc(0, 0),
      vel( randomGaussian()*0.3, randomGaussian()*0.3 - 1.0),
      loc( l),
      lifespan( 100.0) ,
      img( img_) {
   }

   void run() {
      update();
      render();
   }

   // Method to apply a force vector to the Particle object
   // Note we are ignoring "mass" here
   void applyForce(PVector f) {
      acc.add(f);
   }

   // Method to update position
   void update() {
      vel.add(acc);
      loc.add(vel);
      lifespan -= 2.5;
      acc.mult(0); // clear Acceleration
   }

   // Method to display
   void render() {
      imageMode(CENTER);
      tint(255, lifespan);
      image(img, loc.x, loc.y);
      // Drawing a circle instead
      // fill(255,lifespan);
      // noStroke();
      // ellipse(loc.x,loc.y,img.width,img.height);
   }

   // Is the particle still useful?
   boolean isDead() {
      if (lifespan <= 0.0) {
         return true;
      } else {
         return false;
      }
   }
};

// A class to describe a group of Particles
// An ArrayList is used to manage the list of Particles

class ParticleSystem {

public:
   std::vector<Particle> particles;    // An arraylist for all the particles
   PVector origin;                   // An origin point for where particles are birthed
   PImage img;

   ParticleSystem() {}

   ParticleSystem(int num, PVector v, PImage img_):
      origin(v),                                   // Store the origin point
      img(img_)
      {
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

ParticleSystem ps;

void setup() {
   size(640, 360);
   PImage img = loadImage("texture.png");
   ps = ParticleSystem(0, PVector(width/2, height-60), img);
}

// Renders a vector object 'v' as an arrow and a position 'loc'
void drawVector(PVector v, PVector loc, float scayl) {
   pushMatrix();
   float arrowsize = 4;
   // Translate to position to render vector
   translate(loc.x, loc.y);
   stroke(255);
   // Call vector heading function to get direction (note that pointing up is a heading of 0) and rotate
   rotate(v.heading());
   // Calculate length of vector & scale it to be bigger or smaller if necessary
   float len = v.mag()*scayl;
   // Draw three lines to make an arrow (draw pointing up since we've rotate to the proper direction)
   line(0, 0, len, 0);
   line(len, 0, len-arrowsize, +arrowsize/2);
   line(len, 0, len-arrowsize, -arrowsize/2);
   popMatrix();
}

void draw() {
   background(0);

   // Calculate a "wind" force based on mouse horizontal position
   float dx = map(mouseX, 0, width, -0.2, 0.2);
   PVector wind(dx, 0);
   ps.applyForce(wind);
   ps.run();
   for (int i = 0; i < 2; i++) {
      ps.addParticle();
   }

   // Draw an arrow representing the wind force
   drawVector(wind, PVector(width/2, 50, 0), 500);
}
