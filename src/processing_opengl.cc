#include "glad/glad.h"

#include "processing_opengl.h"

#undef DEBUG_METHOD
#undef DEBUG_METHOD_MESSAGE
#define DEBUG_METHOD() do {} while (false)
#define DEBUG_METHOD_MESSAGE(x) do {} while (false)

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

static gl_context::color HSBtoRGB(float h, float s, float v, float a)
{
   int i = floorf(h * 6);
   auto f = h * 6.0 - i;
   auto p = v * (1.0 - s);
   auto q = v * (1.0 - f * s);
   auto t = v * (1.0 - (1.0 - f) * s);

   float r,g,b;
   switch (i % 6) {
   case 0: r = v, g = t, b = p; break;
   case 1: r = q, g = v, b = p; break;
   case 2: r = p, g = v, b = t; break;
   case 3: r = p, g = q, b = v; break;
   case 4: r = t, g = p, b = v; break;
   case 5: r = v, g = p, b = q; break;
   }
   return { r, g, b, a };
}

gl_context::color flatten_color_mode(color c) {
   float r = map(c.r,0,color::scaleR,0,1);
   float g = map(c.g,0,color::scaleG,0,1);
   float b = map(c.b,0,color::scaleB,0,1);
   float a = map(c.a,0,color::scaleA,0,1);
   if (color::mode == HSB) {
      return HSBtoRGB(r,g,b,a);
   }
   return { r, g, b, a };
}

void gl_context::blendMode(int b ) {
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   switch (b) {
   case BLEND:
      glBlendEquation(GL_FUNC_ADD);
      break;
   case ADD:
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
   case SUBTRACT:
      glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
   case DARKEST:
      glBlendEquation(GL_MIN);
      break;
   case LIGHTEST:
      glBlendEquation(GL_MAX);
      break;
   case DIFFERENCE:
      glBlendEquation(GL_FUNC_SUBTRACT);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
   case EXCLUSION:
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      break;
   case MULTIPLY:
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_DST_COLOR, GL_ZERO);
      break;
   case SCREEN:
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
      break;
   case REPLACE:
      glDisable(GL_BLEND);
      break;
   default:
      abort();
   }
}

void gl_context::drawGeometry( const geometry_t &geometry ) {

   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, geometry.vCount * sizeof(vertex), geometry.vbuffer.data(), GL_STREAM_DRAW );

   glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);
   glBufferData(GL_ARRAY_BUFFER, geometry.vCount * sizeof(int), geometry.tbuffer.data(), GL_STREAM_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, 0);


   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.ibuffer.size() * sizeof(unsigned short), geometry.ibuffer.data(), GL_STREAM_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


   localFrame.bind();

#if 0
   fmt::print("### GEOMETRY DUMP START ###\n");
   for ( int i = 0; i < geometry.vCount; ++i ) {
      fmt::print("{:3}: {}", i, geometry.vbuffer[i]);
      fmt::print(" Tr {:3}\n", geometry.tbuffer[i]);
   }
   for ( int i = 0; i < geometry.vCount; ++i ) {
   }
   fmt::print("Triangles: ");
   for ( int i = 0; i < geometry.ibuffer.size(); i+=3 ) {
      fmt::print("{},{},{} ", geometry.ibuffer[i],geometry.ibuffer[i+1],geometry.ibuffer[i+2]);
   }
   fmt::print("\n### GEOMETRY DUMP END   ###\n");
#endif

   glBindVertexArray(VAO);
   glDrawElements(GL_TRIANGLES, geometry.ibuffer.size(), GL_UNSIGNED_SHORT, 0);
   glBindVertexArray(0);
}

void gl_context::setScene( const scene_t &scene ) {
   loadProjectionViewMatrix( (scene.projection_matrix * scene.view_matrix).data() );

   if (scene.lights) {
      int numPointLights = (int)scene.pointLightColors.size();
      glUniform3fv(DirectionLightColor,  1, scene.directionLightColor.data() );
      glUniform3fv(DirectionLightVector, 1, scene.directionLightVector.data() );
      glUniform3fv(AmbientLight,         1, scene.ambientLight.data());
      glUniform1iv(NumberOfPointLights,  1, &numPointLights);
      glUniform3fv(PointLightColor,      numPointLights, scene.pointLightColors[0].data());
      glUniform3fv(PointLightPosition,   numPointLights, scene.pointLightPoss[0].data());
      glUniform3fv(PointLightFalloff,    1, scene.pointLightFalloff.data() );
   } else {
      std::array<float,3> on { 1.0, 1.0, 1.0};
      std::array<float,3> off { 0.0, 0.0, 0.0};
      std::array<float,3> unity { 1.0, 0.0, 0.0 };

      int numPointLights = 0;
      glUniform1iv(NumberOfPointLights,  1, &numPointLights);
      glUniform3fv(DirectionLightColor,  1, off.data() );
      glUniform3fv(AmbientLight,         1, on.data());
      glUniform3fv(PointLightColor,      1, off.data());
      glUniform3fv(PointLightFalloff,    1, unity.data() );
   }
}

void gl_context::hint(int type) {
   flush();
   switch(type) {
   case DISABLE_DEPTH_TEST:
      glDisable(GL_DEPTH_TEST);
      break;
   case ENABLE_DEPTH_TEST:
      glEnable(GL_DEPTH_TEST);
      break;
   case DISABLE_DEPTH_MASK:
      glDepthMask (GL_FALSE);
      break;
   case ENABLE_DEPTH_MASK:
      glDepthMask (GL_TRUE);
      break;
   default:
      break;
   }
}

gl_context::gl_context(int width, int height, float aaFactor) :
   batch(),
   defaultShader( loadShader() ) {
   this->aaFactor = aaFactor;
   this->width = width;
   this->height = height;
   this->window_width = width;
   this->window_height = height;

   // localFrame = gl_framebuffer::constructMainFrame(width, height);
   localFrame = gl_framebuffer( this->width, this->height, aaFactor, MSAA );

   glGenBuffers(1, &index_buffer_id);
   glGenBuffers(1, &vertex_buffer_id);
   glGenBuffers(1, &tindex_buffer_id);

   shader( defaultShader );

   std::array<int, 16> textures = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
   glUniform1iv(uSampler,16,textures.data());

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);

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
   std::ifstream inputFile2("data/"s + vertShader);

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


void gl_context::flush() {
   flushes++;
   batch.draw( this );
   return;
}

template <>
struct fmt::formatter<gl_context> {
    // Format the MyClass object
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const gl_context& v, FormatContext& ctx) {
       return format_to(ctx.out(), "gl_context");
    }
};

int gl_context::bindNextTextureUnit( PImage img ) {
   DEBUG_METHOD();
   if (img.isDirty()) {
      img.updatePixels();
   }
   if (batch.textures.count(img) != 1) {
      glActiveTexture(GL_TEXTURE0 + batch.unit);
      auto textureID = img.getTextureID();
      glBindTexture(GL_TEXTURE_2D, img.getTextureID());
      // This map serves two purposes, first to keep the PImage alive by holding
      // a copy of it until the batch is flushed and secondly mapping from an already
      // used texture to its texture unit in case we see it again. This is paritculaly
      // important for the blankTexture.
      batch.textures[img] = batch.unit;
      DEBUG_METHOD_MESSAGE(fmt::format("cache miss unit={} texID={}", batch.textures[img], img.getTextureID()));
      return batch.unit++;
   } else {
      DEBUG_METHOD_MESSAGE(fmt::format("cache hit! unit={} texID={}", batch.textures[img], img.getTextureID()));
      return batch.textures[img];
   }
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
   movePack.reserve( 16 * mCount );
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

   glVertexAttribPointer( vertex_attrib_id, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,position) );
   glVertexAttribPointer( normal_attrib_id, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,normal) );
   glVertexAttribPointer( coords_attrib_id, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,coord) );
   glVertexAttribIPointer( tunit_attrib_id, 1, GL_INT,             sizeof(vertex), (void*)offsetof(vertex,tunit) );
   glVertexAttribPointer( colors_attrib_id, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,fill) );

   glEnableVertexAttribArray(vertex_attrib_id);
   glEnableVertexAttribArray(normal_attrib_id);
   glEnableVertexAttribArray(coords_attrib_id);
   glEnableVertexAttribArray(colors_attrib_id);
   glEnableVertexAttribArray(tunit_attrib_id);

   glBindBuffer(GL_ARRAY_BUFFER, tindex_buffer_id);

   glVertexAttribIPointer( tindex_attrib_id, 1, GL_INT, 0, 0 );

   glEnableVertexAttribArray(tindex_attrib_id);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
   glBindVertexArray(0);
}

void gl_context::cleanupVAO() {
   if (VAO) {
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glDeleteVertexArrays(1, &VAO);
      VAO = 0;
   }
}

void gl_context::drawTriangles( const std::vector<vertex> &vertices,
                                const std::vector<unsigned short> &indices,
                                PImage texture,
                                const PMatrix &move_matrix ){

   if (indices.size() == 0) abort();

   while (!batch.enqueue( this, vertices, indices, texture, move_matrix ) )
      flush();
}
