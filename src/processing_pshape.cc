#include "processing_pshape.h"
#include "processing_pshape_svg.h"
#include "processing_math.h"
#include <vector>
#include <tesselator_cpp.h>
#include <unordered_map>

#include "processing_math.h"
#include "processing_color.h"
#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_pimage.h"
#include "processing_pmaterial.h"

#include "processing_debug.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

template <> struct fmt::formatter<PShapeImpl>;

class PShapeImpl {
   friend struct fmt::formatter<PShapeImpl>;
   friend struct std::hash<PShapeImpl>;

public:
   static const PImage getBlankTexture() {
      static PImage blankTexture = createBlankImage();
      return blankTexture;
   }

   struct vInfoExtra {
      color stroke;
      float weight;
   };

private:

   bool setNormals = false;
   PVector n = { 0.0, 0.0, 0.0 };

   std::vector<int> contour;
   std::vector<gl::vertex> vertices;
   std::vector<vInfoExtra> extras;
   std::vector<PShape> children;
   std::vector<unsigned short> indices;

   mutable bool dirty = true;

   int type = OPEN;
   int mode = IMAGE;
   float stroke_weight = 1.0f;
   int line_end_cap = ROUND;
   float tightness = 0.0f;
   std::vector<PVector> curve_vertices;
   PMatrix shape_matrix = PMatrix::Identity();
   color stroke_color = BLACK;
   PImage texture_ = getBlankTexture();
   gl::color gl_fill_color = gl::flatten_color_mode(WHITE);
   color fill_color = WHITE;
   color tint_color = WHITE;
   int style = POLYGON;
   bool compiled = false;
   gl::batch_t batch;

public:

   float width = 1.0;
   float height = 1.0;

   const PMatrix& getShapeMatrix() const {
      DEBUG_METHOD();
      return shape_matrix;
   }

   PShapeImpl(const PShapeImpl&) = delete;
   PShapeImpl& operator=(const PShapeImpl&) = delete;

   PShapeImpl(PShapeImpl&& x) {
      *this = std::move(x);
   }

   PShapeImpl& operator=(PShapeImpl&& other) noexcept {
      std::swap(n,other.n);
      std::swap(setNormals,other.setNormals);
      std::swap(contour,other.contour);
      std::swap(vertices,other.vertices);
      std::swap(extras,other.extras);
      std::swap(children,other.children);
      std::swap(indices,other.indices);
      std::swap(dirty,other.dirty);
      std::swap(type,other.type);
      std::swap(mode,other.mode);
      std::swap(stroke_weight,other.stroke_weight);
      std::swap(line_end_cap,other.line_end_cap);
      std::swap(tightness,other.tightness);
      std::swap(curve_vertices,other.curve_vertices);
      std::swap(shape_matrix,other.shape_matrix);
      std::swap(stroke_color,other.stroke_color);
      std::swap(texture_,other.texture_);
      std::swap(gl_fill_color,other.gl_fill_color);
      std::swap(fill_color,other.fill_color);
      std::swap(tint_color,other.tint_color);
      std::swap(style,other.style);
      std::swap(width,other.width);
      std::swap(height,other.height);
      return *this;
   }

   void reserve(int v, int i) {
      DEBUG_METHOD();
      vertices.reserve(v);
      extras.reserve(v);
      indices.reserve(i);
   }

   PShapeImpl() {
      DEBUG_METHOD();
      reserve(4,6);
   }

   ~PShapeImpl() {
      DEBUG_METHOD();
   }

   void addChild( const PShape shape ) {
      DEBUG_METHOD();
      dirty=true;
      children.push_back( shape );
   }

   PShape getChild( int i ) {
      DEBUG_METHOD();
      return children[i];
   }

   color getStrokeColor() const {
      DEBUG_METHOD();
      return stroke_color;
   }

   float getStrokeWeight() const {
      DEBUG_METHOD();
      return stroke_weight;
   }

   color getFillColor() const {
      DEBUG_METHOD();
      return fill_color;
   }

   color getTintColor() const {
      DEBUG_METHOD();
      return tint_color;
   }

   bool isGroup() const {
      DEBUG_METHOD();
      return style == GROUP;
   }

   void copyStyle( const PShapeImpl &other ) {
      DEBUG_METHOD();
      dirty=true;
      texture_= other.texture_;
      n = other.n;
      stroke_color = other.stroke_color;
      fill_color = other.fill_color;
      gl_fill_color = other.gl_fill_color;
      tint_color = other.tint_color;
      stroke_weight = other.stroke_weight;
      line_end_cap = other.line_end_cap;
   }

   void clear() {
      DEBUG_METHOD();
      dirty = true;
      vertices.clear();
      extras.clear();
      indices.clear();
      children.clear();
      batch.clear();
   }

   void rotate(float angle) {
      DEBUG_METHOD();
      rotateZ(angle);
   }

   void rotateZ(float angle) {
      DEBUG_METHOD();
      rotate(angle ,PVector{0,0,1});
   }

   void rotateY(float angle) {
      DEBUG_METHOD();
      rotate(angle, PVector{0,1,0});
   }

   void rotateX(float angle) {
      DEBUG_METHOD();
      rotate(angle, PVector{1,0,0});
   }

   void rotate(float angle, PVector axis) {
      DEBUG_METHOD();
      dirty=true;
      shape_matrix = shape_matrix * RotateMatrix(angle,axis);
   }

   void translate(float x, float y, float z=0) {
      DEBUG_METHOD();
      translate(PVector{x,y,z});
   }

   void translate(PVector t) {
      DEBUG_METHOD();
      dirty=true;
      shape_matrix = shape_matrix * TranslateMatrix(t);
   }

   void scale(float x, float y,float z = 1) {
      DEBUG_METHOD();
      dirty=true;
      shape_matrix = shape_matrix * ScaleMatrix(PVector{x,y,z});
   }

   void scale(float x) {
      DEBUG_METHOD();
      dirty=true;
      scale(x,x,x);
   }

   void transform(const PMatrix &transform) {
      DEBUG_METHOD();
      dirty=true;
      shape_matrix = shape_matrix * transform;
   }

   void resetMatrix() {
      DEBUG_METHOD();
      dirty=true;
      shape_matrix = PMatrix::Identity();
   }

   void beginShape(int style_ = POLYGON) {
      DEBUG_METHOD();
      dirty=true;
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      style = style_;
      clear();
   }

   void beginContour() {
      DEBUG_METHOD();
      dirty=true;
      contour.push_back(vertices.size());
   }

   void endContour() {
      DEBUG_METHOD();
   }

   void textureMode( int mode_ ) {
      DEBUG_METHOD();
      dirty=true;
      mode = mode_;
   }

   bool isTextureSet() const {
      DEBUG_METHOD();
      return texture_ != getBlankTexture();
   }

   void material(PMaterial &mat) {
      DEBUG_METHOD();
      dirty=true;
      noStroke();
      textureMode( NORMAL );
      texture( mat.texture );
   }

   void texture(PImage img) {
      DEBUG_METHOD();
      dirty=true;
      texture_ = img;
   }

   void circleTexture() {
      DEBUG_METHOD();
      dirty=true;
      mode = NORMAL;
      texture_ = PImage::circle();
   }

   void noTexture() {
      DEBUG_METHOD();
      dirty=true;
      texture_ = getBlankTexture();
   }

   void noNormal() {
      DEBUG_METHOD();
      dirty=true;
      setNormals = false;
   }

   void normal(PVector p) {
      DEBUG_METHOD();
      dirty=true;
      setNormals = true;
      n = p;
   }

   void normal(float x, float y, float z) {
      DEBUG_METHOD();
      dirty=true;
      setNormals = true;
      n.x = x;
      n.y = y;
      n.z = z;
   }

   void vertex(float x, float y, float z) {
      DEBUG_METHOD();
      vertex({x, y, z}, {0.0f,0.0f});
   }

   void vertex(float x, float y) {
      DEBUG_METHOD();
      vertex({x, y, 0}, {0.0f,0.0f});
   }

   void vertex(PVector p) {
      DEBUG_METHOD();
      vertex(p, {0.0f,0.0f});
   }

   void vertex(float x, float y, float z, float u, float v) {
      DEBUG_METHOD();
      vertex({x, y, z}, {u, v});
   }

   void vertex(float x, float y, float u, float v) {
      DEBUG_METHOD();
      vertex({x, y, 0.0f}, { u , v });
   }

   void vertex(PVector p, PVector2 t) {
      DEBUG_METHOD();
      dirty=true;
      if (mode == IMAGE) {
         t.x /= texture_.width;
         t.y /= texture_.height;
      }
      vertices.push_back( { p, n, t, gl_fill_color } );
      extras.push_back( {stroke_color, stroke_weight } );
   }

   void index(unsigned short i) {
      DEBUG_METHOD();
      dirty=true;
      indices.push_back(i);
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      DEBUG_METHOD();
      bezierVertex( x2, y2, 0, x3, y3, 0, x4, y4, 0);
   }

   void bezierVertex(PVector v2, PVector v3, PVector v4) {
      DEBUG_METHOD();
      bezierVertex( v2.x, v2.y, v2.z, v3.x, v3.y, v3.z, v4.x, v4.y, v4.z);
   }

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4) {
      DEBUG_METHOD();
      float x1 = vertices.back().position.x;
      float y1 = vertices.back().position.y;
      for (float t = 0.01; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPointCubic( x1, x2, x3, x4, t );
         float y = bezierPointCubic( y1, y2, y3, y4, t );
         vertex(x, y);
      }
   }

   void bezierVertexQuadratic(PVector control, PVector anchor2) {
      DEBUG_METHOD();
      float anchor1_x = vertices.back().position.x;
      float anchor1_y = vertices.back().position.y;
      for (float t = 0.01; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPointQuadratic( anchor1_x, control.x, anchor2.x, t );
         float y = bezierPointQuadratic( anchor1_y, control.y, anchor2.y, t );
         vertex(x, y);
      }
   }

   void curveTightness(float alpha) {
      DEBUG_METHOD();
      dirty=true;
      tightness = alpha;
   }

   void curveVertex(PVector c) {
      DEBUG_METHOD();
      dirty=true;
      curve_vertices.push_back(c);
   }

   void drawCurve() {
      DEBUG_METHOD();
      constexpr int segments = 50;
      constexpr float dt = (1.0 / segments);
      constexpr float hdt = 0.5f * dt;
      constexpr float hdt2x2 = 0.5f * 2.0f * dt * dt;
      constexpr float hdt3x6 = 0.5f * 6.0f * dt * dt * dt;

      size_t size = curve_vertices.size();
      float s = tightness;

      for (int i = 0; i < size; i++) {
         auto p0 = curve_vertices[i];
         auto p1 = curve_vertices[(i+1)%size];
         auto p2 = curve_vertices[(i+2)%size];
         auto p3 = curve_vertices[(i+3)%size];

         float xt3 = ( p0.x * (s-1)   + p1.x * (s+3)  + p2.x * (-3-s)  + p3.x * (1-s));
         float xt2 = ( p0.x * (1-s)*2 + p1.x * (-5-s) + p2.x * (s+2)*2 + p3.x * (s-1));
         float xt1 = ( p0.x * (s-1) /*+ p1.x * 0*/    + p2.x * (1-s) /*+ p3.x * 0*/);
         float xt0 = ( p0.x * 0       + p1.x * 2      + p2.x * 0       + p3.x * 0);

         float yt3 = ( p0.y * (s-1)   + p1.y * (s+3)  + p2.y * (-3-s)  + p3.y * (1-s));
         float yt2 = ( p0.y * (1-s)*2 + p1.y * (-5-s) + p2.y * (s+2)*2 + p3.y * (s-1));
         float yt1 = ( p0.y * (s-1) + /*p1.y * 0*/    + p2.y * (1-s) /*+ p3.y * 0*/);
         float yt0 = ( p0.y * 0       + p1.y * 2      + p2.y * 0       + p3.y * 0);

         float zt3 = ( p0.z * (s-1)   + p1.z * (s+3)  + p2.z * (-3-s)  + p3.z * (1-s));
         float zt2 = ( p0.z * (1-s)*2 + p1.z * (-5-s) + p2.z * (s+2)*2 + p3.z * (s-1));
         float zt1 = ( p0.z * (s-1) /*+ p1.z * 0 */   + p2.z * (1-s) /*+ p3.z * 0*/);
         float zt0 = ( p0.z * 0       + p1.z * 2      + p2.z * 0       + p3.z * 0);

         PVector pos = p1; // {xt0, yt0, zt0};
         PVector vel = PVector{xt1, yt1, zt1} * hdt;
         PVector acc = PVector{xt2, yt2, zt2} * hdt2x2;
         PVector jer = PVector{xt3, yt3, zt3} * hdt3x6;

         // Just draw the control points
         // vertex(p1);

         for (int i = 0; i < segments - 1; ++i) {
            // Use the full quadtraic queation
            float t1 = i * dt;
            float t2 = t1 * t1;
            float t3 = t2 * t1;
            vertex( 0.5f * PVector{
                  xt3 * t3 + xt2 * t2 + xt1 * t1 + xt0,
                  yt3 * t3 + yt2 * t2 + yt1 * t1 + yt0,
                  zt3 * t3 + zt2 * t2 + zt1 * t1 + zt0
               });
            // Use add differential in discrete steps
            // vertex(pos);
            // pos += vel;
            // vel += acc;
            // acc += jer;
         }
      }
   }

   void curveVertex(float x, float y, float z) {
      DEBUG_METHOD();
      curveVertex({x,y,z});
   }

   void curveVertex(float x, float y) {
      DEBUG_METHOD();
      curveVertex(x,y,0);
   }

   bool isClockwise() const;

   void endShape(int type_ = OPEN) {
      const char *typeToTxt(int type);
      DEBUG_METHOD();
      dirty=true;
      // OPEN or CLOSE
      if (curve_vertices.size() > 0) {
         drawCurve();
         curve_vertices.clear();
      }

      if (style == POLYGON || style == LINES)
         type = type_;
      else
         type = CLOSE;
      populateIndices();
      if (!setNormals) {
         // Iterate over all triangles
         for (int i = 0; i < indices.size()/3; i++) {
            // Get the vertices of the current triangle
            PVector v1 = vertices[indices[i * 3]].position;
            PVector v2 = vertices[indices[i * 3 + 1]].position;
            PVector v3 = vertices[indices[i * 3 + 2]].position;

            // Calculate the normal vector of the current triangle
            PVector edge1 = v2 - v1;
            PVector edge2 = v3 - v1;
            glm::vec3 normal = (edge1.cross(edge2)).normalize();

            // Add the normal to the normals list for each vertex of the triangle
            vertices[indices[i * 3]].normal = vertices[indices[i * 3]].normal + normal;
            vertices[indices[i * 3 + 1]].normal = vertices[indices[i * 3 + 1]].normal + normal;
            vertices[indices[i * 3 + 2]].normal = vertices[indices[i * 3 + 2]].normal + normal;
         }
      }
   }

   unsigned short getCurrentIndex() {
      DEBUG_METHOD();
      return vertices.size();
   }

   void populateIndices();

   void populateIndices( std::vector<unsigned short> &&i ) {
      DEBUG_METHOD();
      dirty=true;
      indices = i;
   }

   void fill(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      dirty=true;
      fill_color = {r,g,b,a};
      gl_fill_color = gl::flatten_color_mode( fill_color );
   }

   void fill(float r,float g, float b) {
      DEBUG_METHOD();
      fill(r,g,b,color::scaleA);
   }

   void fill(float r,float a) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         fill(0,0,r,a);
      } else {
         fill(r,r,r,a);
      }
   }

   void fill(float r) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         fill(0,0,r,color::scaleA);
      } else {
         fill(r,r,r,color::scaleA);
      }
   }

   void fill(class color color) {
      DEBUG_METHOD();
      fill(color.r,color.g,color.b,color.a);
   }

   void fill(class color color, float a) {
      DEBUG_METHOD();
      fill(color.r,color.g,color.b,a);
   }

   void stroke(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      dirty=true;
      stroke_color = {r,g,b,a};
   }

   void stroke(float r,float g, float b) {
      DEBUG_METHOD();
      stroke(r,g,b,color::scaleA);
   }

   void stroke(float r,float a) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         stroke(0,0,r,a);
      } else {
         stroke(r,r,r,a);
      }
   }

   void stroke(float r) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         stroke(r,0,0,color::scaleA);
      } else {
         stroke(r,r,r,color::scaleA);
      }
   }

   void stroke(color c) {
      DEBUG_METHOD();
      stroke(c.r,c.g,c.b,c.a);
   }

   void strokeWeight(float x) {
      DEBUG_METHOD();
      dirty=true;
      stroke_weight = x;
   }

   void noStroke() {
      DEBUG_METHOD();
      dirty=true;
      stroke_color = {0,0,0,0};
   }

   void noFill() {
      DEBUG_METHOD();
      dirty=true;
      fill_color = {0,0,0,0};
      gl_fill_color = gl::flatten_color_mode( fill_color );
   }

   bool isStroked() const {
      DEBUG_METHOD();
      return stroke_color.a != 0.0;
   }

   bool isFilled() const {
      DEBUG_METHOD();
      return fill_color.a != 0;
   }

   void tint(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      dirty=true;
      tint_color = {r,g,b,a};
      gl_fill_color = gl::flatten_color_mode( tint_color );
   }

   void tint(float r,float g, float b) {
      DEBUG_METHOD();
      tint(r,g,b,color::scaleA);
   }

   void tint(float r,float a) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         tint(0,0,r,a);
      } else {
         tint(r,r,r,a);
      }
   }

   void tint(float r) {
      DEBUG_METHOD();
      if (color::mode == HSB) {
         tint(r,0,0,color::scaleA);
      } else {
         tint(r,r,r,color::scaleA);
      }
   }

   void tint(color c) {
      DEBUG_METHOD();
      tint(c.r,c.g,c.b,c.a);
   }

   void noTint() {
      DEBUG_METHOD();
      dirty=true;
      tint_color = WHITE;
      gl_fill_color = gl::flatten_color_mode( tint_color );
   }

   void strokeCap(int cap) {
      DEBUG_METHOD();
      dirty=true;
      line_end_cap = cap;
   }

   void setStroke(bool c) {
      DEBUG_METHOD();
      dirty=true;
      setStroke( color( 0.0f, 0.0f, 0.0f, 0.0f ) );
   }

   void setStroke(color c) {
      DEBUG_METHOD();
      dirty=true;
      for ( auto&&v : extras ) {
         v.stroke = c;
      }
   }

   void setStrokeWeight(float w) {
      DEBUG_METHOD();
      dirty=true;
      for ( auto&&v : extras ) {
         v.weight = w;
      }
   }

   void setTexture( PImage img ) {
      DEBUG_METHOD();
      dirty=true;
      texture( img );
   }

   void setFill(bool z) {
      DEBUG_METHOD();
      dirty=true;
      if (!z )
         for ( auto&&v : vertices ) {
            v.fill = gl::flatten_color_mode({0.0,0.0,0.0,0.0});
         }
   }

   void setFill(color c) {
      DEBUG_METHOD();
      dirty=true;
      fill_color = c;
      gl::color clr = gl::flatten_color_mode(fill_color);
      for ( auto&&v : vertices ) {
         v.fill = clr;
      }
   }

   void setTint(color c) {
      DEBUG_METHOD();
      dirty=true;
      tint_color = c;
      gl::color clr = gl::flatten_color_mode(tint_color);
      for ( auto&&v : vertices ) {
         v.fill = clr;
      }
   }

   bool is_dirty() const {
      DEBUG_METHOD();
      if ( dirty ) {
         return true;
      } else if ( style == GROUP ) {
         for (auto &&child : children) {
            if (child.impl->is_dirty())
               return true;
         }
      }
      return false;
   }

   void compile() {
      if (!isCompiled()) {
         batch.clear();
         compiled = true;
         flatten( batch, PMatrix::Identity(), true );
         batch.compile();
      }
   }

   bool isCompiled() const {
      return compiled && !is_dirty();
   }

   gl::batch_t &getBatch() {
      return batch;
   }

   void flatten(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const {
      DEBUG_METHOD();
      auto currentTransform = transform * shape_matrix;
      if ( style == GROUP ) {
         for (auto &&child : children) {
            child.flatten(batch, currentTransform, flatten_transforms);
         }
      } else {
         if ( fill_color.a != 0 )
            draw_fill(batch, currentTransform, flatten_transforms);
         // draw_normals(batch, currentTransform, flatten_transforms);
         if ( stroke_color.a != 0 )
            draw_stroke(batch, currentTransform, flatten_transforms);
      }
      dirty = false;
      return;
   }

   void draw_normals(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const;
   void draw_stroke(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const;
   void draw_fill(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const;

   int getChildCount() const {
      DEBUG_METHOD();
      return children.size();
   }

   int getVertexCount() const {
      DEBUG_METHOD();
      return vertices.size();
   }

   PVector getVertex(int i) const {
      DEBUG_METHOD();
      return vertices[i].position;
   }

   void setVertex(int i, PVector v) {
      DEBUG_METHOD();
      dirty=true;
      vertices[i].position = v;
   }

   void setVertex(int i, float x, float y , float z = 0) {
      DEBUG_METHOD();
      dirty=true;
      vertices[i].position = {x,y,z};
   }

   PVector getVertex(int i, PVector &x) const {
      DEBUG_METHOD();
      return x = getVertex(i);
   }
};

bool PShapeImpl::isClockwise() const {
   DEBUG_METHOD();
   if ( style != POLYGON && style != QUADS && style != QUAD )
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

static std::vector<unsigned short> triangulatePolygon(const std::vector<gl::vertex> &vertices,  std::vector<int> contour) {

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
      tess.addContour(3, vertices.data(), sizeof(gl::vertex), vertices.size(), offsetof(gl::vertex,position));
   } else {
      tess.addContour(3, vertices.data(), sizeof(gl::vertex), contour[0],      offsetof(gl::vertex,position));
      contour.push_back(vertices.size());
      for ( int i = 0; i < contour.size() - 1; ++i ) {
         auto &c = contour[i];
         auto start = vertices.data() + c;
         auto size = contour[i+1] - contour[i];
         tess.addContour(3, start, sizeof(gl::vertex), size, offsetof(gl::vertex,position));
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

void PShapeImpl::populateIndices() {
   DEBUG_METHOD();
   if (indices.size() != 0)
      return;

   if (style == GROUP) return;

   if (vertices.size() == 0) return;
   dirty=true;

   if (style == QUADS || style == QUAD) {
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

PShapeImpl drawLinePoly(int points, const gl::vertex *p, const PShapeImpl::vInfoExtra *extras, bool closed, const PMatrix &transform)  {
   PLine start;
   PLine end;

   if ( points < 3 )
      abort();

   PShapeImpl triangle_strip;
   triangle_strip.beginShape(TRIANGLE_STRIP);
   triangle_strip.transform( transform );
   triangle_strip.noStroke();
   triangle_strip.fill(extras[0].stroke);
   float half_weight = extras[0].weight / 2.0;
   if (closed) {
      start = drawLineMitred(p[points-1].position, p[0].position, p[1].position, half_weight );
      end = start;
   } else {
      PVector normal = PVector{p[1].position - p[0].position}.normal();
      normal.normalize();
      normal.mult(half_weight);
      start = {  PVector{p[0].position} + normal, PVector{p[0].position} - normal };
      normal = PVector{p[points-1].position - p[points-2].position}.normal();
      normal.normalize();
      normal.mult(half_weight);
      end = { PVector{p[points-1].position} + normal, PVector{p[points-1].position} - normal };
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

PShapeImpl drawRoundLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShapeImpl shape;

   int NUMBER_OF_VERTICES=16;

   shape.reserve(NUMBER_OF_VERTICES * 2, NUMBER_OF_VERTICES * 4);

   shape.beginShape(CONVEX_POLYGON);
   shape.transform( transform );

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

PShapeImpl drawLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShapeImpl shape;
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

PShapeImpl drawCappedLine(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {

   PShapeImpl shape;
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
   shape.beginShape(TRIANGLES);
   shape.circleTexture();
   shape.noStroke();
   shape.fill(color);
   shape.transform( transform );
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

void _line(PShapeImpl &triangles, PVector p1, PVector p2, float weight1, float weight2, color color1, color color2 ) {

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

PShapeImpl drawTriangleStrip(int points, const gl::vertex *p, const PShapeImpl::vInfoExtra *extras, const PMatrix &transform ) {
   PShapeImpl triangles;
   triangles.beginShape(TRIANGLES);
   triangles.transform( transform );
   _line(triangles, p[0].position, p[1].position,
         extras[0].weight, extras[1].weight, extras[0].stroke, extras[1].stroke);
   for (int i=2;i<points;++i) {
      _line(triangles, p[i-1].position, p[i].position, extras[i-1].weight, extras[i].weight, extras[i-1].stroke, extras[i].stroke);
      _line(triangles, p[i].position, p[i-2].position, extras[i].weight, extras[i-2].weight, extras[i].stroke, extras[i-2].stroke);
   }
   triangles.endShape();
   return triangles;
}

PShapeImpl drawTriangleNormal(int points, const gl::vertex *p,
                              const PShapeImpl::vInfoExtra *extras, bool closed,
                              const PMatrix &transform) {
   PShapeImpl shape;
   shape.beginShape(TRIANGLES);
   shape.fill(RED);
   shape.transform( transform );
   PVector pos = (p[0].position + p[1].position + p[2].position) / 3;
   PVector n = ((p[0].normal + p[1].normal + p[2].normal) / 3).normalize();
   float length = PVector{p[0].position - p[1].position}.mag() / 10.0f;
   _line(shape, pos, pos + length * n, length/10.0f,length/10.0f,RED,RED);
   shape.endShape();
   return shape;
}

void PShapeImpl::draw_normals(gl::batch_t &batch, const PMatrix &transform, bool flatten_transforms) const {
   DEBUG_METHOD();
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
         std::vector<gl::vertex> triangle;
         std::vector<vInfoExtra> xtras;
         triangle.push_back( vertices[indices[i]] );
         triangle.push_back( vertices[indices[i+1]] );
         triangle.push_back( vertices[indices[i+2]] );
         xtras.push_back( extras[indices[i]] );
         xtras.push_back( extras[indices[i+1]] );
         xtras.push_back( extras[indices[i+2]] );
         drawTriangleNormal( 3, triangle.data(), xtras.data(), false, shape_matrix).draw_fill( batch, transform, flatten_transforms );
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

void PShapeImpl::draw_stroke(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const {
   DEBUG_METHOD();
   switch( style ) {
   case POINTS:
   {
      for (int i = 0; i< vertices.size() ; ++i ) {
         drawUntexturedFilledEllipse(
            vertices[i].position.x, vertices[i].position.y,
            extras[i].weight, extras[i].weight,
            extras[i].stroke, shape_matrix ).draw_fill( batch, transform, flatten_transforms );
      }
      break;
   }
   case TRIANGLES_NOSTROKE:
      break;
   case TRIANGLES:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      PShapeImpl shape;
      shape.reserve(4*vertices.size(), 6 * vertices.size());
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
      shape.draw_fill( batch, transform, flatten_transforms);
      break;
   }
   case LINES:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      PShapeImpl shape;
      shape.reserve(2*vertices.size(), 3 * vertices.size());
      shape.beginShape(TRIANGLES);
      for (int i = 0; i < vertices.size(); i+=2 ) {
         PVector p0 = vertices[i].position;
         PVector p1 = vertices[i+1].position;
         float w0 = extras[i].weight;
         float w1 = extras[i+1].weight;
         color c0 =  extras[i].stroke;
         color c1 =  extras[i+1].stroke;
         _line(shape, p0, p1, w0, w1, c0, c1 );
      }
      shape.endShape();
      shape.draw_fill( batch, transform, flatten_transforms );
      break;
   }
   case POLYGON:
   case CONVEX_POLYGON:
   {
      if (vertices.size() > 2 ) {
         if (type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
            drawLinePoly( vertices.size() - 1, vertices.data() + 1, extras.data()+1, false, shape_matrix).draw_fill( batch, transform, flatten_transforms );
         } else {
            if ( contour.empty() ) {
               drawLinePoly( vertices.size(), vertices.data(), extras.data(), type == CLOSE, shape_matrix).draw_fill( batch, transform, flatten_transforms );
            } else {
               if (contour[0] != 0) {
                  drawLinePoly( contour[0], vertices.data(), extras.data(), type == CLOSE, shape_matrix).draw_fill( batch, transform, flatten_transforms );
               }
               auto q = contour;
               q.push_back(vertices.size());
               for ( int i = 0; i < q.size() - 1; ++i ) {
                  drawLinePoly( q[i+1] - q[i],
                                vertices.data() + q[i],
                                extras.data() + q[i],
                                type == CLOSE, shape_matrix).draw_fill( batch, transform, flatten_transforms );
               }
            }
         }
      } else if (vertices.size() == 2) {
         switch(line_end_cap) {
         case ROUND:
            drawRoundLine( vertices[0].position, vertices[1].position,
                           extras[0].weight, extras[1].weight,
                           extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         case PROJECT:
            drawCappedLine( vertices[0].position, vertices[1].position,
                            extras[0].weight, extras[1].weight,
                            extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         case SQUARE:
            drawLine( vertices[0].position, vertices[1].position,
                      extras[0].weight, extras[1].weight,
                      extras[0].stroke, extras[1].stroke, shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         default:
            abort();
         }
      } else if (vertices.size() == 1) {
         drawUntexturedFilledEllipse(
            vertices[0].position.x, vertices[0].position.y,
            extras[0].weight, extras[0].weight,
            extras[0].stroke, shape_matrix ).draw_fill( batch, transform, flatten_transforms );
      }
      break;
   }
   case QUAD_STRIP:
      // This isn't exactly right since we draw an extra line for every quad,
      // but it's close enought for now.
   case TRIANGLE_STRIP:
      drawTriangleStrip( vertices.size(),  vertices.data(), extras.data(), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
      break;
   case TRIANGLE_FAN:
      abort();
      break;
   default:
      abort();
      break;
   }
}

void PShapeImpl::draw_fill(gl::batch_t &batch, const PMatrix& transform_, bool flatten_transforms) const {
   DEBUG_METHOD();

   auto &vaos = batch.vaos;
   if (vertices.size() > 65536)
     abort();

   if (vertices.size() > 2 && style != POINTS && style != LINES) {
      const PMatrix &transform =
         flatten_transforms ? PMatrix::Identity() : transform_;

      if (vaos.size() == 0 || vaos.back().vertices.size() + vertices.size() > 65536) {
         vaos.emplace_back();
         vaos.back().transforms.push_back( transform.glm_data() );
         vaos.back().textures.push_back(texture_);
      }
      // At this point vaos has a back and that back has a transform and a texture.
      // It also has enough capacity for these triangles.

      if ( transform != vaos.back().transforms.back()) {
         if (vaos.back().transforms.size() == 16) {
            vaos.emplace_back();
            vaos.back().textures.push_back(texture_);
         }
         vaos.back().transforms.push_back(transform.glm_data());
      }

      if ( texture_ != vaos.back().textures.back()) {
         if (vaos.back().textures.size() == 16) {
            vaos.emplace_back();
            vaos.back().transforms.push_back( transform.glm_data() );
         }
         vaos.back().textures.push_back(texture_);
      }

      auto &vao = vaos.back();
      int currentM = vao.transforms.size() - 1;
      int tunit = vao.textures.size() - 1;
      int offset = vao.vertices.size();

      if(texture_ == PImage::circle()) {
         tunit = -1;
      }

      for (auto &v : vertices) {
         vao.vertices.emplace_back(
            flatten_transforms ? transform_ * v.position : v.position,
            v.normal,
            v.coord,
            v.fill,
            tunit,
            currentM);
      }

      for (auto index : indices) {
         vao.indices.push_back( offset + index );
      }
   }
}

static std::vector<std::weak_ptr<PShapeImpl>> &shapeHandles() {
   static std::vector<std::weak_ptr<PShapeImpl>> handles;
   return handles;
}

static void PShape_releaseAllVAOs() {
   for (auto i : shapeHandles()) {
      if (auto p = i.lock()) {
         p->clear();
      }
   }
}

void PShape::init() {
}

void PShape::optimize() {
   PShape::gc();
   auto &handles = shapeHandles();
   for (auto i : handles) {
      if (auto p = i.lock()) {
         if (p->getChildCount() > 0) {
            p->compile();
         }
      }
   }
}

void PShape::gc() {
   static int lastSize = 0;
   auto &oldHandles = shapeHandles();
   if (oldHandles.size() > lastSize + 200 ) {
      std::vector<std::weak_ptr<PShapeImpl>> newHandles;
      for (auto i : oldHandles) {
         if (auto p = i.lock()) {
            newHandles.push_back(p);
         }
      }
      lastSize = newHandles.size();
      shapeHandles() = std::move(newHandles);
   }
}

void PShape::close() {
   PShape_releaseAllVAOs();
}

PShape::PShape( std::shared_ptr<PShapeImpl> impl_ ) : impl(impl_) {
   shapeHandles().push_back(impl_);
}

const PMatrix &PShape::getShapeMatrix() {
   return impl->getShapeMatrix();
}

void PShape::addChild( const PShape shape ) {
   return impl->addChild( shape );
}

PShape PShape::getChild( int i ) {
   return impl->getChild(i);
}

color PShape::getFillColor() const {
   return impl->getFillColor();
}

color PShape::getStrokeColor() const {
   return impl->getStrokeColor();
}

float PShape::getStrokeWeight() const {
   return impl->getStrokeWeight();
}

color PShape::getTintColor() const {
   return impl->getTintColor();
}

bool PShape::isGroup() const {
   return impl->isGroup();
}

void PShape::copyStyle( const PShape other ) {
   return impl->copyStyle( *other.impl );
}

void PShape::clear() {
   return impl->clear();
}

void PShape::rotate(float angle) {
   return impl->rotate(angle);
}

void PShape::rotateZ(float angle) {
   return impl->rotateZ(angle);
}

void PShape::rotateY(float angle){
   return impl->rotateY(angle);
}


void PShape::rotateX(float angle){
   return impl->rotateX(angle);
}


void PShape::rotate(float angle, PVector axis){
   return impl->rotate(angle,axis);
}


void PShape::translate(float x, float y, float z){
   return impl->translate(x,y,z);
}

void PShape::translate(PVector t){
   return impl->translate(t);
}


void PShape::scale(float x, float y,float z){
   return impl->scale(x,y,z);
}


void PShape::scale(float x){
   return impl->scale(x);
}


void PShape::transform(const PMatrix &transform){
   return impl->transform(transform);
}


void PShape::resetMatrix(){
   return impl->resetMatrix();
}


void PShape::beginShape(int style_){
   if ( impl == nullptr) {
      impl = std::make_shared<PShapeImpl>();
      shapeHandles().push_back(impl);
   }
   return impl->beginShape(style_);
}


void PShape::beginContour(){
   return impl->beginContour();
}


void PShape::endContour(){
   return impl->endContour();
}


void PShape::textureMode( int mode_ ){
   return impl->textureMode(mode_);
}


bool PShape::isTextureSet() const{
   return impl->isTextureSet();
}


void PShape::material(PMaterial &mat){
   return impl->material(mat);
}


void PShape::texture(PImage img){
   return impl->texture(img);
}


void PShape::circleTexture(){
   return impl->circleTexture();
}


void PShape::noTexture(){
   return impl->noTexture();
}


void PShape::noNormal(){
   return impl->noNormal();
}


void PShape::normal(PVector p){
   return impl->normal(p);
}


void PShape::normal(float x, float y, float z){
   return impl->normal(x,y,z);
}


void PShape::vertex(float x, float y, float z){
   return impl->vertex(x,y,z);
}


void PShape::vertex(float x, float y){
   return impl->vertex(x,y);
}


void PShape::vertex(PVector p){
   return impl->vertex(p);
}


void PShape::vertex(float x, float y, float z, float u, float v){
   return impl->vertex(x,y,z,u,v);
}


void PShape::vertex(float x, float y, float u, float v){
   return impl->vertex(x,y,u,v);
}


void PShape::vertex(PVector p, PVector2 t){
   return impl->vertex(p,t);
}


void PShape::index(unsigned short i){
   return impl->index(i);
}


void PShape::bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4){
   return impl->bezierVertex(x2,y2,x3,y3,x4,y4);
}


void PShape::bezierVertex(PVector v2, PVector v3, PVector v4){
   return impl->bezierVertex(v2,v3,v4);
}


void PShape::bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4){
   return impl->bezierVertex(x2,y2,z2,x3,y3,z3,x4,y4,z4);
}


void PShape::bezierVertexQuadratic(PVector control, PVector anchor2){
   return impl->bezierVertexQuadratic(control,anchor2);
}


void PShape::curveTightness(float alpha){
   return impl->curveTightness(alpha);
}


void PShape::curveVertex(PVector c){
   return impl->curveVertex(c);
}


void PShape::drawCurve(){
   return impl->drawCurve();
}


void PShape::curveVertex(float x, float y, float z){
   return impl->curveVertex(x,y,z);
}

void PShape::curveVertex(float x, float y){
   return impl->curveVertex(x,y);
}


bool PShape::isClockwise() const{
   return impl->isClockwise();
}


void PShape::endShape(int type_){
   return impl->endShape( type_ );
}


unsigned short PShape::getCurrentIndex(){
   return impl->getCurrentIndex();
}


void PShape::populateIndices(){
   return impl->populateIndices();
}


void PShape::populateIndices( std::vector<unsigned short> &&i ){
   return impl->populateIndices(std::move(i));
}


void PShape::fill(float r,float g,  float b, float a){
   return impl->fill(r,g,b,a);
}


void PShape::fill(float r,float g, float b){
   return impl->fill(r,g,b);
}


void PShape::fill(float r,float a){
   return impl->fill(r,a);
}


void PShape::fill(float r){
   return impl->fill(r);
}


void PShape::fill(class color color){
   return impl->fill(color);
}


void PShape::fill(class color color, float a){
   return impl->fill(color,a);
}


void PShape::stroke(float r,float g,  float b, float a){
   return impl->stroke(r,g,b,a);
}


void PShape::stroke(float r,float g, float b){
   return impl->stroke(r,g,b);
}


void PShape::stroke(float r,float a){
   return impl->stroke(r,a);
}


void PShape::stroke(float r){
   return impl->stroke(r);
}


void PShape::stroke(color c){
   return impl->stroke(c);
}


void PShape::strokeWeight(float x){
   return impl->strokeWeight(x);
}


void PShape::noStroke(){
   return impl->noStroke();
}


void PShape::noFill(){
   return impl->noFill();
}


bool PShape::isStroked() const{
   return impl->isStroked();
}


bool PShape::isFilled() const{
   return impl->isFilled();
}


void PShape::tint(float r,float g,  float b, float a){
   return impl->tint(r,g,b,a);
}


void PShape::tint(float r,float g, float b){
   return impl->tint(r,g,b);
}


void PShape::tint(float r,float a){
   return impl->tint(r,a);
}


void PShape::tint(float r){
   return impl->tint(r);
}


void PShape::tint(color c){
   return impl->tint(c);
}


void PShape::noTint(){
   return impl->noTint();
}


void PShape::strokeCap(int cap){
   return impl->strokeCap(cap);
}


void PShape::setStroke(bool c){
   return impl->setStroke(c);
}


void PShape::setStroke(color c){
   return impl->setStroke(c);
}


void PShape::setStrokeWeight(float w){
   return impl->setStrokeWeight(w);
}


void PShape::setTexture( PImage img ){
   return impl->setTexture(img);
}


void PShape::setFill(bool z){
   return impl->setFill(z);
}


void PShape::setFill(color c){
   return impl->setFill(c);
}


void PShape::setTint(color c){
   return impl->setTint(c);
}


void PShape::compile() {
   return impl->compile();
}

bool PShape::isCompiled() const {
   return impl->isCompiled();
}

gl::batch_t &PShape::getBatch() {
   return impl->getBatch();
}

void PShape::flatten(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const{
   return impl->flatten(batch, transform, flatten_transforms);
}

void PShape::draw_normals(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const{
   return impl->draw_normals(batch,transform, flatten_transforms);
}

void PShape::draw_stroke(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const{
   return impl->draw_stroke(batch,transform, flatten_transforms);
}

void PShape::draw_fill(gl::batch_t &batch, const PMatrix& transform, bool flatten_transforms) const{
   return impl->draw_fill(batch,transform, flatten_transforms);
}


int PShape::getChildCount() const{
   return impl->getChildCount();
}


int PShape::getVertexCount() const{
   return impl->getVertexCount();
}


PVector PShape::getVertex(int i) const{
   return impl->getVertex(i);
}


void PShape::setVertex(int i, PVector v){
   return impl->setVertex(i,v);
}


void PShape::setVertex(int i, float x, float y , float z){
   return impl->setVertex(i,x,y,z);
}


PVector PShape::getVertex(int i, PVector &x) const{
   return impl->getVertex(i,x);
}


PShape loadShape( std::string_view filename ) {
   if ( endsWith( toLowerCase( filename ) , ".obj" ) ) {
      return loadShapeOBJ( filename );
   } else {
      return loadShapeSVG( filename );
   }
}

PShape loadShapeOBJ( std::string_view objPath ) {
   struct VertRef {
      VertRef(int v, int vt, int vn) : v(v), vt(vt), vn(vn) { }
      int v, vt, vn;
   };

   using namespace std::literals;

   std::ifstream inputFile("data/"s + std::string(objPath));

   if (!inputFile.is_open()) {
      abort();
   }

   PShape obj_shape;
   obj_shape.beginShape( TRIANGLES );

   std::vector< glm::vec4 > positions( 1, glm::vec4( 0, 0, 0, 0 ) );
   std::vector< glm::vec3 > normals( 1, glm::vec3( 0, 0, 0 ) );
   std::vector< glm::vec2 > coords( 1, glm::vec2( 0, 0 ) );

   std::string currentMaterial;
   std::string currentGroup;
   std::string lineStr;
   while( std::getline( inputFile, lineStr ) ) {
      std::istringstream lineSS( lineStr );
      std::string lineType;
      lineSS >> lineType;

      if( lineType == "v" ) {
         // position
         float x = 0, y = 0, z = 0, w = 1;
         lineSS >> x >> y >> z >> w;
         positions.emplace_back( x, y, z, w );
      } else if( lineType == "vn" ) {
         // normal
         float i = 0, j = 0, k = 0;
         lineSS >> i >> j >> k;
         normals.push_back( glm::normalize( glm::vec3( i, j, k ) ) );
      } else if( lineType == "vt" ) {
         // texture coordinate
         float i = 0, j = 0;
         lineSS >> i >> j;
         coords.push_back( glm::vec2( i, j ) );
      } else if( lineType == "f" ) {
         // polygon
         std::vector< VertRef > refs;
         std::string refStr;
         while( lineSS >> refStr ) {
            int v = 0, vt = 0, vn = 0;
            char x = 0, y = 0;
            std::istringstream ref( refStr );
            ref >> v >> x >> vt >> y >> vn;
            refs.emplace_back( v, vt, vn );
         }

         // triangulate, assuming n>3-gons are convex and coplanar
         for( size_t i = 1; i+1 < refs.size(); ++i ) {
            const VertRef* p[3] = { &refs[0], &refs[i], &refs[i+1] };

            // http://www.opengl.org/wiki/Calculating_a_Surface_Normal
            glm::vec3 U( positions[ p[1]->v ] - positions[ p[0]->v ] );
            glm::vec3 V( positions[ p[2]->v ] - positions[ p[0]->v ] );
            glm::vec3 faceNormal = glm::normalize( glm::cross( U, V ) );

            for(auto & j : p) {
               obj_shape.normal( j->vn != 0 ? normals[ j->vn ] : faceNormal );
               obj_shape.vertex( glm::vec3( positions[ j->v ] ), {coords[ j->vt].x,1.0f-coords[ j->vt].y}  );
            }
         }
      } else if( lineType == "g" ) {
         std::string name;
         lineSS >> name;
         currentGroup = name;
      } else if( lineType == "usemtl" ) {
         std::string name;
         lineSS >> name;
         currentMaterial = name;
         obj_shape.material( materials[currentMaterial] );
      } else if( lineType == "mtllib" ) {
         std::string name;
         lineSS >> name;
         loadMaterials( name.c_str() );
      } else if( lineType == "#" ) {
      } else {
         fmt::print("Unrecognized OBJ file line: {}\n", lineStr);
      }
   }
   obj_shape.endShape();
   return obj_shape;
}

PShape::PShape() : impl( nullptr ) {}

const char *typeToTxt(int type) {
   switch (type) {
   case OPEN:
      return "OPEN";
   case CLOSE:
      return "CLOSE";
   default:
      return "INVALID";
   }
}

const char *styleToTxt(int style) {
   switch (style) {
   case POINTS:
      return "POINTS";
   case POLYGON:
      return "POLYGON";
   case OPEN_SKIP_FIRST_VERTEX_FOR_STROKE:
      return "OPEN_SKIP_FIRST_VERTEX_FOR_STROKE";
   case LINES:
      return "LINES";
   case GROUP:
      return "GROUP";
   case QUAD:
      return "QUAD";
   case QUADS:
      return "QUADS";
   case QUAD_STRIP:
      return "QUAD_STRIP";
   case TRIANGLE_STRIP:
      return "TRIANGLE_STRIP";
   case TRIANGLE_FAN:
      return "TRIANGLE_FAN";
   case CONVEX_POLYGON:
      return "CONVEX_POLYGON";
   case TRIANGLES:
      return "TRIANGLES";
   case TRIANGLES_NOSTROKE:
      return "TRIANGLES_NOSTROKE";
   default:
      return "INVALID!";
   }
}

template <>
struct fmt::formatter<PShapeImpl> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const PShapeImpl& v, FormatContext& ctx) {
      return format_to(ctx.out(), "vertices={:<4} indices={:<4} style={:20} type={:6}",
                       v.vertices.size(), v.indices.size(),
                       styleToTxt(v.style),
                       typeToTxt(v.type)
         );
   }
};

namespace std {
    template <>
    struct hash<PShapeImpl> {
       std::size_t operator()(const PShapeImpl& p) const {
          std::size_t hash_val = 0;
          hash_combine(hash_val, p.setNormals);
          //hash_combine(hash_val, p.n);
          //hash_combine(hash_val, p.contour);
          //hash_combine(hash_val, p.vertices);
          //hash_combine(hash_val, p.extras);
          //hash_combine(hash_val, p.children);
          //hash_combine(hash_val, p.indices);
          //hash_combine(hash_val, p.vaos);
          hash_combine(hash_val, p.type);
          hash_combine(hash_val, p.type);
          hash_combine(hash_val, p.mode);
          hash_combine(hash_val, p.stroke_weight);
          hash_combine(hash_val, p.line_end_cap);
          hash_combine(hash_val, p.tightness);
          //hash_combine(hash_val, p.curve_vertices);
          //hash_combine(hash_val, p.shape_matrix);
          //hash_combine(hash_val, p.stroke_color);
          //hash_combine(hash_val, p.texture_);
          //hash_combine(hash_val, p.gl_fill_color);
          //hash_combine(hash_val, p.fill_color);
          //hash_combine(hash_val, p.tint_color);
          hash_combine(hash_val, p.style);
          return hash_val;
       }
    };
}
