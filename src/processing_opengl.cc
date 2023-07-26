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

void gl_context::hint(int type) {
   switch(type) {
   case DISABLE_DEPTH_TEST:
      glDisable(GL_DEPTH_TEST);
      break;
   case ENABLE_DEPTH_TEST:
      glEnable(GL_DEPTH_TEST);
      break;
   default:
      break;
   }
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
   glGenBuffers(1, &tindex_buffer_id);

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
   } else {
      // AA factor isn't right but somehow this works.
      localFrame = PFrame::constructMainFrame( width, height );
   }

   // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
   GLubyte white[4] = { 255, 255, 255, 255 };
   glClearTexImage(bufferID, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   initVAO();
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
   cleanupVAO();
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
   if (tindex_buffer_id)
      glDeleteBuffers(1, &tindex_buffer_id);
}

PShader gl_context::loadShader(const char *fragShader) {
   using namespace std::literals;

   std::ifstream inputFile("data/"s + fragShader);

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

PShader gl_context::loadShader(const char *fragShader, const char *vertShader) {
   using namespace std::literals;

   std::ifstream inputFile("data/"s + fragShader);

   if (!inputFile.is_open()) {
      abort();
   }

   std::stringstream buffer;
   buffer << inputFile.rdbuf();

   inputFile.close();
   std::ifstream inputFile2(vertShader);

   if (!inputFile2.is_open()) {
      abort();
   }

   std::stringstream buffer2;
   buffer2 << inputFile2.rdbuf();

   inputFile2.close();

   auto shader = PShader( 0, buffer2.str().c_str(), buffer.str().c_str() );
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

void gl_context::loadMoveMatrix( const float *data, int size ) {
   glUniformMatrix4fv(Mmatrix, size,false, data );
}

void gl_context::loadProjectionViewMatrix( const float *data ) {
   glUniformMatrix4fv(PVmatrix, 1,false, data );
}

void gl_context::loadDirectionLightColor( const float *data ){
   glUniform3fv(DirectionLightColor, 1, data );
}

void gl_context::loadDirectionLightVector( const float *data ){
   glUniform3fv(DirectionLightVector, 1, data );
}

void gl_context::loadAmbientLight( const float *data ){
   glUniform3fv(AmbientLight, 1, data );
}

void gl_context::loadPointLightColor( const float *data ){
   glUniform3fv(PointLightColor, 1, data );
}

void gl_context::loadPointLightPosition( const float *data ){
   glUniform3fv(PointLightPosition, 1, data );
}

void gl_context::loadPointLightFalloff( const float *data ){
   glUniform3fv(PointLightFalloff, 1, data );
}

void gl_context::initVAO() {
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, CAPACITY * sizeof(vertex), nullptr, GL_STREAM_DRAW);
   vbuffer = new vertex[CAPACITY];

   glVertexAttribPointer( vertex_attrib_id, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),  (void*)offsetof(vertex,position) );
   glVertexAttribPointer( normal_attrib_id, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),  (void*)offsetof(vertex,normal) );
   glVertexAttribPointer( coords_attrib_id, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),  (void*)offsetof(vertex,coord) );
   glVertexAttribPointer( colors_attrib_id, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),  (void*)offsetof(vertex,fill) );

   glEnableVertexAttribArray(vertex_attrib_id);
   glEnableVertexAttribArray(normal_attrib_id);
   glEnableVertexAttribArray(coords_attrib_id);
   glEnableVertexAttribArray(colors_attrib_id);

   glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);

   glVertexAttribIPointer( tindex_attrib_id, 1, GL_INT, 0, 0 );

   glEnableVertexAttribArray(tindex_attrib_id);
   glBindVertexArray(0);
}

void gl_context::drawTriangles( const std::vector<vertex> &vertices,
                                const std::vector<unsigned short> &indices,
                                const PMatrix &move_matrix ){

   reserve( vertices.size(), move_matrix);
   if (indices.size() == 0) abort();

   int offset = tbuffer.size();

   for (int i = 0; i < vertices.size(); ++i) {
      vbuffer[offset + i] = vertices[i];
      tbuffer.push_back( currentM );
   }

   // glBindVertexArray(VAO);
   // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   // glBufferSubData(GL_ARRAY_BUFFER,
   //                 offset * sizeof(vertex),
   //                 vertices.size() * sizeof(vertex),
   //                 vertices.data() );
   // glBindBuffer(GL_ARRAY_BUFFER, 0);

   for (auto index : indices) {
      ibuffer.push_back( offset + index  );
   }
}

void gl_context::drawTrianglesDirect( PFrame &fb,
                                      const std::vector<int> &tindex,
                                      const std::vector<unsigned short> &indices ) {

   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   glBufferSubData(GL_ARRAY_BUFFER, 0, tindex.size() * sizeof(vertex), vbuffer );

   glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, tindex.size() * sizeof(int), tindex.data(), GL_STREAM_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STREAM_DRAW);

   fb.bind();

   glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindVertexArray(0);
}

void gl_context::cleanupVAO() {
   glBindVertexArray(VAO);
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   delete [] vbuffer;
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   glDeleteVertexArrays(1, &VAO);
}

void gl_context::draw_texture_over_framebuffer( const PTexture &texture, PFrame &fb, bool flip ) {

   // Reset to default view & lighting settings to draw buffered frame.
   std::array<float,3> directionLightColor =  { 0.0, 0.0, 0.0 };
   std::array<float,3> directionLightVector = { 0.0, 0.0, 0.0 };
   std::array<float,3> ambientLight =         { 1.0, 1.0, 1.0 };
   std::array<float,3> pointLightColor =      { 0.0, 0.0, 0.0 };
   std::array<float,3> pointLightPosition =   { 0.0, 0.0, 0.0 };
   std::array<float,3> pointLightFalloff =    { 1.0, 0.0, 0.0 };

   loadDirectionLightColor( directionLightColor.data() );
   loadDirectionLightVector( directionLightVector.data() );
   loadAmbientLight( ambientLight.data() );
   loadPointLightColor( pointLightColor.data() );
   loadPointLightPosition( pointLightPosition.data() );
   loadPointLightFalloff( pointLightFalloff.data() );

   PMatrix identity_matrix = PMatrix::Identity();

   loadMoveMatrix( PMatrix::Identity().data() );
   loadProjectionViewMatrix( PMatrix::FlipY().data() );

   std::vector<vertex> vertices;
   if ( flip ) {
      vertices = {
         { {-1.0f,  1.0f}, {}, { texture.nleft(),  texture.nbottom(), (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { { 1.0f,  1.0f}, {}, { texture.nright(), texture.nbottom(), (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { { 1.0f, -1.0f}, {}, { texture.nright(), texture.ntop(),    (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { {-1.0f, -1.0f}, {}, { texture.nleft(),  texture.ntop(),    (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f} }};
   } else {
      vertices = {
         { {-1.0f, -1.0f}, {}, { texture.nleft(),  texture.nbottom(), (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { { 1.0f, -1.0f}, {}, { texture.nright(), texture.nbottom(), (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { { 1.0f,  1.0f}, {}, { texture.nright(), texture.ntop(),    (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f}},
         { {-1.0f,  1.0f}, {}, { texture.nleft(),  texture.ntop(),    (float)texture.layer},  {1.0f, 1.0f, 1.0f, 1.0f} }};
   }

   std::vector<unsigned short>  indices = {
      0,1,2, 0,2,3,
   };

   std::vector<int> mindex(vertices.size(), 0);

   // glBindVertexArray(VAO);
   // glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   // glBufferSubData(GL_ARRAY_BUFFER,
   //                 0,
   //                 vertices.size() * sizeof(vertex),
   //                 vertices.data() );
   // glBindBuffer(GL_ARRAY_BUFFER, 0);

   memcpy( vbuffer, vertices.data(), sizeof(vertex) * 4);
   clear( fb, 0.0, 0.0, 0.0, 1.0);
   drawTrianglesDirect( fb, mindex, indices );
}
