#include "processing_pshape.h"
#include "processing_math.h"
#include <vector>
#include <tesselator_cpp.h>

bool PShape::isClockwise() const {
   if ( style != POLYGON && style != QUADS )
      return false;
   if (vertices.size() < 3)
      return true;

   auto current = vertices[0];
   auto prev = vertices[vertices.size()-1];
   int sum = (current.position.x - prev.position.x) * (current.position.y + prev.position.y);
   for( int i = 1 ; i < vertices.size() ; ++i) {
      auto current = vertices[i];
      auto prev = vertices[i-1];
      sum += (current.position.x - prev.position.x) * (current.position.y + prev.position.y);
   }
   return sum < 0;
}

static std::vector<unsigned short> triangulatePolygon(const std::vector<gl_context::vertex> &vertices,  std::vector<int> contour) {

   if (vertices.size() < 3) {
      return {}; // empty vector
   }

   TESSalloc ma;
   int allocated = 0;
   memset(&ma, 0, sizeof(ma));
   ma.memalloc = [] (void* userData, unsigned int size) {
      int* allocated = ( int*)userData;
      TESS_NOTUSED(userData);
      *allocated += (int)size;
      return malloc(size);
   };
   ma.memfree = [] (void* userData, void* ptr) {
      TESS_NOTUSED(userData);
      free(ptr);
   };
   ma.userData = (void*)&allocated;
   ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

   Tesselator tess(&ma);
   if (!tess)
      abort();

   tess.setOption(TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
   const int nvp = 3;

   if ( contour.empty() ) {
      tess.addContour(3, vertices.data(), sizeof(gl_context::vertex), vertices.size(), offsetof(gl_context::vertex,position));
   } else {
      tess.addContour(3, vertices.data(), sizeof(gl_context::vertex), contour[0],      offsetof(gl_context::vertex,position));
      contour.push_back(vertices.size());
      for ( int i = 0; i < contour.size() - 1; ++i ) {
         auto &c = contour[i];
         auto start = vertices.data() + c;
         auto size = contour[i+1] - contour[i];
         tess.addContour(3, start, sizeof(gl_context::vertex), size, offsetof(gl_context::vertex,position));
      }
   }

   if (!tess.tesselate(TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 3, 0))
      abort();

   // Draw tesselated pieces.
   const float* verts = tess.getVertices();
   const int* vinds = tess.getVertexIndices();
   const int* elems = tess.getElements();
   const int nverts = tess.getVertexCount();
   const int nelems = tess.getElementCount();

   // If we can't find a valid triangulation just return a dummy
   if (nelems == 0) {
      return {0,1,2};
   }

   std::vector<unsigned short> triangles;

   for (int i = 0; i < nelems; ++i)
   {
      const int* p = &elems[i*nvp];
      // TODO: Discard any triangle that needs a new vertex
      if( vinds[ p[0] ] == TESS_UNDEF ||
          vinds[ p[1] ] == TESS_UNDEF ||
          vinds[ p[2] ] == TESS_UNDEF ) continue;
      triangles.push_back( vinds[ p[0] ] );
      triangles.push_back( vinds[ p[1] ] );
      triangles.push_back( vinds[ p[2] ] );
   }

   return triangles;
}

void PShape::populateIndices() {
   if (indices.size() != 0)
      return;

   if (style == GROUP) return;

   if (vertices.size() == 0) return;

   if (style == QUADS) {
      if (vertices.size() % 4 != 0) abort();
      for (int i= 0; i< vertices.size(); i+=4) {
         auto quad = triangulatePolygon( {vertices.begin() + i, vertices.begin() + i + 4},{});
         for( auto &&j : quad ) {
            indices.push_back(j + i);
         }
      }
      style = TRIANGLES;
   } else if (style == TRIANGLE_STRIP || style == QUAD_STRIP) {
      bool reverse = false;
      for (int i = 0; i < vertices.size() - 2; i++ ){
         if (reverse) {
            indices.push_back(i+2);
            indices.push_back(i+1);
            indices.push_back(i);
         } else {
            indices.push_back(i);
            indices.push_back(i+1);
            indices.push_back(i+2);
         }
         reverse = !reverse;
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
      indices = triangulatePolygon(vertices, contour);
   } else if (style == TRIANGLES) {
      for (int i = 0; i < vertices.size(); i++ ) {
         indices.push_back( i );
      }
   } else if (style == POINTS || style == LINES) {
      // no indices required for these types.
   } else {
      abort();
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

bool anglesWithinTolerance(float angle1, float angle2, float tolerance) {
    return angularDifference(angle1, angle2)  <= tolerance;
}

PLine drawLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) {
   PLine l1{ p1, p2 };
   PLine l2{ p2, p3 };

   float a = angularDifference( l1.heading(), l2.heading() );

   // The distance the mitred corners are from the actual line corner position
   float w = std::max(half_weight, 2 * ( (half_weight / sinf( PI - a )) * sinf ( a / 2)));

   auto bisect = (l1.normal() + l2.normal()).normalize();
   return { p2 + bisect * w, p2 - bisect * w };
}

PShape drawLinePoly(int points, const gl_context::vertex *p, const PShape::vInfoExtra *extras, bool closed, const PMatrix &transform)  {
   PLine start;
   PLine end;

   PShape triangle_strip;
   triangle_strip.transform( transform );
   triangle_strip.beginShape(TRIANGLE_STRIP);
   triangle_strip.noStroke();
   triangle_strip.fill(extras[0].stroke);
   float half_weight = extras[0].weight / 2.0;
   if (closed) {
      start = drawLineMitred(p[points-1].position, p[0].position, p[1].position, half_weight );
      end = start;
   } else {
      PVector normal = (p[1].position - p[0].position).normal();
      normal.normalize();
      normal.mult(half_weight);
      start = {  p[0].position + normal, p[0].position - normal };
      normal = (p[points-1].position - p[points-2].position).normal();
      normal.normalize();
      normal.mult(half_weight);
      end = { p[points-1].position + normal, p[points-1].position - normal };
   }

   triangle_strip.vertex( start.start );
   triangle_strip.vertex( start.end );

   for (int i =0; i<points-2;++i) {
      PLine next = drawLineMitred(p[i].position, p[i+1].position, p[i+2].position, half_weight);
      triangle_strip.vertex( next.start );
      triangle_strip.vertex( next.end );
   }
   if (closed) {
      PLine next = drawLineMitred(p[points-2].position, p[points-1].position, p[0].position, half_weight);
      triangle_strip.vertex( next.start );
      triangle_strip.vertex( next.end );
   }

   triangle_strip.vertex( end.start );
   triangle_strip.vertex( end.end );

   triangle_strip.endShape(CLOSE);
   return triangle_strip;
}

PShape drawRoundLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   shape.transform( transform );

   int NUMBER_OF_VERTICES=16;

   float start_angle = (p2 - p1).heading() + HALF_PI;

   shape.noStroke();
   shape.fill(color1);
   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      shape.vertex(p1.x + cos(i + start_angle) * weight1/2, p1.y + sin(i+start_angle) * weight1/2, p1.z);
   }

   start_angle += PI;

   shape.fill(color2);
   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      shape.vertex(p2.x + cos(i+start_angle) * weight2/2, p2.y + sin(i+start_angle) * weight2/2, p2.z);
   }
   shape.endShape(CLOSE);
   return shape;
}

PShape drawLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   shape.transform( transform );
   PVector normal1 = (p2 - p1).normal();
   normal1.normalize();
   normal1.mult(weight1/2.0);
   PVector normal2 = (p2 - p1).normal();
   normal2.normalize();
   normal2.mult(weight2/2.0);

   shape.noStroke();

   shape.fill(color1);
   shape.vertex(p1 + normal1);
   shape.vertex(p1 - normal1);

   shape.fill(color2);
   shape.vertex(p2 - normal2);
   shape.vertex(p2 + normal2);

   shape.endShape(CLOSE);
   return shape;
}

PShape drawCappedLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShape shape;
   shape.beginShape(CONVEX_POLYGON);
   shape.transform( transform );
   PVector normal1 = (p2 - p1).normal();
   normal1.normalize();
   normal1.mult(weight1/2.0);

   PVector normal2 = (p2 - p1).normal();
   normal2.normalize();
   normal2.mult(weight2/2.0);

   PVector end_offset1 = (p2 - p1);
   end_offset1.normalize();
   end_offset1.mult(weight1/2.0);
   PVector end_offset2 = (p2 - p1);
   end_offset2.normalize();
   end_offset2.mult(weight2/2.0);

   shape.noStroke();

   shape.fill(color1);
   shape.vertex(p1 + normal1 - end_offset1);
   shape.vertex(p1 - normal1 - end_offset1);

   shape.fill(color2);
   shape.vertex(p2 - normal2 + end_offset2);
   shape.vertex(p2 + normal2 + end_offset2);

   shape.endShape(CLOSE);
   return shape;
}

PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, color color, const PMatrix &transform) {
   PShape shape;
   shape.circleTexture();
   shape.noStroke();
   shape.fill(color);
   shape.transform( transform );
   shape.beginShape(TRIANGLES);
   x = x - width / 2.0;
   y = y - height / 2.0;
   shape.vertex(x,y,0,0);
   shape.vertex(x+width,y,1.0,0);
   shape.vertex(x+width,y+height,1.0,1.0);
   shape.vertex(x,y+height,0,1.0);
   shape.populateIndices( { 0,1,2,0,2,3 } );
   shape.endShape(CLOSE);
   return shape;
}

void _line(PShape &triangles, PVector p1, PVector p2, float weight1, float weight2, color color1, color color2 ) {

   PVector normal1 = (p2 - p1).normal();
   normal1.normalize();
   normal1.mult(weight1/2.0);

   PVector normal2 = (p2 - p1).normal();
   normal2.normalize();
   normal2.mult(weight2/2.0);

   unsigned short i = triangles.getCurrentIndex();
   triangles.fill( color1 );
   triangles.vertex( p1 + normal1 );
   triangles.vertex( p1 - normal1 );
   triangles.fill( color2 );
   triangles.vertex( p2 - normal2 );
   triangles.vertex( p2 + normal2 );

   triangles.index( i + 0 );
   triangles.index( i + 1 );
   triangles.index( i + 2 );

   triangles.index( i + 0 );
   triangles.index( i + 2 );
   triangles.index( i + 3 );
}

PShape drawTriangleStrip(int points, const gl_context::vertex *p, const PShape::vInfoExtra *extras, const PMatrix &transform ) {
   PShape triangles;
   triangles.transform( transform );
   triangles.beginShape(TRIANGLES);
   _line(triangles, p[0].position, p[1].position,
         extras[0].weight, extras[1].weight, extras[0].stroke, extras[1].stroke);
   for (int i=2;i<points;++i) {
      _line(triangles, p[i-1].position, p[i].position, extras[i-1].weight, extras[i].weight, extras[i-1].stroke, extras[i].stroke);
      _line(triangles, p[i].position, p[i-2].position, extras[i].weight, extras[i-2].weight, extras[i].stroke, extras[i-2].stroke);
   }
   triangles.endShape();
   return triangles;
}

PShape drawTriangleNormal(int points, const gl_context::vertex *p,
                          const PShape::vInfoExtra *extras, bool closed,
                          const PMatrix &transform) {
   PShape shape;
   shape.beginShape(TRIANGLES);
   shape.fill(RED);
   shape.transform( transform );
   PVector pos = (p[0].position + p[1].position + p[2].position) / 3;
   PVector n = ((p[0].normal + p[1].normal + p[2].normal) / 3).normalize();
   float length = (p[0].position - p[1].position).mag() / 10.0f;
   _line(shape, pos, pos + length * n, length/10.0f,length/10.0f,RED,RED);
   shape.endShape();
   return shape;
}

void PShape::draw_normals(gl_context &glc, const PMatrix &transform) const {
   switch( style ) {
   case TRIANGLES_NOSTROKE:
   case TRIANGLES:
   case TRIANGLE_STRIP:
   case QUAD_STRIP:
   case POLYGON:
   case CONVEX_POLYGON:
   case TRIANGLE_FAN:
      // All of these should have just been flattened to triangles
      for (int i = 0; i < indices.size(); i+=3 ) {
         std::vector<gl_context::vertex> triangle;
         std::vector<vInfoExtra> xtras;
         triangle.push_back( vertices[indices[i]] );
         triangle.push_back( vertices[indices[i+1]] );
         triangle.push_back( vertices[indices[i+2]] );
         xtras.push_back( extras[indices[i]] );
         xtras.push_back( extras[indices[i+1]] );
         xtras.push_back( extras[indices[i+2]] );
         drawTriangleNormal( 3, triangle.data(), xtras.data(), false, shape_matrix).draw_fill( glc, transform );
      }
      break;
   case POINTS:
   case LINES:
      break;
   default:
      abort();
      break;
   }
}

void PShape::draw_stroke(gl_context &glc, const PMatrix& transform) const {
   switch( style ) {
   case POINTS:
   {
      for (int i = 0; i< vertices.size() ; ++i ) {
         drawUntexturedFilledEllipse(
            vertices[i].position.x, vertices[i].position.y,
            extras[i].weight, extras[i].weight,
            extras[i].stroke, shape_matrix ).draw_fill( glc, transform );
      }
      break;
   }
   case TRIANGLES_NOSTROKE:
      break;
   case TRIANGLES:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      PShape shape;
      shape.beginShape(TRIANGLES);
      for (int i = 0; i < indices.size(); i+=3 ) {
         PVector p0 = vertices[indices[i]].position;
         PVector p1 = vertices[indices[i+1]].position;
         PVector p2 = vertices[indices[i+2]].position;
         float w0 = extras[indices[i]].weight;
         float w1 = extras[indices[i+1]].weight;
         float w2 = extras[indices[i+2]].weight;
         color c0 =  extras[indices[i]].stroke;
         color c1 =  extras[indices[i+1]].stroke;
         color c2 =  extras[indices[i+2]].stroke;

         _line(shape, p0, p1, w0, w1, c0, c1 );
         _line(shape, p1, p2, w1, w2, c1, c2 );
         _line(shape, p2, p0, w2, w0, c2, c0 );
      }
      shape.endShape();
      shape.draw_fill( glc, transform );
      break;
   }
   case POLYGON:
   case CONVEX_POLYGON:
   case LINES:
   {
      if (vertices.size() > 2 ) {
         if (type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
            drawLinePoly( vertices.size() - 1, vertices.data() + 1, extras.data()+1, false, shape_matrix).draw_fill( glc, transform );
         } else {
            if ( contour.empty() ) {
               drawLinePoly( vertices.size(), vertices.data(), extras.data(), type == CLOSE, shape_matrix).draw_fill( glc, transform );
            } else {
               drawLinePoly( contour[0], vertices.data(), extras.data(), type == CLOSE, shape_matrix).draw_fill( glc, transform );
               auto q = contour;
               q.push_back(vertices.size());
               for ( int i = 0; i < q.size() - 1; ++i ) {
                  drawLinePoly( q[i+1] - q[i],
                                vertices.data() + q[i],
                                extras.data() + q[i],
                                type == CLOSE, shape_matrix).draw_fill( glc, transform );
               }
            }
         }
      } else if (vertices.size() == 2) {
         switch(line_end_cap) {
         case ROUND:
            drawRoundLine( vertices[0].position, vertices[1].position,
                           extras[0].weight, extras[1].weight,
                           extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( glc, transform );
            break;
         case PROJECT:
            drawCappedLine( vertices[0].position, vertices[1].position,
                            extras[0].weight, extras[1].weight,
                            extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( glc, transform );
            break;
         case SQUARE:
            drawLine( vertices[0].position, vertices[1].position,
                      extras[0].weight, extras[1].weight,
                      extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( glc, transform );
            break;
         default:
            abort();
         }
      } else if (vertices.size() == 1) {
         drawUntexturedFilledEllipse(
            vertices[0].position.x, vertices[0].position.y,
            extras[0].weight, extras[0].weight,
            extras[0].stroke, shape_matrix ).draw_fill( glc, transform );
      }
      break;
   }
   case QUAD_STRIP:
      // This isn't exactly right since we draw an extra line for every quad,
      // but it's close enought for now.
   case TRIANGLE_STRIP:
      drawTriangleStrip( vertices.size(),  vertices.data(), extras.data(), shape_matrix ).draw_fill( glc, transform );
      break;
   case TRIANGLE_FAN:
      abort();
      break;
   default:
      abort();
      break;
   }
}

void PShape::draw_fill(gl_context &glc, const PMatrix& transform) const {
   if (vertices.size() > 2 && style != POINTS && style != LINES) {
      glc.drawTriangles( vertices, indices, texture_, transform * shape_matrix );
   }
}
