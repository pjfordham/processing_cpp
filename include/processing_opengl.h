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
#include <fmt/core.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;

class gl_framebuffer {
   GLuint id = 0;
   int width = 0;
   int height = 0;
   GLuint depthBufferID = 0;
   GLuint colorBufferID = 0;
public:

   auto getWidth() const {
      return width;
   }

   auto getHeight() const {
      return height;
   }

   bool isMainFrame() const {
      return id == 0;
   }

   static gl_framebuffer constructMainFrame(int width, int height) {
      gl_framebuffer frame;
      frame.id = 0;
      frame.width = width;
      frame.height = height;
      return frame;
   }

   gl_framebuffer() {
   }

   gl_framebuffer(int width_, int height_);

   gl_framebuffer(const gl_framebuffer &x) = delete;

   gl_framebuffer(gl_framebuffer &&x) noexcept {
      *this = std::move(x);
   }

   gl_framebuffer& operator=(const gl_framebuffer&) = delete;

   gl_framebuffer& operator=(gl_framebuffer&&x) noexcept {
      std::swap(id,x.id);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(colorBufferID,x.colorBufferID);
      std::swap(width,x.width);
      std::swap(height,x.height);
      return *this;
   }

   ~gl_framebuffer();

   void bind();

   void blit(gl_framebuffer &dest);
};

class gl_context {

   const int CAPACITY = 65536;

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
   struct batch {
      GLuint bufferID;

      gl_context *glc;
      int width, height;
      TextureManager tm;
      std::vector<PMatrix> move;
      vertex *vbuffer;
      std::vector<int> tbuffer;
      std::vector<unsigned short> ibuffer;
      bool lights = false;;
      std::array<float,3> directionLightColor =  { 0.0, 0.0, 0.0 };
      std::array<float,3> directionLightVector = { 0.0, 0.0, 0.0 };
      std::array<float,3> ambientLight =         { 1.0, 1.0, 1.0 };
      std::array<float,3> pointLightColor =      { 0.0, 0.0, 0.0 };
      std::array<float,3> pointLightPosition =   { 0.0, 0.0, 0.0 };
      std::array<float,3> pointLightFalloff =    { 1.0, 0.0, 0.0 };
      PMatrix projection_matrix = PMatrix::Identity();
      PMatrix view_matrix = PMatrix::Identity();
      int currentM;
      int CAPACITY;

      batch(int width, int height, int CAPACITY, gl_context *glc) : tm(width * 3, height * 3) {
         this->width = width;
         this->height = height;
         this->CAPACITY = CAPACITY;
         this->glc = glc;
         bufferID = 0;
         vbuffer = nullptr;

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

         vbuffer = new vertex[CAPACITY];
      }

      batch(const batch &x) = delete;

      batch(batch &&x) noexcept : batch(x.width,x.height, x.CAPACITY, x.glc) {

         *this = std::move(x);
      }

      bool enqueue( const std::vector<vertex> &vertices,
                    const std::vector<unsigned short> &indices,
                    const PMatrix &move_matrix ) {
         if (vertices.size() > CAPACITY) {
            abort();
         } else {
            int new_size = vertices.size() + ibuffer.size();
            if (new_size >= CAPACITY) {
               return false;
            }
         }
         if ( move.size() == 16) {
            return false;
         }
         if ( move.size() > 0 && move_matrix == move.back() ) {
         } else {
            move.push_back( move_matrix );
            currentM = move.size() - 1;
         }

         int offset = tbuffer.size();

         for (int i = 0; i < vertices.size(); ++i) {
            vbuffer[offset + i] = vertices[i];
            tbuffer.push_back( currentM );
         }
         for (auto index : indices) {
            ibuffer.push_back( offset + index  );
         }
         return true;
      }

      void draw( gl_framebuffer &fb,GLuint VAO, GLuint vertex_buffer_id, GLuint tindex_buffer_id, GLuint index_buffer_id) {

         if (tbuffer.size() != 0 ) {
            // Push back array of Ms
            std::vector<float> movePack;
            for (const auto& mat : move) {
               for (int i = 0; i < 16; i++) {
                  movePack.push_back(mat.data()[i]);
               }
            }

            glc->loadMoveMatrix( movePack.data(), move.size() );
            glc->loadProjectionViewMatrix( (projection_matrix * view_matrix).data() );

            if (lights) {
               glc->loadDirectionLightColor( directionLightColor.data() );
               glc->loadDirectionLightVector( directionLightVector.data() );
               glc->loadAmbientLight( ambientLight.data() );
               glc->loadPointLightColor( pointLightColor.data() );
               glc->loadPointLightPosition( pointLightPosition.data() );
               glc->loadPointLightFalloff( pointLightFalloff.data() );
            } else {
               std::array<float,3> on { 1.0, 1.0, 1.0};
               std::array<float,3> off { 0.0, 0.0, 0.0};
               std::array<float,3> unity { 1.0, 0.0, 0.0 };

               glc->loadPointLightFalloff( unity.data() );
               glc->loadDirectionLightColor( off.data() );
               glc->loadAmbientLight( on.data() );
               glc->loadPointLightColor(off.data() );
            }

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
            glBufferSubData(GL_ARRAY_BUFFER, 0, tbuffer.size() * sizeof(vertex), vbuffer );

            glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);
            glBufferData(GL_ARRAY_BUFFER, tbuffer.size() * sizeof(int), tbuffer.data(), GL_STREAM_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibuffer.size() * sizeof(unsigned short), ibuffer.data(), GL_STREAM_DRAW);

            fb.bind();

            glDrawElements(GL_TRIANGLES, ibuffer.size(), GL_UNSIGNED_SHORT, 0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindVertexArray(0);
         }
         ibuffer.clear();
         tbuffer.clear();
         move.clear();
         tm.clear();
         currentM = 0;
      }

      batch& operator=(const batch&) = delete;

      batch& operator=(batch&&x) noexcept {

         std::swap(currentM,x.currentM);

         std::swap(width,x.width);
         std::swap(height,x.height);
         std::swap(move,x.move);
         std::swap(vbuffer,x.vbuffer);
         std::swap(tbuffer,x.tbuffer);
         std::swap(ibuffer,x.ibuffer);
         std::swap(tm,x.tm);

         std::swap(bufferID,x.bufferID);

         std::swap(lights,x.lights);
         std::swap(projection_matrix,x.projection_matrix);
         std::swap(view_matrix,x.view_matrix);
         std::swap(directionLightColor,x.directionLightColor);
         std::swap(directionLightVector,x.directionLightVector);
         std::swap(ambientLight,x.ambientLight);
         std::swap(pointLightColor,x.pointLightColor);
         std::swap(pointLightPosition,x.pointLightPosition);
         std::swap(pointLightFalloff,x.pointLightFalloff);

         std::swap(currentM,x.currentM);

         return *this;
      }

      ~batch(){
         delete [] vbuffer;
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

   // void setMoveMatrix( const PMatrix &move ){
   //    if(batches.size() > 0)
   //       batches.back().move_matrix = move;
   // }

   void setProjectionMatrix( const PMatrix &PV ) {
      if(batches.size() > 0)
         batches.back().projection_matrix = PV;
   }

   void setViewMatrix( const PMatrix &PV ) {
      if(batches.size() > 0)
         batches.back().view_matrix = PMatrix::FlipY() * PV ;
   }

   void setDirectionLightColor(const std::array<float,3>  &color ){
      if(batches.size() > 0)
         batches.back().directionLightColor = color;
   }

   void setDirectionLightVector(const std::array<float,3>  &dir  ){
      if(batches.size() > 0)
         batches.back().directionLightVector = dir;
   }

   void setAmbientLight(const std::array<float,3>  &color ){
      if(batches.size() > 0)
         batches.back().ambientLight = color;
   }

   void setPointLightColor(const std::array<float,3>  &color){
      if(batches.size() > 0)
         batches.back().pointLightColor = color;
   }

   void setPointLightPosition( const std::array<float,3>  &pos ){
      if(batches.size() > 0)
         batches.back().pointLightPosition = pos;
   }

   void setPointLightFalloff( const std::array<float,3>  &data){
      if(batches.size() > 0)
         batches.back().pointLightFalloff = data;
   }

   void setLights( bool data ) {
      if(batches.size() > 0)
         batches.back().lights = data;
   }

   ~gl_context();

   void hint(int type);

   void saveFrame(const std::string& fileName);

   void loadPixels( std::vector<unsigned int> &pixels );

   // Draw our pixels into a new texture and call drawTexturedQuad over whole screen
   void updatePixels( std::vector<unsigned int> &pixels ) {
      PTexture texture = getTexture(window_width, window_height, pixels.data());
      draw_texture_over_framebuffer(texture, localFrame, true);
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

   void loadMoveMatrix( const float *data, int size = 1 );

   void loadProjectionViewMatrix( const float *data );

   void loadDirectionLightColor( const float *data );

   void loadDirectionLightVector( const float *data );

   void loadAmbientLight( const float *data );

   void loadPointLightColor( const float *data );

   void loadPointLightPosition( const float *data );

   void loadPointLightFalloff( const float *data );


   void flush() {

      flushes++;
      if(batches.size() > 0)
         batches.back().draw( localFrame, VAO, vertex_buffer_id, tindex_buffer_id, index_buffer_id);
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

   void draw_texture_over_framebuffer( const PTexture &texture, gl_framebuffer &fb, bool flip = false);

   void draw_main();
   void initVAO();
   void cleanupVAO();
};


#endif
