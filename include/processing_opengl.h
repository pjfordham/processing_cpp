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

   struct material {
      glm::vec4 ambient;
      glm::vec4 specular;
      glm::vec4 emmisive;
      float shininess;
   };

   struct light {
      glm::vec4 position =  { 0.0, 0.0, 0.0, 0.0 };
      glm::vec3 normal =  { 0.0, 0.0, 0.0 };;
      glm::vec3 ambient =  { 1.0, 1.0, 1.0 };;
      glm::vec3 diffuse =  { 0.0, 0.0, 0.0 };;
      glm::vec3 specular =  { 0.0, 0.0, 0.0 };;
      glm::vec3 falloff =  { 1.0, 0.0, 0.0 };;
      glm::vec2 spot =  { 0.0, 0.0 };;
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
      void bind_float(std::size_t stride, void *offset);
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
      void set(const std::vector<glm::vec2> &value) const;
      void set(const std::vector<glm::vec3> &value) const;
      void set(const std::vector<glm::vec4> &value) const;
      void set(const std::vector<glm::mat4> &value) const;
      void set(const std::vector<glm::mat3> &value) const;
      void set(const glm::mat4 &value) const;
   };

   class scene_t {
      glm::mat4 projection_matrix;
      glm::mat4 view_matrix;

      uniform LightCount;
      uniform LightPosition;
      uniform LightNormal;
      uniform LightAmbient;
      uniform LightDiffuse;
      uniform LightSpecular;
      uniform LightFalloff;
      uniform LightSpot;
      uniform PVmatrix;
      uniform Eye;

      public:
      std::vector<light> lights;
      scene_t() {}
      void setup( uniform LightCount_, uniform LightPosition_, uniform LightNormal_, uniform LightAmbient_,
                  uniform LightDiffuse_, uniform LightSpecular_, uniform LightFalloff_, uniform LightSpot_,
                  uniform PVmatrix_, uniform Eye_) {
         LightCount = LightCount_;
         LightPosition = LightPosition_;
         LightNormal = LightNormal_;
         LightAmbient = LightAmbient_;
         LightDiffuse = LightDiffuse_;
         LightSpecular = LightSpecular_;
         LightFalloff = LightFalloff_;
         LightSpot = LightSpot_;
         PVmatrix = PVmatrix_;
         Eye = Eye_;
      }

      float screenX(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (projection_matrix * (view_matrix * in)).x;
      }

      float screenY(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (projection_matrix * (view_matrix * in)).y;
      }

      void set();
      void setProjectionMatrix( const glm::mat4 &PV ) {
         projection_matrix = PV;
      }

      void setViewMatrix( const glm::mat4 &PV ) {
         view_matrix = PMatrix::FlipY().glm_data() * PV ;
      }

      void pushAmbientLight( glm::vec3 color ) {
         lights.emplace_back( light{{0.0f,0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushDirectionalLight( glm::vec3 color, glm::vec3 vector ) {
         lights.emplace_back( light{{0.0f,0.0f,0.0f,0.0f}, vector, {0,0,0}, color, {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushPointLight( glm::vec3 color, glm::vec4 position, glm::vec3 falloff) {
         lights.emplace_back( light{position, {0.0f,0.0f,0.0f}, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, falloff, {0.0f,0.0f}} );
      }

      void pushSpotLight(  glm::vec3 color, glm::vec4 position, glm::vec3 direction,  glm::vec3 falloff, glm::vec2 spot) {
         lights.emplace_back( light{position, direction, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, falloff, spot });
      }

      void clearLights() {
         lights.clear();
      }

      void flatLight() {
         clearLights();
         pushAmbientLight(glm::vec3{1.0f,1.0f,1.0f});
      }
   };

   class VAO {
      GLuint vao = 0;
      GLuint indexId = 0;
      GLuint vertexId = 0;
      GLuint materialId= 0;
   public:
      friend struct fmt::formatter<gl::VAO>;

      std::vector<vertex> vertices;
      std::vector<material> materials;
      std::vector<unsigned short> indices;
      std::vector<PImage> textures;
      std::vector<glm::mat4> transforms;

      VAO() noexcept;

      VAO(const VAO& x) noexcept = delete;

      VAO(VAO&& x) noexcept;

      VAO& operator=(const VAO&) = delete;

      VAO& operator=(VAO&& other) noexcept;

      void bind( attribute Position, attribute Normal, attribute Color,
                 attribute Coord,    attribute TUnit,  attribute MIndex,
                 attribute Ambient,  attribute Specular, attribute Emmisive, attribute Shininess);
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
      uniform Nmatrix;
      uniform TexOffset;
      attribute Ambient, Specular, Emmisive, Shininess;
      std::vector<VAO> vaos;

   public:
      batch_t() {}
      void setup( attribute Position_, attribute Normal_, attribute Color_,
                  attribute Coord_,    attribute TUnit_,  attribute MIndex_,
                  uniform Mmatrix_, uniform Nmatrix_, uniform TexOffset_,
                  attribute Ambient_, attribute Specular_, attribute Emmisive_, attribute Shininess_) {
         Position = Position_;
         Normal = Normal_;
         Color = Color_;
         Coord = Coord_;
         TUnit = TUnit_;
         MIndex = MIndex_;
         Mmatrix = Mmatrix_;
         Nmatrix = Nmatrix_;
         TexOffset = TexOffset_;
         Ambient = Ambient_;
         Specular = Specular_;
         Emmisive = Emmisive_;
         Shininess = Shininess_;
      }
      size_t size();
      void compile();
      void draw();
      void draw(const glm::mat4& transform);
      void clear();
      bool usesCircles() const;
      bool usesTextures() const;

      void vertices( const std::vector<vertex> &vertices,const std::vector<material> &materials,  const std::vector<unsigned short> &indices,
                     const glm::mat4 &transform, bool flatten_transform, PImage texture );
   };

   class context {
   public:
      void blendMode( int b );

      void init();

      void hint(int type);

      void setShader(const shader_t &shader, scene_t &scene, batch_t &batch) {
         shader.bind();

         uniform uSampler = shader.get_uniform("texture");
         uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

         // TransformMatrix = shader.get_uniform("transformMatrix");

         scene.setup(
            shader.get_uniform("lightCount"),
            shader.get_uniform("lightPosition"),
            shader.get_uniform("lightNormal"),
            shader.get_uniform("lightAmbient"),
            shader.get_uniform("lightDiffuse"),
            shader.get_uniform("lightSpecular"),
            shader.get_uniform("lightFalloff"),
            shader.get_uniform("lightSpot"),
            shader.get_uniform("PVmatrix"),
            shader.get_uniform("eye"));

         batch.setup(
            shader.get_attribute("position"),
            shader.get_attribute("normal"),
            shader.get_attribute("color"),
            shader.get_attribute("texCoord"),
            shader.get_attribute("tunit"),
            shader.get_attribute("mindex"),
            shader.get_uniform("Mmatrix"),
            shader.get_uniform("Nmatrix"),
            shader.get_uniform("texOffset"),
            shader.get_attribute("ambient"),
            shader.get_attribute("specular"),
            shader.get_attribute("emisive"),
            shader.get_attribute("shininess"));
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
