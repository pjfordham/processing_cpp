#include "processing_opengl.h"

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

PFrame::PFrame(int width_, int height_)  : width(width_), height(height_) {
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

PFrame::PFrame( int width_, int height_, GLuint colorBufferID, int layer ) : width(width_), height(height_) {
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

PFrame::~PFrame() {
   if (id)
      glDeleteFramebuffers(1, &id);
   if (depthBufferID)
      glDeleteRenderbuffers(1, &depthBufferID);
   if (colorBufferID)
      glDeleteRenderbuffers(1, &colorBufferID);
}

void PFrame::bind() {
   // Bind the framebuffer and get its dimensions
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   glViewport(0, 0, width, height);
}

gl_context::gl_context(int width, int height, float aaFactor) : tm(width * aaFactor, height * aaFactor) {
   this->aaFactor = aaFactor;
   this->width = width * aaFactor;
   this->height = height * aaFactor;
   this->window_width = width;
   this->window_height = height;

   bufferID = 0;

   windowFrame = PFrame::constructMainFrame( window_width, window_height );

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

PTexture gl_context::getTexture( SDL_Surface *surface ) {
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

void gl_context::draw_main() {
   // Already drawn directly to framebuffer so we don't need to do anything
   if (!localFrame.isMainFrame()) {
      draw_texture_over_framebuffer(PTexture{1,0,0,width, height, width, height}, windowFrame);
      clearDepthBuffer(windowFrame);
   }
   SDL_GL_SwapWindow(window);
}

gl_context::~gl_context() {
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

PShader gl_context::loadShader(const char *fragShader) {
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

void gl_context::saveFrame(const std::string& fileName) {

   int wfWidth = windowFrame.getWidth();
   int wfHeight = windowFrame.getHeight();

   PFrame frame(wfWidth, wfHeight);

   draw_texture_over_framebuffer(PTexture {1,0,0,width, height, width, height}, frame);

   // Create SDL surface from framebuffer data
   SDL_Surface* surface = SDL_CreateRGBSurface(0, wfWidth, wfHeight,
                                               24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
   glReadPixels(0, 0, wfWidth, wfHeight, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

   // Save the image as PNG
   IMG_SavePNG(surface, fileName.c_str());

   // Cleanup
   SDL_FreeSurface(surface);
}

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

void gl_context::loadPixels( std::vector<unsigned int> &pixels ) {

   int wfWidth = windowFrame.getWidth();
   int wfHeight = windowFrame.getHeight();

   PFrame frame(wfWidth, wfHeight);

   draw_texture_over_framebuffer(PTexture {1,0,0,width, height, width, height}, frame);

   pixels.resize(width*height);
   glReadPixels(0, 0, wfWidth, wfHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
}

PTexture gl_context::getTexture( int width, int height, void *pixels ) {
   PTexture texture = tm.getFreeBlock(width, height);
   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                   texture.left, texture.top, texture.layer,
                   width, height, 1,
                   GL_RGBA, GL_UNSIGNED_BYTE, pixels);
   return texture;
}

PTexture gl_context::getTexture( gl_context &source ) {
   PTexture texture = tm.getFreeBlock(source.width, source.height);
   glBindTexture(GL_TEXTURE_2D_ARRAY, source.bufferID);
   glCopyImageSubData(source.bufferID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
                      bufferID, GL_TEXTURE_2D_ARRAY, 0, texture.left, texture.top, texture.layer,
                      source.width, source.height, 1);
   glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);
   return texture;
}

void gl_context::clear(PFrame &fb, float r, float g, float b, float a) {
   fb.bind();
   glClearColor(r, g, b, a);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_context::clearDepthBuffer(PFrame &fb) {
   fb.bind();
   glClear(GL_DEPTH_BUFFER_BIT);
}

void gl_context::loadMoveMatrix( float *data, int size ) {
   glUniformMatrix4fv(Mmatrix, size,false, data );
}

void gl_context::loadViewMatrix( float *data ) {
   glUniformMatrix4fv(Vmatrix, 1,false, data );
}

void gl_context::loadProjectionMatrix( float *data ) {
   glUniformMatrix4fv(Pmatrix, 1,false, data );
}

void gl_context::loadDirectionLightColor( float *data ){
   glUniform3fv(DirectionLightColor, 1, data );
}

void gl_context::loadDirectionLightVector( float *data ){
   glUniform3fv(DirectionLightVector, 1, data );
}

void gl_context::loadAmbientLight( float *data ){
   glUniform3fv(AmbientLight, 1, data );
}

void gl_context::loadPointLightColor( float *data ){
   glUniform3fv(PointLightColor, 1, data );
}

void gl_context::loadPointLightPosition( float *data ){
   glUniform3fv(PointLightPosition, 1, data );
}

void gl_context::loadPointLightFalloff( float *data ){
   glUniform3fv(PointLightFalloff, 1, data );
}

void gl_context::drawTrianglesDirect( PFrame &fb,
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
