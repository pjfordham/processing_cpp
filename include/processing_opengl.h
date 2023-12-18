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
#include "processing_pshader.h"

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
      attribute(PShader pshader, const std::string &attribute);
      void bind_vec2(std::size_t stride, void *offset);
      void bind_vec3(std::size_t stride, void *offset);
      void bind_vec4(std::size_t stride, void *offset);
      void bind_int(std::size_t stride, void *offset);
   };

   class uniform {
      GLint id = -1;
   public:
      uniform() {}
      uniform(PShader pshader, const std::string &uniform);
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
      void compile();
      void draw();
      void clear();
   };

   class context {
      int flushes = 0;
      scene_t scene;
      int width;
      int height;
      int window_width;
      int window_height;
      framebuffer localFrame;
      batch_t batch;

   public:

      float aaFactor;

      PShader defaultShader;
      PShader currentShader;

      uniform AmbientLight, DirectionLightColor, DirectionLightVector, NumberOfPointLights,
         PointLightColor,PointLightPosition,PointLightFalloff, uSampler, TransformMatrix;

      int MaxTextureImageUnits;

   public:
      MAKE_GLOBAL(getColorBufferID, localFrame);

      context() : width(0), height(0) {}

      context(int width, int height, float aaFactor);

      context(const context &x) = delete;

      context(context &&x) noexcept : context() {
         *this = std::move(x);
      }

      context& operator=(const context&) = delete;

      context& operator=(context&&x) noexcept {
         std::swap(scene,x.scene);
         std::swap(batch,x.batch);
         std::swap(flushes,x.flushes);
         std::swap(localFrame,x.localFrame);

         std::swap(width,x.width);
         std::swap(height,x.height);
         std::swap(window_width,x.window_width);
         std::swap(window_height,x.window_height);

         std::swap(aaFactor,x.aaFactor);

         std::swap(defaultShader,x.defaultShader);
         std::swap(currentShader,x.currentShader);

         std::swap(MaxTextureImageUnits,x.MaxTextureImageUnits);

         return *this;
      }

      void blendMode( int b );

      float screenX(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (scene.projection_matrix * (scene.view_matrix * in)).x;
      }

      float screenY(float x, float y, float z) {
         glm::vec3 in = { x, y, z };
         return (scene.projection_matrix * (scene.view_matrix * in)).y;
      }


      void setProjectionMatrix( const glm::mat4 &PV ) {
         flush();
         scene.projection_matrix = PV;
      }

      void setViewMatrix( const glm::mat4 &PV ) {
         flush();
         scene.view_matrix = PMatrix::FlipY().glm_data() * PV ;
      }

      void setDirectionLightColor(const glm::vec3 &color ){
         flush();
         scene.directionLightColor = color;
      }

      void setDirectionLightVector(const glm::vec3 &dir  ){
         flush();
         scene.directionLightVector = dir;
      }

      void setAmbientLight(const glm::vec3  &color ){
         flush();
         scene.ambientLight = color;
      }

      void pushPointLightColor( const glm::vec3  &color ) {
         if (scene.pointLightColors.size() < 8) {
            flush();
            scene.pointLightColors.push_back( color );
         } else {
            fmt::print("Ignoring >8 point lights\n.");
         }
      }

      void pushPointLightPosition( const glm::vec3 &pos  ) {
         if (scene.pointLightColors.size() < 8) {
            flush();
            scene.pointLightPoss.push_back( pos );
         }
      }

      void clearPointLights() {
         flush();
         scene.pointLightColors.clear();
         scene.pointLightPoss.clear();
      }

      void setPointLightFalloff( const glm::vec3 &data){
         flush();
         scene.pointLightFalloff = data;
      }

      void setLights( bool data ) {
         flush();
         scene.lights = data;
      }

      ~context();

      void hint(int type);

      void blit( framebuffer &target ) {
         flush();
         localFrame.blit( target );
      }

      void loadPixels( std::vector<unsigned int> &pixels ) {
         flush();
         framebuffer frame(window_width, window_height, 1, SSAA);
         localFrame.blit( frame );
         frame.loadPixels( pixels );
      };

      void updatePixels( const std::vector<unsigned int> &pixels) {
         flush();
         framebuffer frame(window_width, window_height, 1, SSAA);
         frame.updatePixels( pixels );
         frame.blit( localFrame );
      }

      void clear(framebuffer &fb, float r, float g, float b, float a);

      void clear( float r, float g, float b, float a) {
         clear( localFrame, r, g, b, a);
      }

      void clearDepthBuffer(framebuffer &fb);

      void clearDepthBuffer() {
         clearDepthBuffer( localFrame );
      }

      void shader(PShader shader, int kind = TRIANGLES) {
         flush();
         if ( currentShader != shader ) {
            currentShader = shader;
            shader.useProgram();

            uSampler = shader.get_uniform("myTextures");
            uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

            TransformMatrix = shader.get_uniform("transformMatrix");

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
         shader.set_uniforms();
      }

      void draw(PShape shape, const glm::mat4 &transform);

      void flush();

      int getFlushCount() const {
         return flushes;
      }

      void resetFlushCount() {
         flushes = 0;
      }

      PShader loadShader(const char *fragShader, const char *vertShader);

      PShader loadShader(const char *fragShader);

      PShader loadShader() {
         auto shader = PShader( 0 );
         shader.compileShaders();
         return shader;
      }

      void resetShader(int kind = TRIANGLES) {
         shader( defaultShader );
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
