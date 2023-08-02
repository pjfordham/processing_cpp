#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_opengl.h"

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>

void gl_context::drawGeometry( const geometry_t &geometry, GLuint bufferID ) {
   glBindVertexArray(VAO);

   glBindTexture(GL_TEXTURE_2D, bufferID);
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   glBufferSubData(GL_ARRAY_BUFFER, 0, geometry.vCount * sizeof(vertex), geometry.vbuffer.data() );

   glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, geometry.vCount * sizeof(int), geometry.tbuffer.data(), GL_STREAM_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.iCount * sizeof(unsigned short), geometry.ibuffer.data(), GL_STREAM_DRAW);

   localFrame.bind();

#if 0
   fmt::print("### GEOMETRY DUMP START ###\n");
   for ( int i = 0; i < geometry->vCount; ++i ) {
      fmt::print("{}: ", i);
      geometry->vbuffer[i].position.print();
   }
   for ( int i = 0; i < geometry->iCount; ++i ) {
      fmt::print("{}",  geometry->ibuffer[i]);
   }
   fmt::print("\n### GEOMETRY DUMP END   ###\n");
#endif

   glDrawElements(GL_TRIANGLES, geometry.iCount, GL_UNSIGNED_SHORT, 0);

   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glBindVertexArray(0);
}

void gl_context::setScene( const scene_t &scene ) {
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

gl_context::gl_context(int width, int height, float aaFactor) : batch( width * aaFactor, height * aaFactor ) {
   this->aaFactor = aaFactor;
   this->width = width * aaFactor;
   this->height = height * aaFactor;
   this->window_width = width;
   this->window_height = height;

   windowFrame = gl_framebuffer::constructMainFrame( window_width, window_height );

   bool useMainFramebuffer = false;

   localFrame = gl_framebuffer( this->width, this->height );

   glGenBuffers(1, &index_buffer_id);
   glGenBuffers(1, &vertex_buffer_id);
   glGenBuffers(1, &tindex_buffer_id);

   defaultShader = loadShader();
   shader( defaultShader );

   int textureUnitIndex = 0;
   glUniform1i(uSampler,textureUnitIndex);
   glActiveTexture(GL_TEXTURE0 + textureUnitIndex);

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

void gl_context::draw_main() {
   // Already drawn directly to framebuffer so we don't need to do anything
   if (!localFrame.isMainFrame()) {
      localFrame.blit( windowFrame );
   }
}

gl_context::~gl_context() {
   cleanupVAO();
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

   gl_framebuffer frame(wfWidth, wfHeight);

   localFrame.blit( frame );

   // Create SDL surface from framebuffer data
   SDL_Surface* surface = SDL_CreateRGBSurface(0, wfWidth, wfHeight,
                                               24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
   frame.bind();
   glReadPixels(0, 0, wfWidth, wfHeight, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

   // Save the image as PNG
   IMG_SavePNG(surface, fileName.c_str());

   // Cleanup
   SDL_FreeSurface(surface);
}

void gl_context::loadPixels( std::vector<unsigned int> &pixels ) {

   int wfWidth = windowFrame.getWidth();
   int wfHeight = windowFrame.getHeight();

   gl_framebuffer frame(wfWidth, wfHeight);

   localFrame.blit( frame );

   pixels.resize(width*height);
   frame.bind();
   glReadPixels(0, 0, wfWidth, wfHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
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

// We need to handle textures over flushes.
PTexture gl_context::getTexture( int width, int height, void *pixels ) {
   glBindTexture(GL_TEXTURE_2D, batch.tm.getTextureID() );
   PTexture texture = batch.tm.getFreeBlock(width, height);
   glTexSubImage2D(GL_TEXTURE_2D, 0,
                   texture.left, texture.top,
                   width, height,
                   GL_RGBA, GL_UNSIGNED_BYTE, pixels);
   return texture;
}

PTexture gl_context::getTexture( gl_context &source ) {
   PTexture texture = batch.tm.getFreeBlock(source.width, source.height);
   GLuint source_texture = source.localFrame.getColorBufferID();
   GLuint target_texture = batch.tm.getTextureID();
   glCopyImageSubData(source_texture, GL_TEXTURE_2D, 0, 0, 0, 0,
                      target_texture, GL_TEXTURE_2D, 0, texture.left, texture.top, 0,
                      source.width, source.height, 1);
   return texture;
}

void gl_context::clear(gl_framebuffer &fb, float r, float g, float b, float a) {
   fb.bind();
   glClearColor(r, g, b, a);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_context::clearDepthBuffer(gl_framebuffer &fb) {
   fb.bind();
   glClear(GL_DEPTH_BUFFER_BIT);
}

void gl_context::loadMoveMatrix( const std::array<PMatrix,16> &transforms, int mCount ) {
   std::vector<float> movePack;
   for (int j = 0 ; j < mCount; j++ ) {
      for (int i = 0; i < 16; i++) {
         movePack.push_back(transforms[j].data()[i]);
      }
   }
   glUniformMatrix4fv(Mmatrix, mCount, false, movePack.data() );
}

void gl_context::loadProjectionViewMatrix( const float *data ) {
   glUniformMatrix4fv(PVmatrix, 1,false, data );
}

void gl_context::initVAO() {
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, geometry_t::CAPACITY * sizeof(vertex), nullptr, GL_STREAM_DRAW);

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

   if (indices.size() == 0) abort();

   while (!batch.enqueue( vertices, indices, move_matrix ) )
      batch.draw( this );
}

void gl_context::cleanupVAO() {
   glBindVertexArray(VAO);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   glDeleteVertexArrays(1, &VAO);
}
