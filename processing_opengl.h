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

class PFrame {
public:
   GLuint id = 0;
   int width = 0;
   int height = 0;
   GLuint depthBufferID = 0;
   GLuint colorBufferID = 0;

   PFrame() {
   }

   PFrame(int width_, int height_)  : width(width_), height(height_) {
      glGenFramebuffers(1, &id);
      bind();

      glGenRenderbuffers(1, &depthBufferID);
      glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

      glGenRenderbuffers(1, &colorBufferID);
      glBindRenderbuffer(GL_RENDERBUFFER, colorBufferID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, this->width, this->height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferID);

      auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (err != GL_FRAMEBUFFER_COMPLETE) {
         fprintf(stderr,"Z %d\n",err);
      }
   }

   PFrame( int width_, int height_, GLuint colorBufferID, int layer ) : width(width_), height(height_) {
      glGenFramebuffers(1, &id);
      bind();

      glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBufferID, 0, layer);
      glGenRenderbuffers(1, &depthBufferID);
      glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

      auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (err != GL_FRAMEBUFFER_COMPLETE) {
         fprintf(stderr,"%d\n",err);
      }
   }

   PFrame(const PFrame &x) = delete;

   PFrame(PFrame &&x) noexcept {
      *this = std::move(x);
   }

   PFrame& operator=(const PFrame&) = delete;

   PFrame& operator=(PFrame&&x) noexcept {
      std::swap(id,x.id);
      std::swap(depthBufferID,x.depthBufferID);
      std::swap(colorBufferID,x.colorBufferID);
      std::swap(width,x.width);
      std::swap(height,x.height);
      return *this;
   }

    ~PFrame() {
      if (id)
         glDeleteFramebuffers(1, &id);
      if (depthBufferID)
         glDeleteRenderbuffers(1, &depthBufferID);
      if (colorBufferID)
         glDeleteRenderbuffers(1, &colorBufferID);
   }

   void bind() {
      // Bind the framebuffer and get its dimensions
      glBindFramebuffer(GL_FRAMEBUFFER, id);
      glViewport(0, 0, width, height);
   }
};

class gl_context {

   void GL_FLOAT_buffer(GLuint buffer_id, const void *data, int size, GLuint attribId, int count, int stride, void* offset) {
      if (size > 0) {
         glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
         glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, GL_STREAM_DRAW);
         glVertexAttribPointer(
            attribId, // attribute
            count,    // size
            GL_FLOAT, // type
            GL_FALSE, // normalized
            stride,   // stride
            offset    // array buffer offset
            );
         glEnableVertexAttribArray(attribId);
      }
   }

   void GL_INT_buffer(GLuint buffer_id, const void *data, int size, GLuint attribId, int count, int stride, void* offset) {
      if (size > 0) {
         glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
         glBufferData(GL_ARRAY_BUFFER, size * sizeof(int), data, GL_STREAM_DRAW);
         glVertexAttribIPointer(
            attribId, // attribute
            count,    // size
            GL_INT,   // type
            stride,   // stride
            offset    // array buffer offset
            );
         glEnableVertexAttribArray(attribId);
      }
   }

   const int CAPACITY = 65536;

   SDL_Window *window = NULL;
   SDL_Renderer *renderer =NULL;
   SDL_GLContext glContext = NULL;

   int currentM;
   int flushes = 0;

   int width;
   int height;
   int window_width;
   int window_height;
   TextureManager tm;
   std::vector<Eigen::Matrix4f> move;
   std::vector<PVector> vbuffer;
   std::vector<PVector> nbuffer;
   std::vector<PVector> cbuffer;
   std::vector<float> xbuffer;
   std::vector<int> tbuffer;
   std::vector<unsigned short> ibuffer;

   GLuint bufferID;

   PFrame localFrame;
   PFrame windowFrame;

   float aaFactor;

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

public:
   gl_context() : width(0), height(0) {
      bufferID = 0;
      index_buffer_id = 0;
      vertex_buffer_id = 0;
      coords_buffer_id = 0;
      colors_buffer_id = 0;
      tindex_buffer_id = 0;
      normal_buffer_id = 0;
   }

   gl_context(int width, int height, float aaFactor) : tm(width * aaFactor, height * aaFactor) {
      this->aaFactor = aaFactor;
      this->width = width * aaFactor;
      this->height = height * aaFactor;
      this->window_width = width;
      this->window_height = height;

      bufferID = 0;

      windowFrame.id = 0;
      windowFrame.width = window_width;
      windowFrame.height = window_height;

      bool useMainFramebuffer = false;
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
                                window_width,
                                window_height,
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
      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, this->width, this->height, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      // set texture parameters
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (!useMainFramebuffer) {
         localFrame = PFrame( this->width, this->height, bufferID, 1 );
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

   gl_context(gl_context &&x) noexcept : gl_context() {
      *this = std::move(x);
   }

   gl_context& operator=(const gl_context&) = delete;

   gl_context& operator=(gl_context&&x) noexcept {
      std::swap(window, x.window);
      std::swap(renderer, x.renderer);
      std::swap(glContext, x.glContext);

      std::swap(currentM,x.currentM);
      std::swap(flushes,x.flushes);

      std::swap(width,x.width);
      std::swap(height,x.height);
      std::swap(window_width,x.window_width);
      std::swap(window_height,x.window_height);
      std::swap(tm,x.tm);
      std::swap(move,x.move);
      std::swap(vbuffer,x.vbuffer);
      std::swap(nbuffer,x.nbuffer);
      std::swap(cbuffer,x.cbuffer);
      std::swap(xbuffer,x.xbuffer);
      std::swap(tbuffer,x.tbuffer);
      std::swap(ibuffer,x.ibuffer);

      std::swap(bufferID,x.bufferID);

      std::swap(localFrame,x.localFrame);
      std::swap(windowFrame,x.windowFrame);

      std::swap(aaFactor,x.aaFactor);

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
      std::swap(defaultShader,x.defaultShader);
      std::swap(Mmatrix,x.Mmatrix);
      std::swap(Pmatrix,x.Pmatrix);
      std::swap(Vmatrix,x.Vmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);

      return *this;
   }

   ~gl_context() {
      if (bufferID)
         glDeleteTextures(1, &bufferID);
      if (glContext)
         SDL_GL_DeleteContext(glContext);
      if (window)
         SDL_DestroyWindow(window);
      if (index_buffer_id)
         glDeleteBuffers(1, &index_buffer_id);
      if (vertex_buffer_id)
         glDeleteBuffers(1, &vertex_buffer_id);
      if (coords_buffer_id)
         glDeleteBuffers(1, &coords_buffer_id);
      if (colors_buffer_id)
         glDeleteBuffers(1, &colors_buffer_id);
      if (tindex_buffer_id)
         glDeleteBuffers(1, &tindex_buffer_id);
      if (normal_buffer_id)
         glDeleteBuffers(1, &normal_buffer_id);
   }

   void saveFrame(const std::string& fileName) {

      PFrame frame(windowFrame.width, windowFrame.height);

      draw_texture_over_framebuffer(PTexture {1,0,0,width, height, width, height}, frame);

      // Create SDL surface from framebuffer data
      SDL_Surface* surface = SDL_CreateRGBSurface(0, windowFrame.width, windowFrame.height,
                                                  24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
      glReadPixels(0, 0, windowFrame.width, windowFrame.height, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

      // Save the image as PNG
      IMG_SavePNG(surface, fileName.c_str());

      // Cleanup
      SDL_FreeSurface(surface);
   }

   void loadPixels( std::vector<unsigned int> &pixels ) {
       PFrame frame(windowFrame.width, windowFrame.height);

       draw_texture_over_framebuffer(PTexture {1,0,0,width, height, width, height}, frame);

       pixels.resize(width*height);
       glReadPixels(0, 0, windowFrame.width, windowFrame.height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
   }

   // Draw our pixels into a new texture and call drawTexturedQuad over whole screen
   void updatePixels( std::vector<unsigned int> &pixels ) {
      PTexture texture = getTexture(window_width, window_height, pixels.data());
      draw_texture_over_framebuffer(texture, localFrame);
   }

   SDL_Surface* crop_surface(SDL_Surface* surface, int max_width, int max_height) {
      // Determine the width and height of the cropped surface
      int crop_width = SDL_min(max_width, surface->w);
      int crop_height = SDL_min(max_height, surface->h);

      // Create a new surface with the desired dimensions
      SDL_Surface* cropped_surface = SDL_CreateRGBSurfaceWithFormat(0, crop_width, crop_height, surface->format->BitsPerPixel, surface->format->format);

      // Copy the portion of the original surface that fits within the new surface
      SDL_Rect src_rect = { 0, 0, crop_width, crop_height };
      SDL_Rect dst_rect = { 0, 0, crop_width, crop_height };
      SDL_BlitSurface(surface, &src_rect, cropped_surface, &dst_rect);

      return cropped_surface;
   }

   PTexture getTexture( int width, int height, void *pixels ) {
      PTexture texture = tm.getFreeBlock(width, height);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                      texture.left, texture.top, texture.layer,
                      width, height, 1,
                      GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      return texture;
   }

   PTexture getTexture( SDL_Surface *surface ) {
      if ( surface->w > width || surface->h > height ) {
         int new_width = std::min(surface->w, width);
         int new_height = std::min(surface->h, height );
         SDL_Surface *new_surface = crop_surface( surface, new_width, new_height);
         PTexture tex = getTexture( new_surface );
         SDL_FreeSurface( new_surface );
         return tex;
      } else {
         return getTexture( surface->w, surface->h, surface->pixels );
      }
   }

   PTexture getTexture( PImage &pimage ) {
      return getTexture( pimage.surface );
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

   void clear(PFrame &fb, float r, float g, float b, float a) {
      fb.bind();
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void clear( float r, float g, float b, float a) {
      clear( localFrame, r, g, b, a);
   }

   void clearDepthBuffer(PFrame &fb) {
      fb.bind();
      glClear(GL_DEPTH_BUFFER_BIT);
   }

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
      Pmatrix = shader.getUniformLocation("Pmatrix");
      Vmatrix = shader.getUniformLocation("Vmatrix");
      AmbientLight = shader.getUniformLocation("ambientLight");
      DirectionLightColor = shader.getUniformLocation("directionLightColor");
      DirectionLightVector = shader.getUniformLocation("directionLightVector");
      uSampler = shader.getUniformLocation("myTextures");
      shader.useProgram();
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

      if (vbuffer.size() != 0 ) {

         // Push back array of Ms
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

         drawTrianglesDirect( localFrame, vbuffer, nbuffer, cbuffer, xbuffer, tbuffer, ibuffer, ibuffer.size() );

         vbuffer.clear();
         nbuffer.clear();
         cbuffer.clear();
         ibuffer.clear();
         tbuffer.clear();
         xbuffer.clear();
      }

      move.clear();
      tm.clear();
      return;
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

   void drawTrianglesDirect( PFrame &fb,
                             const std::vector<PVector> &vertices,
                             const std::vector<PVector> &normals,
                             const std::vector<PVector> &coords,
                             const std::vector<float> &colors,
                             const std::vector<int> &tindex,
                             const std::vector<unsigned short> &indices,
                             int count ) {

      fb.bind();
      // Create a vertex array object (VAO)
      GLuint VAO;
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      GL_FLOAT_buffer( vertex_buffer_id, vertices.data(), vertices.size() * 3,
                       vertex_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer( normal_buffer_id, normals.data(),  normals.size() * 3,
                       normal_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer( coords_buffer_id, coords.data(),   coords.size()  * 3,
                       coords_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer( colors_buffer_id, colors.data(),   colors.size(),
                       colors_attrib_id, 4, 0, 0);
      GL_INT_buffer( tindex_buffer_id, tindex.data(),   tindex.size(),
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

   int getFlushCount() const {
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

   void draw_texture_over_framebuffer( const PTexture &texture, PFrame &fb) {

     // Reset to default view & lighting settings to draw buffered frame.
      std::array<float,3> directionLightColor =  { 0.0, 0.0, 0.0 };
      std::array<float,3> directionLightVector = { 0.0, 0.0, 0.0 };
      std::array<float,3> ambientLight =         { 1.0, 1.0, 1.0 };

      loadDirectionLightColor( directionLightColor.data() );
      loadDirectionLightVector( directionLightVector.data() );
      loadAmbientLight( ambientLight.data() );

      Eigen::Matrix4f identity_matrix = Eigen::Matrix4f::Identity();

      loadMoveMatrix( identity_matrix.data() );
      loadProjectionMatrix( identity_matrix.data());
      loadViewMatrix( identity_matrix.data());

      std::vector<PVector> vertices{
         {-1.0f, -1.0f},
         { 1.0f, -1.0f},
         { 1.0f,  1.0f},
         {-1.0f,  1.0f}};

      float z = 1.0;
      // For drawing the main screen we need to flip the texture and remove any tint
      std::vector<PVector> coords {
         { texture.nleft(),  texture.nbottom(), (float)texture.layer},
         { texture.nright(), texture.nbottom(), (float)texture.layer},
         { texture.nright(), texture.ntop(),    (float)texture.layer},
         { texture.nleft(),  texture.ntop(),    (float)texture.layer},
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

      clear( fb, 0.0, 0.0, 0.0, 1.0);
      drawTrianglesDirect( fb, vertices, normals, coords, colors, mindex, indices,
                           indices.size());
   }

   void draw_main() {
      // Already drawn directly to framebuffer so we don't need to do anything
      if (localFrame.id != 0) {
         draw_texture_over_framebuffer(PTexture{1,0,0,width, height, width, height}, windowFrame);
         clearDepthBuffer(windowFrame);
      }
      SDL_GL_SwapWindow(window);
   }


};


#endif
