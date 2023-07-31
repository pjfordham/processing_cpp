#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include "processing_math.h"
#include "processing_pshader.h"
#include "processing_texture_manager.h"
#include "processing_enum.h"
#include "processing_opengl_framebuffer.h"

#include <fmt/core.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;


class gl_context {

   SDL_Window *window = NULL;
   SDL_Renderer *renderer =NULL;
   void *glContext = NULL;

   int flushes = 0;

   int width;
   int height;
   int window_width;
   int window_height;
   gl_framebuffer localFrame;

public:
   struct color {
      float r,g,b,a;
      void print() {
         fmt::print("R{} G{} B{} A{}\n",r,g,b,a);
      }
   };
   struct vertex {
      PVector position;
      PVector normal;
      PVector coord;
      color fill;
   };
   struct scene_t {
      bool lights = false;;
      std::array<float,3> directionLightColor =  { 0.0, 0.0, 0.0 };
      std::array<float,3> directionLightVector = { 0.0, 0.0, 0.0 };
      std::array<float,3> ambientLight =         { 1.0, 1.0, 1.0 };
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

   struct batch {
   private:
      int width, height;
      int currentM;
   public:
      GLuint bufferID;
      TextureManager tm;
      gl_context *glc;

      scene_t scene;
      geometry_t geometry;

      batch(int width, int height, gl_context *glc) : tm(width * 3, height * 3) {
         this->width = width;
         this->height = height;
         this->glc = glc;
         bufferID = 0;

         // create the texture array
         glGenTextures(1, &bufferID);
         glBindTexture(GL_TEXTURE_2D, bufferID);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width * 3, this->height * 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

         // set texture parameters
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

         // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
         GLubyte white[4] = { 255, 255, 255, 255 };
         glClearTexImage(bufferID, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
      }

      batch(const batch &x) = delete;

      batch(batch &&x) noexcept : batch(x.width,x.height, x.glc) {

         *this = std::move(x);
      }

      bool enqueue( const std::vector<vertex> &vertices,
                    const std::vector<unsigned short> &indices,
                    const PMatrix &move_matrix ) {
         if (vertices.size() > geometry.CAPACITY) {
            abort();
         } else {
            int new_size = vertices.size() + geometry.vCount;
            if (new_size >= geometry.CAPACITY) {
               return false;
            }
         }
         if ( geometry.mCount == geometry.move.size()) {
            return false;
         }
         if ( geometry.mCount > 0 && move_matrix == geometry.move.back() ) {
         } else {
            geometry.move[geometry.mCount] = move_matrix;
            currentM = geometry.mCount;
            geometry.mCount++;;
         }

         int offset = geometry.vCount;

         for (int i = 0; i < vertices.size(); ++i) {
            geometry.vbuffer[offset + i] = vertices[i];
            geometry.tbuffer[offset + i] = currentM;
         }
         geometry.vCount += vertices.size();

         int i_offset = geometry.iCount;
         for (auto index : indices) {
            geometry.ibuffer[i_offset++] = offset + index;
         }
         geometry.iCount += indices.size();

         return true;
      }

      void draw( gl_framebuffer &fb ) {

         if (geometry.vCount != 0 ) {
            glc->loadMoveMatrix( geometry.move, geometry.mCount );
            glc->setScene( scene );
            glc->drawGeometry( geometry, fb, bufferID );
         }
         geometry.vCount = 0;
         geometry.mCount = 0;
         geometry.iCount = 0;

         tm.clear();
         currentM = 0;
      }

      batch& operator=(const batch&) = delete;

      batch& operator=(batch&&x) noexcept {

         std::swap(currentM,x.currentM);

         std::swap(width,x.width);
         std::swap(height,x.height);
         std::swap(tm,x.tm);

         std::swap(bufferID,x.bufferID);

         std::swap(scene,x.scene);
         std::swap(geometry,x.geometry);

         std::swap(currentM,x.currentM);

         return *this;
      }

      ~batch(){
         if (bufferID)
            glDeleteTextures(1, &bufferID);
      }

   };

private:


   std::vector<batch> batches;

   gl_framebuffer windowFrame;

   float aaFactor;

   GLuint index_buffer_id;
   GLuint vertex_buffer_id;
   GLuint tindex_buffer_id;
   GLuint vertex_attrib_id;
   GLuint coords_attrib_id;
   GLuint colors_attrib_id;
   GLuint tindex_attrib_id;
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

public:

public:
   gl_context() : width(0), height(0) {
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
      std::swap(window, x.window);
      std::swap(renderer, x.renderer);
      std::swap(glContext, x.glContext);

      std::swap(batches,x.batches);
      if (batches.size() > 0)
         batches.back().glc = this;

      std::swap(flushes,x.flushes);
      std::swap(localFrame,x.localFrame);

      std::swap(width,x.width);
      std::swap(height,x.height);
      std::swap(window_width,x.window_width);
      std::swap(window_height,x.window_height);

      std::swap(windowFrame,x.windowFrame);

      std::swap(aaFactor,x.aaFactor);

      std::swap(index_buffer_id,x.index_buffer_id);
      std::swap(vertex_buffer_id,x.vertex_buffer_id);
      std::swap(tindex_buffer_id,x.tindex_buffer_id);
      std::swap(vertex_attrib_id,x.vertex_attrib_id);
      std::swap(coords_attrib_id,x.coords_attrib_id);
      std::swap(colors_attrib_id,x.colors_attrib_id);
      std::swap(tindex_attrib_id,x.tindex_attrib_id);
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

      return *this;
   }


   void drawGeometry( const geometry_t &geometry, gl_framebuffer &fb, GLuint bufferID ) {
      glBindVertexArray(VAO);

      glBindTexture(GL_TEXTURE_2D, bufferID);
      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
      glBufferSubData(GL_ARRAY_BUFFER, 0, geometry.vCount * sizeof(vertex), geometry.vbuffer.data() );

      glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);
      glBufferData(GL_ARRAY_BUFFER, geometry.vCount * sizeof(int), geometry.tbuffer.data(), GL_STREAM_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.iCount * sizeof(unsigned short), geometry.ibuffer.data(), GL_STREAM_DRAW);

      fb.bind();

      #if 0
      fmt::print("### GEOMETRY DUMP START ###\n");
      for ( int i = 0; i < geometry.vCount; ++i ) {
         fmt::print("{}: ", i);
         geometry.vbuffer[i].position.print();
      }
      for ( int i = 0; i < geometry.iCount; ++i ) {
         fmt::print("{}",  geometry.ibuffer[i]);
      }
      fmt::print("\n### GEOMETRY DUMP END   ###\n");
      #endif

      glDrawElements(GL_TRIANGLES, geometry.iCount, GL_UNSIGNED_SHORT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindVertexArray(0);
   }

   void setScene( const scene_t &scene ) {
      loadProjectionViewMatrix( (scene.projection_matrix * scene.view_matrix).data() );

      if (scene.lights) {
         glUniform3fv(DirectionLightColor,  1, scene.directionLightColor.data() );
         glUniform3fv(DirectionLightVector, 1, scene.directionLightVector.data() );
         glUniform3fv(AmbientLight,         1, scene.ambientLight.data());
         glUniform3fv(PointLightColor,      1, scene.pointLightColor.data() );
         glUniform3fv(PointLightPosition,   1, scene.pointLightPosition.data()  );
         glUniform3fv(PointLightFalloff,    1, scene.pointLightFalloff.data() );
      } else {
         std::array<float,3> on { 1.0, 1.0, 1.0};
         std::array<float,3> off { 0.0, 0.0, 0.0};
         std::array<float,3> unity { 1.0, 0.0, 0.0 };

         glUniform3fv(DirectionLightColor,  1, off.data() );
         glUniform3fv(AmbientLight,         1, on.data());
         glUniform3fv(PointLightColor,      1, off.data());
         glUniform3fv(PointLightFalloff,    1, unity.data() );
      }
   }

   void setProjectionMatrix( const PMatrix &PV ) {
      if(batches.size() > 0)
         batches.back().scene.projection_matrix = PV;
   }

   void setViewMatrix( const PMatrix &PV ) {
      if(batches.size() > 0)
         batches.back().scene.view_matrix = PMatrix::FlipY() * PV ;
   }

   void setDirectionLightColor(const std::array<float,3>  &color ){
      if(batches.size() > 0)
         batches.back().scene.directionLightColor = color;
   }

   void setDirectionLightVector(const std::array<float,3>  &dir  ){
      if(batches.size() > 0)
         batches.back().scene.directionLightVector = dir;
   }

   void setAmbientLight(const std::array<float,3>  &color ){
      if(batches.size() > 0)
         batches.back().scene.ambientLight = color;
   }

   void setPointLightColor(const std::array<float,3>  &color){
      if(batches.size() > 0)
         batches.back().scene.pointLightColor = color;
   }

   void setPointLightPosition( const std::array<float,3>  &pos ){
      if(batches.size() > 0)
         batches.back().scene.pointLightPosition = pos;
   }

   void setPointLightFalloff( const std::array<float,3>  &data){
      if(batches.size() > 0)
         batches.back().scene.pointLightFalloff = data;
   }

   void setLights( bool data ) {
      if(batches.size() > 0)
         batches.back().scene.lights = data;
   }

   ~gl_context();

   void hint(int type);

   void saveFrame(const std::string& fileName);

   void loadPixels( std::vector<unsigned int> &pixels );

   // Draw our pixels into a new texture and call drawTexturedQuad over whole screen
   void updatePixels( std::vector<unsigned int> &pixels) {
      localFrame.updatePixels(pixels, window_width, window_height);
   }

   PTexture getTexture( int width, int height, void *pixels );

   PTexture getTexture( SDL_Surface *surface );

   PTexture getTexture( gl_context &source );

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

   void flush() {
      flushes++;
      if(batches.size() > 0)
         batches.back().draw( localFrame );
      return;
   }

   void drawTriangles( const std::vector<vertex> &vertices,
                       const std::vector<unsigned short> &indices,
                       const PMatrix &move_matrix );

   void drawTrianglesDirect( gl_framebuffer &fb );

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

   void draw_main();
   void initVAO();
   void cleanupVAO();
};


#endif
