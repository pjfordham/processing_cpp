#include "processing_pshape.h"
#include "processing_pshape_svg.h"
#include "processing_math.h"
#include <vector>
#include <tesselator_cpp.h>
#include <unordered_map>

#include "processing_color.h"
#include "processing_enum.h"
#include "processing_opengl.h"
#include "processing_pimage.h"
#include "processing_pmaterial.h"

#include "processing_debug.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

template <> struct fmt::formatter<PShapeImpl>;

class PShapeImpl {
   friend struct fmt::formatter<PShapeImpl>;
   friend struct std::hash<PShapeImpl>;

public:
   struct vInfoExtra {
      color stroke;
      float weight;
   };

private:

   bool useGlobalStyle = false;

   bool setNormals = false;
   PVector n = { 0.0, 0.0, 0.0 };

   std::string id;
   std::vector<int> contour;
   std::vector<gl::vertex> vertices;
   std::vector<PMaterial> materials;
   std::vector<vInfoExtra> extras;
   std::vector<PShape> children;
   std::vector<unsigned short> indices;

   mutable bool dirty = true;

   int type = OPEN;
   float tightness = 0.0f;
   std::vector<PVector> curve_vertices;
   PMatrix shape_matrix = PMatrix::Identity();

   struct style_t {

      std::optional<color> fill_color = WHITE;
      gl::color gl_fill_color = flatten_color_mode(WHITE);

      std::optional<color> stroke_color = BLACK;
      float stroke_weight = 1.0f;
      int line_end_cap = ROUND;

      PMaterial currentMaterial;

      std::optional<PImage> texture_;
      color tint_color = WHITE;
      int mode = IMAGE;

      std::optional<std::optional<color>> override_fill_color;
      std::optional<std::optional<color>> override_stroke_color;
      std::optional<float> override_stroke_weight;
   };

   style_t style;

   int kind = POLYGON;
   bool compiled = false;
   gl::batch_t batch;

public:

   float width = 1.0;
   float height = 1.0;

   void setID(std::string_view s) {
      id = s;
   }

   void enableStyle() {
      dirty = true;
      useGlobalStyle = false;
   }

   void disableStyle() {
      dirty = true;
      useGlobalStyle = true;
   }

   const PMatrix& getShapeMatrix() const {
      DEBUG_METHOD();
      return shape_matrix;
   }

   PShapeImpl& operator=(const PShapeImpl&) = delete;

   PShapeImpl(PShapeImpl&& x) noexcept {
      *this = std::move(x);
   }

   PShapeImpl& operator=(PShapeImpl&& other) noexcept {
      std::swap(n,other.n);
      std::swap(setNormals,other.setNormals);
      std::swap(contour,other.contour);
      std::swap(vertices,other.vertices);
      std::swap(materials,other.materials);
      std::swap(extraAttributes,other.extraAttributes);
      std::swap(extras,other.extras);
      std::swap(children,other.children);
      std::swap(indices,other.indices);
      std::swap(dirty,other.dirty);
      std::swap(type,other.type);
      std::swap(style,other.style);
      std::swap(tightness,other.tightness);
      std::swap(curve_vertices,other.curve_vertices);
      std::swap(shape_matrix,other.shape_matrix);
      std::swap(kind,other.kind);
      std::swap(width,other.width);
      std::swap(height,other.height);
      std::swap(useGlobalStyle,other.useGlobalStyle);
      return *this;
   }

   void reserve(int v, int i) {
      DEBUG_METHOD();
      vertices.reserve(v);
      materials.reserve(v);
      extras.reserve(v);
      indices.reserve(i);
   }

   PShapeImpl() {
      DEBUG_METHOD();
      reserve(4,6);
      style.currentMaterial = {
         gl::color{1.0f,1.0f,1.0f,1.0f},
         gl::color{0.0f,0.0f,0.0f,1.0f},
         gl::color{0.0f,0.0f,0.0f,1.0f},
         gl::color{0.0f,0.0f,0.0f,1.0f},
         1.0, 1.0, 4, PShape::getBlankTexture() };
   }

   PShapeImpl(const PShapeImpl &copy) = default;

   ~PShapeImpl() {
      DEBUG_METHOD();
   }

   void addChild( const PShape &shape ) {
      DEBUG_METHOD();
      dirty=true;
      children.push_back( shape );
   }

   PShape getChild( int i ) {
      DEBUG_METHOD();
      return children[i];
   }

   PShape getChild( std::string_view s ) const {
      DEBUG_METHOD();
      for (auto &&child : children) {
         if (child.impl->id == s) {
            return child;
         }
      }
      return createShape();
   }

   color getStrokeColor() const {
      DEBUG_METHOD();
      return style.stroke_color.value_or( color(0,0,0,0) );
   }

   float getStrokeWeight() const {
      DEBUG_METHOD();
      return style.stroke_weight;
   }

   color getFillColor() const {
      DEBUG_METHOD();
      return style.fill_color.value_or( color(0,0,0,0) );
   }

   color getTintColor() const {
      DEBUG_METHOD();
      return style.tint_color;
   }

   bool isGroup() const {
      DEBUG_METHOD();
      return kind == GROUP;
   }

   void copyStyle( const PShapeImpl &other ) {
      DEBUG_METHOD();
      dirty=true;
      style = other.style;
   }

   void clear() {
      DEBUG_METHOD();
      dirty = true;
      vertices.clear();
      materials.clear();
      extras.clear();
      indices.clear();
      children.clear();
      batch.clear();
   }

   void rotate(float angle, float x, float y, float z) {
      DEBUG_METHOD();
      rotate(angle, PVector(x,y,z));
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

   void beginShape(int kind_ = POLYGON) {
      DEBUG_METHOD();
      dirty=true;
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      kind = kind_;
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
      style.mode = mode_;
   }

   bool isTextureSet() const {
      DEBUG_METHOD();
      return !!style.texture_;
   }

   void material(PMaterial &mat) {
      DEBUG_METHOD();
      dirty=true;
      noStroke();
      textureMode( NORMAL );
      texture( mat.texture );
      style.currentMaterial = mat;
   }

   void texture(PImage img) {
      DEBUG_METHOD();
      dirty=true;
      style.texture_ = img;
   }

   void circleTexture() {
      DEBUG_METHOD();
      dirty=true;
      style.mode = NORMAL;
      style.texture_ = PImage::circle();
   }

   void noTexture() {
      DEBUG_METHOD();
      dirty = true;
      style.texture_.reset();
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
      if (style.texture_ && style.mode == IMAGE) {
         t.x /= style.texture_.value().width;
         t.y /= style.texture_.value().height;
      }
      vertices.push_back( { p, n, t, style.gl_fill_color } );
      materials.push_back( style.currentMaterial );
      extras.push_back( { getStrokeColor(), style.stroke_weight } );
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

   void endShape(int type_ = OPEN) {
      DEBUG_METHOD();
      dirty=true;
      // OPEN or CLOSE
      if (curve_vertices.size() > 0) {
         drawCurve();
         curve_vertices.clear();
      }

      if (kind == POLYGON || kind == LINES)
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

   void specular(float r, float g, float b, float a) {
      DEBUG_METHOD();
      dirty=true;
      color specular_color = {r,g,b,a};
      auto gl_specular_color = flatten_color_mode( specular_color );
      style.currentMaterial.specularColor = gl_specular_color;
   }

   void shininess(float r) {
      DEBUG_METHOD();
      dirty=true;
      style.currentMaterial.specularExponent = r;
   }

   void fill(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      dirty = true;
      style.fill_color = {r,g,b,a};
      style.gl_fill_color = flatten_color_mode( style.fill_color.value() );
      style.currentMaterial.ambientColor = style.gl_fill_color;
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
      dirty = true;
      style.stroke_color = {r,g,b,a};
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
      style.stroke_weight = x;
   }

   void noStroke() {
      DEBUG_METHOD();
      dirty = true;
      style.stroke_color.reset();
   }

   void noFill() {
      DEBUG_METHOD();
      dirty = true;
      style.fill_color.reset();
   }

   bool isStroked() const {
      DEBUG_METHOD();
      return (style.override_stroke_color && style.override_stroke_color.value()) ||
         (!style.override_stroke_color && style.stroke_color);
   }

   bool isFilled() const {
      DEBUG_METHOD();
      return (style.override_fill_color && style.override_fill_color.value()) ||
         (!style.override_fill_color && style.fill_color);
   }

   void tint(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      dirty = true;
      style.tint_color = {r,g,b,a};
      style.gl_fill_color = flatten_color_mode( style.tint_color );
      style.currentMaterial.ambientColor = style.gl_fill_color;
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
      style.tint_color = WHITE;
      style.gl_fill_color = flatten_color_mode( WHITE );
   }

   void strokeCap(int cap) {
      DEBUG_METHOD();
      dirty=true;
      style.line_end_cap = cap;
   }

   void setStroke(bool c) {
      DEBUG_METHOD();
      dirty = true;
      for (auto &&child : children) {
         child.setStroke(c);
      }
      style.override_stroke_color = std::optional<color>();
   }

   void setStroke(color c) {
      DEBUG_METHOD();
      dirty = true;
      style.override_stroke_color = std::optional<color>(c);
      for (auto &&child : children) {
         child.setStroke(c);
      }
   }

   void setStrokeWeight(float w) {
      DEBUG_METHOD();
      dirty=true;
      style.override_stroke_weight = w;
      for (auto &&child : children) {
         child.setStrokeWeight(w);
      }
   }

   void setTexture( PImage img ) {
      DEBUG_METHOD();
      dirty=true;
      for (auto &&child : children) {
         child.setTexture(img);
      }
      texture( img );
   }

   void setFill(bool z) {
      DEBUG_METHOD();
      dirty=true;
      for (auto &&child : children) {
         child.setFill(z);
      }
      style.override_fill_color = std::optional<color>();
      // if (!z ) {
      //    for ( auto&&v : vertices ) {
      //       v.fill = flatten_color_mode({0.0,0.0,0.0,0.0});
      //    }
      //    for ( auto&&v : materials ) {
      //       v.ambientColor = flatten_color_mode({0.0,0.0,0.0,0.0});
      //    }
      // }
   }

   void setFill(color c) {
      DEBUG_METHOD();
      dirty = true;
      for (auto &&child : children) {
         child.setFill(c);
      }
      style.override_fill_color = std::optional<color>(c);
      // gl::color clr = flatten_color_mode(style.fill_color.value());
      // for ( auto&&v : vertices ) {
      //    v.fill = clr;
      // }
      // for ( auto&&v : materials ) {
      //    v.ambientColor = clr;
      // }
   }

   void setTint(color c) {
      DEBUG_METHOD();
      dirty=true;
      for (auto &&child : children) {
         child.setFill(c);
      }
      style.tint_color = c;
      gl::color clr = flatten_color_mode(style.tint_color);
      for ( auto&&v : vertices ) {
         v.fill = clr;
      }
   }

   bool is_dirty() const {
      DEBUG_METHOD();
      if ( dirty ) {
         return true;
      } else if ( kind == GROUP ) {
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
         batch.load();
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
      if ( kind == GROUP ) {
         for (auto &&child : children) {
            child.flatten(batch, currentTransform, flatten_transforms);
         }
      } else {
         if ( isFilled() )
            draw_fill(batch, currentTransform, flatten_transforms);
         // draw_normals(batch, currentTransform, flatten_transforms);
         if ( isStroked() )
            draw_stroke(batch, currentTransform, flatten_transforms);
      }
      dirty = false;
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

   std::map<std::string,std::vector<glm::vec4>> extraAttributes;
   void attribPosition(std::string_view name, float x, float y, float z, float w) {
      extraAttributes[std::string(name)].emplace_back(x,y,z,w);
      // for (auto &i : extraAttributes) {
      //   fmt::print("{}\n", i.first);
      //   for (auto &j : i.second) {
      //      fmt::print("{} ", j);
      //   }
      // }
   }
};

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

// Function to project a 3D quad onto a 2D plane
static void projectTo2D(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D,
                        glm::vec2& A2D, glm::vec2& B2D, glm::vec2& C2D, glm::vec2& D2D) {
   // Compute normal using cross product
   glm::vec3 normal = glm::normalize(glm::cross(B - A, D - A));

   // Find the dominant axis to ignore (largest absolute component of normal)
   int ignoreAxis = 0; // Default to X
   if (std::abs(normal.y) > std::abs(normal.x) && std::abs(normal.y) > std::abs(normal.z))
      ignoreAxis = 1; // Ignore Y
   else if (std::abs(normal.z) > std::abs(normal.x) && std::abs(normal.z) > std::abs(normal.y))
      ignoreAxis = 2; // Ignore Z

   // Project 3D points onto the best-fit 2D plane
   switch (ignoreAxis) {
   case 0: // Ignore X -> Use YZ plane
      A2D = {A.y, A.z};
      B2D = {B.y, B.z};
      C2D = {C.y, C.z};
      D2D = {D.y, D.z};
      break;
   case 1: // Ignore Y -> Use XZ plane
      A2D = {A.x, A.z};
      B2D = {B.x, B.z};
      C2D = {C.x, C.z};
      D2D = {D.x, D.z};
      break;
   case 2: // Ignore Z -> Use XY plane
      A2D = {A.x, A.y};
      B2D = {B.x, B.y};
      C2D = {C.x, C.y};
      D2D = {D.x, D.y};
      break;
   }
}

// Function to check if a quad is non-convex (arrow-shaped)
static bool isNonConvex(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& D) {
   glm::vec2 AB = B - A;
   glm::vec2 BC = C - B;
   glm::vec2 CD = D - C;
   glm::vec2 DA = A - D;

   float cross1 = AB.x * BC.y - AB.y * BC.x;
   float cross2 = BC.x * CD.y - BC.y * CD.x;
   float cross3 = CD.x * DA.y - CD.y * DA.x;
   float cross4 = DA.x * AB.y - DA.y * AB.x;

   bool sign1 = cross1 > 0;
   bool sign2 = cross2 > 0;
   bool sign3 = cross3 > 0;
   bool sign4 = cross4 > 0;

   return (sign1 != sign2) || (sign1 != sign3) || (sign1 != sign4);
}

// Function to choose a valid diagonal for triangulation
static std::pair<int, int> chooseValidDiagonal(const glm::vec2& A, const glm::vec2& B,
                                               const glm::vec2& C, const glm::vec2& D) {
   if (!isNonConvex(A, B, C, D)) {
      return {0, 2};  // Default diagonal AC
   }

   float crossAC1 = glm::cross(glm::vec3(B - A, 0.0f), glm::vec3(C - A, 0.0f)).z;
   float crossAC2 = glm::cross(glm::vec3(C - A, 0.0f), glm::vec3(D - C, 0.0f)).z;
   float crossBD1 = glm::cross(glm::vec3(A - B, 0.0f), glm::vec3(D - B, 0.0f)).z;
   float crossBD2 = glm::cross(glm::vec3(D - B, 0.0f), glm::vec3(C - D, 0.0f)).z;

   bool validAC = (crossAC1 >= 0 && crossAC2 >= 0);
   bool validBD = (crossBD1 >= 0 && crossBD2 >= 0);

   return validAC ? std::make_pair(0, 2) : std::make_pair(1, 3);
}



void PShapeImpl::populateIndices() {
   // Becuase we flip the Y-axis to maintain processings coordinate system
   // we have to reverse the triangle winding direction for any geometry
   // we don't create and index ourselves.
   DEBUG_METHOD();
   if (indices.size() != 0)
      return;

   if (kind == GROUP) return;

   if (vertices.size() == 0) return;

   if ( !isFilled() ) return;

   dirty=true;

   if (kind == QUADS || kind == QUAD) {
      if (vertices.size() % 4 != 0) abort();
      for (int i = 0; i < vertices.size(); i += 4) {
        glm::vec2 A2D, B2D, C2D, D2D;
        projectTo2D(vertices[i].position, vertices[i + 1].position, vertices[i + 2].position, vertices[i + 3].position,
                    A2D, B2D, C2D, D2D);

         auto diag = chooseValidDiagonal(A2D, B2D, C2D, D2D);
         int d1 = diag.first;
         int d2 = diag.second;

         // First Triangle
         indices.push_back(i + d1);
         indices.push_back(i + ((d1 + 1) % 4));
         indices.push_back(i + d2);

         // Second Triangle
         indices.push_back(i + d1);
         indices.push_back(i + d2);
         indices.push_back(i + ((d2 + 1) % 4));
    }
   }
   else if (kind == QUAD_STRIP) {
      for (int i = 0; i + 3 < vertices.size(); i += 2) {
         // Just discard any trailing odd vertex
         glm::vec2 A2D, B2D, C2D, D2D;
         projectTo2D(vertices[i].position, vertices[i + 1].position, vertices[i + 2].position, vertices[i + 3].position,
                     A2D, B2D, C2D, D2D);

         auto diag = chooseValidDiagonal(A2D, B2D, C2D, D2D);
         int d1 = diag.first;
         int d2 = diag.second;

         // Mapping function to correct order for QUAD_STRIP
         auto s = [](int i) { return (i == 2) ? 3 : (i == 3) ? 2 : i; };

         // First Triangle
         indices.push_back(i + s(d1));
         indices.push_back(i + s((d1 + 1) % 4));
         indices.push_back(i + s(d2));

         // Second Triangle
         indices.push_back(i + s(d1));
         indices.push_back(i + s(d2));
         indices.push_back(i + s((d2 + 1) % 4));
     }
   } else if (kind == TRIANGLE_STRIP) {
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
   } else if (kind == CONVEX_POLYGON) {
      // Fill with triangle fan
      for (int i = 1; i < vertices.size() - 1 ; i++ ) {
         indices.push_back( 0 );
         indices.push_back( i );
         indices.push_back( i+1 );
      }
   }  else if (kind == TRIANGLE_FAN) {
      // Fill with triangle fan
      for (int i = 1; i < vertices.size() - 2 ; i++ ) {
         indices.push_back( 0 );
         indices.push_back( i );
         indices.push_back( i+1 );
      }
   } else if (kind == POLYGON) {
      indices = triangulatePolygon(vertices, contour);
   } else if (kind == TRIANGLES) {
      for (int i = 0; i < vertices.size(); i+=3 ) {
         indices.push_back( i );
         indices.push_back( i+1 );
         indices.push_back( i+2 );
      }
   } else if (kind == POINTS || kind == LINES) {
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
   return {
      center.x + xradius * xsincos[index][0],
      center.y + yradius * xsincos[index][1],
      center.z };
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

PShapeImpl drawLinePoly(int points, const gl::vertex *p, const PShapeImpl::vInfoExtra *extras, bool closed, const PMatrix &transform,
                        std::optional<color> override_color, std::optional<float> override_weight)  {
   PLine start;
   PLine end;

   if ( points < 3 )
      abort();

   PShapeImpl triangle_strip;
   triangle_strip.beginShape(TRIANGLE_STRIP);
   triangle_strip.transform( transform );
   triangle_strip.noStroke();
   triangle_strip.fill(override_color.value_or(extras[0].stroke));
   float half_weight = override_weight.value_or(extras[0].weight) / 2.0;
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
      shape.vertex(p1.x + cosf(i + start_angle) * weight1/2, p1.y + sinf(i+start_angle) * weight1/2, p1.z);
   }

   start_angle += PI;

   shape.fill(color2);
   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      shape.vertex(p2.x + cosf(i+start_angle) * weight2/2, p2.y + sinf(i+start_angle) * weight2/2, p2.z);
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
   PShape shape = createShape();;
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
   shape.populateIndices( { 0,2,1,0,3,2 } );
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

PShapeImpl drawTriangleNormal(const gl::vertex &p0, const gl::vertex &p1, const gl::vertex &p2, const PMatrix &transform) {
   PShapeImpl shape;
   shape.beginShape(TRIANGLES);
   shape.fill(RED);
   shape.transform( transform );
   PVector pos = (p0.position + p1.position + p2.position) / 3;
   PVector n = ((p0.normal + p1.normal + p2.normal) / 3).normalize();
   float length = PVector{p0.position - p1.position}.mag() / 10.0f;
   _line(shape, pos, pos + length * n, length/10.0f,length/10.0f,RED,RED);
   shape.endShape();
   return shape;
}

void PShapeImpl::draw_normals(gl::batch_t &batch, const PMatrix &transform, bool flatten_transforms) const {
   DEBUG_METHOD();
   switch( kind ) {
   case TRIANGLES_NOSTROKE:
   case TRIANGLES:
   case TRIANGLE_STRIP:
   case QUAD_STRIP:
   case QUAD:
   case QUADS:
   case POLYGON:
   case CONVEX_POLYGON:
   case TRIANGLE_FAN:
      // All of these should have just been flattened to triangles
      for (int i = 0; i < indices.size(); i+=3 ) {
         drawTriangleNormal( vertices[indices[i]],vertices[indices[i+1]], vertices[indices[i+2]],
                             shape_matrix).draw_fill( batch, transform, flatten_transforms );
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
   std::optional<color> override_color = style.override_stroke_color ? style.override_stroke_color.value() : std::optional<color>();
   std::optional<float> override_weight = style.override_stroke_weight;
   switch( kind ) {
   case POINTS:
   {
      for (int i = 0; i< vertices.size() ; ++i ) {
         drawUntexturedFilledEllipse(
            vertices[i].position.x, vertices[i].position.y,
            override_weight.value_or(extras[i].weight), override_weight.value_or(extras[i].weight),
            override_color.value_or(extras[i].stroke), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
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
         float w0 = override_weight.value_or(extras[indices[i]].weight);
         float w1 = override_weight.value_or(extras[indices[i+1]].weight);
         float w2 = override_weight.value_or(extras[indices[i+2]].weight);
         color c0 = override_color.value_or(extras[indices[i]].stroke);
         color c1 = override_color.value_or(extras[indices[i+1]].stroke);
         color c2 = override_color.value_or(extras[indices[i+2]].stroke);

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
         float w0 = override_weight.value_or(extras[i].weight);
         float w1 = override_weight.value_or(extras[i+1].weight);
         color c0 = override_color.value_or(extras[i].stroke);
         color c1 = override_color.value_or(extras[i+1].stroke);
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
            drawLinePoly( vertices.size() - 1, vertices.data() + 1, extras.data()+1, false, shape_matrix, override_color, override_weight ).draw_fill( batch, transform, flatten_transforms);
         } else {
            if ( contour.empty() ) {
               drawLinePoly( vertices.size(), vertices.data(), extras.data(), type == CLOSE, shape_matrix, override_color, override_weight ).draw_fill( batch, transform, flatten_transforms);
            } else {
               if (contour[0] != 0) {
                  drawLinePoly( contour[0], vertices.data(), extras.data(), type == CLOSE, shape_matrix, override_color, override_weight ).draw_fill( batch, transform, flatten_transforms);
               }
               auto q = contour;
               q.push_back(vertices.size());
               for ( int i = 0; i < q.size() - 1; ++i ) {
                  drawLinePoly( q[i+1] - q[i],
                                vertices.data() + q[i],
                                extras.data() + q[i],
                                type == CLOSE, shape_matrix, override_color, override_weight ).draw_fill( batch, transform, flatten_transforms);
               }
            }
         }
      } else if (vertices.size() == 2) {
         switch(style.line_end_cap) {
         case ROUND:
            drawRoundLine( vertices[0].position, vertices[1].position,
                           override_weight.value_or(extras[0].weight), override_weight.value_or(extras[1].weight),
                           override_color.value_or(extras[0].stroke), override_color.value_or(extras[1].stroke), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         case PROJECT:
            drawCappedLine( vertices[0].position, vertices[1].position,
                            override_weight.value_or(extras[0].weight), override_weight.value_or(extras[1].weight),
                            override_color.value_or(extras[0].stroke), override_color.value_or(extras[1].stroke), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         case SQUARE:
            drawLine( vertices[0].position, vertices[1].position,
                      override_weight.value_or(extras[0].weight), override_weight.value_or(extras[1].weight),
                      override_color.value_or(extras[0].stroke), override_color.value_or(extras[1].stroke), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
            break;
         default:
            abort();
         }
      } else if (vertices.size() == 1) {
         drawUntexturedFilledEllipse(
            vertices[0].position.x, vertices[0].position.y,
            override_weight.value_or(extras[0].weight), override_weight.value_or(extras[0].weight),
            override_color.value_or(extras[0].stroke), shape_matrix ).draw_fill( batch, transform, flatten_transforms );
      }
      break;
   }
   case QUAD:
   case QUADS:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      PShapeImpl shape;
      shape.reserve(4*vertices.size(), 6 * vertices.size());
      shape.beginShape(TRIANGLES);
      for (int i = 0; i < vertices.size(); i+=4 ) {
         PVector p0 = vertices[i].position;
         PVector p1 = vertices[i+1].position;
         PVector p2 = vertices[i+2].position;
         PVector p3 = vertices[i+3].position;
         float w0 = override_weight.value_or(extras[i].weight);
         float w1 = override_weight.value_or(extras[i+1].weight);
         float w2 = override_weight.value_or(extras[i+2].weight);
         float w3 = override_weight.value_or(extras[i+3].weight);
         color c0 = override_color.value_or(extras[i].stroke);
         color c1 = override_color.value_or(extras[i+1].stroke);
         color c2 = override_color.value_or(extras[i+2].stroke);
         color c3 = override_color.value_or(extras[i+3].stroke);

         _line(shape, p0, p1, w0, w1, c0, c1 );
         _line(shape, p1, p2, w1, w2, c1, c2 );
         _line(shape, p2, p3, w2, w3, c2, c3 );
         _line(shape, p3, p0, w3, w0, c3, c0 );
      }
      shape.endShape();
      shape.draw_fill( batch, transform, flatten_transforms);
      break;
   }
   case QUAD_STRIP:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      PShapeImpl shape;
      shape.reserve(4*vertices.size(), 6 * vertices.size());
      shape.beginShape(TRIANGLES);
      for (int i = 0; i < vertices.size()-2; i+=2 ) {
         PVector p0 = vertices[i+0].position;
         PVector p1 = vertices[i+1].position;
         PVector p2 = vertices[i+2].position;
         PVector p3 = vertices[i+3].position;
         float w0 = override_weight.value_or(extras[i+0].weight);
         float w1 = override_weight.value_or(extras[i+1].weight);
         float w2 = override_weight.value_or(extras[i+2].weight);
         float w3 = override_weight.value_or(extras[i+3].weight);
         color c0 = override_color.value_or(extras[i+0].stroke);
         color c1 = override_color.value_or(extras[i+1].stroke);
         color c2 = override_color.value_or(extras[i+2].stroke);
         color c3 = override_color.value_or(extras[i+3].stroke);

         _line(shape, p0, p1, w0, w1, c0, c1 );
         _line(shape, p1, p3, w1, w3, c1, c3 );
         _line(shape, p3, p2, w3, w2, c3, c2 );
         _line(shape, p2, p0, w2, w0, c2, c0 );
      }
      shape.endShape();
      shape.draw_fill( batch, transform, flatten_transforms);
      break;
   }
   case TRIANGLE_STRIP:
   {
      PShapeImpl triangles;
      triangles.beginShape(TRIANGLES);
      triangles.transform( shape_matrix );
      _line(triangles,
            vertices[0].position, vertices[1].position,
            override_weight.value_or(extras[0].weight), override_weight.value_or(extras[1].weight),
            override_color.value_or(extras[0].stroke),  override_color.value_or(extras[1].stroke));
      for (int i=2;i<vertices.size();++i) {
         PVector p0 = vertices[i-2].position;
         PVector p1 = vertices[i-1].position;
         PVector p2 = vertices[i-0].position;
         float w0 = override_weight.value_or(extras[i-2].weight);
         float w1 = override_weight.value_or(extras[i-1].weight);
         float w2 = override_weight.value_or(extras[i-0].weight);
         color c0 = override_color.value_or(extras[i-2].stroke);
         color c1 = override_color.value_or(extras[i-1].stroke);
         color c2 = override_color.value_or(extras[i-0].stroke);

         _line(triangles, p1, p2, w1, w2, c1, c2);
         _line(triangles, p2, p0, w1, w0, c2, c0);
      }
      triangles.endShape();
      triangles.draw_fill( batch, transform, flatten_transforms );
      break;
   }
   case TRIANGLE_FAN:
   {
      // TODO: Proper 3D miters for triangle fan edges
      PShapeImpl shape;
      int n = vertices.size();
      if (n < 3) break;

      shape.reserve(3 * (n-2), 3 * (n-2));
      shape.beginShape(TRIANGLES);

      PVector center = vertices[0].position;
      float centerWeight = extras[0].weight;
      color centerColor = extras[0].stroke;

      for (int i = 1; i < n - 1; ++i) {
         PVector p0 = vertices[i].position;
         PVector p1 = vertices[i + 1].position;
         float w0 = override_weight.value_or(extras[i].weight);
         float w1 = override_weight.value_or(extras[i + 1].weight);
         color c0 = override_color.value_or(extras[i].stroke);
         color c1 = override_color.value_or(extras[i + 1].stroke);

         // Stroke outer edges of each triangle
         _line(shape, center, p0, centerWeight, w0, centerColor, c0);
         _line(shape, p0, p1, w0, w1, c0, c1);
         _line(shape, p1, center, w1, centerWeight, c1, centerColor);
      }

      shape.endShape();
      shape.draw_fill(batch, transform, flatten_transforms);
      break;
   }
   default:
      abort();
      break;
   }
}

void PShapeImpl::draw_fill(gl::batch_t &batch, const PMatrix& transform_, bool flatten_transforms) const {
   DEBUG_METHOD();
   if (vertices.size() > 2 && kind != POINTS && kind != LINES) {
      std::vector<gl::material> m;
      m.reserve( materials.size() );
      for (const auto &material : materials ) {
         m.emplace_back(
            material.ambientColor,
            material.specularColor,
            material.emissiveColor,
            material.specularExponent
            );
      }
      std::optional<gl::color> override = style.override_fill_color ? flatten_color_mode(style.override_fill_color.value()) : std::optional<gl::color>();
      batch.vertices( vertices, m, indices, transform_.glm_data(), flatten_transforms, style.texture_.value_or(PShape::getBlankTexture()).getTextureID(), override);
   }
}

static std::vector<std::weak_ptr<PShapeImpl>> &shapeHandles() {
   static std::vector<std::weak_ptr<PShapeImpl>> handles;
   return handles;
}

static void PShape_releaseAllVAOs() {
   for (const auto &i : shapeHandles()) {
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

PShape::PShape(std::shared_ptr<PShapeImpl> impl_) : impl(impl_) {
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

PShape PShape::getChild( std::string_view i ) {
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

void PShape::rotate(float x, float y, float z, float w) {
   return impl->rotate(x,y,z,w);
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


void PShape::beginShape(int kind_){
   return impl->beginShape(kind_);
}

PShape createShape() {
   return {std::make_shared<PShapeImpl>()};
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


void PShape::shininess(float r) {
   return impl->shininess(r);
}

void PShape::specular(float r,float g,  float b, float a){
   return impl->specular(r,g,b,a);
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

void PShape::setID(std::string_view s) {
   return impl->setID(s);
}

void PShape::enableStyle() {
   return impl->enableStyle();
}

void PShape::disableStyle() {
   return impl->disableStyle();
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

void PShape::attribPosition(std::string_view name, float x, float y, float z, float w) {
   return impl->attribPosition(name, x, y, z, w);
}

PShape PShape::copy() const {
   return { std::make_shared<PShapeImpl>(*impl) };
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

   PShape obj_shape = createShape();
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

const char *kindToTxt(int kind) {
   switch (kind) {
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
      return format_to(ctx.out(), "vertices={:<4} indices={:<4} kind={:20} type={:6}",
                       v.vertices.size(), v.indices.size(),
                       kindToTxt(v.kind),
                       typeToTxt(v.type)
         );
   }
};
