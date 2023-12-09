#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>

#include "processing_pshader.h"
#include "processing_enum.h"
#include "processing_opengl_framebuffer.h"
#include "processing_utils.h"
#include "processing_pimage.h"

#include <fmt/core.h>

template <typename T>
static void loadBufferData(GLenum target, GLint bufferId, const std::vector<T> &data, GLenum usage) {
   glBindBuffer(target, bufferId);
   glBufferData(target, data.size() * sizeof(T), data.data(), usage);
   glBindBuffer(target, 0);
}

class gl_context {
public:
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
   struct VAO {
      GLuint vao = 0;
      GLuint indexId = 0;
      GLuint vertexId = 0;
      void alloc(  PShader::Attribute Position,  PShader::Attribute Normal,  PShader::Attribute Color,
                   PShader::Attribute Coord,  PShader::Attribute TUnit,  PShader::Attribute MIndex) {
         glGenVertexArrays(1, &vao);
         glGenBuffers(1, &indexId);
         glGenBuffers(1, &vertexId);
         glBindVertexArray(vao);
         glBindVertexArray(vao);

         glBindBuffer(GL_ARRAY_BUFFER, vertexId);

         Position.bind_vec3( sizeof(vertex), (void*)offsetof(vertex,position) );
         Normal.bind_vec3( sizeof(vertex),  (void*)offsetof(vertex,normal));
         Coord.bind_vec2( sizeof(vertex), (void*)offsetof(vertex,coord));
         Color.bind_vec4( sizeof(vertex), (void*)offsetof(vertex,fill));
         TUnit.bind_int( sizeof(vertex), (void*)offsetof(vertex,tunit));
         MIndex.bind_int( sizeof(vertex), (void*)offsetof(vertex,mindex));

         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
         glBindVertexArray(0);
      }
      void loadBuffers() {
         loadBufferData(GL_ARRAY_BUFFER, vertexId, vertices, GL_STREAM_DRAW);
         loadBufferData(GL_ELEMENT_ARRAY_BUFFER, indexId, indices, GL_STREAM_DRAW);
      }
      void draw() {
         glBindVertexArray(vao);
         glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
         glBindVertexArray(0);
      }

      ~VAO() {
         if (vao) {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &vao);
            vao = 0;
            glDeleteBuffers(1, &indexId);
            glDeleteBuffers(1, &vertexId);
         }
    }
      std::vector<gl_context::vertex> vertices;
      std::vector<unsigned short> indices;
      std::vector<PImage> textures;
      std::vector<glm::mat4> transforms;
   };

   struct scene_t {
      bool lights = false;;
      glm::vec3 directionLightColor =  { 0.0, 0.0, 0.0 };
      glm::vec3 directionLightVector = { 0.0, 0.0, 0.0 };
      glm::vec3 ambientLight =         { 0.0, 0.0, 0.0 };
      std::vector<glm::vec3> pointLightColors;
      std::vector<glm::vec3> pointLightPoss;
      glm::vec3 pointLightFalloff =    { 1.0, 0.0, 0.0 };
      glm::mat4 projection_matrix;
      glm::mat4 view_matrix;
   };

private:
   int flushes = 0;

   scene_t scene;
   int width;
   int height;
   int window_width;
   int window_height;
   gl_framebuffer localFrame;

 public:

   float aaFactor;

   GLuint index_buffer_id;
   GLuint vertex_buffer_id;

   PShader defaultShader;
   PShader currentShader;

   PShader::Attribute Position, Normal, Color, Coord, TUnit, MIndex;
   PShader::Uniform AmbientLight, DirectionLightColor, DirectionLightVector, NumberOfPointLights,
      PointLightColor,PointLightPosition,PointLightFalloff, uSampler, Mmatrix, PVmatrix, TransformMatrix;

   int MaxTextureImageUnits;

public:
   MAKE_GLOBAL(getColorBufferID, localFrame);

   gl_context() : width(0), height(0) {
      index_buffer_id = 0;
      vertex_buffer_id = 0;
   }

   gl_context(int width, int height, float aaFactor);

   gl_context(const gl_context &x) = delete;

   gl_context(gl_context &&x) noexcept : gl_context() {
      *this = std::move(x);
   }

   gl_context& operator=(const gl_context&) = delete;

   gl_context& operator=(gl_context&&x) noexcept {
      std::swap(scene,x.scene);
      std::swap(flushes,x.flushes);
      std::swap(localFrame,x.localFrame);

      std::swap(width,x.width);
      std::swap(height,x.height);
      std::swap(window_width,x.window_width);
      std::swap(window_height,x.window_height);

      std::swap(aaFactor,x.aaFactor);

      std::swap(index_buffer_id,x.index_buffer_id);
      std::swap(vertex_buffer_id,x.vertex_buffer_id);

      std::swap(Position,x.Position);
      std::swap(Normal,x.Normal);
      std::swap(Color,x.Color);
      std::swap(Coord,x.Coord);
      std::swap(TUnit, x.TUnit);
      std::swap(MIndex,x.MIndex);

      std::swap(defaultShader,x.defaultShader);
      std::swap(currentShader,x.currentShader);

      std::swap(Mmatrix,x.Mmatrix);
      std::swap(PVmatrix,x.PVmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);
      std::swap(PointLightPosition,x.PointLightPosition);
      std::swap(PointLightColor,x.PointLightColor);
      std::swap(NumberOfPointLights,x.NumberOfPointLights);
      std::swap(PointLightFalloff,x.PointLightFalloff);

      std::swap(MaxTextureImageUnits,x.MaxTextureImageUnits);

      return *this;
   }

   int bindNextTextureUnit( PImage img );

   void blendMode( int b );

   float screenX(float x, float y, float z) {
      PVector in = { x, y, z };
      return (scene.projection_matrix * (scene.view_matrix * in)).x;
   }
   float screenY(float x, float y, float z) {
      PVector in = { x, y, z };
      return (scene.projection_matrix * (scene.view_matrix * in)).y;
   }

   void setScene( const scene_t &scene );

   void setProjectionMatrix( const glm::mat4 &PV ) {
      scene.projection_matrix = PV;
   }

   void setViewMatrix( const glm::mat4 &PV ) {
      scene.view_matrix = PMatrix::FlipY().glm_data() * PV ;
   }

   void setDirectionLightColor(const glm::vec3 &color ){
      scene.directionLightColor = color;
   }

   void setDirectionLightVector(const glm::vec3 &dir  ){
      scene.directionLightVector = dir;
   }

   void setAmbientLight(const glm::vec3  &color ){
      scene.ambientLight = color;
   }

   void pushPointLightColor( const glm::vec3  &color ) {
      if (scene.pointLightColors.size() < 8) {
         scene.pointLightColors.push_back( color );
      } else {
         fmt::print("Ignoring >8 point lights\n.");
      }
   }

   void pushPointLightPosition( const glm::vec3 &pos  ) {
      if (scene.pointLightColors.size() < 8) {
         scene.pointLightPoss.push_back( pos );
      }
   }

   void clearPointLights() {
      scene.pointLightColors.clear();
      scene.pointLightPoss.clear();
   }

   void setPointLightFalloff( const glm::vec3 &data){
      scene.pointLightFalloff = data;
   }

   void setLights( bool data ) {
      scene.lights = data;
   }

   ~gl_context();

   void hint(int type);

   void blit( gl_framebuffer &target ) {
      localFrame.blit( target );
   }

   void loadPixels( std::vector<unsigned int> &pixels ) {
      gl_framebuffer frame(window_width, window_height, 1, SSAA);
      localFrame.blit( frame );
      frame.loadPixels( pixels );
   };

   void updatePixels( const std::vector<unsigned int> &pixels) {
      gl_framebuffer frame(window_width, window_height, 1, SSAA);
      frame.updatePixels( pixels );
      frame.blit( localFrame );
   }

   void clear(gl_framebuffer &fb, float r, float g, float b, float a);

   void clear( float r, float g, float b, float a) {
      clear( localFrame, r, g, b, a);
   }

   void clearDepthBuffer(gl_framebuffer &fb);

   void clearDepthBuffer() {
      clearDepthBuffer( localFrame );
   }

   void shader(PShader shader, int kind = TRIANGLES) {
      if ( currentShader != shader ) {
         currentShader = shader;
         shader.useProgram();

         uSampler = shader.get_uniform("myTextures");
         uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

         Mmatrix = shader.get_uniform("Mmatrix");
         PVmatrix = shader.get_uniform( "PVmatrix");
         TransformMatrix = shader.get_uniform("transformMatrix");
         DirectionLightColor = shader.get_uniform("directionLightColor");
         DirectionLightVector = shader.get_uniform("directionLightVector");
         AmbientLight = shader.get_uniform("ambientLight");
         NumberOfPointLights = shader.get_uniform("numberOfPointLights");
         PointLightColor = shader.get_uniform("pointLightColor");
         PointLightPosition = shader.get_uniform("pointLightPosition");
         PointLightFalloff = shader.get_uniform("pointLightFalloff");

         Position = shader.get_attribute("position");
         Normal = shader.get_attribute("normal");
         Color = shader.get_attribute("color");
         Coord = shader.get_attribute("coord");
         TUnit = shader.get_attribute("tunit");
         MIndex = shader.get_attribute("mindex");
      }
      shader.set_uniforms();
   }

   void loadMoveMatrix(  const std::vector<glm::mat4> &transforms );

   void loadProjectionViewMatrix( const glm::mat4 &data );

   void flush();

   void drawVAO(std::vector<VAO> &vao, const glm::mat4 &currentTransform);
   void compile(std::vector<VAO> &vao);

   void drawTriangles( const std::vector<vertex> &vertices,
                       const std::vector<unsigned short> &indices,
                       PImage texture,
                       const glm::mat4 &move_matrix );

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

gl_context::color flatten_color_mode(color c);

template <>
struct fmt::formatter<gl_context::color> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const gl_context::color& v, FormatContext& ctx) {
       return format_to(ctx.out(), "R{:8.2f},G{:8.2f},B{:8.2f},A{:8.2f}",v.r,v.g,v.b,v.a);
    }
};

template <>
struct fmt::formatter<gl_context::vertex> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const gl_context::vertex& v, FormatContext& ctx) {
       return format_to(ctx.out(), "T{} Tu{} N{} P{} C{}",
                        v.coord,
                        v.tunit,
                        v.normal,
                        v.position,
                        v.fill);
    }
};


#endif
