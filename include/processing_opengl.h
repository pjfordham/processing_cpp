#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <vector>
#include <fmt/core.h>

#include "processing_opengl_shader.h"
#include "processing_opengl_texture.h"
#include "processing_opengl_color.h"
#include "processing_utils.h"
#include "processing_enum.h"
#include "processing_math.h"
#include "processing_color.h"

typedef int GLint;
typedef unsigned int GLuint;


namespace gl {

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
      glm::vec4 emissive;
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

      int currentBlendMode = BLEND;
      bool depth_test = true;
      bool depth_mask = true;

   public:
      int blendMode( int b );
      void hint(int type);

      std::vector<light> lights;
      scene_t() {}
      void setup( const shader_t &shader) {
         LightCount = shader.get_uniform("lightCount");
         LightPosition = shader.get_uniform("lightPosition");
         LightNormal = shader.get_uniform("lightNormal");
         LightAmbient = shader.get_uniform("lightAmbient");
         LightDiffuse = shader.get_uniform("lightDiffuse");
         LightSpecular = shader.get_uniform("lightSpecular");
         LightFalloff = shader.get_uniform("lightFalloff");
         LightSpot = shader.get_uniform("lightSpot");
         PVmatrix = shader.get_uniform("PVmatrix");
         Eye = shader.get_uniform("eye");
      }

      float screenX(float x, float y, float z) {
         glm::vec4 in = { x, y, z, 1 };
         return (projection_matrix * (view_matrix * in)).x;
      }

      float screenY(float x, float y, float z) {
         glm::vec4 in = { x, y, z, 1 };
         return (projection_matrix * (view_matrix * in)).y;
      }

      void set();
      void setProjectionMatrix( const glm::mat4 &PV ) {
         projection_matrix = PV;
      }

      void setViewMatrix( const glm::mat4 &PV ) {
         glm::mat4 flipY = {
            { 1.0f,  0.0f, 0.0f, 0.0f },
            { 0.0f, -1.0f, 0.0f, 0.0f },
            { 0.0f,  0.0f, 1.0f, 0.0f } ,
            { 0.0f,  0.0f, 0.0f, 1.0f } };
         view_matrix = flipY * PV ;
      }

      void pushAmbientLight( glm::vec3 color ) {
         lights.emplace_back( light{{0.0f,0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushDirectionalLight( glm::vec3 color, glm::vec3 vector, glm::vec3 specular ) {
         lights.emplace_back( light{{0.0f,0.0f,0.0f,0.0f}, vector, {0,0,0}, color, specular, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushPointLight( glm::vec3 color, glm::vec4 position, glm::vec3 specular, glm::vec3 falloff) {
         lights.emplace_back( light{position, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, specular, falloff, {0.0f,0.0f}} );
      }

      void pushSpotLight(  glm::vec3 color, glm::vec4 position, glm::vec3 direction,  glm::vec3 specular, glm::vec3 falloff, glm::vec2 spot) {
         lights.emplace_back( light{position, direction, {0.0f,0.0f,0.0f}, color, specular, falloff, spot });
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
      std::vector<gl::texture_ptr> textures;
      std::vector<glm::mat4> transforms;

      VAO() noexcept;

      VAO(const VAO& x) noexcept;

      VAO(VAO&& x) noexcept;

      VAO& operator=(const VAO&) = delete;

      VAO& operator=(VAO&& other) noexcept;

      void bind( attribute Position, attribute Normal, attribute Color,
                 attribute Coord,    attribute TUnit,  attribute MIndex,
                 attribute Ambient,  attribute Specular, attribute Emissive, attribute Shininess);
      int hasTexture(gl::texture_ptr texture);
      void loadBuffers() const;
      void draw() const;
      void debugPrint() const;
      ~VAO();
   };

   typedef std::shared_ptr<VAO>  VAO_ptr;

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
      attribute Ambient, Specular, Emissive, Shininess;
      std::vector<VAO_ptr> vaos;
      bool uses_textures = false;
      bool uses_circles = false;

   public:
      batch_t() noexcept {}

      batch_t(const batch_t& x) noexcept = default;
      batch_t& operator=(const batch_t&) = delete;

      batch_t(batch_t&& x) noexcept =default;
      batch_t& operator=(batch_t&& other) noexcept=default;

      void setup( const shader_t &shader );
      size_t size();
      void load();
      void bind();

      void setupTextures(VAO_ptr);
      void draw();
      void draw(const glm::mat4& transform);
      void clear();
      bool usesCircles() const;
      bool usesTextures() const;

      void vertices( const std::vector<vertex> &vertices,const std::vector<material> &materials,  const std::vector<unsigned short> &indices,
                     const glm::mat4 &transform, bool flatten_transform, gl::texture_ptr texture );
   };

   class framebuffer;
   class frame_t {
      struct geometry_t {
         batch_t batch;
         scene_t scene;
         const shader_t &shader;
      };
      std::vector<geometry_t> geometries;
      color background_={0,0,0,1};
      bool c = false;

   public:
      void background(color b) {
         c = true;
         background_ = b;
      }

      void add(batch_t &&b, scene_t sc, const shader_t &sh) {
         geometries.emplace_back( std::move(b), sc, sh );

      }

      void clear() {
         c = false;
         geometries.clear();
      }

      void render(framebuffer &fb);
   };

   void renderDirect( framebuffer &fb, gl::batch_t &batch, const glm::mat4 &transform, scene_t scene, const shader_t &shader );

} // namespace gl

// template <>
// struct fmt::formatter<gl::color> {
//    // Format the MyClass object
//    template <typename ParseContext>
//    constexpr auto parse(ParseContext& ctx) {
//       return ctx.begin();
//    }

//    template <typename FormatContext>
//    auto format(const gl::color& v, FormatContext& ctx) {
//       return fmt::format_to(ctx.out(), "R{:8.2f},G{:8.2f},B{:8.2f},A{:8.2f}",v.r,v.g,v.b,v.a);
//    }
// };

template <>
struct fmt::formatter<gl::vertex> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::vertex& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "P{} N{} Tu{} Tc{} C{}",
                       v.position,
                       v.normal,
                       v.tunit,
                       v.coord,
                       v.fill);
   }
};


#endif
