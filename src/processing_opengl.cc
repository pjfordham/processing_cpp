#include "glad/glad.h"

#include "processing_opengl.h"
#include "processing_debug.h"

#undef DEBUG_METHOD
#undef DEBUG_METHOD_MESSAGE
#define DEBUG_METHOD() do {} while (false)
#define DEBUG_METHOD_MESSAGE(x) do {} while (false)

#include <fstream>     // For std::ifstream
#include <sstream>     // For std::stringstream

namespace gl {

   static color HSBtoRGB(float h, float s, float v, float a)
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

   color flatten_color_mode(::color c) {
      float r = map(c.r,0,::color::scaleR,0,1);
      float g = map(c.g,0,::color::scaleG,0,1);
      float b = map(c.b,0,::color::scaleB,0,1);
      float a = map(c.a,0,::color::scaleA,0,1);
      if (::color::mode == HSB) {
         return HSBtoRGB(r,g,b,a);
      }
      return { r, g, b, a };
   }

   void context::blendMode(int b ) {
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

   void context::drawVAO(std::vector<VAO> &vaos, const glm::mat4 &currentTransform) {
#if 0
      fmt::print("### GEOMETRY DUMP START ###\n");
      int i = 0;
      for (auto &vao : vaos) {
         fmt::print("\n### GEOMETRY DUMP VAO {}   ###\n",i++);
         PMatrix(scene.view_matrix).print();
         for (auto &m : vao.transforms) {
            PMatrix(m).print();
         }
         fmt::print("Vertices: {}\n", vao.vertices.size() );
         for ( int i = 0; i < vao.vertices.size(); ++i ) {
            fmt::print("{:3}: {}\n", i, vao.vertices[i]);
         }
         fmt::print("Triangles: {}\n", vao.indices.size() );
         for ( int i = 0; i < vao.indices.size(); i+=3 ) {
            fmt::print("{},{},{} ", vao.indices[i],vao.indices[i+1],vao.indices[i+2]);
         }
      }
      fmt::print("\n### GEOMETRY DUMP END   ###\n");
#endif


      for (auto &draw: vaos ) {
         loadMoveMatrix( draw.transforms );
         auto scenex = scene;
         scenex.view_matrix = scenex.view_matrix * currentTransform;
         setScene( scenex );
         //  TransformMatrix.set(scene.projection_matrix * scene.view_matrix * draw.transforms[0]);

         for ( int i = 0; i < draw.textures.size() ; ++i ) {
            PImage &img = draw.textures[i];
            if (img != PImage::circle()) {
               if (img.isDirty()) {
                  img.updatePixels();
               }
               glActiveTexture(GL_TEXTURE0 + i);
               auto textureID = img.getTextureID();
               glBindTexture(GL_TEXTURE_2D, img.getTextureID());
            }
         }
         localFrame.bind();

         draw.draw();
      }
   }

   void context::compile(std::vector<VAO> &vaos) {
      for (auto &draw: vaos ) {
         draw.alloc( Position, Normal, Color, Coord, TUnit, MIndex );
         draw.loadBuffers();
      }
   }


   void context::setScene( const scene_t &scene ) {
      loadProjectionViewMatrix( scene.projection_matrix * scene.view_matrix );
      auto id = currentShader.getProgramID();

      if (scene.lights) {
         int numPointLights = (int)scene.pointLightColors.size();
         DirectionLightColor.set( scene.directionLightColor);
         DirectionLightVector.set( scene.directionLightVector );
         AmbientLight.set( scene.ambientLight );
         NumberOfPointLights.set( numPointLights);
         PointLightColor.set( scene.pointLightColors );
         PointLightPosition.set( scene.pointLightPoss );
         PointLightFalloff.set( scene.pointLightFalloff );
      } else {
         int numPointLights = 0;
         glm::vec3 on {1.0, 1.0, 1.0};
         glm::vec3 off {0.0, 0.0, 0.0};
         glm::vec3 unity {1.0, 0.0, 0.0};

         NumberOfPointLights.set( 0 );
         DirectionLightColor.set( off );
         AmbientLight.set( on );
         PointLightFalloff.set( unity );
      }
   }

   void context::hint(int type) {
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

   context::context(int width, int height, float aaFactor) :
      defaultShader( loadShader() ) {
      this->aaFactor = aaFactor;
      this->width = width;
      this->height = height;
      this->window_width = width;
      this->window_height = height;

      // localFrame = framebuffer::constructMainFrame(width, height);
      localFrame = framebuffer( this->width, this->height, aaFactor, MSAA );

      shader( defaultShader );

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);

      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);

   }

   context::~context() {
   }

   void context::draw(PShape shape, const PMatrix &transform) {
      scene.elements.emplace_back( shape, transform );
   }

   PShader context::loadShader(const char *fragShader) {
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

   PShader context::loadShader(const char *fragShader, const char *vertShader) {
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


   void context::flush() {
      DEBUG_METHOD();
      flushes++;
      // This is where we can batch the disaprate shapes into a single VAO
      // VAO optimizations happen here.
#if 1
      std::vector<VAO> vaos;
      for ( auto &element : scene.elements) {
         // Flatten will call drawVAOs
         element.shape.flatten(vaos, element.transform);
      }
      compile(vaos);
      drawVAO( vaos, PMatrix::Identity().glm_data() );
#else
      for ( auto &element : scene.elements) {
         std::vector<VAO> vaos;
         // Flatten will call drawVAOs
         element.shape.flatten(vaos, element.transform);
         compile(vaos);
         drawVAO( vaos, PMatrix::Identity().glm_data() );
      }
#endif
      scene.elements.clear();
      return;
   }

   void context::clear(framebuffer &fb, float r, float g, float b, float a) {
      fb.bind();
      glClearColor(r, g, b, a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void context::clearDepthBuffer(framebuffer &fb) {
      fb.bind();
      glClear(GL_DEPTH_BUFFER_BIT);
   }

   void context::loadMoveMatrix( const std::vector<glm::mat4> &transforms ) {
      if (transforms.size() > 16 || transforms.size() < 1)
         abort();
      Mmatrix.set( transforms );
   }

   void context::loadProjectionViewMatrix( const glm::mat4 &data ) {
      PVmatrix.set( data );
   }

   void VAO::alloc(  attribute Position, attribute Normal, attribute Color,
                     attribute Coord,  attribute TUnit, attribute MIndex) {
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &indexId);
      glGenBuffers(1, &vertexId);
      glBindVertexArray(vao);
      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vertexId);

      Position.bind_vec3( sizeof(vertex), (void*)offsetof(vertex,position) );
      Normal.bind_vec3( sizeof(vertex),  (void*)offsetof(vertex,normal));
      Coord.bind_vec2( sizeof(vertex), (void*)offsetof(vertex,coord));
      Color.bind_vec4( sizeof(vertex), (void*)offsetof(vertex,fill));
      TUnit.bind_int( sizeof(vertex), (void*)offsetof(vertex,tunit));
      MIndex.bind_int( sizeof(vertex), (void*)offsetof(vertex,mindex));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
      glBindVertexArray(0);
   }

   template <typename T>
   static void loadBufferData(GLenum target, GLint bufferId, const std::vector<T> &data, GLenum usage) {
      glBindBuffer(target, bufferId);
      glBufferData(target, data.size() * sizeof(T), data.data(), usage);
      glBindBuffer(target, 0);
   }

   void VAO::loadBuffers() {
      loadBufferData(GL_ARRAY_BUFFER, vertexId, vertices, GL_STREAM_DRAW);
      loadBufferData(GL_ELEMENT_ARRAY_BUFFER, indexId, indices, GL_STREAM_DRAW);
   }

   void VAO::draw() {
      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
      glBindVertexArray(0);
   }

   VAO::~VAO() {
      if (vao) {
         glBindVertexArray(vao);
         glBindBuffer(GL_ARRAY_BUFFER, 0);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
         glBindVertexArray(0);
         glDeleteVertexArrays(1, &vao);
         vao = 0;
         glDeleteBuffers(1, &indexId);
         glDeleteBuffers(1, &vertexId);
      }
   }

   int VAO::hasTexture(PImage texture) {
      auto it = std::find(textures.begin(), textures.end(), texture);
      if (it == textures.end())
         return -1;
      else
         return std::distance(textures.begin(), it) ;
   }

   attribute::attribute(const PShader &pshader, const std::string &attribute) {
      id = pshader.getAttribLocation( attribute.c_str() );
      shaderId = pshader.getProgramID();
   }

   void attribute::bind_vec2(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute::bind_vec3(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute::bind_vec4(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute::bind_int(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribIPointer( id, 1, GL_INT, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   uniform::uniform(const PShader &pshader, const std::string &uniform) {
      id = pshader.getUniformLocation( uniform.c_str() );
   }

   void uniform::set(float value) const {
      if ( id != -1 )
         glUniform1f(id,value);
   }

   void uniform::set(int value) const {
      if ( id != -1 )
         glUniform1i(id,value);
   }

   void uniform::set(const glm::vec2 &value) const {
      if ( id != -1 )
         glUniform2fv(id, 1, glm::value_ptr(value) );
   }

   void uniform::set(const glm::vec3 &value) const {
      if ( id != -1 )
         glUniform3fv(id, 1, glm::value_ptr(value) );
   }

   void uniform::set(const glm::vec4 &value) const {
      if ( id != -1 )
         glUniform4fv(id, 1, glm::value_ptr(value) );
   }

   void uniform::set(const std::vector<int> &value) const {
      if ( id != -1 )
         glUniform1iv(id,value.size(),value.data());
   }

   void uniform::set(const std::vector<glm::vec3> &value) const {
      if ( id != -1 )
         glUniform3fv(id, value.size(), glm::value_ptr(value[0]) );
   }

   void uniform::set(const std::vector<glm::mat4> &value) const {
      if ( id != -1 )
         glUniformMatrix4fv(id, value.size(), false, glm::value_ptr(value[0]) );
   }

   void uniform::set(const glm::mat4 &value) const {
      if ( id != -1 )
         glUniformMatrix4fv(id, 1, false, glm::value_ptr(value) );
   }

} // namespace gl

template <>
struct fmt::formatter<gl::context> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::context& v, FormatContext& ctx) {
      return format_to(ctx.out(), "gl_context");
   }
};

