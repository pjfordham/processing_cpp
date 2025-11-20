#include "processing_pshape.h"
#include "processing.h"
#include "processing_opengl_texture.h"
#include "processing_pshape_svg.h"
#include "processing_math.h"
#include <memory>
#include <vector>
#include <tesselator_cpp.h>
#include <map>

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

const char *typeToTxt(int type);
const char *kindToTxt(int kind);

template <> struct fmt::formatter<PShapeImpl>;

class PShapeImpl {
   friend struct fmt::formatter<PShapeImpl>;
   friend struct std::hash<PShapeImpl>;

public:
   bool enable_debug = false;
   bool should_compile = false;

   struct vInfoExtra {
      glm::vec3 position;
      gl::color_t gl_stroke;
      float weight;
   };

private:

   std::map<flat_style_t, gl::batch_t_ptr> cache;

   bool immediateMode = true;

   std::optional<PVector> normal_;

   std::vector<command_t> cmds;
   std::string id;
   PShape parent;
   std::vector<PShape> children;

   bool dirty = true;

   int type = OPEN;
   float tightness = 0.0F;
   std::vector<PVector> curve_vertices;
   PMatrix shape_matrix = PMatrix::Identity();

   std::vector<style_t> style_stack;
   style_t style;

   std::optional<gl::batch_t::sub_batch_t> sub_batch_fill;
   std::optional<gl::batch_t::sub_batch_t> sub_batch_stroke;

   int kind = POLYGON;
   int ci = 0;
   int qi = 0;

   float width = 1.0;
   float height = 1.0;

   int reserve_c = 0;
   int reserved_vertices = 0;

public:

   style_t getStyle() const {
      DEBUG_METHOD();
      return style;
   }

   void pushStyle() {
      DEBUG_METHOD();
      style_stack.push_back(style);
   }

   void popStyle() {
      DEBUG_METHOD();
      style = style_stack.back();
      style_stack.pop_back();
   }

   void setID(std::string_view s) {
      DEBUG_METHOD();
      id = s;
   }

   void enableStyle() {
      DEBUG_METHOD();
      style.style_enabled = true;
   }

   void showNormals(bool x) {
      DEBUG_METHOD();
      style.draw_normals= x;
   }

   void disableStyle() {
      DEBUG_METHOD();
      style.style_enabled = false;
   }

   const PMatrix& getShapeMatrix() const {
      DEBUG_METHOD();
      return shape_matrix;
   }

   PShapeImpl& operator=(const PShapeImpl&) = delete;

   PShapeImpl(PShapeImpl&& x) noexcept {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   PShapeImpl& operator=(PShapeImpl&& other) noexcept {
      DEBUG_METHOD();
      std::swap(enable_debug, other.enable_debug);
      std::swap(should_compile, other.should_compile);
      std::swap(cache, other.cache);
      std::swap(immediateMode, other.immediateMode);
      std::swap(normal_,other.normal_);
      std::swap(cmds,other.cmds);
      std::swap(id,other.id);
      std::swap(parent, other.parent);
      std::swap(children, other.children);
      std::swap(dirty,other.dirty);
      std::swap(type,other.type);
      std::swap(tightness,other.tightness);
      std::swap(curve_vertices,other.curve_vertices);
      std::swap(shape_matrix,other.shape_matrix);
      std::swap(style_stack,other.style_stack);
      std::swap(style,other.style);
      std::swap(kind,other.kind);
      std::swap(ci,other.ci);
      std::swap(qi,other.qi);
      std::swap(width,other.width);
      std::swap(height,other.height);
      std::swap(reserve_c, other.reserve_c);
      std::swap(reserved_vertices, other.reserved_vertices);
      std::swap(extraAttributes,other.extraAttributes);
      return *this;
   }

   void reserve(int c, int vertices) {
      DEBUG_METHOD();
      reserve_c = c;
      reserved_vertices = vertices;
      cmds.reserve(c);
   }

   PShapeImpl() {
      DEBUG_METHOD();
      reserve(20,4);
   }

   PShapeImpl(const PShapeImpl &copy) = default;

   ~PShapeImpl() {
      DEBUG_METHOD();
   }

   void setParentDirty() {
      DEBUG_METHOD();
      if (parent.impl.get() == this)
         abort();
      if (parent.impl)
         parent.impl->setDirty();
   }

   void setDirty() {
      DEBUG_METHOD();
      dirty = true;
      setParentDirty();
   }

   void addChild( const PShape &shape ) {
      DEBUG_METHOD();
      setDirty();
      children.push_back( shape );
   }

   void addChild( const PShapeImpl &shape ) {
      DEBUG_METHOD();
      setDirty();
      children.push_back( std::make_shared<PShapeImpl>(shape) );
   }

   void setParent(PShape p) {
      DEBUG_METHOD();
      parent = p;
   }

   PShape getParent() const {
      DEBUG_METHOD();
      return parent;
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
      return mkShape();
   }

   bool isGroup() const {
      DEBUG_METHOD();
      return kind == GROUP;
   }

   void copyStyle( const PShapeImpl &other ) {
      DEBUG_METHOD();
      style = other.style;
   }

   void clear() {
      DEBUG_METHOD();
      setDirty();
      ci = 0;
      qi = 0;
      cmds.clear();
      children.clear();
      cache.clear();
      sub_batch_fill.reset();
      sub_batch_stroke.reset();
      immediateMode = false;
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
      transform( RotateMatrix( angle, axis ) );
   }

   void translate(float x, float y, float z=0) {
      DEBUG_METHOD();
      translate(PVector{x,y,z});
   }

   void translate(PVector t) {
      DEBUG_METHOD();
      transform( TranslateMatrix(t) );
   }

   void scale(float x, float y,float z = 1) {
      DEBUG_METHOD();
      transform( ScaleMatrix( PVector{x,y,z} ) );
   }

   void scale(float x) {
      DEBUG_METHOD();
      scale(x,x,x);
   }

   PMatrix accumulateTransform(PMatrix &global_transform) {
      if (parent.impl) {
         return parent.impl->accumulateTransform( global_transform ) * shape_matrix;
      } else {
         return global_transform * shape_matrix;
      }
   }

   PMatrix global_transform; // How do I get this?

   void setShapeMatrix( const PMatrix &transform ) {
      DEBUG_METHOD();
      if (sub_batch_fill) {
         shape_matrix = transform;
         PMatrix total = accumulateTransform( global_transform );
         sub_batch_fill->setTransform( total.glm_data() );
         // Transforms may be reloaded for each draw call
         // which maybe we should optimize
         // reloadTransforms();
      } else {
         shape_matrix = transform;
         setParentDirty();
      }
   }

   void transform(const PMatrix &transform) {
      DEBUG_METHOD();
      setShapeMatrix( shape_matrix * transform );
      // Calculate appropriate transform, walk parents and rebuild
      // - walk up to first shape without a parent
      // - get global transform?, pop off each parent calculign new transform
      // - update transform in place
      // - reloead uniform array to GPU
      // = need to migrate flattened call to same code path as normal call
      // = break join between flattening transforms and compiling shape
      // won't work with flatten
      // - have to regen vertices
      // - reverse old transform and apply new one, getVerts from somewhere else instaed?
      // - just force regen of whole shape?
      // Apply same logic to stroke
      // If we are a group, just trigger a rebuild in each child and it should work
   }

   void resetMatrix() {
      DEBUG_METHOD();
      setShapeMatrix( PMatrix::Identity() );
   }

   void beginShape(int kind_ = POLYGON) {
      DEBUG_METHOD();
      if ( !this->immediateMode ) {
         fmt::print("beginshape inside beginshape/endShape doesn't make sense.\n");
         abort();
      }
      // Supported types, POLYGON, POINTS, TRIANGLES, TRINALGE_STRIP, GROUP
      clear();
      kind = kind_;
   }

   void beginContour() {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         fmt::print("beginContour not allowed outside of beginShape/endShape\n");
         abort();
      } else {
         cmds.emplace_back( command_t::type_t::CONTOUR, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, ci );
      }
   }

   void endContour() {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         fmt::print("endContour not allowed outside of beginShape/endShape\n");
         abort();
      }
   }

   void textureMode( int mode ) {
      DEBUG_METHOD();
      style.texture_mode = mode;
   }

   void material(PMaterial &mat) {
      DEBUG_METHOD();
      noStroke();
      if ( mat.texture ) {
         textureMode( NORMAL );
         texture( mat.texture.value() );
      }
      ambient(mat.ambientColor.r,mat.ambientColor.g,mat.ambientColor.b,mat.ambientColor.a);
      fill(mat.diffuseColor);
      specular(mat.specularColor.r,mat.specularColor.g,mat.specularColor.b,mat.specularColor.a);
      emissive(mat.emissiveColor.r,mat.emissiveColor.g,mat.emissiveColor.b);
      shininess(mat.specularExponent);
   }

   void texture(PImage img) {
      DEBUG_METHOD();
      if ( style.texture_enabled && style.texture_enabled.value() && style.texture_img == img )
         return;
      style.texture_enabled = true;
      style.texture_img = img;
   }

   void circleTexture() {
      DEBUG_METHOD();
      if ( style.texture_enabled && style.texture_enabled.value() && style.texture_img == PImage::circle() )
         return;
      style.texture_enabled = true;
      style.texture_mode = NORMAL;
      style.texture_img = PImage::circle();
   }

   void noTexture() {
      DEBUG_METHOD();
      if ( style.texture_enabled && !style.texture_enabled.value())
         return;
      style.texture_enabled = false;
   }

   void noNormal() {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         normal_.reset();
      } else {
         cmds.emplace_back( command_t::type_t::NONORMAL, 0,0,0, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void normal(PVector p) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         normal_ = p;
      } else {
         cmds.emplace_back( command_t::type_t::NORMAL, p.x, p.y, p.z, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void normal(float x, float y, float z) {
      DEBUG_METHOD();
      normal( PVector{x,y,z} );
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
      if ( this->immediateMode ) {
         fmt::print("vertex not allowed outside of beginShape/endShape\n");
         abort();
      } else {
         cmds.emplace_back( command_t::type_t::VERTEX, p.x, p.y, p.z, t.x, t.y, 0, PImage{} );
         ci++;
      }
   }

   void index(unsigned short i) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         fmt::print("index not allowed outside of beginShape/endShape\n");
         abort();
      } else {
         cmds.emplace_back( command_t::type_t::INDEX,  0.0F,0.0F, 0.0F,0.0F, 0.0F, i );
         qi++;
      }
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
      auto v = getVertex( getVertexCount() - 1 );
      float x1 = v.x;
      float y1 = v.y;
      for (float t = 0.01; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float x = bezierPointCubic( x1, x2, x3, x4, t );
         float y = bezierPointCubic( y1, y2, y3, y4, t );
         vertex(x, y);
      }
   }

   void bezierVertexQuadratic(PVector control, PVector anchor2) {
      DEBUG_METHOD();
      auto v = getVertex( getVertexCount() - 1 );
       float anchor1_x = v.x;
       float anchor1_y = v.y;
       for (float t = 0.01; t <= 1; t += 0.01) {
          // Compute the Bezier curve points
          float x = bezierPointQuadratic( anchor1_x, control.x, anchor2.x, t );
          float y = bezierPointQuadratic( anchor1_y, control.y, anchor2.y, t );
          vertex(x, y);
      }
   }

   void curveTightness(float alpha) {
      DEBUG_METHOD();
      tightness = alpha;
   }

   void curveVertex(PVector c) {
      DEBUG_METHOD();
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
      // OPEN or CLOSE
      if (curve_vertices.size() > 0) {
         drawCurve();
         curve_vertices.clear();
      }
      immediateMode = true;

      if (reserve_c != 0 && reserve_c != 20 && cmds.size() != reserve_c) {
         fmt::print(stderr, "cmds reservation was {} but actual size was {}\n", reserve_c, cmds.size());
         //    abort();
      }
      if (reserved_vertices != 0 && reserved_vertices != 4 && ci != reserved_vertices) {
         fmt::print(stderr,"vertices reservation was {} but actual size was {}\n", reserved_vertices, ci);
         //    abort();
      }

      if (kind == POLYGON || kind == LINES)
         type = type_;
      else
         type = CLOSE;
   }

   unsigned short getCurrentIndex() const {
      DEBUG_METHOD();
      return ci;
   }

   void populateIndices(gl::batch_t::sub_batch_t &sb, const std::vector<int> &contour);

   void index( std::vector<unsigned short> &&i ) {
      DEBUG_METHOD();
      for (auto &j : i ) {
         index( j );
      }
   }

   void specular(float r, float g, float b, float a) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.specular = {r,g,b,a};
      } else {
         cmds.emplace_back( command_t::type_t::SPECULAR, r, g, b, a, 0.0F, 0, PImage{} );
      }
   }

   void shininess(float r) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.shininess = r;
      } else {
         cmds.emplace_back( command_t::type_t::SHININESS, r, 0.0F, 0.0F, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void noAmbient() {
      DEBUG_METHOD();
        if ( this->immediateMode ) {
           style.ambient_enabled = false;
        }
   }

   void ambient(float r, float g, float b, float a) {
      DEBUG_METHOD();
        if ( this->immediateMode ) {
           style.ambient_enabled = true;
           style.ambient_color = {r,g,b,a};
        } else {
           cmds.emplace_back( command_t::type_t::AMBIENT, r, g, b, a, 0.0F, 0, PImage{} );
        }
   }

   void emissive(float r, float g, float b) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.emissive =  {r,g,b,255 };
      } else {
         cmds.emplace_back( command_t::type_t::EMISSIVE, r, g, b, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void fill(color c) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.fill_enabled = true;
         style.fill_color = c;
      } else {
         cmds.emplace_back( command_t::type_t::FILL, c.r, c.g, c.b, c.a, 0.0F, 0, PImage{} );
      }
   }

   void stroke(color c) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.stroke_enabled = true;
         style.stroke_color = c;
      } else {
         cmds.emplace_back( command_t::type_t::STROKE, c.r, c.g, c.b, c.a, 0.0F, 0, PImage{} );
      }
   }

   void strokeWeight(float x) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.stroke_weight = x;
      } else {
         cmds.emplace_back( command_t::type_t::STROKE_WEIGHT, x, 0.0F, 0.0F, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void noStroke() {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.stroke_enabled = false;
      } else {
         cmds.emplace_back( command_t::type_t::NOSTROKE, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   void noFill() {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.fill_enabled = false;
      } else {
         cmds.emplace_back( command_t::type_t::NOFILL, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0, PImage{} );
      }
   }

   bool isStroked( const flat_style_t &parent_style ) const {
      DEBUG_METHOD();
      return style.resolve_style( parent_style ).stroke_enabled;
   }

   bool isFilled( const flat_style_t &parent_style ) const {
      DEBUG_METHOD();
      return style.resolve_style( parent_style ).fill_enabled;
   }

   void tint(float r,float g,  float b, float a) {
      DEBUG_METHOD();
      if ( this->immediateMode ) {
         style.texture_tint = {r,g,b,a};
      } else {
         cmds.emplace_back( command_t::type_t::TINT, r, g, b, a, 0.0F, 0, PImage{});
      }
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
      if ( this->immediateMode ) {
         style.texture_tint = WHITE;
      } else {
         cmds.emplace_back( command_t::type_t::NOTINT, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0, PImage{});
      }
   }

   void strokeCap(int cap) {
      DEBUG_METHOD();
      if ( !this->immediateMode ) {
         fmt::print("strokeCap in beginShape/endShape doesn't really make sense, but I'll allow it.\n"); 
      }
      style.stroke_end_cap = cap;
   }

   void setStroke(bool z) {
      DEBUG_METHOD();
      if (sub_batch_stroke) {
         for (int i = 0; i < sub_batch_stroke->getVertexCount(); ++i) {
            sub_batch_stroke->vertices(i).fill = {0,0,0,0};
            sub_batch_stroke->vertices(i).emissive = {0,0,0,0};
         }
         reloadSubBatch(&sub_batch_stroke.value());
      }
      noStroke();
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::STROKE || cmd.type == command_t::type_t::NOSTROKE) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   void setStroke(std::optional<color> c) {
      DEBUG_METHOD();
      if (c) {
         setStroke(c.value());
      } else {
         setStroke(false);
      }
   }

   void setStroke(color c) {
      DEBUG_METHOD();
      if (sub_batch_stroke) {
         gl::color_t gl_color = flatten_color_mode(c);
         for (int i = 0; i < sub_batch_stroke->getVertexCount(); ++i) {
            sub_batch_stroke->vertices(i).fill = gl_color;
            sub_batch_stroke->vertices(i).emissive = gl_color;
         }
         reloadSubBatch(&sub_batch_stroke.value());
      }
      stroke(c);
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::STROKE || cmd.type == command_t::type_t::NOSTROKE) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   void setStrokeWeight(float w) {
      DEBUG_METHOD();
      if (sub_batch_stroke) {
         setDirty();
      }
      strokeWeight(w);
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::STROKE_WEIGHT) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   void setTexture( PImage img ) {
      DEBUG_METHOD();
      if (sub_batch_fill) {
         sub_batch_fill->setTexture( img.getTextureID() );
      } else {
         texture( img );
         setDirty(); // ??
      }
   }

   void setFill(bool z) {
      DEBUG_METHOD();
      // Set fill to transparent, can be restored with setFill( color )
      if (sub_batch_fill) {
         for (int i = 0; i < sub_batch_fill->getVertexCount(); ++i) {
            sub_batch_fill->vertices(i).fill = {0,0,0,0};
            sub_batch_fill->vertices(i).ambient = {0,0,0,0};
         }
         reloadSubBatch(&sub_batch_fill.value());
      }
      noFill();
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::FILL || cmd.type == command_t::type_t::NOFILL) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   void setFill(std::optional<color> c) {
      DEBUG_METHOD();
      if (c) {
         setFill(c.value());
      } else {
         setFill(false);
      }
   }

   void setFill(color c) {
      DEBUG_METHOD();
      // If we already have precompiled this shape's fill then just update in place
      if (sub_batch_fill) {
         gl::color_t gl_color = flatten_color_mode(c);
         for (int i = 0; i < sub_batch_fill->getVertexCount(); ++i) {
            sub_batch_fill->vertices(i).fill = gl_color;
            // TODO: need to check style to make sure we should do this
            sub_batch_fill->vertices(i).ambient = gl_color;
         }
         reloadSubBatch(&sub_batch_fill.value());
      }
      // Update cmds either way
      fill(c);
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::FILL || cmd.type == command_t::type_t::NOFILL) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   void setTint(color c) {
      DEBUG_METHOD();
      if (sub_batch_fill) {
         gl::color_t gl_color = flatten_color_mode(c);
         for (int i = 0; i < sub_batch_fill->getVertexCount(); ++i) {
            sub_batch_fill->vertices(i).fill = gl_color;
            // TODO: need to check style to make sure we should do this
            sub_batch_fill->vertices(i).ambient = gl_color;
         }
         reloadSubBatch(&sub_batch_fill.value());
      }
      tint(c);
      for ( auto &cmd : cmds) {
         if (cmd.type == command_t::type_t::TINT) {
            cmd.type = command_t::type_t::NOP;
         }
      }
   }

   gl::batch_t_ptr getCompiledBatch(const flat_style_t &global_style) {
      DEBUG_METHOD();
      flat_style_t local_style = style.resolve_style( global_style );
      if (dirty) {
         cache.clear();
      }
      if (cache.contains(local_style)) {
         return cache[local_style];
      } else if ( should_compile ) {
         cache.emplace(local_style, std::make_shared<gl::batch_t>());
         auto batch = cache[local_style];
         flatten( batch, PMatrix::Identity(), true, global_style );
         batch->load();
         dirty = false;
         return cache[local_style];
      } else {
         return {};
      }
   }

    void populate_normals(gl::batch_t::sub_batch_t &sb) {
      // Iterate over all triangles
      for (int i = 0; i < sb.getIndexCount(); i+=3) {
         // Get the vertices of the current triangle
         gl::vertex_t &v1 = sb.verticesByIndex(i);
         gl::vertex_t &v2 = sb.verticesByIndex(i + 1);
         gl::vertex_t &v3 = sb.verticesByIndex(i + 2);

         // Calculate the normal vector of the current triangle
         PVector edge1 = v2.position - v1.position;
         PVector edge2 = v3.position - v1.position;
         glm::vec3 normal = (edge1.cross(edge2)).normalize();

         // Add the normal to the normals list for each vertex of the triangle
         v1.normal = v1.normal + normal;
         v2.normal = v2.normal + normal;
         v3.normal = v3.normal + normal;
      }
   }

   void flatten(gl::batch_t_ptr batch, const PMatrix& transform, bool flatten_transforms, const flat_style_t &parent_style)  {
      DEBUG_METHOD();

      // Resolve accumulated style and transforms
      flat_style_t local_style = style.resolve_style( parent_style );

      // have stack of GLM matricies for transforms
      if ( kind == GROUP ) {
         for (auto &&child : children) {
            // For all child shapes pass down transform and style from this shape
            // and continue to flatten.
            child.impl->flatten(batch, transform * child.getShapeMatrix(), flatten_transforms, local_style);
         }
      } else {
         auto currentTransform = transform;// * shape_matrix;
         sub_batch_fill.emplace( *batch, ci, currentTransform.glm_data(), flatten_transforms,  style.texture_img ? style.texture_img.value().getTextureID() : std::optional<gl::texture_t_ptr>());
         auto &sb = sub_batch_fill.value();

         std::vector<int> contour; // TODO: reserve amount for this.
         std::vector<vInfoExtra> extras;
         extras.reserve(ci);
         std::vector<unsigned short> idx;
         idx.reserve(qi);

         bool fill = false;
         bool stroke = false;
         bool indices = false;

         std::optional<glm::vec3> local_normal = normal_;

         gl::color_t gl_stroke_color = flatten_color_mode( local_style.stroke_color );
         gl::color_t gl_fill_color = flatten_color_mode( local_style.fill_color );
         gl::color_t gl_tint_color = flatten_color_mode( local_style.texture_tint );
         gl::color_t gl_ambient_color = local_style.ambient_enabled ? flatten_color_mode( local_style.ambient_color ) : local_style.texture_enabled ? gl_tint_color : gl_fill_color;
         gl::color_t gl_specular_color = flatten_color_mode( local_style.specular );
         gl::color_t gl_emissive_color = flatten_color_mode( local_style.emissive );

         if (enable_debug) {
            fmt::print("SHAPE COMMANDS: {} {} [\n", kindToTxt(kind), typeToTxt(type));
            for ( const auto &cmd : cmds) {
               fmt::print("{}\n", cmd);
            }
            fmt::print("]\n");
         }
         for ( const auto &cmd : cmds) {
            if ( !local_style.style_enabled ) {
               // Skip these commands if we are using global
               // style
               switch(cmd.type) {
               case command_t::type_t::FILL:
               case command_t::type_t::NOFILL:
               case command_t::type_t::STROKE:
               case command_t::type_t::NOSTROKE:
               case command_t::type_t::TINT:
               case command_t::type_t::NOTINT:
               case command_t::type_t::STROKE_WEIGHT:
                  continue;
               default:
                  break;
               }
            }
            switch(cmd.type) {
            case command_t::type_t::VERTEX:
            {
               auto d = cmd.d;
               auto e = cmd.e;
               // TODO, move inside fill, what about tint? do fill stuff when texture is enabled too
               if (local_style.texture_enabled && local_style.texture_mode == IMAGE) {
                  d /= local_style.texture_img.width;
                  e /= local_style.texture_img.height;
               }
               if (local_style.fill_enabled || local_style.texture_enabled) {
                  fill = true;
                  sb.add_vertex(
                     {cmd.a, cmd.b, cmd.c},
                     local_normal.value_or(PVector{0.0F,0.0F,0.0F}),
                     {d,e},
                     local_style.texture_enabled ? gl_tint_color : gl_fill_color,
                     gl_ambient_color,
                     gl_specular_color,
                     gl_emissive_color,
                     local_style.shininess );
               }
               if (local_style.stroke_enabled) {
                  stroke = true;
                  extras.emplace_back( glm::vec3{cmd.a,cmd.b, cmd.c}, gl_stroke_color, local_style.stroke_weight );
               }
               if (!fill && !stroke) {
                  fmt::print("Vertex specified but fill, stroke and texture are all off ignoring. Indices will probably be inconsistent.\n");
               }
            }
            break;
            case command_t::type_t::INDEX:
               indices = true;
               // TODO can skip some of these for !fill or !stroke
               sb.add_index(cmd.f);
               idx.push_back(cmd.f);
               break;
            case command_t::type_t::CONTOUR:
               contour.push_back(cmd.f);
               break;
            case command_t::type_t::FILL:
               // abort if texture
               local_style.fill_enabled = true;
               local_style.fill_color = {cmd.a, cmd.b, cmd.c, cmd.d};
               if (!local_style.ambient_enabled) {
                  gl_ambient_color = flatten_color_mode( local_style.fill_color );
               }
               gl_fill_color = flatten_color_mode( local_style.fill_color );
               break;
            case command_t::type_t::NOP:
               break;
            case command_t::type_t::TINT:
               // abort if no texture
               local_style.texture_tint = {cmd.a, cmd.b, cmd.c, cmd.d};
               if (!local_style.ambient_enabled) {
                  gl_ambient_color = flatten_color_mode( local_style.texture_tint );
               }
               gl_tint_color = flatten_color_mode( local_style.texture_tint );
               break;
            case command_t::type_t::NOTINT:
               // abort if no texture
               local_style.texture_tint = WHITE;
               if (!local_style.ambient_enabled) {
                  gl_ambient_color = flatten_color_mode( local_style.texture_tint );
               }
               gl_tint_color = flatten_color_mode( local_style.texture_tint );
               break;
            case command_t::type_t::STROKE:
               local_style.stroke_enabled = true;
               local_style.stroke_color = {cmd.a, cmd.b, cmd.c, cmd.d};
               gl_stroke_color = flatten_color_mode(local_style.stroke_color);
               break;
            case command_t::type_t::AMBIENT:
               local_style.ambient_enabled = true;
               local_style.ambient_color = {cmd.a, cmd.b, cmd.c, cmd.d};
               gl_ambient_color = flatten_color_mode( local_style.ambient_color );
               break;
            case command_t::type_t::SPECULAR:
               local_style.specular = {cmd.a, cmd.b, cmd.c, cmd.d};
               gl_specular_color = flatten_color_mode( local_style.specular );
               break;
            case command_t::type_t::EMISSIVE:
               local_style.emissive = {cmd.a, cmd.b, cmd.c, cmd.d};
               gl_emissive_color = flatten_color_mode( local_style.emissive );
               break;
            case command_t::type_t::SHININESS:
               local_style.shininess = cmd.a;
               break;
            case command_t::type_t::NOFILL:
               // set fill color to black?
               local_style.fill_enabled = false;
               break;
            case command_t::type_t::NOSTROKE:
               local_style.stroke_enabled = false;
               break;
            case command_t::type_t::NORMAL:
               local_normal = {cmd.a, cmd.b, cmd.c};
               break;
            case command_t::type_t::NONORMAL:
               local_normal.reset();
               break;
            case command_t::type_t::STROKE_WEIGHT:
               local_style.stroke_weight = cmd.a;
               break;
            }
         }
         if ( fill ) {

            // Generate indices, from vertex info, if this shape doesn't have it.
            if (!indices) {
               populateIndices(sb, contour);
            }

            if (!local_normal) {
               // If no global normal is not set walk over triangles calculating a reasonable nomal.
               populate_normals(sb);
            }

            if (local_style.draw_normals) {
               draw_normals(sb, batch, currentTransform.glm_data(), flatten_transforms);
            }
         } else {
            sb.drop();
            sub_batch_fill.reset();
         }
         if (stroke) {
            draw_stroke( batch, currentTransform.glm_data(), flatten_transforms, local_style.stroke_end_cap, contour, extras, idx );
         }
      }
      dirty = false;
      if (!flatten_transforms) {
         // If we're compile the base _shape from PGraphics then the batch will
         // be destroyed every frame so don't save pointers to it. Probably
         // should use smart pointer in view.
         sub_batch_fill.reset();
         sub_batch_stroke.reset();
      }
   }

   void draw_normals(gl::batch_t::sub_batch_t &sb, gl::batch_t_ptr batch, const glm::mat4 &currentTransform, bool flatten_transforms);
   void draw_stroke( gl::batch_t_ptr batch, const glm::mat4 &currentTransform, bool flatten_transforms, int strokeEndCap,
                     const std::vector<int> &contour, const std::vector<vInfoExtra> &extras, const std::vector<unsigned short> &idx);

   int getChildCount() const {
      DEBUG_METHOD();
      return children.size();
   }

   int getVertexCount() const {
      DEBUG_METHOD();
      return ci;
   }

   PVector getVertex(int i) const {
      DEBUG_METHOD();
      int j = 0;
      for (const auto &cmd : cmds ) {
         if (cmd.type ==  command_t::type_t::VERTEX ) {
            if (j == i)
               return {cmd.a,cmd.b, cmd.c};
            j++;
         }
      }
      return {};
   }

   void reloadSubBatch(gl::batch_t::sub_batch_t *sb) {
      if (parent.impl) {
         parent.impl->reloadSubBatch(sb);
      } else {
         for (auto& [key, batch] : cache) {
            batch->reload(*sb);
         }
      }
   }

   void setVertex(int i, PVector v) {
      DEBUG_METHOD();
      if (sub_batch_fill) {
         sub_batch_fill->vertices(i).position = v;
         reloadSubBatch(&sub_batch_fill.value());
      }
      if (sub_batch_stroke) {
         setDirty();
      }
      int j = 0;
      for (auto &cmd : cmds ) {
         if (cmd.type ==  command_t::type_t::VERTEX ) {
            if (j == i) {
               cmd.a = v.x;
               cmd.b = v.y;
               cmd.c = v.z;
            }
            j++;
         }
      }
   }

   void setVertex(int i, float x, float y , float z = 0) {
      DEBUG_METHOD();
      setVertex(i, {x,y,z});
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

static std::vector<unsigned short> triangulatePolygon(gl::batch_t::sub_batch_t &sb,  std::vector<int> contour) {

   if (sb.getVertexCount() < 3) {
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
      tess.addContour(3, sb.vertices_data(), sizeof(gl::vertex_t), sb.getVertexCount(), offsetof(gl::vertex_t,position));
   } else {
      tess.addContour(3, sb.vertices_data(), sizeof(gl::vertex_t), contour[0],      offsetof(gl::vertex_t,position));
      contour.push_back(sb.getVertexCount());
      for ( int i = 0; i < contour.size() - 1; ++i ) {
         auto &c = contour[i];
         auto start = sb.vertices_data() + c;
         auto size = contour[i+1] - contour[i];
         tess.addContour(3, start, sizeof(gl::vertex_t), size, offsetof(gl::vertex_t,position));
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



void PShapeImpl::populateIndices( gl::batch_t::sub_batch_t &sb, const std::vector<int> &contour ) {
   // Becuase we flip the Y-axis to maintain processings coordinate system
   // we have to reverse the triangle winding direction for any geometry
   // we don't create and index ourselves.
   DEBUG_METHOD();

   if (kind == GROUP) return;

   if (sb.getVertexCount() == 0) return;

   if (kind == QUADS || kind == QUAD) {
      if (sb.getVertexCount() % 4 != 0) abort();
      for (int i = 0; i < sb.getVertexCount(); i += 4) {
         glm::vec2 A2D, B2D, C2D, D2D;
         projectTo2D(sb.vertices(i).position,
                     sb.vertices(i+1).position,
                     sb.vertices(i+2).position,
                     sb.vertices(i+3).position,
                     A2D, B2D, C2D, D2D);

         auto diag = chooseValidDiagonal(A2D, B2D, C2D, D2D);
         int d1 = diag.first;
         int d2 = diag.second;

         // First Triangle
         sb.add_index( i + d1);
         sb.add_index( i + ((d1 + 1) % 4));
         sb.add_index( i + d2);

         // Second Triangle
         sb.add_index( i + d1);
         sb.add_index( i + d2);
         sb.add_index( i + ((d2 + 1) % 4));
      }
   }
   else if (kind == QUAD_STRIP) {
      for (int i = 0; i + 3 < sb.getVertexCount(); i += 2) {
         // Just discard any trailing odd vertex
         glm::vec2 A2D, B2D, C2D, D2D;
         projectTo2D(sb.vertices(i).position,
                     sb.vertices(i+1).position,
                     sb.vertices(i+2).position,
                     sb.vertices(i+3).position,
                     A2D, B2D, C2D, D2D);

         auto diag = chooseValidDiagonal(A2D, B2D, C2D, D2D);
         int d1 = diag.first;
         int d2 = diag.second;

         // Mapping function to correct order for QUAD_STRIP
         auto s = [](int i) { return (i == 2) ? 3 : (i == 3) ? 2 : i; };

         // First Triangle
         sb.add_index( i + s(d1));
         sb.add_index( i + s((d1 + 1) % 4));
         sb.add_index( i + s(d2));

         // Second Triangle
         sb.add_index( i + s(d1));
         sb.add_index( i + s(d2));
         sb.add_index( i + s((d2 + 1) % 4));
      }
   } else if (kind == TRIANGLE_STRIP || kind == TRIANGLE_STRIP_NOSTROKE) {
      bool reverse = false;
      for (int i = 0; i < sb.getVertexCount() - 2; i++ ){
         if (reverse) {
            sb.add_index(i+2);
            sb.add_index(i+1);
            sb.add_index(i);
         } else {
            sb.add_index(i);
            sb.add_index(i+1);
            sb.add_index(i+2);
         }
         reverse = !reverse;
      }
   } else if (kind == CONVEX_POLYGON || kind == TRIANGLE_FAN) {
      // Fill with triangle fan
      for (int i = 1; i < sb.getVertexCount() - 1 ; i++ ) {
         sb.add_index( 0 );
         sb.add_index( i );
         sb.add_index( i+1 );
      }
   } else if (kind == POLYGON) {
      auto indices = triangulatePolygon(sb, contour);
      for (auto &i : indices ) {
         sb.add_index( i );
      }
   } else if (kind == TRIANGLES) {
      for (int i = 0; i < sb.getVertexCount(); i+=3 ) {
         sb.add_index( i );
         sb.add_index( i+1 );
         sb.add_index( i+2 );
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

void drawLinePoly(gl::batch_t::sub_batch_t &sb, int points, const PShapeImpl::vInfoExtra *extras, bool closed)  {
   PLine start;
   PLine end;

   if ( points < 3 )
      abort();

   int vstart = sb.getVertexCount();
   gl::color_t c0 = extras[0].gl_stroke;
   float half_weight = extras[0].weight / 2.0F;

   if (closed) {
      start = drawLineMitred(extras[points-1].position, extras[0].position, extras[1].position, half_weight );
      end = start;
   } else {
      PVector normal = PVector{extras[1].position - extras[0].position}.normal();
      normal.normalize();
      normal.mult(half_weight);
      start = {  PVector{extras[0].position} + normal, PVector{extras[0].position} - normal };
      normal = PVector{extras[points-1].position - extras[points-2].position}.normal();
      normal.normalize();
      normal.mult(half_weight);
      end = { PVector{extras[points-1].position} + normal, PVector{extras[points-1].position} - normal };
   }

   sb.add_stroke_vertex( start.start, c0 );
   sb.add_stroke_vertex( start.end, c0 );

   for (int i =0; i<points-2;++i) {
      PLine next = drawLineMitred(extras[i].position, extras[i+1].position, extras[i+2].position, half_weight);
      sb.add_stroke_vertex( next.start, c0 );
      sb.add_stroke_vertex( next.end, c0 );
   }
   if (closed) {
      PLine next = drawLineMitred(extras[points-2].position, extras[points-1].position, extras[0].position, half_weight);
      sb.add_stroke_vertex( next.start, c0 );
      sb.add_stroke_vertex( next.end, c0 );
   }

   sb.add_stroke_vertex( end.start, c0 );
   sb.add_stroke_vertex(end.end, c0 );

   bool reverse = false;
   for (int i = 0; i < (sb.getVertexCount() - vstart) - 2; i++ ){
      if (reverse) {
         sb.add_index(vstart + i+2);
         sb.add_index(vstart + i+1);
         sb.add_index(vstart + i);
      } else {
         sb.add_index(vstart + i);
         sb.add_index(vstart + i+1);
         sb.add_index(vstart + i+2);
      }
      reverse = !reverse;
   }
}

void drawRoundLine(gl::batch_t::sub_batch_t &sb, PVector p1, PVector p2, float weight1, float weight2, gl::color_t color1, gl::color_t color2 ) {

   int NUMBER_OF_VERTICES=16;

   float start_angle = (p2 - p1).heading() + HALF_PI;

   int start = sb.getVertexCount();

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      sb.add_stroke_vertex( {p1.x + cosf(i + start_angle) * weight1/2, p1.y + sinf(i+start_angle) * weight1/2, p1.z}, color1);
   }

   start_angle += PI;

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      sb.add_stroke_vertex( {p2.x + cosf(i+start_angle) * weight2/2, p2.y + sinf(i+start_angle) * weight2/2, p2.z}, color2);
   }

   // Fill with triangle fan
   for (int i = 1; i < (sb.getVertexCount() - start) - 1 ; i++ ) {
      sb.add_index(  start + 0 );
      sb.add_index(  start + i );
      sb.add_index(  start + i + 1 );
   }
}

void drawCappedLine(gl::batch_t::sub_batch_t &sb, PVector p1, PVector p2, float weight1, float weight2, gl::color_t color1, gl::color_t color2 ) {

   PVector normal1 = (p2 - p1).normal();
   normal1.normalize();
   normal1.mult(weight1/2.0F);

   PVector normal2 = (p2 - p1).normal();
   normal2.normalize();
   normal2.mult(weight2/2.0F);

   PVector end_offset1 = (p2 - p1);
   end_offset1.normalize();
   end_offset1.mult(weight1/2.0F);

   PVector end_offset2 = (p2 - p1);
   end_offset2.normalize();
   end_offset2.mult(weight2/2.0F);

   auto i0 = sb.add_stroke_vertex( p1 + normal1 - end_offset1, color1);
   auto i1 = sb.add_stroke_vertex( p1 - normal1 - end_offset1, color1);
   auto i2 = sb.add_stroke_vertex( p2 + normal2 + end_offset2, color2);
   auto i3 = sb.add_stroke_vertex( p2 - normal2 + end_offset2, color2);
   sb.add_index( i0 );
   sb.add_index( i1 );
   sb.add_index( i2 );
   sb.add_index( i0 );
   sb.add_index( i2 );
   sb.add_index( i3 );
}

PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, color color, const PMatrix &transform) {
   PShape shape = mkShape();
   shape.circleTexture();
   shape.beginShape(TRIANGLES_NOSTROKE);
   shape.noStroke();
   shape.tint(color);
   shape.transform( transform );
   x = x - width / 2.0F;
   y = y - height / 2.0F;
   shape.vertex(x,y,0,0);
   shape.vertex(x+width,y,1.0F,0);
   shape.vertex(x+width,y+height,1.0F,1.0F);
   shape.vertex(x,y+height,0,1.0F);
   shape.index( { 0,2,1,0,3,2 } );
   shape.endShape(CLOSE);
   return shape;
}

void drawUntexturedFilledEllipse_sb(gl::batch_t::sub_batch_t &sb, float x, float y, float width, float height, gl::color_t color) {
   x = x - width / 2.0F;
   y = y - height / 2.0F;
   auto i0 = sb.add_vertex( {x,y,0},              {0,0,1}, {0,0}, color, color, {},{},{} );
   auto i1 = sb.add_vertex( {x+width,y,0},        {0,0,1}, {1.0F,0}, color, color, {},{},{} );
   auto i2 = sb.add_vertex( {x+width,y+height,0}, {0,0,1}, {1.0F,1.0F}, color, color, {},{},{} );
   auto i3 = sb.add_vertex( {x,y+height,0},       {0,0,1}, {0,1.0F}, color, color, {},{},{} );
   sb.add_index( i0 );
   sb.add_index( i2 );
   sb.add_index( i1 );
   sb.add_index( i0 );
   sb.add_index( i3 );
   sb.add_index( i2 );
}

void drawLine(gl::batch_t::sub_batch_t &sb, PVector p1, PVector p2, float weight1, float weight2, gl::color_t color1, gl::color_t color2 ) {

   PVector normal1 = (p2 - p1).normal();
   normal1.normalize();
   normal1.mult(weight1/2.0F);

   PVector normal2 = (p2 - p1).normal();
   normal2.normalize();
   normal2.mult(weight2/2.0F);

   auto i0 = sb.add_stroke_vertex( p1 + normal1, color1 );
   auto i1 = sb.add_stroke_vertex( p1 - normal1, color1 );
   auto i2 = sb.add_stroke_vertex( p2 - normal2, color2 );
   auto i3 = sb.add_stroke_vertex( p2 + normal2, color2 );
   sb.add_index( i0 );
   sb.add_index( i1 );
   sb.add_index( i2 );
   sb.add_index( i0 );
   sb.add_index( i2 );
   sb.add_index( i3 );
}

void drawTriangleNormal(gl::batch_t::sub_batch_t &ssb, const gl::vertex_t &p0, const gl::vertex_t &p1, const gl::vertex_t &p2) {
   PVector pos = (p0.position + p1.position + p2.position) / 3;
   PVector n = ((p0.normal + p1.normal + p2.normal) / 3).normalize();
   gl::color_t GL_RED = {1,0,0,1};
   float length = PVector{p0.position - p1.position}.mag() / 10.0f;
   drawLine(ssb, pos, pos + length * n, length/10.0f,length/10.0f,GL_RED,GL_RED);
}

void PShapeImpl::draw_normals(gl::batch_t::sub_batch_t &sb, gl::batch_t_ptr batch, const glm::mat4 &currentTransform, bool flatten_transforms) {
   DEBUG_METHOD();
   gl::batch_t::sub_batch_t ssb( *batch, 4 * (sb.getIndexCount() / 3), currentTransform, flatten_transforms, gl::texture_t::circle());

   switch( kind ) {
   case TRIANGLE_STRIP_NOSTROKE:
   case TRIANGLES_NOSTROKE:
   case TRIANGLES:
   case TRIANGLE_STRIP:
   case QUAD_STRIP:
   case QUAD:
   case QUADS:
   case POLYGON:
   case CONVEX_POLYGON:
   case TRIANGLE_FAN:
   {     // All of these should have just been flattened to triangles
      for (int i = 0; i < sb.getIndexCount(); i+=3 ) {
         drawTriangleNormal( ssb, sb.verticesByIndex(i), sb.verticesByIndex(i+1), sb.verticesByIndex(i+2) );
      }
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

void PShapeImpl::draw_stroke( gl::batch_t_ptr batch, const glm::mat4 &currentTransform, bool flatten_transforms,
                              int strokeEndCap, const std::vector<int> &contour, const std::vector<vInfoExtra> &extras, const std::vector<unsigned short> &idx)  {

   DEBUG_METHOD();

   switch( kind ) {
   case TRIANGLES_NOSTROKE:
   case TRIANGLE_STRIP_NOSTROKE:
      return;
      break;
   case POINTS:
   {
      sub_batch_stroke.emplace( *batch, 4 * extras.size(), currentTransform, flatten_transforms, gl::texture_t::circle() );
      for (int i = 0; i< extras.size() ; ++i ) {
         drawUntexturedFilledEllipse_sb( sub_batch_stroke.value(),
                                         extras[i].position.x, extras[i].position.y,
                                         extras[i].weight, extras[i].weight,
                                         extras[i].gl_stroke );
      }
      return;
      break;
   }
   case TRIANGLES:
   {
      // TODO: Fix mitred lines to somehow work in 3D
      if (idx.size() > 0) {
         sub_batch_stroke.emplace( *batch, idx.size() * 4, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
         for (int i = 0; i < idx.size(); i+=3 ) {
            unsigned short i0 = idx[i];
            unsigned short i1 = idx[i+1];
            unsigned short i2 = idx[i+2];
            PVector p0 = extras[i0].position;
            PVector p1 = extras[i1].position;
            PVector p2 = extras[i2].position;
            float w0 = extras[i0].weight;
            float w1 = extras[i1].weight;
            float w2 = extras[i2].weight;
            gl::color_t c0 = extras[i0].gl_stroke;
            gl::color_t c1 = extras[i1].gl_stroke;
            gl::color_t c2 = extras[i2].gl_stroke;

            drawLine(sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1 );
            drawLine(sub_batch_stroke.value(), p1, p2, w1, w2, c1, c2 );
            drawLine(sub_batch_stroke.value(), p2, p0, w2, w0, c2, c0 );
         }
         return;
      } else {
         sub_batch_stroke.emplace( *batch,  extras.size() * 4, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
         for (int i = 0; i < extras.size(); i+=3 ) {
            PVector p0 = extras[i].position;
            PVector p1 = extras[i+1].position;
            PVector p2 = extras[i+2].position;
            float w0 = extras[i].weight;
            float w1 = extras[i+1].weight;
            float w2 = extras[i+2].weight;
            gl::color_t c0 = extras[i].gl_stroke;
            gl::color_t c1 = extras[i+1].gl_stroke;
            gl::color_t c2 = extras[i+2].gl_stroke;

            drawLine(sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1 );
            drawLine(sub_batch_stroke.value(), p1, p2, w1, w2, c1, c2 );
            drawLine(sub_batch_stroke.value(), p2, p0, w2, w0, c2, c0 );
         }
         return;
      }
   }
   case LINES:
   {
      sub_batch_stroke.emplace( *batch,  extras.size() * 2, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
      // TODO: Fix mitred lines to somehow work in 3D
      for (int i = 0; i < extras.size(); i+=2 ) {
         PVector p0 = extras[i].position;
         PVector p1 = extras[i+1].position;
         float w0 = extras[i].weight;
         float w1 = extras[i+1].weight;
         gl::color_t c0 = extras[i].gl_stroke;
         gl::color_t c1 = extras[i+1].gl_stroke;
         drawLine(sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1 );
      }
      return;
   }
   case POLYGON:
   case CONVEX_POLYGON:
   {
      if (extras.size() > 2 ) {
         if (type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
            sub_batch_stroke.emplace( *batch,  4 + (extras.size() - 3) * 2, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
            drawLinePoly( sub_batch_stroke.value(), extras.size() - 1, extras.data()+1, false );
            return;
         } else {
            if ( contour.empty() ) {
               sub_batch_stroke.emplace( *batch,  4 + (extras.size() - 2) * 2 + (type == CLOSE ? 2:0), currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
               drawLinePoly( sub_batch_stroke.value(), extras.size(), extras.data(), type == CLOSE );
               return;
            } else {
               // TODO: Tidy up this hack to calculate the reservation size
               int res = 0;
               if (contour[0] != 0) {
                  res += 4 + (contour[0] - 2) * 2 + (type == CLOSE ? 2:0);
               }
               auto qq = contour;
               qq.push_back(extras.size());
               for ( int i = 0; i < qq.size() - 1; ++i ) {
                  res += 4 + ((qq[i+1]-qq[i]) - 2) * 2 + (type == CLOSE ? 2:0);
               }
               sub_batch_stroke.emplace( *batch,  res, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
               if (contour[0] != 0) {
                  drawLinePoly( sub_batch_stroke.value(), contour[0], extras.data(), type == CLOSE);
               }
               auto q = contour;
               q.push_back(extras.size());
               for ( int i = 0; i < q.size() - 1; ++i ) {
                  drawLinePoly( sub_batch_stroke.value(), q[i+1] - q[i], extras.data() + q[i], type == CLOSE );
               }
               return;
            }
         }
      } else if (extras.size() == 2) {
         switch(strokeEndCap) {
         case ROUND: {
            sub_batch_stroke.emplace( *batch,  18, currentTransform, flatten_transforms, std::optional<gl::texture_t_ptr>() );
            drawRoundLine( sub_batch_stroke.value(),
                           extras[0].position, extras[1].position,
                           extras[0].weight, extras[1].weight,
                           extras[0].gl_stroke, extras[1].gl_stroke );
         }
            return;
            break;
         case PROJECT: {
            sub_batch_stroke.emplace( *batch,  4, currentTransform, flatten_transforms, std::optional<gl::texture_t_ptr>() );
            drawCappedLine( sub_batch_stroke.value(),
                            extras[0].position, extras[1].position,
                            extras[0].weight, extras[1].weight,
                            extras[0].gl_stroke, extras[1].gl_stroke );
            return;
         }
            break;
         case SQUARE: {
            sub_batch_stroke.emplace( *batch,  4, currentTransform, flatten_transforms, std::optional<gl::texture_t_ptr>() );
            drawLine( sub_batch_stroke.value(),
                      extras[0].position, extras[1].position,
                      extras[0].weight, extras[1].weight,
                      extras[0].gl_stroke, extras[1].gl_stroke );
            return;
         }
            break;
         default:
            abort();
         }
      } else if (extras.size() == 1) {
         sub_batch_stroke.emplace( *batch,  4, currentTransform, flatten_transforms, gl::texture_t::circle() );
         drawUntexturedFilledEllipse_sb(sub_batch_stroke.value(),
                                        sub_batch_stroke.value().vertices(0).position.x, sub_batch_stroke.value().vertices(0).position.y,
                                        extras[0].weight, extras[0].weight,
                                        extras[0].gl_stroke );
         return;
      }
      break;
   }
   case QUAD:
   case QUADS:
   {
      sub_batch_stroke.emplace( *batch,  extras.size() * 4, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
      // TODO: Fix mitred lines to somehow work in 3D
      for (int i = 0; i < extras.size(); i+=4 ) {
         PVector p0 = extras[i].position;
         PVector p1 = extras[i+1].position;
         PVector p2 = extras[i+2].position;
         PVector p3 = extras[i+3].position;
         float w0 = extras[i].weight;
         float w1 = extras[i+1].weight;
         float w2 = extras[i+2].weight;
         float w3 = extras[i+3].weight;
         gl::color_t c0 = extras[i].gl_stroke;
         gl::color_t c1 = extras[i+1].gl_stroke;
         gl::color_t c2 = extras[i+2].gl_stroke;
         gl::color_t c3 = extras[i+3].gl_stroke;

         drawLine( sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1 );
         drawLine( sub_batch_stroke.value(), p1, p2, w1, w2, c1, c2 );
         drawLine( sub_batch_stroke.value(), p2, p3, w2, w3, c2, c3 );
         drawLine( sub_batch_stroke.value(), p3, p0, w3, w0, c3, c0 );
      }
      return;
      break;
   }
   case QUAD_STRIP:
   {
      sub_batch_stroke.emplace( *batch,  (extras.size()-2) * 4 * 2, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
      // TODO: Fix mitred lines to somehow work in 3D
      for (int i = 0; i < extras.size()-2; i+=2 ) {
         PVector p0 = extras[i+0].position;
         PVector p1 = extras[i+1].position;
         PVector p2 = extras[i+2].position;
         PVector p3 = extras[i+3].position;
         float w0 = extras[i+0].weight;
         float w1 = extras[i+1].weight;
         float w2 = extras[i+2].weight;
         float w3 = extras[i+3].weight;
         gl::color_t c0 = extras[i+0].gl_stroke;
         gl::color_t c1 = extras[i+1].gl_stroke;
         gl::color_t c2 = extras[i+2].gl_stroke;
         gl::color_t c3 = extras[i+3].gl_stroke;

         drawLine( sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1 );
         drawLine( sub_batch_stroke.value(), p1, p3, w1, w3, c1, c3 );
         drawLine( sub_batch_stroke.value(), p3, p2, w3, w2, c3, c2 );
         drawLine( sub_batch_stroke.value(), p2, p0, w2, w0, c2, c0 );
      }
      return;
      break;
   }
   case TRIANGLE_STRIP:
   {
      sub_batch_stroke.emplace( *batch,  (extras.size()-2) * 8 + 4, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
      drawLine(sub_batch_stroke.value(),
               extras[0].position, extras[1].position,
               extras[0].weight, extras[1].weight,
               extras[0].gl_stroke, extras[1].gl_stroke);
      for (int i=2;i<extras.size();++i) {
         PVector p0 = extras[i-2].position;
         PVector p1 = extras[i-1].position;
         PVector p2 = extras[i-0].position;
         float w0 = extras[i-2].weight;
         float w1 = extras[i-1].weight;
         float w2 = extras[i-0].weight;
         gl::color_t c0 = extras[i-2].gl_stroke;
         gl::color_t c1 = extras[i-1].gl_stroke;
         gl::color_t c2 = extras[i-0].gl_stroke;

         drawLine(sub_batch_stroke.value(), p1, p2, w1, w2, c1, c2);
         drawLine(sub_batch_stroke.value(), p2, p0, w1, w0, c2, c0);
      }
      return;
      break;
   }
   case TRIANGLE_FAN:
   {
      sub_batch_stroke.emplace( *batch,  (extras.size()-1) * 4 * 3, currentTransform, flatten_transforms,std::optional<gl::texture_t_ptr>());
      // TODO: Proper 3D miters for triangle fan edges
      int n = extras.size();
      if (n < 3) break;

      PVector center = extras[0].position;
      float centerWeight = extras[0].weight;
      gl::color_t centerColor = extras[0].gl_stroke;

      for (int i = 1; i < n - 1; ++i) {
         PVector p0 = extras[i].position;
         PVector p1 = extras[i+1].position;
         float w0 = extras[i].weight;
         float w1 = extras[i + 1].weight;
         gl::color_t c0 = extras[i].gl_stroke;
         gl::color_t c1 = extras[i + 1].gl_stroke;

         // Stroke outer edges of each triangle
         drawLine(sub_batch_stroke.value(), center, p0, centerWeight, w0, centerColor, c0);
         drawLine(sub_batch_stroke.value(), p0, p1, w0, w1, c0, c1);
         drawLine(sub_batch_stroke.value(), p1, center, w1, centerWeight, c1, centerColor);
      }
      return;
      break;
   }
   }
   abort();
   return;
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
   for (const auto &i : handles) {
      if (auto p = i.lock()) {
         if (p->getChildCount() > 0) {
            p->should_compile = true;
         }
      }
   }
}

void PShape::gc() {
   static std::size_t lastSize = 0;
   auto &oldHandles = shapeHandles();
   if (oldHandles.size() > lastSize + 200 ) {
      std::vector<std::weak_ptr<PShapeImpl>> newHandles;
      for (const auto &i : oldHandles) {
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
   shape.impl->setParent( *this );
   return impl->addChild( shape );
}

PShape PShape::getChild( int i ) {
   return impl->getChild(i);
}

PShape PShape::getChild( std::string_view i ) {
   return impl->getChild(i);
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

PShape mkShape() {
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

void PShape::index( std::vector<unsigned short> &&i ){
   return impl->index(std::move(i));
}

void PShape::shininess(float r) {
   return impl->shininess(r);
}

void PShape::ambient(float r, float g, float b) {
   return impl->ambient(r,g,b,color::scaleA);
}

void PShape::ambient(float r, float g, float b, float a) {
   return impl->ambient(r,g,b,a);
}

void PShape::noAmbient() {
   return impl->noAmbient();
}

void PShape::emissive(float r, float g, float b) {
   return impl->emissive(r,g,b);
}

void PShape::specular(float r,float g,  float b, float a){
   return impl->specular(r,g,b,a);
}


void PShape::fill(float r,float g,  float b, float a){
   return impl->fill({r,g,b,a});
}

void PShape::fill(float r,float g, float b){
   return impl->fill({r,g,b, color::scaleA});
}

void PShape::fill(float r,float a){
   if (color::mode == HSB) {
      impl->fill({0,0,r,a});
   } else {
      impl->fill({r,r,r,a});
   }
}

void PShape::fill(float r){
   if (color::mode == HSB) {
      impl->fill({0,0,r,color::scaleA});
   } else {
      impl->fill({r,r,r,color::scaleA});
   }
}

void PShape::fill(color color){
   return impl->fill(color);
}

void PShape::fill(color c, float a){
   return impl->fill({c.r,c.g,c.b,a});
}


void PShape::stroke(float r,float g, float b, float a){
   return impl->stroke({r,g,b,a});
}

void PShape::stroke(float r,float g, float b){
   return impl->stroke({r,g,b,color::scaleA});
}

void PShape::stroke(float r,float a){
   if (color::mode == HSB) {
      return impl->stroke({0,0,r,a});
   } else {
      return stroke({r,r,r,a});
   }
}

void PShape::stroke(float r){
   if (color::mode == HSB) {
      return impl->stroke({r,0,0,color::scaleA});
   } else {
      return impl->stroke({r,r,r,color::scaleA});
   }
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


bool PShape::isStroked(const flat_style_t &parent_style) const{
   return impl->isStroked(parent_style);
}

bool PShape::isFilled(const flat_style_t &parent_style) const{
   return impl->isFilled(parent_style);
}


void PShape::tint(float r,float g,  float b, float a){
   return impl->tint(r,g,b,a);
}

style_t PShape::getStyle() {
   return impl->getStyle();
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


void PShape::setStroke(std::optional<color> c){
   return impl->setStroke(c);
}

void PShape::setStroke(color c){
   return impl->setStroke(c);
}


void PShape::setStrokeWeight(float w){
   impl->setStrokeWeight(w);
}


void PShape::setTexture( PImage img ){
   impl->setTexture(img);
}


void PShape::setFill(bool z){
   return impl->setFill(z);
}


void PShape::setFill(std::optional<color> c){
   return impl->setFill(c);
}

void PShape::setFill(color c){
   return impl->setFill(c);
}


void PShape::setTint(color c){
   return impl->setTint(c);
}


gl::batch_t_ptr PShape::getCompiledBatch(const flat_style_t &style) {
   return impl->getCompiledBatch(style);
}

void PShape::flatten(gl::batch_t_ptr batch, const PMatrix& transform, bool flatten_transforms, const flat_style_t &style) {
   return impl->flatten(batch, transform, flatten_transforms, style);
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

void PShape::showNormals(bool x) {
   return impl->showNormals(x);
}

void PShape::disableStyle() {
   return impl->disableStyle();
}

void PShape::reserve(int cmds, int vertices) {
   return impl->reserve(cmds, vertices);
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

   PShape obj_shape = mkShape();
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
         coords.emplace_back( i, j );
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
               obj_shape.vertex( glm::vec3( positions[ j->v ] ), {coords[ j->vt].x,1.0F-coords[ j->vt].y}  );
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
   case TRIANGLE_STRIP_NOSTROKE:
      return "TRIANGLE_STRIP_NOSTROKE";
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
      return format_to(ctx.out(), "cmds={:<4} kind={:20} type={:6}",
                       v.cmds.size(),
                       kindToTxt(v.kind),
                       typeToTxt(v.type)
         );
   }
};
