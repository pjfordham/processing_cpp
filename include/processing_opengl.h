#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <fmt/core.h>

#include "processing_enum.h"
#include "processing_opengl_framebuffer.h"
#include "processing_utils.h"
#include "processing_pimage.h"
#include "processing_pshape.h"

typedef int GLint;
typedef unsigned int GLuint;


namespace gl {

   struct color {
      float r,g,b,a;
   };

   struct vertex {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 coord;
      color fill;
      int tunit;
      int mindex;
   };

   class attribute {
      GLint id = -1;
      GLint shaderId = -1;
   public:
      attribute() {}
      attribute(GLuint shaderID, const std::string &attribute);
      void bind_vec2(std::size_t stride, void *offset);
      void bind_vec3(std::size_t stride, void *offset);
      void bind_vec4(std::size_t stride, void *offset);
      void bind_int(std::size_t stride, void *offset);
   };

   class uniform {
      GLint id = -1;
   public:
      uniform() {}
      uniform(GLuint shaderID, const std::string &uniform);
      void set(float value) const;
      void set(int value) const;
      void set(const glm::vec2 &value) const;
      void set(const glm::vec3 &value) const;
      void set(const glm::vec4 &value) const;
      void set(const std::vector<int> &value) const;
      void set(const std::vector<glm::vec3> &value) const;
      void set(const std::vector<glm::mat4> &value) const;
      void set(const glm::mat4 &value) const;
   };

   class scene_t {
      uniform AmbientLight;
      uniform DirectionLightColor;
      uniform DirectionLightVector;
      uniform NumberOfPointLights;
      uniform PointLightColor;
      uniform PointLightPosition;
      uniform PointLightFalloff;
      uniform PVmatrix;
   public:
      scene_t() {}
      void setup(uniform AmbientLight_, uniform DirectionLightColor_, uniform DirectionLightVector_,
                 uniform NumberOfPointLights_,
                 uniform PointLightColor_,
                 uniform PointLightPosition_,
                 uniform PointLightFalloff_,
                 uniform PVmatrix_) {
         AmbientLight = AmbientLight_;
         DirectionLightColor = DirectionLightColor_;
         DirectionLightVector = DirectionLightVector_;
         NumberOfPointLights = NumberOfPointLights_;
         PointLightColor = PointLightColor_;
         PointLightPosition = PointLightPosition_;
         PointLightFalloff = PointLightFalloff_;
         PVmatrix = PVmatrix_;
      }

      float screenX(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (projection_matrix * (view_matrix * in)).x;
      }

      float screenY(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (projection_matrix * (view_matrix * in)).y;
      }

      bool lights = false;
      glm::vec3 directionLightColor =  { 0.0, 0.0, 0.0 };
      glm::vec3 directionLightVector = { 0.0, 0.0, 0.0 };
      glm::vec3 ambientLight =         { 0.0, 0.0, 0.0 };
      std::vector<glm::vec3> pointLightColors;
      std::vector<glm::vec3> pointLightPoss;
      glm::vec3 pointLightFalloff =    { 1.0, 0.0, 0.0 };
      glm::mat4 projection_matrix;
      glm::mat4 view_matrix;
      void set();
      void setProjectionMatrix( const glm::mat4 &PV ) {
         projection_matrix = PV;
      }

      void setViewMatrix( const glm::mat4 &PV ) {
         view_matrix = PMatrix::FlipY().glm_data() * PV ;
      }

      void setDirectionLightColor(const glm::vec3 &color ){
         directionLightColor = color;
      }

      void setDirectionLightVector(const glm::vec3 &dir  ){
         directionLightVector = dir;
      }

      void setAmbientLight(const glm::vec3  &color ){
         ambientLight = color;
      }

      void pushPointLightColor( const glm::vec3  &color ) {
         if (pointLightColors.size() < 8) {
            pointLightColors.push_back( color );
         } else {
            fmt::print("Ignoring >8 point lights\n.");
         }
      }

      void pushPointLightPosition( const glm::vec3 &pos  ) {
         if (pointLightColors.size() < 8) {
            pointLightPoss.push_back( pos );
         }
      }

      void clearPointLights() {
         pointLightColors.clear();
         pointLightPoss.clear();
      }

      void setPointLightFalloff( const glm::vec3 &data){
         pointLightFalloff = data;
      }

      void setLights( bool data ) {
         lights = data;
      }
   };

   class VAO {
      GLuint vao = 0;
      GLuint indexId = 0;
      GLuint vertexId = 0;
   public:
      friend struct fmt::formatter<gl::vertex>;

      std::vector<vertex> vertices;
      std::vector<unsigned short> indices;
      std::vector<PImage> textures;
      std::vector<glm::mat4> transforms;

      VAO() noexcept;

      VAO(const VAO& x) noexcept = delete;

      VAO(VAO&& x) noexcept;

      VAO& operator=(const VAO&) = delete;

      VAO& operator=(VAO&& other) noexcept;

      void alloc( attribute Position, attribute Normal, attribute Color,
                  attribute Coord,    attribute TUnit,  attribute MIndex);
      int hasTexture(PImage texture);
      void loadBuffers() const;
      void draw() const;
      ~VAO();
   };

   class shader_t {
   public:
      bool operator!=(const shader_t &other) {
         return programID != other.programID;
      }
      GLuint programID = 0;
      shader_t() {}
      shader_t(const char *vertSource, const char *fragSource);
      ~shader_t();
      void bind() const;
      uniform get_uniform(const std::string &uniform_name) const {
         return {programID, uniform_name};
      }
      attribute get_attribute(const std::string &attribute_name) const {
         return {programID, attribute_name};
      }
   };

   class batch_t {
      attribute Position;
      attribute Normal;
      attribute Color;
      attribute Coord;
      attribute TUnit;
      attribute MIndex;
      uniform Mmatrix;
   public:
      std::vector<VAO> vaos;
      batch_t() {}
      void setup( attribute Position_, attribute Normal_, attribute Color_,
                  attribute Coord_,    attribute TUnit_,  attribute MIndex_,
                  uniform Mmatrix_ ) {
         Position = Position_;
         Normal = Normal_;
         Color = Color_;
         Coord = Coord_;
         TUnit = TUnit_;
         MIndex = MIndex_;
         Mmatrix = Mmatrix_;
      }
      size_t size();
      void compile();
      void draw();
      void clear();
   };

   class context {
      int MaxTextureImageUnits;

   public:
      context();

      context(const context &x) = delete;

      context(context &&x) noexcept : context() {
         *this = std::move(x);
      }

      context& operator=(const context&) = delete;

      context& operator=(context&&x) noexcept {
         std::swap(MaxTextureImageUnits,x.MaxTextureImageUnits);
         return *this;
      }

      void blendMode( int b );

      void init();

      ~context();

      void hint(int type);

      void setShader(const shader_t &shader, scene_t &scene, batch_t &batch) {
         shader.bind();

         uniform uSampler = shader.get_uniform("myTextures");
         uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

         // TransformMatrix = shader.get_uniform("transformMatrix");

         scene.setup(
            shader.get_uniform("ambientLight"),
            shader.get_uniform("directionLightColor"),
            shader.get_uniform("directionLightVector"),
            shader.get_uniform("numberOfPointLights"),
            shader.get_uniform("pointLightColor"),
            shader.get_uniform("pointLightPosition"),
            shader.get_uniform("pointLightFalloff"),
            shader.get_uniform("PVmatrix"));

         batch.setup(
            shader.get_attribute("position"),
            shader.get_attribute("normal"),
            shader.get_attribute("color"),
            shader.get_attribute("coord"),
            shader.get_attribute("tunit"),
            shader.get_attribute("mindex"),
            shader.get_uniform("Mmatrix"));
      }


   };

   color flatten_color_mode(::color c);

} // namespace gl

template <>
struct fmt::formatter<gl::color> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::color& v, FormatContext& ctx) {
      return format_to(ctx.out(), "R{:8.2f},G{:8.2f},B{:8.2f},A{:8.2f}",v.r,v.g,v.b,v.a);
   }
};

template <>
struct fmt::formatter<gl::vertex> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::vertex& v, FormatContext& ctx) {
      return format_to(ctx.out(), "P{} N{} Tu{} Tc{} C{}",
                       v.position,
                       v.normal,
                       v.tunit,
                       v.coord,
                       v.fill);
   }
};


#endif
