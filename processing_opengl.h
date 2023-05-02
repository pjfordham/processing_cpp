#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include "processing_math.h"
#include "processing_pshader.h"
#include "processing_color.h"
#include "processing_texture_manager.h"


class gl_context {
public:
   class GL_FLOAT_buffer {
      GLuint buffer_id = 0;
      GLuint attribId;
   public:
      GL_FLOAT_buffer(GLuint buffer_id, GLuint programID, const void *data, int size, GLuint attribId, int count, int stride, void* offset) {
         if (size > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
            glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, GL_STREAM_DRAW);
            glVertexAttribPointer(
               attribId,                         // attribute
               count,                                // size
               GL_FLOAT,                         // type
               GL_FALSE,                         // normalized?
               stride,                           // stride
               offset                     // array buffer offset
               );
            glEnableVertexAttribArray(attribId);
         }
      }
      ~GL_FLOAT_buffer() {
      }
   };
   class GL_INT_buffer {
      GLuint buffer_id = 0;
      GLuint attribId;
   public:
      GL_INT_buffer(GLuint buffer_id, GLuint programID, const void *data, int size, GLuint attribId, int count, int stride, void* offset) {
         if (size > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
            glBufferData(GL_ARRAY_BUFFER, size * sizeof(int), data, GL_STREAM_DRAW);
            glVertexAttribIPointer(
               attribId,                         // attribute
               count,                                // size
               GL_INT,                         // type
               stride,                           // stride
               offset                     // array buffer offset
               );
            glEnableVertexAttribArray(attribId);
         }
      }
      ~GL_INT_buffer() {
      }
   };

   const int CAPACITY = 65536;
   int currentM;
   int flushes = 0;

   TextureManager tm;
   std::vector<Eigen::Matrix4f> move;
   std::vector<PVector> vbuffer;
   std::vector<PVector> nbuffer;
   std::vector<PVector> cbuffer;
   std::vector<float> xbuffer;
   std::vector<int> tbuffer;
   std::vector<unsigned short> ibuffer;

   GLuint bufferID;
   GLuint localFboID;
   GLuint depthBufferID;

   GLuint index_buffer_id;
   GLuint vertex_buffer_id;
   GLuint coords_buffer_id;
   GLuint colors_buffer_id;
   GLuint tindex_buffer_id;
   GLuint normal_buffer_id;
   GLuint vertex_attrib_id;
   GLuint coords_attrib_id;
   GLuint colors_attrib_id;
   GLuint tindex_attrib_id;
   GLuint normal_attrib_id;
   PShader defaultShader;
   GLuint Mmatrix;
   GLuint Pmatrix;
   GLuint Vmatrix;
   GLuint uSampler;
   GLuint AmbientLight;
   GLuint DirectionLightColor;
   GLuint DirectionLightVector;
   GLuint programID;

   gl_context(int width, int height) : tm(width, height) {
      localFboID = 0;
      bufferID = 0;
      depthBufferID = 0;
   }
   gl_context(const gl_context &x) = delete;

   gl_context(gl_context &&x) {
      std::swap(currentM,x.currentM);
      std::swap(flushes,x.flushes);
      std::swap(tm,x.tm);
      std::swap(move,x.move);
      std::swap(vbuffer,x.vbuffer);
      std::swap(nbuffer,x.nbuffer);
      std::swap(cbuffer,x.cbuffer);
      std::swap(xbuffer,x.xbuffer);
      std::swap(tbuffer,x.tbuffer);
      std::swap(ibuffer,x.ibuffer);
      std::swap(bufferID,x.bufferID);
      std::swap(localFboID,x.localFboID);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(defaultShader,x.defaultShader);
      std::swap(Mmatrix,x.Mmatrix);
      std::swap(Pmatrix,x.Pmatrix);
      std::swap(Vmatrix,x.Vmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);
      std::swap(programID,x.programID);
      std::swap(index_buffer_id,x.index_buffer_id);
      std::swap(vertex_buffer_id,x.vertex_buffer_id);
      std::swap(coords_buffer_id,x.coords_buffer_id);
      std::swap(colors_buffer_id,x.colors_buffer_id);
      std::swap(tindex_buffer_id,x.tindex_buffer_id);
      std::swap(normal_buffer_id,x.normal_buffer_id);
      std::swap(vertex_attrib_id,x.vertex_attrib_id);
      std::swap(coords_attrib_id,x.coords_attrib_id);
      std::swap(colors_attrib_id,x.colors_attrib_id);
      std::swap(tindex_attrib_id,x.tindex_attrib_id);
      std::swap(normal_attrib_id,x.normal_attrib_id);
   }

   gl_context& operator=(const gl_context&) = delete;

   gl_context& operator=(gl_context&&x){
      std::swap(currentM,x.currentM);
      std::swap(flushes,x.flushes);
      std::swap(tm,x.tm);
      std::swap(move,x.move);
      std::swap(vbuffer,x.vbuffer);
      std::swap(nbuffer,x.nbuffer);
      std::swap(cbuffer,x.cbuffer);
      std::swap(xbuffer,x.xbuffer);
      std::swap(tbuffer,x.tbuffer);
      std::swap(ibuffer,x.ibuffer);
      std::swap(bufferID,x.bufferID);
      std::swap(localFboID,x.localFboID);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(defaultShader,x.defaultShader);
      std::swap(Mmatrix,x.Mmatrix);
      std::swap(Pmatrix,x.Pmatrix);
      std::swap(Vmatrix,x.Vmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);
      std::swap(programID,x.programID);
      std::swap(index_buffer_id,x.index_buffer_id);
      std::swap(vertex_buffer_id,x.vertex_buffer_id);
      std::swap(coords_buffer_id,x.coords_buffer_id);
      std::swap(colors_buffer_id,x.colors_buffer_id);
      std::swap(tindex_buffer_id,x.tindex_buffer_id);
      std::swap(normal_buffer_id,x.normal_buffer_id);
      std::swap(vertex_attrib_id,x.vertex_attrib_id);
      std::swap(coords_attrib_id,x.coords_attrib_id);
      std::swap(colors_attrib_id,x.colors_attrib_id);
      std::swap(tindex_attrib_id,x.tindex_attrib_id);
      std::swap(normal_attrib_id,x.normal_attrib_id);
      return *this;
   }

   ~gl_context() {
      if (localFboID)
         glDeleteFramebuffers(1, &localFboID);
      if (bufferID)
         glDeleteTextures(1, &bufferID);
      if (depthBufferID)
         glDeleteRenderbuffers(1, &depthBufferID);
   }


   void clear(float r, float g, float b, float a) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

   void clearDepthBuffer() {
      // Clear the depth buffer but not what was drawn for the next frame
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      glClear(GL_DEPTH_BUFFER_BIT);
   }

   void shader(PShader &shader, int kind = TRIANGLES) {
      vertex_attrib_id = glGetAttribLocation(shader.ProgramID, "position");
      normal_attrib_id = glGetAttribLocation(shader.ProgramID, "normal");
      coords_attrib_id = glGetAttribLocation(shader.ProgramID, "coords");
      colors_attrib_id = glGetAttribLocation(shader.ProgramID, "colors");
      tindex_attrib_id = glGetAttribLocation(shader.ProgramID, "mindex");
      Mmatrix = glGetUniformLocation(shader.ProgramID, "Mmatrix");
      Pmatrix = glGetUniformLocation(shader.ProgramID, "Pmatrix");
      Vmatrix = glGetUniformLocation(shader.ProgramID, "Vmatrix");
      AmbientLight = glGetUniformLocation(shader.ProgramID, "ambientLight");
      DirectionLightColor = glGetUniformLocation(shader.ProgramID, "directionLightColor");
      DirectionLightVector = glGetUniformLocation(shader.ProgramID, "directionLightVector");
      uSampler = glGetUniformLocation(shader.ProgramID, "myTextures");
      glUseProgram(shader.ProgramID);
      programID = shader.ProgramID;
      shader.set_uniforms();
   }

   void init(int width, int height, bool useMainFramebuffer = false) {
      vbuffer.reserve(CAPACITY);
      nbuffer.reserve(CAPACITY);
      cbuffer.reserve(CAPACITY);
      xbuffer.reserve(CAPACITY);
      tbuffer.reserve(CAPACITY);
      ibuffer.reserve(CAPACITY);
      currentM = 0;

      glGenBuffers(1, &index_buffer_id);
      glGenBuffers(1, &vertex_buffer_id);
      glGenBuffers(1, &coords_buffer_id);
      glGenBuffers(1, &colors_buffer_id);
      glGenBuffers(1, &tindex_buffer_id);
      glGenBuffers(1, &normal_buffer_id);

      defaultShader = loadShader();
      shader( defaultShader );

      int textureUnitIndex = 0;
      glUniform1i(uSampler,textureUnitIndex);
      glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
      // create the texture array
      glGenTextures(1, &bufferID);
      glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);
      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      // set texture parameters
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (useMainFramebuffer) {
         // Use main framebuffer
         localFboID = 0;
      } else {
         // Create a framebuffer object
         glGenFramebuffers(1, &localFboID);
         glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
         glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, bufferID, 0, 1);

         glGenRenderbuffers(1, &depthBufferID);
         glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

         // Attach the depth buffer to the framebuffer object
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
//         glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, bufferID, 0, 2);
         auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if (err != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr,"%d\n",err);
         }
      }

      // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
      GLubyte white[4] = { 255, 255, 255, 255 };
      glClearTexImage(bufferID, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);

   }

   void loadMoveMatrix( float *data ) {
      glUniformMatrix4fv(Mmatrix, 1,false, data );
   }

   void loadViewMatrix( float *data ) {
      glUniformMatrix4fv(Vmatrix, 1,false, data );
   }

   void loadProjectionMatrix( float *data ) {
      glUniformMatrix4fv(Pmatrix, 1,false, data );
   }

   void loadDirectionLightColor( float *data ){
      glUniform3fv(DirectionLightColor, 1, data );
   }

   void loadDirectionLightVector( float *data ){
      glUniform3fv(DirectionLightVector, 1, data );
   }

   void loadAmbientLight( float *data ){
      glUniform3fv(AmbientLight, 1, data );
   }

   void reserve(int n_vertices, const Eigen::Matrix4f &move_matrix, float *projection, float *view,
                float *directionLightColor, float *directionLightVector, float *ambientLight) {
      if (n_vertices > CAPACITY) {
         abort();
      } else {
         int new_size = n_vertices + ibuffer.size();
         if (new_size >= CAPACITY) {
            flush(projection, view, directionLightColor, directionLightVector, ambientLight);
         }
      }
      if ( move.size() == 8) {
         flush(projection, view, directionLightColor, directionLightVector, ambientLight);
      }
      if ( move.size() > 0 && move_matrix == move.back() ) {
      } else {
         move.push_back( move_matrix );
         currentM = move.size() - 1;
      }
   }

   void flush( float *projection, float *view,
               float *directionLightColor, float *directionLightVector, float *ambientLight );

   void drawTriangles( const std::vector<PVector> &vertices,
                       const std::vector<PVector> &normals,
                       const std::vector<PVector> &coords,
                       const std::vector<unsigned short> &indices,
                       color color){
      if (indices.size() == 0) abort();

      auto starti = vbuffer.size();

      vbuffer.insert(vbuffer.end(), vertices.begin(), vertices.end());
      cbuffer.insert(cbuffer.end(), coords.begin(),   coords.end());
      nbuffer.insert(nbuffer.end(), normals.begin(),  normals.end());

      for (auto vertex : vertices) {
         xbuffer.push_back( color.r / 255.0f );
         xbuffer.push_back( color.g / 255.0f );
         xbuffer.push_back( color.b / 255.0f );
         xbuffer.push_back( color.a / 255.0f );
         tbuffer.push_back( currentM );
      }

      for (auto index : indices) {
         ibuffer.push_back( starti + index  );
      }
   }
   void drawTrianglesDirect( const std::vector<PVector> &vertices,
                             const std::vector<PVector> &normals,
                             const std::vector<PVector> &coords,
                             const std::vector<float> &colors,
                             const std::vector<int> &tindex,
                             const std::vector<unsigned short> &indices,
                             int count ) {
      // Create a vertex array object (VAO)
      GLuint VAO;
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      GL_FLOAT_buffer vertex( vertex_buffer_id, programID, vertices.data(), vertices.size() * 3,
                              vertex_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer normal( normal_buffer_id, programID, normals.data(),  normals.size() * 3,
                              normal_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer coord(  coords_buffer_id, programID, coords.data(),   coords.size()  * 3,
                              coords_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer color(  colors_buffer_id, programID, colors.data(),   colors.size(),
                              colors_attrib_id, 4, 0, 0);
      GL_INT_buffer  mindex(  tindex_buffer_id, programID, tindex.data(),   tindex.size(),
                              tindex_attrib_id, 1, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      // Unbind the buffer objects and VAO
      glDeleteVertexArrays(1, &VAO);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
   }

   int getFlushCount() {
      return flushes;
   }

   void resetFlushCount() {
      flushes = 0;
   }

   PShader loadShader(const char *fragShader, const char *vertShader) {
      auto shader = PShader( 0, vertShader, fragShader );
      shader.compileShaders();
      return shader;
   }

   PShader loadShader(const char *fragShader) {
      std::ifstream inputFile(fragShader);

      if (!inputFile.is_open()) {
         abort();
      }

      std::stringstream buffer;
      buffer << inputFile.rdbuf();

      inputFile.close();

      auto shader = PShader( 0, buffer.str().c_str() );
      shader.compileShaders();
      return shader;
   }

   PShader loadShader() {
      auto shader = PShader( 0 );
      shader.compileShaders();
      return shader;
   }

   void resetShader(int kind = TRIANGLES) {
      shader( defaultShader );
   }
};

std::vector<float> packMatrices(const std::vector<Eigen::Matrix4f>& matrices) {
   std::vector<float> result;

   for (const auto& mat : matrices) {
      for (int i = 0; i < 4; i++) {
         for (int j = 0; j < 4; j++) {
            result.push_back(mat(j, i));
         }
      }
   }

   return result;
}

void gl_context::flush( float *projection, float *view,
                        float *directionLightColor, float *directionLightVector, float *ambientLight ) {
   flushes++;
   if (flushes > 1000) abort();
   // Push back array of Ms
   if (vbuffer.size() == 0 ) return;
   auto movePack = packMatrices(move);
   glUniformMatrix4fv(Mmatrix, move.size(),false,movePack.data());

   glUniformMatrix4fv(Pmatrix, 1,false, projection);
   glUniformMatrix4fv(Vmatrix, 1,false, view);

   glUniform3fv(DirectionLightColor, 1, directionLightColor );
   glUniform3fv(DirectionLightVector, 1, directionLightVector );
   glUniform3fv(AmbientLight, 1, ambientLight );

   tm.clear();
   move.clear();

   glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
   glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);

   drawTrianglesDirect( vbuffer, nbuffer, cbuffer, xbuffer, tbuffer, ibuffer, ibuffer.size() );
   vbuffer.clear();
   nbuffer.clear();
   cbuffer.clear();
   ibuffer.clear();
   tbuffer.clear();
   xbuffer.clear();
}

#endif
