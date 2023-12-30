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

   void batch_t::draw( const glm::mat4 &transform ) {
      std::vector<glm::mat4> transforms;
      transforms.push_back(transform);
      for (auto &draw: vaos ) {
         Mmatrix.set( transforms );

         for ( int i = 0; i < draw.textures.size() ; ++i ) {
            PImage &img = draw.textures[i];
            if (img != PImage::circle()) {
               // Set this here so if updatePixels needs to create
               // a new texture have the correct unit bound already.
               glActiveTexture(GL_TEXTURE0 + i);
               if (img.isDirty()) {
                  img.updatePixels();
               }
               auto textureID = img.getTextureID();
               glBindTexture(GL_TEXTURE_2D, img.getTextureID());
            }
         }
         draw.bind( Position, Normal, Color, Coord, TUnit, MIndex );
         draw.draw();
      }
   }

   void batch_t::draw() {
#if 0
      fmt::print("### GEOMETRY DUMP START ###\n");
      int i = 0;
      for (auto &vao : vaos) {
         fmt::print("\n### GEOMETRY DUMP VAO {}   ###\n",i++);
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
         Mmatrix.set( draw.transforms );

         for ( int i = 0; i < draw.textures.size() ; ++i ) {
            PImage &img = draw.textures[i];
            if (img != PImage::circle()) {
               // Set this here so if updatePixels needs to create
               // a new texture have the correct unit bound already.
               glActiveTexture(GL_TEXTURE0 + i);
               if (img.isDirty()) {
                  img.updatePixels();
               }
               auto textureID = img.getTextureID();
               glBindTexture(GL_TEXTURE_2D, img.getTextureID());
            }
         }
         draw.draw();
      }
   }

   size_t batch_t::size() {
      size_t s = 0;
      for (auto &draw: vaos ) {
         s += vaos.size();
      }
      return s;
   }

   void batch_t::compile() {
      for (auto &draw: vaos ) {
         draw.bind( Position, Normal, Color, Coord, TUnit, MIndex );
         draw.loadBuffers();
      }
   }

   bool batch_t::usesCircles() const {
      for (auto &draw: vaos ) {
         if (std::find(draw.textures.begin(), draw.textures.end(), PImage::circle()) != draw.textures.end())
         {
            return true;
         }
      }
      return false;
   }

   bool batch_t::usesTextures() const {
      for (auto &draw: vaos ) {
         // This could be a better test
         if (!(draw.textures.size() == 1 && draw.textures[0] != PImage::circle() &&
               draw.textures[0].width == 1 && draw.textures[0].height == 1)) {
            return true;
         }
      }
      return false;
   }

   void batch_t::clear() {
      vaos.clear();
   }

   void scene_t::set() {
      PVmatrix.set( projection_matrix * view_matrix  );
      if (lights) {
         int numPointLights = (int)pointLightColors.size();
         DirectionLightColor.set( directionLightColor);
         DirectionLightVector.set( directionLightVector );
         AmbientLight.set( ambientLight );
         NumberOfPointLights.set( numPointLights);
         PointLightColor.set( pointLightColors );
         PointLightPosition.set( pointLightPoss );
         PointLightFalloff.set( pointLightFalloff );
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

   shader_t::shader_t(const char *vertex, const char *fragment) {
      // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

      glShaderSource(VertexShaderID, 1, &vertex , NULL);
      glCompileShader(VertexShaderID);

      glShaderSource(FragmentShaderID, 1, &fragment , NULL);
      glCompileShader(FragmentShaderID);

      GLint Result = GL_FALSE;
      int InfoLogLength;

      // Check Vertex Shader
      glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
         glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
         fmt::print("{}\n", &VertexShaderErrorMessage[0]);
      }

      // Check Fragment Shader
      glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
         glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
         fmt::print("{}\n", &FragmentShaderErrorMessage[0]);
      }

      // Link the program
      programID = glCreateProgram();
      glAttachShader(programID, VertexShaderID);
      glAttachShader(programID, FragmentShaderID);
      glLinkProgram(programID);

      // Check the program
      glGetProgramiv(programID, GL_LINK_STATUS, &Result);
      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> ProgramErrorMessage(InfoLogLength+1);
         glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
         fmt::print("{}\n", &ProgramErrorMessage[0]);
      }

      glDetachShader(programID, VertexShaderID);
      glDetachShader(programID, FragmentShaderID);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);

   }

   shader_t::~shader_t() {
      if (programID) {
         glDeleteProgram(programID);
      }
   }

   void context::hint(int type) {
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

   context::context() {}

   void context::init() {
      blendMode( BLEND );
      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);
   }

   context::~context() {
   }

   void shader_t::bind() const {
      glUseProgram(programID);
   }

   VAO::VAO() noexcept {
      DEBUG_METHOD();
      vertices.reserve(65536);
      indices.reserve(65536);
      textures.reserve(16);
      transforms.reserve(16);
      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &indexId);
      glGenBuffers(1, &vertexId);
   }

   VAO::VAO(VAO&& x) noexcept : VAO() {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   VAO& VAO::operator=(VAO&& other) noexcept {
      DEBUG_METHOD();
      std::swap(vao, other.vao);
      std::swap(indexId, other.indexId);
      std::swap(vertexId, other.vertexId);
      std::swap(vertices, other.vertices);
      std::swap(indices, other.indices);
      std::swap(textures, other.textures);
      std::swap(transforms, other.transforms);
      return *this;
   }

   void VAO::bind( attribute Position, attribute Normal, attribute Color,
                   attribute Coord,  attribute TUnit, attribute MIndex) {
      DEBUG_METHOD();

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vertexId);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);

      Position.bind_vec3( sizeof(vertex), (void*)offsetof(vertex,position) );
      Normal.bind_vec3( sizeof(vertex),  (void*)offsetof(vertex,normal));
      Coord.bind_vec2( sizeof(vertex), (void*)offsetof(vertex,coord));
      Color.bind_vec4( sizeof(vertex), (void*)offsetof(vertex,fill));
      TUnit.bind_int( sizeof(vertex), (void*)offsetof(vertex,tunit));
      MIndex.bind_int( sizeof(vertex), (void*)offsetof(vertex,mindex));

      glBindVertexArray(0);
   }

   template <typename T>
   static void loadBufferData(GLenum target, GLint bufferId, const std::vector<T> &data, GLenum usage) {
      glBindBuffer(target, bufferId);
      glBufferData(target, data.size() * sizeof(T), data.data(), usage);
   }

   void VAO::loadBuffers() const {
      DEBUG_METHOD();
      loadBufferData(GL_ARRAY_BUFFER, vertexId, vertices, GL_STREAM_DRAW);
      loadBufferData(GL_ELEMENT_ARRAY_BUFFER, indexId, indices, GL_STREAM_DRAW);
   }

   void VAO::draw() const {
      DEBUG_METHOD();
      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
      glBindVertexArray(0);
   }

   VAO::~VAO() {
      DEBUG_METHOD();
      if (vao) {
         glBindVertexArray(vao);
         glBindBuffer(GL_ARRAY_BUFFER, 0);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
         glBindVertexArray(0);
         glDeleteVertexArrays(1, &vao);
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

   attribute::attribute(GLuint shaderID, const std::string &attribute) {
      id = glGetAttribLocation(shaderID, attribute.c_str());
      shaderId = shaderID;
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

   uniform::uniform(GLuint programID, const std::string &uniform) {
      id = glGetUniformLocation(programID, uniform.c_str());
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

   void enumerateUniforms(GLuint programID) {
      GLint size; // size of the variable
      GLenum type; // type of the variable (float, vec3 or mat4, etc)

      const GLsizei bufSize = 64; // maximum name length
      GLchar name[bufSize]; // variable name in GLSL
      GLsizei length; // name length

      GLint count;
      glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

      for (int i = 0; i < count; i++) {
         glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name);
         // uniformLocation[name] = glGetUniformLocation(programID, name);
      }
   }

   void enumerateAttributes(GLuint programID) {
      GLint size; // size of the variable
      GLenum type; // type of the variable (float, vec3 or mat4, etc)

      const GLsizei bufSize = 64; // maximum name length
      GLchar name[bufSize]; // variable name in GLSL
      GLsizei length; // name length

      GLint count;
      glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &count);

      for (int i = 0; i < count; i++) {
         glGetActiveAttrib(programID, (GLuint)i, bufSize, &length, &size, &type, name);
         // attribLocation[name] = glGetAttribLocation(programID, name);
      }

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

template <>
struct fmt::formatter<gl::VAO> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::VAO& v, FormatContext& ctx) {
      return format_to(ctx.out(), "VAO:{:2} VID:{:2} IID:{:2} V{:8} I{:8} Tx{:2} Tr{:2}",
                       v.vao, v.vertexId, v.indexId,
                       v.vertices.size(), v.indices.size(), v.textures.size(), v.transforms.size());
   }
};
