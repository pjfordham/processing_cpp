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

   struct vertex_t {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 coord;
      color_t fill;
      int tunit;
      int mindex;
   };

   struct material_t {
      glm::vec4 ambient;
      glm::vec4 specular;
      glm::vec4 emissive;
      float shininess;
   };

   struct light_t {
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

      uniform_t LightCount;
      uniform_t LightPosition;
      uniform_t LightNormal;
      uniform_t LightAmbient;
      uniform_t LightDiffuse;
      uniform_t LightSpecular;
      uniform_t LightFalloff;
      uniform_t LightSpot;
      uniform_t PVmatrix;
      uniform_t Eye;

      int currentBlendMode = BLEND;
      bool depth_test = true;
      bool depth_mask = true;

   public:
      int blendMode( int b );
      void hint(int type);

      std::vector<light_t> lights;
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
         lights.emplace_back( light_t{{0.0f,0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushDirectionalLight( glm::vec3 color, glm::vec3 vector, glm::vec3 specular ) {
         lights.emplace_back( light_t{{0.0f,0.0f,0.0f,0.0f}, vector, {0,0,0}, color, specular, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
      }

      void pushPointLight( glm::vec3 color, glm::vec4 position, glm::vec3 specular, glm::vec3 falloff) {
         lights.emplace_back( light_t{position, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, specular, falloff, {0.0f,0.0f}} );
      }

      void pushSpotLight(  glm::vec3 color, glm::vec4 position, glm::vec3 direction,  glm::vec3 specular, glm::vec3 falloff, glm::vec2 spot) {
         lights.emplace_back( light_t{position, direction, {0.0f,0.0f,0.0f}, color, specular, falloff, spot });
      }

      void clearLights() {
         lights.clear();
      }

      void flatLight() {
         clearLights();
         pushAmbientLight(glm::vec3{1.0f,1.0f,1.0f});
      }
   };

   class VAO_t {
      GLuint vao = 0;
      GLuint indexId = 0;
      GLuint vertexId = 0;
      GLuint materialId= 0;
   public:
      friend struct fmt::formatter<gl::VAO_t>;

      std::vector<vertex_t> vertices;
      std::vector<material_t> materials;
      std::vector<unsigned short> indices;
      std::vector<texture_t_ptr> textures;
      std::vector<glm::mat4> transforms;

      VAO_t() noexcept;

      VAO_t(const VAO_t& x) noexcept;

      VAO_t(VAO_t&& x) noexcept;

      VAO_t& operator=(const VAO_t&) = delete;

      VAO_t& operator=(VAO_t&& other) noexcept;

      void bind( attribute_t Position, attribute_t Normal, attribute_t Color,
                 attribute_t Coord,    attribute_t TUnit,  attribute_t MIndex,
                 attribute_t Ambient,  attribute_t Specular, attribute_t Emissive, attribute_t Shininess);
      int hasTexture(texture_t_ptr texture);
      void loadBuffers() const;
      void draw() const;
      void debugPrint() const;
      ~VAO_t();
   };

   typedef std::shared_ptr<VAO_t>  VAO_t_ptr;

   class batch_t {
      attribute_t Position;
      attribute_t Normal;
      attribute_t Color;
      attribute_t Coord;
      attribute_t TUnit;
      attribute_t MIndex;
      uniform_t Mmatrix;
      uniform_t Nmatrix;
      uniform_t TexOffset;
      attribute_t Ambient, Specular, Emissive, Shininess;
      std::vector<VAO_t_ptr> vaos;
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
      void _load();
      void load();
      void bind();

      void setupTextures(VAO_t_ptr);
      void draw();
      void draw(const glm::mat4& transform);
      void clear();
      bool usesCircles() const;
      bool usesTextures() const;

      void vertices( const std::vector<vertex_t> &vertices,const std::vector<material_t> &materials,  const std::vector<unsigned short> &indices,
                     const glm::mat4 &transform, bool flatten_transform, std::optional<texture_t_ptr> texture, std::optional<color_t> override );
   };

   typedef std::shared_ptr<batch_t>  batch_t_ptr;

   class framebuffer_t;
   class frame_t {
      struct geometry_t {
         batch_t_ptr batch;
         scene_t scene;
         const shader_t &shader;
      };
      std::vector<geometry_t> geometries;
      color_t background_={0,0,0,1};
      bool c = false;

   public:
      void background(color_t b) {
         c = true;
         background_ = b;
      }

      void add(batch_t_ptr &b, scene_t sc, const shader_t &sh) {
         geometries.emplace_back( b, sc, sh );

      }

      void clear() {
         c = false;
         geometries.clear();
      }

      void render(framebuffer_t &fb);
   };

   void renderDirect( framebuffer_t &fb, batch_t_ptr batch, const glm::mat4 &transform, scene_t scene, const shader_t &shader );

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
struct fmt::formatter<gl::material_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::material_t& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "A{} S{} E{} Shine{}",
                            v.ambient,
                            v.specular,
                            v.emissive,
                            v.shininess);
   }
};

template <>
struct fmt::formatter<gl::vertex_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::vertex_t& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "P{} N{} M{} Tu{} Tc{} C{}",
                            v.position,
                            v.normal,
                            v.mindex,
                            v.tunit,
                            v.coord,
                            v.fill);
   }
};


#endif
