#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <vector>
#include "processing_math.h"
#include "processing_pshader.h"
#include "processing_pimage.h"
#include "processing_color.h"
#include "processing_texture_manager.h"


class gl_context {

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

   SDL_Window *window = NULL;
   SDL_Renderer *renderer =NULL;
   SDL_GLContext glContext = NULL;

   const int CAPACITY = 65536;
   int currentM;
   int flushes = 0;

   int width;
   int height;
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

public:
   gl_context(int width, int height) : tm(width, height) {
      this->width = width;
      this->height = height;
      localFboID = 0;
      bufferID = 0;
      depthBufferID = 0;

      bool useMainFramebuffer = false;
      if (width == 0 && height == 0)
         return;
      vbuffer.reserve(CAPACITY);
      nbuffer.reserve(CAPACITY);
      cbuffer.reserve(CAPACITY);
      xbuffer.reserve(CAPACITY);
      tbuffer.reserve(CAPACITY);
      ibuffer.reserve(CAPACITY);
      currentM = 0;

      window = SDL_CreateWindow("Proce++ing",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                width,
                                height,
                                SDL_WINDOW_OPENGL);

      if (window == nullptr) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
         abort();
      }

      // Set OpenGL attributes
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

      // Create OpenGL context
      glContext = SDL_GL_CreateContext(window);
      if (glContext == nullptr) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
         abort();
      }

      // Initialize GLEW
      glewExperimental = true; // Needed for core profile
      if (glewInit() != GLEW_OK) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glew init error\n");
         abort();
      }

      if (!glewIsSupported("GL_EXT_framebuffer_object")) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
         abort();
      }

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
   gl_context(const gl_context &x) = delete;

   gl_context(gl_context &&x) {
      std::swap(window, x.window);
      std::swap(renderer, x.renderer);
      std::swap(glContext, x.glContext);

      std::swap(width,x.width);
      std::swap(height,x.height);
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
      std::swap(window, x.window);
      std::swap(renderer, x.renderer);
      std::swap(glContext, x.glContext);

      std::swap(width,x.width);
      std::swap(height,x.height);
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
      if (glContext)
         SDL_GL_DeleteContext(glContext);
      if (window)
         SDL_DestroyWindow(window);
   }

   void saveFrame(const std::string& fileName) {

      // Bind the framebuffer and get its dimensions
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);

      // Create SDL surface from framebuffer data
      SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
      glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

      // Flip the image vertically
      SDL_Surface* flippedSurface = SDL_CreateRGBSurface(0, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
      SDL_BlitSurface(surface, nullptr, flippedSurface, nullptr);
      SDL_FreeSurface(surface);

      // Save the image as PNG
      IMG_SavePNG(flippedSurface, fileName.c_str());

      // Cleanup
      SDL_FreeSurface(flippedSurface);
   }

   void loadPixels( std::vector<unsigned int> &pixels ) {
      pixels.resize(width*height);
      // Read the pixel data from the framebuffer into the array
      glGetTextureSubImage(bufferID, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.size(), pixels.data());
   }

   void updatePixels( std::vector<unsigned int> &pixels ) {
      // Write the pixel data to the framebuffer
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      // _pixels.clear();
      // pixels = NULL;
   }

   PTexture getTexture( PImage &pimage ) {
      return getTexture( pimage.surface );
   }

   PTexture getTexture( SDL_Surface  *surface ) {
      PTexture texture = tm.getFreeBlock(surface->w, surface->h);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                      texture.left, texture.top, texture.layer,
                      surface->w, surface->h, 1,
                      GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
      return texture;
   }

   PTexture getTexture( gl_context &source ) {
      PTexture texture = tm.getFreeBlock(source.width, source.height);
      glBindTexture(GL_TEXTURE_2D_ARRAY, source.bufferID);
      glCopyImageSubData(source.bufferID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
                         bufferID, GL_TEXTURE_2D_ARRAY, 0, texture.left, texture.top, texture.layer,
                         source.width, source.height, 1);
      glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);
      return texture;
   }

   void clear(GLuint fb, float r, float g, float b, float a) {
      glBindFramebuffer(GL_FRAMEBUFFER, fb);
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void clear( float r, float g, float b, float a) {
      clear( localFboID, r, g, b, a);
   }

   void clearDepthBuffer(GLuint fb) {
      glBindFramebuffer(GL_FRAMEBUFFER, fb);
      // Clear the depth buffer but not what was drawn for the next frame
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      glClear(GL_DEPTH_BUFFER_BIT);
   }

   void clearDepthBuffer() {
      clearDepthBuffer( localFboID );
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

   void loadMoveMatrix( float *data, int size = 1 ) {
      glUniformMatrix4fv(Mmatrix, size,false, data );
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
      if ( move.size() == 16) {
         flush(projection, view, directionLightColor, directionLightVector, ambientLight);
      }
      if ( move.size() > 0 && move_matrix == move.back() ) {
      } else {
         move.push_back( move_matrix );
         currentM = move.size() - 1;
      }
   }

   void flush( float *projection, float *view,
               float *directionLightColor, float *directionLightVector, float *ambientLight ) {
      flushes++;
      if (flushes > 1000) abort();
      // Push back array of Ms
      if (vbuffer.size() == 0 ) return;

      std::vector<float> movePack;
      for (const auto& mat : move) {
         for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
               movePack.push_back(mat(j, i));
            }
         }
      }

      loadMoveMatrix( movePack.data(), move.size() );
      loadProjectionMatrix( projection );
      loadViewMatrix( view );
      loadDirectionLightColor( directionLightColor );
      loadDirectionLightVector( directionLightVector );
      loadAmbientLight( ambientLight );

      tm.clear();
      move.clear();

      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);

      drawTrianglesDirect( localFboID, vbuffer, nbuffer, cbuffer, xbuffer, tbuffer, ibuffer, ibuffer.size() );
      vbuffer.clear();
      nbuffer.clear();
      cbuffer.clear();
      ibuffer.clear();
      tbuffer.clear();
      xbuffer.clear();
   }

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

   void drawTrianglesDirect( GLuint fb,
                             const std::vector<PVector> &vertices,
                             const std::vector<PVector> &normals,
                             const std::vector<PVector> &coords,
                             const std::vector<float> &colors,
                             const std::vector<int> &tindex,
                             const std::vector<unsigned short> &indices,
                             int count ) {

      glBindFramebuffer(GL_FRAMEBUFFER, fb);
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

   void draw_main() {

      // Already drawn directly to framebuffer so we don't need to do anything
      if (localFboID == 0) {
         SDL_GL_SwapWindow(window);
         return;
      }

      // Reset to default view & lighting settings to draw buffered frame.
      std::array<float,3> directionLightColor =  { 0.0 ,0.0,0.0 };
      std::array<float,3> directionLightVector = { 0.0 ,0.0,0.0 };
      std::array<float,3> ambientLight =         { 1.0 ,1.0,1.0 };

      loadDirectionLightColor( directionLightColor.data() );
      loadDirectionLightVector( directionLightVector.data() );
      loadAmbientLight( ambientLight.data() );

      Eigen::Matrix4f identity_matrix = Eigen::Matrix4f::Identity();
      Eigen::Matrix4f projection_matrix = TranslateMatrix(PVector{-1,-1,0}) * ScaleMatrix(PVector{2.0f/width, 2.0f/height,1.0});

      loadMoveMatrix( identity_matrix.data() );
      loadProjectionMatrix( projection_matrix.data());
      loadViewMatrix( identity_matrix.data());

      // For drawing the main screen we need to flip the texture and remove any tint
      std::vector<PVector> vertices{
         {0.0f,       0.0f+height},
         {0.0f+width ,0.0f+height},
         {0.0f+width, 0.0f},
         {0.0f,       0.0f}};

      float z = 1.0;
      std::vector<PVector> coords {
         { 0.0f, 0.0f, z},
         { 1.0f, 0.0f, z},
         { 1.0f, 1.0f, z},
         { 0.0f, 1.0f, z},
      };
      std::vector<PVector> normals;

      std::vector<unsigned short>  indices = {
         0,1,2, 0,2,3,
      };
      std::vector<float> colors {
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
      };

      std::vector<int> mindex(vertices.size(), 0);

      // Clear the color and depth buffers
      clear( 0, 0.0, 0.0, 0.0, 1.0);
      drawTrianglesDirect( 0, vertices, normals, coords, colors, mindex, indices,
                           indices.size());
      clearDepthBuffer();
      SDL_GL_SwapWindow(window);
   }


};


#endif
