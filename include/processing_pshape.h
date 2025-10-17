#ifndef PROCESSING_PSHAPE_H
#define PROCESSING_PSHAPE_H

#include "processing_math.h"
#include "processing_color.h"
#include "processing_enum.h"
#include "processing_pimage.h"
#include "processing_java_compatability.h"
#include "processing_pmaterial.h"

#include <vector>
#include <string_view>
#include <memory>

class PShapeImpl;

namespace gl {
   class batch_t;
   typedef std::shared_ptr<batch_t> batch_t_ptr;
}

struct command_t {
   enum class type_t {
      NOP,
      VERTEX,
      INDEX,
      INDEXQUAD,
      CONTOUR,
      FILL,
      TINT,
      NOTINT,
      STROKE,
      AMBIENT,
      SPECULAR,
      EMISSIVE,
      SHININESS,
      NOFILL,
      NOSTROKE,
      NORMAL,
      NONORMAL,
      STROKE_WEIGHT,
   } type;
   float a,b,c,d,e;
   int f;
   PImage tex;
};

template <>
struct fmt::formatter<command_t::type_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const command_t::type_t& v, FormatContext& ctx) {
      switch(v) {
      case command_t::type_t::NOP:
         return fmt::format_to(ctx.out(), "NOP");
      case command_t::type_t::VERTEX:
         return fmt::format_to(ctx.out(), "VERTEX");
      case command_t::type_t::INDEX:
         return fmt::format_to(ctx.out(), "INDEX");
      case command_t::type_t::INDEXQUAD:
         return fmt::format_to(ctx.out(), "INDEXQUAD");
      case command_t::type_t::CONTOUR:
         return fmt::format_to(ctx.out(), "CONTOUR");
      case command_t::type_t::FILL:
         return fmt::format_to(ctx.out(), "FILL");
      case command_t::type_t::TINT:
         return fmt::format_to(ctx.out(), "TINT");
      case command_t::type_t::NOTINT:
         return fmt::format_to(ctx.out(), "NOTINT");
      case command_t::type_t::STROKE:
         return fmt::format_to(ctx.out(), "STROKE");
      case command_t::type_t::AMBIENT:
         return fmt::format_to(ctx.out(), "AMBIENT");
      case command_t::type_t::SPECULAR:
         return fmt::format_to(ctx.out(), "SPECULAR");
      case command_t::type_t::EMISSIVE:
         return fmt::format_to(ctx.out(), "EMISSIVE");
      case command_t::type_t::SHININESS:
         return fmt::format_to(ctx.out(), "SHININESS");
      case command_t::type_t::NOFILL:
         return fmt::format_to(ctx.out(), "NOFILL");
      case command_t::type_t::NOSTROKE:
         return fmt::format_to(ctx.out(), "NOSTROKE");
      case command_t::type_t::NORMAL:
         return fmt::format_to(ctx.out(), "NORMAL");
      case command_t::type_t::NONORMAL:
         return fmt::format_to(ctx.out(), "NONORMAL");
      case command_t::type_t::STROKE_WEIGHT:
         return fmt::format_to(ctx.out(), "STROKE_WEIGHT");
      }
      abort();
      return fmt::format_to(ctx.out(), "ABORT");
   }
};
template <>

struct fmt::formatter<command_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const command_t& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "{} {} {} {} {} {} {}", v.type, v.a, v.b, v.c, v.d, v.e, v.f);
   }
};

struct style_t {
   std::optional<bool> style_enabled;
   std::optional<bool> fill_enabled;
   std::optional<color> fill_color;

   std::optional<bool> draw_normals;
   std::optional<bool> stroke_enabled;
   std::optional<color> stroke_color;
   std::optional<float> stroke_weight;
   std::optional<int> stroke_end_cap;

   std::optional<bool> ambient_enabled;
   std::optional<color> ambient_color;

   std::optional<bool> texture_enabled;
   std::optional<PImage> texture_img;
   std::optional<color> texture_tint;
   std::optional<int> texture_mode;

   std::optional<color> specular;
   std::optional<color> emissive;
   std::optional<float> shininess;

   auto tie() const {
      return std::tie(
         style_enabled, fill_enabled, fill_color, draw_normals,
         stroke_enabled, stroke_color, stroke_weight, stroke_end_cap,
         ambient_enabled, ambient_color,
         texture_enabled, texture_img, texture_tint, texture_mode,
         specular, emissive, shininess
         );
   }

   bool operator<(const style_t& other) const {
      return tie() < other.tie();
   }

   bool operator==(const style_t &other) const = default;

};

struct flat_style_t {
   bool style_enabled;
   bool fill_enabled;
   color fill_color;

   bool draw_normals;
   bool stroke_enabled;
   color stroke_color;
   float stroke_weight;
   int stroke_end_cap;

   bool ambient_enabled;
   color ambient_color;

   bool texture_enabled;
   PImage texture_img;
   color texture_tint;
   int texture_mode;

   color specular;
   color emissive;
   float shininess;

   flat_style_t(const style_t& s) 
      : style_enabled(*s.style_enabled),
        fill_enabled(*s.fill_enabled),
        fill_color(*s.fill_color),
        draw_normals(*s.draw_normals),
        stroke_enabled(*s.stroke_enabled),
        stroke_color(*s.stroke_color),
        stroke_weight(*s.stroke_weight),
        stroke_end_cap(*s.stroke_end_cap),
        ambient_enabled(*s.ambient_enabled),
        ambient_color(*s.ambient_color),
        texture_enabled(*s.texture_enabled),
        texture_img( texture_enabled ? *s.texture_img : PImage()),
        texture_tint(*s.texture_tint),
        texture_mode(*s.texture_mode),
        specular(*s.specular),
        emissive(*s.emissive),
        shininess(*s.shininess)
      {}
};

class PShape {
   friend PShapeImpl;
   std::shared_ptr<PShapeImpl> impl;
   PShape( std::shared_ptr<PShapeImpl> impl_ );
   friend PShape mkShape();
 public:
   static void init();
   static void optimize(style_t global_shape);
   static void gc();
   static void close();

   float width=0, height=0;

   PShape();

   bool operator==(const PShape &x) const {
      return impl == x.impl;
   };

   bool operator!=(const PShape &x) const {
      return !(x==*this);
   };

   struct vInfoExtra {
      color stroke;
      float weight;
   };

   const PMatrix& getShapeMatrix();

   void addChild( const PShape shape );

   PShape getChild( int i );

   PShape getChild( std::string_view i );

   float getStrokeWeight() const;

   std::optional<color> getStrokeColor() const;

   std::optional<color> getFillColor() const;

   color getTintColor() const;

   bool isGroup() const;

   void setID( std::string_view );

   bool usesGlobalStyle() const;

   void showNormals(bool x);

   void enableStyle();

   void disableStyle();

   void reserve(int cmds, int vertices);

   void copyStyle( const PShape other );

   void clear();

   void rotate(float angle);

   void rotateZ(float angle);

   void rotateY(float angle);

   void rotateX(float angle);

   void rotate(float angle, PVector axis);

   void rotate(float x, float y, float z, float w);

   void translate(PVector t);

   void translate(float x, float y, float z=0);

   void scale(float x, float y,float z = 1);

   void scale(float x);

   void transform(const PMatrix &transform);

   void resetMatrix();

   void beginShape(int style_ = POLYGON);

   void beginContour();

   void endContour();

   void textureMode( int mode_ );

   bool isTextureSet( const style_t &parent_style ) const;

   void material(PMaterial &mat);

   void texture(PImage img);

   void circleTexture();

   void noTexture();

   void noNormal();

   void normal(PVector p);

   void normal(float x, float y, float z);

   void vertex(float x, float y, float z);

   void vertex(float x, float y);

   void vertex(PVector p);

   void vertex(float x, float y, float z, float u, float v);

   void vertex(float x, float y, float u, float v);

   void vertex(PVector p, PVector2 t);

   void index(unsigned short i);

   void index( std::vector<unsigned short> &&i );

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4);

   void bezierVertex(PVector v2, PVector v3, PVector v4);

   void bezierVertex(float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);

   void bezierVertexQuadratic(PVector control, PVector anchor2);

   void curveTightness(float alpha);

   void curveVertex(PVector c);

   void drawCurve();

   void curveVertex(float x, float y, float z);
   void curveVertex(float x, float y);

   void endShape(int type_ = OPEN);

   unsigned short getCurrentIndex();

   void ambient(float r,float g, float b);

   void emissive(float r,float g, float b);

   void specular(float r,float g, float b, float a);

   void shininess(float r);

   void noAmbient();

   void fill(float r,float g,  float b, float a);

   void fill(float r,float g, float b);

   void fill(float r,float a);

   void fill(float r);

   void fill(class color color);

   void fill(class color color, float a);

   void stroke(float r,float g,  float b, float a);

   void stroke(float r,float g, float b);

   void stroke(float r,float a);

   void stroke(float r);

   void stroke(color c);

   void strokeWeight(float x);

   void noStroke();

   void noFill();

   bool isStroked(style_t parent_style) const;

   bool isFilled(style_t parent_style) const;

   void tint(float r,float g,  float b, float a);

   void tint(float r,float g, float b);

   void tint(float r,float a);

   void tint(float r);

   void tint(color c);

   void noTint();

   void strokeCap(int cap);

   void setStroke(bool c);

   void setStroke(std::optional<color> c);

   void setStroke(color c);

   void setStrokeWeight(float w);

   void setTexture( PImage img );

   void setFill(bool z);

   void setFill(std::optional<color> c);

   void setFill(color c);

   void setTint(color c);

   void compile(style_t global_style);

   bool isCompiled(style_t global_style);

   bool shouldCompile() const;

   gl::batch_t_ptr getCompiledBatch(style_t style);

   style_t getStyle();

   gl::batch_t_ptr getBatch() ;

   void flatten(gl::batch_t_ptr parent_batch, const PMatrix& transform, bool flatten_transforms, style_t global_shape) ;

   int getChildCount() const;

   int getVertexCount() const;

   PVector getVertex(int i) const;

   PShape copy() const;

   void setVertex(int i, PVector v);

   void setVertex(int i, float x, float y , float z = 0);

   PVector getVertex(int i, PVector &x) const;

   void attribPosition(std::string_view name, float x, float y, float z=0.0, float w=0.0);
};

PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius);
PShape drawUntexturedFilledEllipse(float x, float y, float width, float height, color color, const PMatrix &transform);

PShape loadShapeOBJ(std::string_view objPath);
PShape loadShape(std::string_view objPath);
PShape mkShape();

#endif
