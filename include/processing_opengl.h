#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <vector>
#include <optional>
#include <algorithm>
#include <fmt/core.h>

#include "processing_opengl_shader.h"
#include "processing_opengl_texture.h"
#include "processing_opengl_color.h"
#include "processing_enum.h"
#include "processing_math.h"

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
      glm::vec4 ambient;
      glm::vec4 specular;
      glm::vec4 emissive;
      float shininess;
   };

   struct light_t {
      glm::vec4 position = { 0.0, 0.0, 0.0, 0.0 };
      glm::vec3 normal = { 0.0, 0.0, 0.0 };
      glm::vec3 ambient = { 1.0, 1.0, 1.0 };
      glm::vec3 diffuse = { 0.0, 0.0, 0.0 };
      glm::vec3 specular = { 0.0, 0.0, 0.0 };
      glm::vec3 falloff = { 1.0, 0.0, 0.0 };
      glm::vec2 spot = { 0.0, 0.0 };
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

      std::vector<light_t> lights;

   public:
      int blendMode( int b );
      void hint(int type);

      scene_t();
      void setup( const shader_t &shader);
      float screenX(float x, float y, float z) const;
      float screenY(float x, float y, float z) const;
      void set();
      void setProjectionMatrix( const glm::mat4 &PV );
      void setViewMatrix( const glm::mat4 &PV );
      void pushAmbientLight( glm::vec3 color );
      void pushDirectionalLight( glm::vec3 color, glm::vec3 vector, glm::vec3 specular );
      void pushPointLight( glm::vec3 color, glm::vec4 position, glm::vec3 specular, glm::vec3 falloff);
      void pushSpotLight(  glm::vec3 color, glm::vec4 position, glm::vec3 direction,  glm::vec3 specular, glm::vec3 falloff, glm::vec2 spot);
      void clearLights();
      void flatLight();
      bool anyLights() const;
   };

   class VAO_t;

   struct range_t {
      int vao, start, size;
   };

   class batch_t : public std::enable_shared_from_this<batch_t> {
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
      std::vector<std::unique_ptr<VAO_t>> vaos;
      bool uses_textures = false;
      bool uses_circles = false;
      std::vector<range_t> vertices_to_reload;
      std::vector<range_t> indices_to_reload;

   public:
      batch_t() noexcept;

      batch_t(const batch_t& x) noexcept = delete;
      batch_t& operator=(const batch_t&) = delete;

      batch_t(batch_t&& x) noexcept;
      batch_t& operator=(batch_t&& other) noexcept;

      ~batch_t();

      class sub_batch_t {
         int vao;
         int vertex;
         int index;
         int vertex_count;
         int index_count;
         std::vector<vertex_t> *_vertices;
         std::vector<unsigned short> *_indices;
         int trID;
         int txID;
         const glm::mat4 *transform;
         int reservation;
         batch_t *batch_ptr; // maybe smart pointer?

         public:
         sub_batch_t(batch_t &batch, int reservation, const glm::mat4 &transform_, bool flatten_transforms, std::optional<texture_t_ptr> texture_);
         ~sub_batch_t() {
            if (reservation != vertex_count) {
               fmt::print("Reservation {} and vertext_count was {}\n", reservation, vertex_count);
            }
         }

         range_t getRange() const {
            return { vao, vertex, vertex_count };
         }

         void upload(batch_t &batch) const ;

         void setTransform( const glm::mat4 &transform ) {
            batch_ptr->set_transform( transform, vao, trID );
         }

         void setTexture( texture_t_ptr texture ) {
            batch_ptr->set_texture( texture, vao, trID );
         }

         int getVAO() const {
            return vao;
         }

         int getVertexCount() const {
            return vertex_count;
         }

         int getIndexCount() const {
            return index_count;
         }

         vertex_t &verticesByIndex(int i) {
            return (*_vertices)[ (*_indices)[index + i] ];
         }

         vertex_t &vertices(int i) {
            return (*_vertices)[ vertex + i ];
         }

         const vertex_t *vertices_data() const {
            return (*_vertices).data() + vertex;
         }

         const unsigned short *indices_data() const {
            return (*_indices).data() + index;
         }

         void update_stroke_vertex( int i, const glm::vec3 &position, const color_t &fill ) {
            return update_vertex(i, position, {0,0,0}, {0,0}, fill, {}, {}, fill, {});
         }

         int add_stroke_vertex( const glm::vec3 &position, const color_t &fill ) {
            return add_vertex(position, {0,0,0}, {0,0}, fill, {}, {}, fill, {});
         }

         void update_vertex( int i,
                             const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2& coord,
                             const color_t &fill, const glm::vec4 &ambient, const glm::vec4 &specular,
                             const glm::vec4 &emissive, float shininess ) {
            vertices(i) = {
               position,
               normal,
               coord, fill, txID, trID,
               ambient, specular, emissive, shininess
            };
         }

         int add_vertex( const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2& coord,
                         const color_t &fill, const glm::vec4 &ambient, const glm::vec4 &specular,
                         const glm::vec4 &emissive, float shininess ) {
            (*_vertices).emplace_back( position,
                                       normal,
                                       coord, fill, txID, trID,
                                       ambient, specular, emissive, shininess);
            return vertex_count++;
         }

         unsigned short indices(int i) {
            return (*_indices)[ index + i ] - vertex;
         }

         void add_index(int i) {
            (*_indices).push_back( vertex + i );
            index_count++;
         }

         void drop() {
            (*_indices).erase( (*_indices).end() - index_count, (*_indices).end());
            (*_vertices).erase( (*_vertices).end() - vertex_count, (*_vertices).end());
            index_count = 0;
            vertex_count = 0;
            reservation = 0; // Supresss warning message about wrong reservation.
         }
      };

      void setup( const shader_t &shader );
      size_t size();
      void _load();
      void load();
      void reload();
      void reload(const sub_batch_t &s);
      void bind();
      void setupTextures(VAO_t&);
      void draw();
      void draw(const glm::mat4& transform);
      void clear();
      bool usesCircles() const;
      bool usesTextures() const;

      void reserve(int count);
      void add_transform( const glm::mat4 &transform_);
      void set_transform( const glm::mat4 &transform_ , int existing_vao, int existing_trID);
      void add_texture( texture_t_ptr texture_ );
      void set_texture( texture_t_ptr texture_, int existing_vao, int existing_txID);
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
      color_t background_ = { 0.0F, 0.0F, 0.0F, 1.0F };
      bool c = false;

   public:
      void background(color_t b);
      void add(batch_t_ptr b, scene_t sc, const shader_t &sh);
      void clear();
      void render(framebuffer_t &fb);
   };

   void renderDirect( framebuffer_t &fb, batch_t_ptr batch, const glm::mat4 &transform, scene_t scene, const shader_t &shader );

} // namespace gl

template <>
struct fmt::formatter<gl::vertex_t> {
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::vertex_t& v, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "P{} N{} M{} Tu{} Tc{} C{} A{} S{} E{} Shine{}",
                            v.position,
                            v.normal,
                            v.mindex,
                            v.tunit,
                            v.coord,
                            v.fill,
                            v.ambient,
                            v.specular,
                            v.emissive,
                            v.shininess);
   }
};


#endif
