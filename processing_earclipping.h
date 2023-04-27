#ifndef PROCESSING_EARCLIPPING_H
#define PROCESSING_EARCLIPPING_H

#include "processing_math.h"
#include <vector>


struct indexed_PVector : public PVector {
   unsigned short int i;
   indexed_PVector(PVector v,unsigned short i_) : PVector(v), i(i_) {}
};

bool isPointInTriangle(const indexed_PVector &point, const indexed_PVector &v0,
                       const indexed_PVector &v1, const indexed_PVector &v2) {
    // Calculate barycentric coordinates of the point with respect to the triangle
    double alpha = ((v1.y - v2.y) * (point.x - v2.x) + (v2.x - v1.x) * (point.y - v2.y)) / 
                   ((v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y));
    double beta = ((v2.y - v0.y) * (point.x - v2.x) + (v0.x - v2.x) * (point.y - v2.y)) / 
                  ((v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y));
    double gamma = 1.0 - alpha - beta;
    // Check if the barycentric coordinates are all positive
    return alpha >= 0 && beta >= 0 && gamma >= 0;
}

bool isEar(const std::vector<indexed_PVector>& polygon, int i) {
    int n = polygon.size();
    int pi = (i + n - 1) % n;
    int ni = (i + 1) % n;
    // Check if there are any other vertices inside the triangle
    for (int j = 0; j < n; j++) {
       if (j != i && j != ni && j != pi) {
          if (isPointInTriangle(polygon[j], polygon[pi], polygon[i], polygon[ni])) {
             return false;
          }
       }
    }
    return true;
}

bool isConvexPVector(const std::vector<indexed_PVector>& polygon, int i) {
    int n = polygon.size();
    PVector prev = polygon[(i + n - 1) % n];
    PVector curr = polygon[i];
    PVector next = polygon[(i + 1) % n];
    double crossProduct = (curr.x - prev.x) * (next.y - curr.y) - (curr.y - prev.y) * (next.x - curr.x);
    return !(crossProduct < 0);
}

int findEar(const std::vector<indexed_PVector>& polygon) {
    int n = polygon.size();
    for (int i = 0; i < n; i++) {
        // Check if vertex i is a convex vertex
        if (isConvexPVector(polygon, i)) {
            // Check if the triangle i-1, i, i+1 is an ear
            if (isEar(polygon, i)) {
                return i;
            }
        }
    }
    // If no ear is found, return the first vertex
    return 0;
}

bool isClockwise(const std::vector<PVector> &polygon) {
   auto current = polygon[0];
   auto prev = polygon[polygon.size()-1];
   int sum = (current.x - prev.x) * (current.y + prev.y);
   for( int i = 1 ; i < polygon.size() ; ++i) {
      auto current = polygon[i];
      auto prev = polygon[i+1];
      sum += (current.x - prev.x) * (current.y + prev.y);
   }
   return sum < 0;
}

std::vector<unsigned short> triangulatePolygon(const std::vector<PVector> &polygon_) {
   std::vector<indexed_PVector> polygon;
   int index = 0;
   if (isClockwise(polygon_)) {
      for (auto v = polygon_.rbegin(); v != polygon_.rend(); v++ ) {
         polygon.emplace_back( *v, index++ );
      }
   } else {
      for (auto &&vertex : polygon_) {
         polygon.emplace_back( vertex, index++ );
     }
   }

   std::vector<unsigned short> triangles;
   if (polygon.size() < 3) {
      return triangles; // empty vector
   }
   while (polygon.size() > 3) {
      int i = findEar(polygon); // find an ear of the polygon
      // add the ear as separate triangles to the output vector
      int n = polygon.size();
      int pi = (i + n - 1) % n;
      int ni = (i + 1) % n;
      triangles.push_back(polygon[pi].i);
      triangles.push_back(polygon[i].i);
      triangles.push_back(polygon[ni].i);
      polygon.erase(polygon.begin() + i); // remove the ear vertex from the polygon
   }
   // add the final triangle to the output vector
   triangles.push_back(polygon[0].i);
   triangles.push_back(polygon[1].i);
   triangles.push_back(polygon[2].i);
   return triangles;
}

#endif
