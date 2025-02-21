#include <glm/gtx/norm.hpp>

void setup() {
   size(640, 480);
   //noLoop();
}

// Function to compute signed distance from point P to a finite line segment (x1, y1) -> (x2, y2)
float dLine(const glm::vec2& P, const glm::vec2& A, const glm::vec2& B) {
   glm::vec2 AB = B - A;  // Segment direction
   glm::vec2 AP = P - A;  // Vector from A to P

   float proj = glm::dot(AP, AB) / glm::length2(AB); // Project AP onto AB (normalized)

   // Clamp projection to stay within the segment
   proj = glm::clamp(proj, 0.0f, 1.0f);

   glm::vec2 closestPoint = A + proj * AB;  // Compute the closest point on the segment
   glm::vec2 normal = P - closestPoint;  // Vector from closest point to P

   return glm::length2(normal);  // Return sqaured distance (positive always)
}

float dClosedPoly(const glm::vec2& P, const std::vector<glm::vec2> &A) {
   float a = dLine(P,A[0],A[1]);
   for(int i = 1; i < A.size()-1; ++i){
      a = std::min( a, dLine(P,A[i],A[i+1]) );
   }
   a = std::min( a, dLine(P, A[A.size()-1], A[0]));
   return a;
}

void draw() {
   float dWeight = (10.0 * 10.0) / 2.0;

   loadPixels();
   for(int i = 0; i < width; i++) {
      for( int j = 0; j < height; j++) {
         glm::vec2 P(i,j);
         float dist = dClosedPoly(P, {
               glm::vec2(mouseX,mouseY),
               glm::vec2(20,height-20),
               glm::vec2(width-20,height-20),
               glm::vec2(width-20,20)
            } );
         dist = std::min( dist, dLine(P, glm::vec2(40,40), glm::vec2(width-40,height-40)) );
         if ( dist > dWeight )
            pixels[j*width+i] = LIGHT_GRAY;
         else
            pixels[j*width+i] = WHITE;
      }
   }
   updatePixels();
}
