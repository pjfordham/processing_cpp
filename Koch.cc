#include "processing.h"

/**
 * Koch Curve
 * by Daniel Shiffman.
 *
 * Renders a simple fractal, the Koch snowflake.
 * Each recursive level is drawn in sequence.
 */


// The Nature of Code
// Daniel Shiffman
// http://natureofcode.com

// Koch Curve
// A class to describe one line segment in the fractal
// Includes methods to calculate midPVectors along the line according to the Koch algorithm

class KochLine {

   // Two PVectors,
   // a is the "left" PVector and
   // b is the "right PVector
   PVector a;
   PVector b;

public:
   KochLine(PVector start, PVector end) : a(start), b(end) {
   }

   void display() {
      stroke(255);
      line(a.x, a.y, b.x, b.y);
   }

   PVector start() {
      return a;
   }

   PVector end() {
      return b;
   }

   // This is easy, just 1/3 of the way
   PVector kochleft() {
      PVector v = PVector::sub(b, a);
      v.div(3);
      v.add(a);
      return v;
   }

   // More complicated, have to use a little trig to figure out where this PVector is!
   PVector kochmiddle() {
      PVector v = PVector::sub(b, a);
      v.div(3);

      PVector p = a;
      p.add(v);

      v.rotate(-radians(60));
      p.add(v);

      return p;
   }

   // Easy, just 2/3 of the way
   PVector kochright() {
      PVector v = PVector::sub(a, b);
      v.div(3);
      v.add(b);
      return v;
   }
};

// Koch Curve
// A class to manage the list of line segments in the snowflake pattern

class KochFractal {
   PVector start;       // A PVector for the start
   PVector end;         // A PVector for the end
   std::vector<KochLine> lines;   // A list to keep track of all the lines
   int count;

public:
   KochFractal() : start(0,height-20), end(width,height-20) {
      restart();
   }

   void nextLevel() {
      // For every line that is in the arraylist
      // create 4 more lines in a new arraylist
      lines = iterate(lines);
      count++;
   }

   void restart() {
      count = 0;      // Reset count
      lines.clear();  // Empty the array list
      lines.emplace_back(start,end);  // Add the initial line (from one end PVector to the other)
   }

   int getCount() {
      return count;
   }

   // This is easy, just draw all the lines
   void render() {
      for(KochLine l : lines) {
         l.display();
      }
   }

   // This is where the **MAGIC** happens
   // Step 1: Create an empty arraylist
   // Step 2: For every line currently in the arraylist
   //   - calculate 4 line segments based on Koch algorithm
   //   - add all 4 line segments into the new arraylist
   // Step 3: Return the new arraylist and it becomes the list of line segments for the structure

   // As we do this over and over again, each line gets broken into 4 lines, which gets broken into 4 lines, and so on. . .
   std::vector<KochLine> iterate(std::vector<KochLine> &before) {
      std::vector<KochLine> now;    // Create emtpy list
      for(KochLine l : before) {
         // Calculate 5 koch PVectors (done for us by the line object)
         PVector a = l.start();
         PVector b = l.kochleft();
         PVector c = l.kochmiddle();
         PVector d = l.kochright();
         PVector e = l.end();
         // Make line segments between all the PVectors and add them
         now.emplace_back(a,b);
         now.emplace_back(b,c);
         now.emplace_back(c,d);
         now.emplace_back(d,e);
      }
      return now;
   }

};

KochFractal k;

void setup() {
   size(640, 360);
   frameRate(1);  // Animate slowly
   k = KochFractal(); // redo it here to pick up width and height.
}

void draw() {
   background(0);
   // Draws the snowflake!
   k.render();
   // Iterate
   k.nextLevel();
   // Let's not do it more than 5 times. . .
   if (k.getCount() > 5) {
      k.restart();
   }
}
