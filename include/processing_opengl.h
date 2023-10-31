#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <memory>
#include <vector>
#include "processing_math.h"
#include "processing_color.h"
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
      PVector position;
      PVector normal;
      PVector2 coord;
      int tunit;
      color fill;
   };
   struct scene_t {
      bool lights = false;;
      std::array<float,3> directionLightColor =  { 0.0, 0.0, 0.0 };
      std::array<float,3> directionLightVector = { 0.0, 0.0, 0.0 };
      std::array<float,3> ambientLight =         { 0.0, 0.0, 0.0 };
      std::array<float,3> pointLightColor =      { 0.0, 0.0, 0.0 };
      std::array<float,3> pointLightPosition =   { 0.0, 0.0, 0.0 };
      std::array<float,3> pointLightFalloff =    { 1.0, 0.0, 0.0 };
      PMatrix projection_matrix = PMatrix::Identity();
      PMatrix view_matrix = PMatrix::Identity();
   };
   struct geometry_t {
      static const int CAPACITY = 65536;
      std::array<vertex, CAPACITY> vbuffer;
      std::array<int, CAPACITY> tbuffer;
      unsigned int vCount = 0;
      std::array<unsigned short, CAPACITY> ibuffer;
      unsigned int iCount = 0;
      std::array<PMatrix,16> move;
      unsigned int mCount = 0;
   };

   struct batch_t {
   private:
      int width, height;
      int currentM;
   public:
      int unit;
      scene_t scene;
      std::unique_ptr<geometry_t> geometry;
      struct PImageHash {
         std::size_t operator()(const PImage& obj) const {
            std::size_t hash = std::hash<int>()(obj.width);
            hash ^= std::hash<int>()(obj.height) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<void*>()(obj.pixels);
            return hash;
         }
      };
      std::unordered_map<PImage, int, PImageHash> textures;

      batch_t(int width, int height) :
         geometry( std::make_unique<geometry_t>() ) {

         this->width = width;
         this->height = height;
      }

      batch_t(const batch_t &x) = delete;

      batch_t(batch_t &&x) noexcept : batch_t(x.width,x.height) {
         *this = std::move(x);
      }

      bool enqueue( gl_context *glc,
                    const std::vector<vertex> &vertices,
                    const std::vector<unsigned short> &indices,
                    PImage texture,
                    const PMatrix &move_matrix ) {

         if (vertices.size() > geometry->CAPACITY) {
            abort();
         }

         if (unit == glc->MaxTextureImageUnits ) {
            return false;
         }

         if ((vertices.size() + geometry->vCount) > geometry->CAPACITY) {
            return false;
         }

         if ((indices.size() + geometry->iCount) > geometry->CAPACITY) {
            return false;
         }

         if ( geometry->mCount == 0 || !(move_matrix == geometry->move[geometry->mCount - 1]) ) {
            if (geometry->mCount == geometry->move.size()) {
               return false;
            }
            geometry->move[geometry->mCount] = move_matrix;
            currentM = geometry->mCount;
            geometry->mCount++;
         }

         int tunit;
         if (texture == PImage::circle()) {
            tunit = -1;
         } else {
            tunit = glc->bindNextTextureUnit( texture );
         }

         int offset = geometry->vCount;
         for (int i = 0; i < vertices.size(); ++i) {
            geometry->vbuffer[offset + i] = vertices[i];
            geometry->vbuffer[offset + i].tunit = tunit;
            geometry->tbuffer[offset + i] = currentM;
         }
         geometry->vCount += vertices.size();

         int i_offset = geometry->iCount;
         for (auto index : indices) {
            geometry->ibuffer[i_offset++] = offset + index;
         }
         geometry->iCount += indices.size();

         return true;
      }

      static const int INTERNAL_TEXTURE_UNIT = 0;
      void reset() {
         geometry->vCount = 0;
         geometry->mCount = 0;
         geometry->iCount = 0;
         currentM = 0;
         unit = INTERNAL_TEXTURE_UNIT + 1;
         textures.clear();
     }

      void draw( gl_context *glc ) {
         if (geometry->vCount != 0 ) {
            glc->loadMoveMatrix( geometry->move, geometry->mCount );
            glc->setScene( scene );
            glc->drawGeometry( *geometry );
         }
         reset();
      }

      batch_t& operator=(const batch_t&) = delete;

      batch_t& operator=(batch_t&&x) noexcept {
         std::swap(currentM,x.currentM);
         std::swap(width,x.width);
         std::swap(height,x.height);
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
   GLuint tindex_buffer_id;

   GLuint vertex_attrib_id;
   GLuint coords_attrib_id;
   GLuint colors_attrib_id;
   GLuint tindex_attrib_id;
   GLuint tunit_attrib_id;
   GLuint normal_attrib_id;

   PShader defaultShader;
   GLuint Mmatrix;
   GLuint PVmatrix;
   GLuint uSampler;
   GLuint AmbientLight;
   GLuint DirectionLightColor;
   GLuint DirectionLightVector;
   GLuint PointLightColor;
   GLuint PointLightPosition;
   GLuint PointLightFalloff;

   GLuint VAO;

   int MaxTextureImageUnits;

public:
   MAKE_GLOBAL(getColorBufferID, localFrame);

   gl_context() : width(0), height(0), batch(0,0) {
      index_buffer_id = 0;
      vertex_buffer_id = 0;
      tindex_buffer_id = 0;
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
      std::swap(tindex_buffer_id,x.tindex_buffer_id);
      std::swap(vertex_attrib_id,x.vertex_attrib_id);
      std::swap(coords_attrib_id,x.coords_attrib_id);
      std::swap(colors_attrib_id,x.colors_attrib_id);
      std::swap(tindex_attrib_id,x.tindex_attrib_id);
      std::swap(tunit_attrib_id,x.tunit_attrib_id);
      std::swap(normal_attrib_id,x.normal_attrib_id);
      std::swap(defaultShader,x.defaultShader);
      std::swap(Mmatrix,x.Mmatrix);
      std::swap(PVmatrix,x.PVmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);
      std::swap(PointLightPosition,x.PointLightPosition);
      std::swap(PointLightColor,x.PointLightColor);
      std::swap(PointLightFalloff,x.PointLightFalloff);

      std::swap(VAO,x.VAO);

      std::swap(MaxTextureImageUnits,x.MaxTextureImageUnits);

      return *this;
   }

   int bindNextTextureUnit( PImage img );

   void blendMode( int b );

   float screenX(float x, float y, float z) {
      PVector4 in = { x, y, z, 1.0 };
      return (batch.scene.projection_matrix * (batch.scene.view_matrix * in)).data[0];
   }
   float screenY(float x, float y, float z) {
      PVector4 in = { x, y, z, 1.0 };
      return (batch.scene.projection_matrix * (batch.scene.view_matrix * in)).data[1];
   }

   void drawGeometry( const geometry_t &geometry );

   void setScene( const scene_t &scene );

   void setProjectionMatrix( const PMatrix &PV ) {
      batch.scene.projection_matrix = PV;
   }

   void setViewMatrix( const PMatrix &PV ) {
      batch.scene.view_matrix = PMatrix::FlipY() * PV ;
   }

   void setDirectionLightColor(const std::array<float,3>  &color ){
      batch.scene.directionLightColor = color;
   }

   void setDirectionLightVector(const std::array<float,3>  &dir  ){
      batch.scene.directionLightVector = dir;
   }

   void setAmbientLight(const std::array<float,3>  &color ){
      batch.scene.ambientLight = color;
   }

   void setPointLightColor(const std::array<float,3>  &color){
      batch.scene.pointLightColor = color;
   }

   void setPointLightPosition( const std::array<float,3>  &pos ){
      batch.scene.pointLightPosition = pos;
   }

   void setPointLightFalloff( const std::array<float,3>  &data){
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

   void shader(PShader &shader, int kind = TRIANGLES) {
      vertex_attrib_id = shader.getAttribLocation("position");
      normal_attrib_id = shader.getAttribLocation("normal");
      coords_attrib_id = shader.getAttribLocation("coords");
      colors_attrib_id = shader.getAttribLocation("colors");
      tindex_attrib_id = shader.getAttribLocation("mindex");
      tunit_attrib_id = shader.getAttribLocation("tunit");
      Mmatrix = shader.getUniformLocation("Mmatrix");
      PVmatrix = shader.getUniformLocation("PVmatrix");
      AmbientLight = shader.getUniformLocation("ambientLight");
      DirectionLightColor = shader.getUniformLocation("directionLightColor");
      DirectionLightVector = shader.getUniformLocation("directionLightVector");
      PointLightColor = shader.getUniformLocation("pointLightColor");
      PointLightPosition = shader.getUniformLocation("pointLightPosition");
      PointLightFalloff = shader.getUniformLocation("pointLightFalloff");
      uSampler = shader.getUniformLocation("myTextures");
      shader.useProgram();
      shader.set_uniforms();
   }

   void loadMoveMatrix(  const std::array<PMatrix,16> &transforms, int mCount );

   void loadProjectionViewMatrix( const float *data );

   void flush();

   void drawTriangles( const std::vector<vertex> &vertices,
                       const std::vector<unsigned short> &indices,
                       PImage texture,
                       const PMatrix &move_matrix );

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
