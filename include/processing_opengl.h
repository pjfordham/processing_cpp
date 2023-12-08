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
   struct geometry_t {
      static const int CAPACITY = 65536;
      std::vector<vertex> vbuffer;
      std::vector<unsigned short> ibuffer;
      std::vector<glm::mat4> move;
   };

   struct batch_t {
   private:
      int currentM;
   public:
      int unit;
      scene_t scene;
      geometry_t geometry;
      struct PImageHash {
         std::size_t operator()(const PImage& obj) const {
            std::size_t hash = std::hash<int>()(obj.width);
            hash ^= std::hash<int>()(obj.height) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<void*>()(obj.pixels);
            return hash;
         }
      };
      std::unordered_map<PImage, int, PImageHash> textures;

      batch_t() {
      }

      batch_t(const batch_t &x) = delete;

      batch_t(batch_t &&x) noexcept : batch_t() {
         *this = std::move(x);
      }

      bool enqueue( gl_context *glc,
                    const std::vector<vertex> &vertices,
                    const std::vector<unsigned short> &indices,
                    PImage texture,
                    const glm::mat4 &move_matrix ) {

         if (vertices.size() > geometry.CAPACITY) {
            abort();
         }

         if (unit == glc->MaxTextureImageUnits ) {
            return false;
         }

         if ((vertices.size() + geometry.vbuffer.size()) > geometry.CAPACITY) {
            return false;
         }

         if ( geometry.move.size() == 0 || !(move_matrix == geometry.move.back()) ) {
            if (geometry.move.size() == 16) {
               return false;
            }
            currentM = geometry.move.size();
            geometry.move.push_back(move_matrix) ;
         }

         int tunit;
         if (texture == PImage::circle()) {
            tunit = -1;
         } else {
            tunit = glc->bindNextTextureUnit( texture );
         }

         int offset = geometry.vbuffer.size();
         for (int i = 0; i < vertices.size(); ++i) {
            geometry.vbuffer.push_back( vertices[i] );
            geometry.vbuffer.back().tunit = tunit;
            geometry.vbuffer.back().mindex = currentM;
         }

         for (auto index : indices) {
            geometry.ibuffer.push_back( offset + index );
         }

         return true;
      }

      static const int INTERNAL_TEXTURE_UNIT = 0;
      void reset() {
         geometry.vbuffer.clear();
         geometry.ibuffer.clear();
         geometry.move.clear();
         currentM = 0;
         unit = INTERNAL_TEXTURE_UNIT + 1;
         textures.clear();
     }

      void draw( gl_context *glc ) {
         if (geometry.vbuffer.size() != 0 ) {
            glc->loadMoveMatrix( geometry.move );
            glc->setScene( scene );
            glc->TransformMatrix.set(scene.projection_matrix * scene.view_matrix * geometry.move[0]);

            glc->drawGeometry( geometry );
         }
         reset();
      }

      batch_t& operator=(const batch_t&) = delete;

      batch_t& operator=(batch_t&&x) noexcept {
         std::swap(currentM,x.currentM);
         std::swap(scene,x.scene);
         std::swap(geometry,x.geometry);
         std::swap(textures,x.textures);
         std::swap(unit,x.unit);

         return *this;
      }

      ~batch_t(){
      }

   };

private:
   int flushes = 0;

   int width;
   int height;
   int window_width;
   int window_height;
   gl_framebuffer localFrame;

   batch_t batch;

   float aaFactor;

   GLuint index_buffer_id;
   GLuint vertex_buffer_id;

   PShader defaultShader;
   PShader currentShader;

   PShader::Attribute Position, Normal, Color, Coord, TUnit, MIndex;
   PShader::Uniform AmbientLight, DirectionLightColor, DirectionLightVector, NumberOfPointLights,
      PointLightColor,PointLightPosition,PointLightFalloff, uSampler, Mmatrix, PVmatrix, TransformMatrix;

   GLuint VAO;

   int MaxTextureImageUnits;

public:
   MAKE_GLOBAL(getColorBufferID, localFrame);

   gl_context() : width(0), height(0), batch() {
      index_buffer_id = 0;
      vertex_buffer_id = 0;
      VAO = 0;
   }

   gl_context(int width, int height, float aaFactor);

   gl_context(const gl_context &x) = delete;

   gl_context(gl_context &&x) noexcept : gl_context() {
      *this = std::move(x);
   }

   gl_context& operator=(const gl_context&) = delete;

   gl_context& operator=(gl_context&&x) noexcept {
      std::swap(batch,x.batch);

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

      std::swap(VAO,x.VAO);

      std::swap(MaxTextureImageUnits,x.MaxTextureImageUnits);

      return *this;
   }

   int bindNextTextureUnit( PImage img );

   void blendMode( int b );

   float screenX(float x, float y, float z) {
      PVector in = { x, y, z };
      return (batch.scene.projection_matrix * (batch.scene.view_matrix * in)).x;
   }
   float screenY(float x, float y, float z) {
      PVector in = { x, y, z };
      return (batch.scene.projection_matrix * (batch.scene.view_matrix * in)).y;
   }

   void drawGeometry( const geometry_t &geometry );

   void setScene( const scene_t &scene );

   void setProjectionMatrix( const glm::mat4 &PV ) {
      batch.scene.projection_matrix = PV;
   }

   void setViewMatrix( const glm::mat4 &PV ) {
      batch.scene.view_matrix = PMatrix::FlipY().glm_data() * PV ;
   }

   void setDirectionLightColor(const glm::vec3 &color ){
      batch.scene.directionLightColor = color;
   }

   void setDirectionLightVector(const glm::vec3 &dir  ){
      batch.scene.directionLightVector = dir;
   }

   void setAmbientLight(const glm::vec3  &color ){
      batch.scene.ambientLight = color;
   }

   void pushPointLightColor( const glm::vec3  &color ) {
      if (batch.scene.pointLightColors.size() < 8) {
         batch.scene.pointLightColors.push_back( color );
      } else {
         fmt::print("Ignoring >8 point lights\n.");
      }
   }

   void pushPointLightPosition( const glm::vec3 &pos  ) {
      if (batch.scene.pointLightColors.size() < 8) {
         batch.scene.pointLightPoss.push_back( pos );
      }
   }

   void clearPointLights() {
      batch.scene.pointLightColors.clear();
      batch.scene.pointLightPoss.clear();
   }

   void setPointLightFalloff( const glm::vec3 &data){
      batch.scene.pointLightFalloff = data;
   }

   void setLights( bool data ) {
      batch.scene.lights = data;
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
         cleanupVAO();
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

         initVAO();
      }
      shader.set_uniforms();
   }

   void loadMoveMatrix(  const std::vector<glm::mat4> &transforms );

   void loadProjectionViewMatrix( const glm::mat4 &data );

   void flush();

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

   void initVAO();
   void cleanupVAO();
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
