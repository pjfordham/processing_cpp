/**
 * Flocking
 * by Daniel Shiffman.
 *
 * An implementation of Craig Reynold's Boids program to simulate
 * the flocking behavior of birds. Each boid steers itself based on
 * rules of avoidance, alignment, and coherence.
 *
 * Click the mouse to add a new boid.
 */

// The Boid class

class Boid {

   PVector position;
   PVector velocity;
   PVector acceleration;
   float r;
   float maxforce;    // Maximum steering force
   float maxspeed;    // Maximum speed

public:
   Boid(float x, float y) :
      acceleration(0, 0) ,
      velocity(PVector::random2D()),
      position(PVector(x, y)),
      r(2.0),
      maxspeed(2),
      maxforce(0.03) {
   }

   void run(std::vector<Boid> &boids) {
      flock(boids);
      update();
      borders();
      render();
   }

   void applyForce(PVector force) {
      // We could add mass here if we want A = F / M
      acceleration.add(force);
   }

   // We accumulate a new acceleration each time based on three rules
   void flock(std::vector<Boid> &boids) {
      PVector sep = separate(boids);   // Separation
      PVector ali = align(boids);      // Alignment
      PVector coh = cohesion(boids);   // Cohesion
      // Arbitrarily weight these forces
      sep.mult(1.5);
      ali.mult(1.0);
      coh.mult(1.0);
      // Add the force vectors to acceleration
      applyForce(sep);
      applyForce(ali);
      applyForce(coh);
   }

   // Method to update position
   void update() {
      // Update velocity
      velocity.add(acceleration);
      // Limit speed
      velocity.limit(maxspeed);
      position.add(velocity);
      // Reset accelertion to 0 each cycle
      acceleration.mult(0);
   }

   // A method that calculates and applies a steering force towards a target
   // STEER = DESIRED MINUS VELOCITY
   PVector seek(PVector target) {
      PVector desired = PVector::sub(target, position);  // A vector pointing from the position to the target
      // Scale to maximum speed
      desired.setMag(maxspeed);

      // Steering = Desired minus Velocity
      PVector steer = PVector::sub(desired, velocity);
      steer.limit(maxforce);  // Limit to maximum steering force
      return steer;
   }

   void render() {
      // Draw a triangle rotated in the direction of velocity
      float theta = velocity.heading() + radians(90);
 
      fill(200, 100);
      stroke(255);
      pushMatrix();
      translate(position.x, position.y);
      rotate(theta);
      beginShape(TRIANGLES);
      vertex(0, -r*2);
      vertex(-r, r*2);
      vertex(r, r*2);
      endShape();
      popMatrix();
  }

   // Wraparound
   void borders() {
      if (position.x < -r) position.x = width+r;
      if (position.y < -r) position.y = height+r;
      if (position.x > width+r) position.x = -r;
      if (position.y > height+r) position.y = -r;
   }

   // Separation
   // Method checks for nearby boids and steers away
   PVector separate (std::vector<Boid> &boids) {
      float desiredseparation = 25.0f;
      PVector steer(0, 0, 0);
      int count = 0;
      // For every boid in the system, check if it's too close
      for (Boid other : boids) {
         float d = PVector::dist(position, other.position);
         // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
         if ((d > 0) && (d < desiredseparation)) {
            // Calculate vector pointing away from neighbor
            PVector diff = PVector::sub(position, other.position);
            diff.normalize();
            diff.div(d);        // Weight by distance
            steer.add(diff);
            count++;            // Keep track of how many
         }
      }
      // Average -- divide by how many
      if (count > 0) {
         steer.div((float)count);
      }

      // As long as the vector is greater than 0
      if (steer.mag() > 0) {
         // Implement Reynolds: Steering = Desired - Velocity
         steer.setMag(maxspeed);
         steer.sub(velocity);
         steer.limit(maxforce);
      }
      return steer;
   }

   // Alignment
   // For every nearby boid in the system, calculate the average velocity
   PVector align (std::vector<Boid> &boids) {
      float neighbordist = 50;
      PVector sum(0, 0);
      int count = 0;
      for (Boid other : boids) {
         float d = PVector::dist(position, other.position);
         if ((d > 0) && (d < neighbordist)) {
            sum.add(other.velocity);
            count++;
         }
      }
      if (count > 0) {
         sum.div((float)count);
         // Implement Reynolds: Steering = Desired - Velocity
         sum.setMag(maxspeed);
         PVector steer = PVector::sub(sum, velocity);
         steer.limit(maxforce);
         return steer;
      }
      else {
         return PVector(0, 0);
      }
   }

   // Cohesion
   // For the average position (i.e. center) of all nearby boids, calculate steering vector towards that position
   PVector cohesion (std::vector<Boid> &boids) {
      float neighbordist = 50;
      PVector sum(0, 0);   // Start with empty vector to accumulate all positions
      int count = 0;
      for (Boid other : boids) {
         float d = PVector::dist(position, other.position);
         if ((d > 0) && (d < neighbordist)) {
            sum.add(other.position); // Add position
            count++;
         }
      }
      if (count > 0) {
         sum.div(count);
         return seek(sum);  // Steer towards the position
      }
      else {
         return PVector(0, 0);
      }
   }
};


// The Flock (a list of Boid objects)

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

Flock flock;

void setup() {
   size(640, 360);
   // Add an initial set of boids into the system
   for (int i = 0; i < 150; i++) {
      flock.addBoid(Boid(width/2,height/2));
   }
}

void draw() {
  background(50);
   flock.run();
}

// Add a new boid into the System
void mousePressed() {
   flock.addBoid(Boid(mouseX,mouseY));
}
