#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <vector>
#include "processing_math.h"
#include "processing_pshader.h"
#include "processing_color.h"
#include "processing_texture_manager.h"
#include <fmt/core.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;

class PFrame {
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

   static PFrame constructMainFrame(int width, int height) {
      PFrame frame;
      frame.id = 0;
      frame.width = width;
      frame.height = height;
      return frame;
   }

   PFrame() {
   }

   PFrame(int width_, int height_);

   PFrame( int width_, int height_, GLuint colorBufferID, int layer );

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

   ~PFrame();

   void bind();
};

class gl_context {

   const int CAPACITY = 65536;

   SDL_Window *window = NULL;
   SDL_Renderer *renderer =NULL;
   void *glContext = NULL;

   int currentM;
   int flushes = 0;

   int width;
   int height;
   int window_width;
   int window_height;
   TextureManager tm;
   std::vector<PMatrix> move;
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
   GLuint PVmatrix;
   GLuint uSampler;
   GLuint AmbientLight;
   GLuint DirectionLightColor;
   GLuint DirectionLightVector;
   GLuint PointLightColor;
   GLuint PointLightPosition;
   GLuint PointLightFalloff;

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
      std::swap(PVmatrix,x.PVmatrix);
      std::swap(uSampler,x.uSampler);
      std::swap(AmbientLight,x.AmbientLight);
      std::swap(DirectionLightColor,x.DirectionLightColor);
      std::swap(DirectionLightVector,x.DirectionLightVector);
      std::swap(PointLightPosition,x.PointLightPosition);
      std::swap(PointLightColor,x.PointLightColor);
      std::swap(PointLightFalloff,x.PointLightFalloff);

      return *this;
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

   void clear(PFrame &fb, float r, float g, float b, float a);

   void clear( float r, float g, float b, float a) {
      clear( localFrame, r, g, b, a);
   }

   void clearDepthBuffer(PFrame &fb);

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

   void loadMoveMatrix( float *data, int size = 1 );

   void loadProjectionViewMatrix( float *data );

   void loadDirectionLightColor( float *data );

   void loadDirectionLightVector( float *data );

   void loadAmbientLight( float *data );

   void loadPointLightColor( float *data );

   void loadPointLightPosition( float *data );

   void loadPointLightFalloff( float *data );

   void reserve(int n_vertices, const PMatrix &move_matrix, const PMatrix& projection_matrix, const PMatrix &view_matrix,
                float *directionLightColor, float *directionLightVector, float *ambientLight,
                float *pointLightColor, float *pointLightPosition, float *pointLightFalloff) {
      if (n_vertices > CAPACITY) {
         abort();
      } else {
         int new_size = n_vertices + ibuffer.size();
         if (new_size >= CAPACITY) {
            flush(projection_matrix, view_matrix, directionLightColor, directionLightVector, ambientLight,
                  pointLightColor, pointLightPosition, pointLightFalloff);
         }
      }
      if ( move.size() == 16) {
         flush(projection_matrix, view_matrix, directionLightColor, directionLightVector, ambientLight,
               pointLightColor, pointLightPosition, pointLightFalloff);
      }
      if ( move.size() > 0 && move_matrix == move.back() ) {
      } else {
         move.push_back( move_matrix );
         currentM = move.size() - 1;
      }
   }

   void flush( const PMatrix &projection_matrix, const PMatrix &view_matrix,
               float *directionLightColor, float *directionLightVector, float *ambientLight,
               float *pointLightColor, float *pointLightPosition, float *pointLightFalloff) {

      flushes++;
      // if (flushes > 1000) abort();

      if (vbuffer.size() != 0 ) {

         // Push back array of Ms
         std::vector<float> movePack;
         for (const auto& mat : move) {
            for (int i = 0; i < 16; i++) {
               movePack.push_back(mat.data[i]);
            }
         }

         loadMoveMatrix( movePack.data(), move.size() );
         loadProjectionViewMatrix( (PMatrix::FlipY() * projection_matrix * view_matrix).data );
         loadDirectionLightColor( directionLightColor );
         loadDirectionLightVector( directionLightVector );
         loadAmbientLight( ambientLight );
         loadPointLightColor( pointLightColor );
         loadPointLightPosition( pointLightPosition );
         loadPointLightFalloff( pointLightFalloff );

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

   struct vertex {
      PVector position;
      PVector normal;
      PVector coord;
      color fill;
   };

   void drawTriangles( const std::vector<vertex> &vertices,
                       const std::vector<unsigned short> &indices ){

      if (indices.size() == 0) abort();

      auto starti = vbuffer.size();

      for (auto vertex : vertices) {
         vbuffer.push_back( vertex.position );
         nbuffer.push_back( vertex.normal );
         cbuffer.push_back( vertex.coord );
         xbuffer.push_back( vertex.fill.r / 255.0 );
         xbuffer.push_back( vertex.fill.g / 255.0 );
         xbuffer.push_back( vertex.fill.b / 255.0 );
         xbuffer.push_back( vertex.fill.a / 255.0 );
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
                             int count );

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

   void draw_texture_over_framebuffer( const PTexture &texture, PFrame &fb, bool flip = false) {

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

      loadMoveMatrix( PMatrix::Identity().data );
      loadProjectionViewMatrix( PMatrix::FlipY().data );

      std::vector<PVector> vertices;
      if ( flip ) {
         vertices = {
            {-1.0f,  1.0f},
            { 1.0f,  1.0f},
            { 1.0f, -1.0f},
            {-1.0f, -1.0f}};
      } else {
         vertices = {
            {-1.0f, -1.0f},
            { 1.0f, -1.0f},
            { 1.0f,  1.0f},
            {-1.0f,  1.0f}};
      }

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

   void draw_main();

};


#endif
