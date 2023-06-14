#include "processing_pshape.h"
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

void PShape::populateIndices() {
   if (indices.size() != 0)
      return;

   if (vertices.size() == 0) abort();

   if (style == QUADS) {
      if (vertices.size() % 4 != 0) abort();
      for (int i= 0; i< vertices.size(); i+=4) {
         auto quad = triangulatePolygon( {vertices.begin() + i, vertices.begin() + i + 4} );
         for( auto &&j : quad ) {
            indices.push_back(j + i);
         }
      }
      style = TRIANGLES;
   } else if (style == TRIANGLE_STRIP || style == QUAD_STRIP) {
      for (int i = 0; i < vertices.size() - 2; i++ ){
         indices.push_back(i);
         indices.push_back(i+1);
         indices.push_back(i+2);
      }
      style = TRIANGLE_STRIP;
   } else if (style == CONVEX_POLYGON) {
      // Fill with triangle fan
      for (int i = 1; i < vertices.size() - 1 ; i++ ) {
         indices.push_back( 0 );
         indices.push_back( i );
         indices.push_back( i+1 );
      }
   }  else if (style == TRIANGLE_FAN) {
      // Fill with triangle fan
      for (int i = 1; i < vertices.size() - 1 ; i++ ) {
         indices.push_back( 0 );
         indices.push_back( i );
         indices.push_back( i+1 );
      }
   } else if (style == POLYGON) {
      indices = triangulatePolygon({vertices.begin(),vertices.end()});
   } else if (style == TRIANGLES) {
      for (int i = 0; i < vertices.size(); i++ ) {
         indices.push_back( i );
      }
   }
}

const constexpr float xsincos[32][2] = {
   { 1.000000, -0.000000 },
   { 0.980785, 0.195090 },
   { 0.923880, 0.382683 },
   { 0.831470, 0.555570 },
   { 0.707107, 0.707107 },
   { 0.555570, 0.831470 },
   { 0.382683, 0.923880 },
   { 0.195090, 0.980785 },
   { 0.000000, 1.000000 },
   { -0.195090, 0.980785 },
   { -0.382683, 0.923880 },
   { -0.555570, 0.831470 },
   { -0.707107, 0.707107 },
   { -0.831470, 0.555570 },
   { -0.923880, 0.382683 },
   { -0.980785, 0.195090 },
   { -1.000000, -0.000000 },
   { -0.980785, -0.195090 },
   { -0.923880, -0.382683 },
   { -0.831470, -0.555570 },
   { -0.707107, -0.707107 },
   { -0.555570, -0.831470 },
   { -0.382683, -0.923880 },
   { -0.195091, -0.980785 },
   { -0.000000, -1.000000 },
   { 0.195090, -0.980785 },
   { 0.382683, -0.923880 },
   { 0.555570, -0.831470 },
   { 0.707107, -0.707107 },
   { 0.831469, -0.555570 },
   { 0.923880, -0.382684 },
   { 0.980785, -0.195090 } };

PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius) {
   return PVector( center.x + xradius * xsincos[index][0],
                   center.y + yradius * xsincos[index][1],
                   center.z);
}

PLine drawLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) {
   PLine l1{ p1, p2 };
   PLine l2{ p2, p3 };
   PLine low_l1 = l1.offset(-half_weight);
   PLine high_l1 = l1.offset(half_weight);
   PLine low_l2 = l2.offset(-half_weight);
   PLine high_l2 = l2.offset(half_weight);
   return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
}

PShape drawLinePoly(int points, const PVector *p, int weight, bool closed)  {
   PLine start;
   PLine end;

   PShape triangle_strip;
   triangle_strip.beginShape(TRIANGLE_STRIP);

   float half_weight = weight / 2.0;
   if (closed) {
      start = drawLineMitred(p[points-1], p[0], p[1], half_weight );
      end = start;
   } else {
      PVector normal = (p[1] - p[0]).normal();
      normal.normalize();
      normal.mult(half_weight);
      start = {  p[0] + normal, p[0] - normal };
      normal = (p[points-1] - p[points-2]).normal();
      normal.normalize();
      normal.mult(half_weight);
      end = { p[points-1] + normal, p[points-1] - normal };
   }

   triangle_strip.vertex( start.start );
   triangle_strip.vertex( start.end );

   for (int i =0; i<points-2;++i) {
      PLine next = drawLineMitred(p[i], p[i+1], p[i+2], half_weight);
      triangle_strip.vertex( next.start );
      triangle_strip.vertex( next.end );
   }
   if (closed) {
      PLine next = drawLineMitred(p[points-2], p[points-1], p[0], half_weight);
      triangle_strip.vertex( next.start );
      triangle_strip.vertex( next.end );
   }

   triangle_strip.vertex( end.start );
   triangle_strip.vertex( end.end );

   triangle_strip.endShape(CLOSE);
   return triangle_strip;
}

PShape drawRoundLine(PVector p1, PVector p2, int weight) {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   int NUMBER_OF_VERTICES=16;

   float start_angle = PVector{p2.x-p1.x,p2.y-p1.y}.heading() + HALF_PI;

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      shape.vertex(p1.x + cos(i + start_angle) * weight/2, p1.y + sin(i+start_angle) * weight/2);
   }

   start_angle += PI;

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      shape.vertex(p2.x + cos(i+start_angle) * weight/2, p2.y + sin(i+start_angle) * weight/2);
   }
   shape.endShape(CLOSE);
   return shape;
}

PShape drawLine(PVector p1, PVector p2, int weight) {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   shape.vertex(p1 + normal);
   shape.vertex(p1 - normal);
   shape.vertex(p2 - normal);
   shape.vertex(p2 + normal);
   shape.endShape(CLOSE);
   return shape;
}

PShape drawCappedLine(PVector p1, PVector p2, int weight)  {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   PVector end_offset = PVector{p2.x-p1.x,p2.y-p1.y};
   end_offset.normalize();
   end_offset.mult(weight/2.0);

   shape.vertex(p1 + normal - end_offset);
   shape.vertex(p1 - normal - end_offset);
   shape.vertex(p2 - normal + end_offset);
   shape.vertex(p2 + normal + end_offset);

   shape.endShape(CLOSE);
   return shape;
}


PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, color color) {
   PShape shape;
   // Hack to get the circle texture
   shape.texture( PTexture::circle() );
   shape.noStroke();
   shape.fill( color );
   shape.beginShape(TRIANGLES);
   x = x - width / 2.0;
   y = y - height / 2.0;
   shape.vertex(x,y,0,0);
   shape.vertex(x+width,y,1.0,0);
   shape.vertex(x+width,y+height,1.0,1.0);
   shape.vertex(x,y+height,0,1.0);
   shape.indices = { 0,1,2,0,2,3 };
   shape.endShape(CLOSE);
   return shape;
}

void _line(PShape &triangles, PVector p1, PVector p2, int weight) {

   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   unsigned short i = triangles.getCurrentIndex();
   triangles.vertex( p1 + normal );
   triangles.vertex( p1 - normal );
   triangles.vertex( p2 - normal );
   triangles.vertex( p2 + normal );

   triangles.index( i + 0 );
   triangles.index( i + 1 );
   triangles.index( i + 2 );

   triangles.index( i + 0 );
   triangles.index( i + 2 );
   triangles.index( i + 3 );
}

PShape drawTriangleStrip(int points, const PVector *p,int weight) {
   PShape triangles;
   triangles.beginShape(TRIANGLES);
   _line(triangles, p[0], p[1], weight);
   for (int i=2;i<points;++i) {
      _line(triangles, p[i-1], p[i], weight);
      _line(triangles, p[i], p[i-2], weight);
   }
   triangles.endShape();
   return triangles;
}

void PShape::draw_stroke(TriangleDrawer &glc) {
   switch( style ) {
   case POINTS:
   {
      for (auto z : vertices ) {
         drawUntexturedFilledEllipse(z.x, z.y, stroke_weight, stroke_weight, stroke_color ).draw( glc );
      }
      break;
   }
   case TRIANGLES_NOSTROKE:
      break;
   case TRIANGLES:
   case POLYGON:
   case CONVEX_POLYGON:
   case LINES:
   {
      if (vertices.size() > 2 ) {
         if (type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
            drawLinePoly( vertices.size() - 1, vertices.data() + 1, stroke_weight, false).draw_fill( glc, stroke_color );
         } else {
            drawLinePoly( vertices.size(), vertices.data(), stroke_weight, type == CLOSE).draw_fill( glc, stroke_color );
         }
      } else if (vertices.size() == 2) {
         switch(line_end_cap) {
         case ROUND:
            drawRoundLine(vertices[0], vertices[1], stroke_weight).draw_fill( glc, stroke_color );
            break;
         case PROJECT:
            drawCappedLine(vertices[0], vertices[1], stroke_weight).draw_fill( glc, stroke_color );
            break;
         case SQUARE:
            drawLine(vertices[0], vertices[1], stroke_weight).draw_fill( glc, stroke_color );
            break;
         default:
            abort();
         }
      } else if (vertices.size() == 1) {
         drawUntexturedFilledEllipse(vertices[0].x, vertices[0].y, stroke_weight, stroke_weight, stroke_color).draw( glc );
      }
      break;
   }
   case TRIANGLE_STRIP:
   {
      drawTriangleStrip( vertices.size(), vertices.data(), stroke_weight).draw_fill( glc, stroke_color );
      break;
   }
   default:
      abort();
      break;
   }
}

void PShape::draw_fill(TriangleDrawer &glc, color color)  {
   if (vertices.size() > 2 && style != POINTS) {
      if (normals.size() == 0) {
         normals.resize( vertices.size(), {0.0f,0.0f,0.0f });
         // Iterate over all triangles
         for (int i = 0; i < indices.size()/3; i++) {
            // Get the vertices of the current triangle
            PVector v1 = vertices[indices[i * 3]];
            PVector v2 = vertices[indices[i * 3 + 1]];
            PVector v3 = vertices[indices[i * 3 + 2]];

            // Calculate the normal vector of the current triangle
            PVector edge1 = v2 - v1;
            PVector edge2 = v3 - v1;
            PVector normal = (edge1.cross(edge2)).normalize();

            // Add the normal to the normals list for each vertex of the triangle
            normals[indices[i * 3]] = normals[indices[i * 3]] + normal;
            normals[indices[i * 3 + 1]] = normals[indices[i * 3 + 1]] + normal;
            normals[indices[i * 3 + 2]] = normals[indices[i * 3 + 2]] + normal;
         }

         // Normalize all the normals
         for (int i = 0; i < normals.size(); i++) {
            normals[i].normalize();
         }
      }
      // not glc, need to call pgraphics drawTriangles.
      glc.drawTriangles(  vertices, normals, coords, indices, color );
   }
}
