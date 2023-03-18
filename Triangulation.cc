#include "processing.h"

/**
 * Triangulation
 *
 * Demostrate triangulation of arbitaty polygons.
 */

std::vector<PVector> star(float x, float y, float radius1, float radius2, int npoints) {
   float angle = TWO_PI / npoints;
   float halfAngle = angle/2.0;
   std::vector<PVector> shape;
   for (float a = 0; a < TWO_PI; a += angle) {
      float sx = x + cos(a) * radius2;
      float sy = y + sin(a) * radius2;
      shape.emplace_back(sx, sy);
      sx = x + cos(a+halfAngle) * radius1;
      sy = y + sin(a+halfAngle) * radius1;
      shape.emplace_back(sx, sy);
   }
   return shape;
}

std::vector<PVector> polygon;
std::vector<PVector> triangles;

void setup() {
   size(640, 360);
   frameRate(2);
}

void draw() {

   if (polygon.size() == 0) {
      triangles.clear();
      // Setup our convex polygon
      polygon = star(0, 0, 30, 70, 5);
   } else if (polygon.size() > 3) {
      int i = findEar(polygon); // find an ear of the polygon
      int n = polygon.size();
      int pi = (i + n - 1) % n;
      int ni = (i + 1) % n;
      // add the ear as separate triangles to the output vector
      triangles.push_back(polygon[pi]);
      triangles.push_back(polygon[i]);
      triangles.push_back(polygon[ni]);
      polygon.erase(polygon.begin() + i); // remove the ear vertex from the polygon
   } else {
      triangles.push_back(polygon[0]);
      triangles.push_back(polygon[1]);
      triangles.push_back(polygon[2]);
      polygon.clear(); // remove last triangle completely
   }

   // Clear background
   background(BLACK);

   // Draw current triangles
   noStroke();
   fill(RED);
   translate(width/4,height/2);
   for (int i = 0 ; i<triangles.size() ; i+=3) {
      triangle(triangles[i].x,   triangles[i].y,
               triangles[i+1].x, triangles[i+1].y,
               triangles[i+2].x, triangles[i+2].y);
   }

   // Draw remaining polygon
   noFill();
   strokeWeight(2);
   stroke(YELLOW);
   translate(width/2,0);
   beginShape();
   for (int i = 0 ; i<polygon.size() ; i++) {
      vertex(polygon[i].x,polygon[i].y);
   }
   endShape(CLOSE);

}
