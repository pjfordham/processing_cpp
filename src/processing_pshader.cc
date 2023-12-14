#include "processing_pshader.h"
#include "processing_debug.h"
#include <vector>
#include <map>
#include <string>
#include <array>
#include <fmt/core.h>

#include "glad/glad.h"
#include "processing_opengl.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

template <> struct fmt::formatter<PShaderImpl>;

static const char *defaultVertexShader = R"glsl(
      #version 400
      in vec3 position;
      in vec3 normal;
      in vec2 coord;
      in int tunit;
      in vec4 color;
      in int mindex;
      uniform mat4 PVmatrix;
      uniform mat4 Mmatrix[16];
      flat out int vTindex;
      out vec2 vTexture;
      out vec4 vColor;
      out vec3 vNormal;
      out vec4 vPosition;

      void main()
      {
          mat4 M = Mmatrix[mindex];
          vPosition = M * vec4(position,1.0);
          vNormal = normalize((M * (vec4(position,1.0) + vec4(normal,0.0))) - vPosition).xyz;
          vTexture = coord;
          vTindex = tunit;
          vColor = color;

          gl_Position = PVmatrix * vPosition;
       }
)glsl";

static const char *defaultFragmentShader = R"glsl(
      #version 400
      in vec2 vTexture;
      in vec3 vNormal;
      in vec4 vColor;
      in vec4 vPosition;
      flat in int vTindex;
      out vec4 fragColor;
      uniform sampler2D myTextures[16];
      uniform vec3 ambientLight;
      uniform vec3 directionLightColor;
      uniform vec3 directionLightVector;
      uniform int  numberOfPointLights;
      uniform vec3 pointLightColor[8];
      uniform vec3 pointLightPosition[8];
      uniform vec3 pointLightFalloff;
      void main()
      {
          vec3 totalPointLight = vec3(0.0); // Accumulate point light contribution
          for (int i = 0; i < numberOfPointLights; ++i) {
              vec3 pointLightDirection = vPosition.xyz - pointLightPosition[i];
              float d = length(pointLightDirection);
              float pointLightIntensity = 1.0 / (pointLightFalloff.x + pointLightFalloff.y * d + pointLightFalloff.z * d * d);
              float pointLight = max(dot(vNormal, normalize(-pointLightDirection)), 0.0) * pointLightIntensity;
              totalPointLight += pointLightColor[i] * pointLight;
         }

          float directional = max(dot(vNormal, -directionLightVector), 0.0);
          vec3 vLighting = ambientLight + (directionLightColor * directional) + totalPointLight;

          vec4 texelColor;
          if ( vTindex == -1 ) { // It's a circle
              vec2 pos = vTexture.xy;
              vec2 centre = vec2(0.5,0.5);
               if (distance(pos,centre) > 0.5)
                   discard;
               else
                   texelColor = vec4(1.0,1.0,1.0,1.0);
          } else {
              texelColor =  texture(myTextures[vTindex], vTexture.xy);
          }
          fragColor = vec4(vLighting,1.0) * vColor * texelColor;
      }
)glsl";

class PShaderImpl {

   std::map<std::string, glm::vec3> uniforms3fv;
   std::map<std::string, glm::vec2> uniforms2fv;
   std::map<std::string, float>     uniforms1f;

   std::map<std::string, GLuint> attribLocation;
   std::map<std::string, GLuint> uniformLocation;

   std::string vertexShader;
   std::string fragmentShader;

public:
   GLuint programID;

   ~PShaderImpl();

   void compileShaders();

public:
   PShaderImpl(GLuint parent, const char *vertSource, const char *fragSource);

   void releaseShaders();

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);

   GLuint getAttribLocation(const char *attribute) const;

   GLuint getUniformLocation(const char *uniform) const;

   void useProgram();
   friend struct fmt::formatter<PShaderImpl>;

   void enumerateAttributes();
   void enumerateUniforms();

};

static std::vector<std::weak_ptr<PShaderImpl>> &shaderHandles() {
   static std::vector<std::weak_ptr<PShaderImpl>> handles;
   return handles;
}

static void PShader_releaseAllShaders() {
   for (auto i : shaderHandles()) {
      if (auto p = i.lock()) {
         p->releaseShaders();
      }
   }
}

PShaderImpl::PShaderImpl(GLuint parent, const char *vertSource,
                         const char *fragSource)
   : vertexShader(vertSource), fragmentShader(fragSource), programID(0) {
   DEBUG_METHOD();
}

PShaderImpl::~PShaderImpl() {
   DEBUG_METHOD();
   if (programID) {
      glDeleteProgram(programID);
   }
}

void PShaderImpl::set_uniforms() {
   DEBUG_METHOD();
   for (const auto& [id, value] : uniforms1f) {
      GLint loc = getUniformLocation( id.c_str() );
      if ( loc != -1 )
         glUniform1f(loc,value);
   }
   for (const auto& [id, value] : uniforms2fv) {
      GLint loc = getUniformLocation( id.c_str() );
      if ( loc != -1 )
         glUniform2fv(loc, 1, glm::value_ptr(value) );
   }
   for (const auto& [id, value] : uniforms3fv) {
      GLint loc = getUniformLocation( id.c_str() );
      if ( loc != -1 )
         glUniform3fv(loc, 1, glm::value_ptr(value) );
   }
}

void PShaderImpl::set(const char *id, float value) {
   DEBUG_METHOD();
   uniforms1f[id] = value;
}

void PShaderImpl::set(const char *id, float v1, float v2) {
   DEBUG_METHOD();
   uniforms2fv[id] = {v1,v2};
}

void PShaderImpl::set(const char *id, float v1, float v2, float v3) {
   DEBUG_METHOD();
   uniforms3fv[id] = {v1, v2, v3};
}

GLuint PShaderImpl::getAttribLocation(const char *attribute) const {
   DEBUG_METHOD();
   return glGetAttribLocation(programID, attribute);
}

GLuint PShaderImpl::getUniformLocation(const char *uniform) const {
   DEBUG_METHOD();
   return glGetUniformLocation(programID, uniform);
}

void PShaderImpl::useProgram() {
   DEBUG_METHOD();
   glUseProgram(programID);
}

void PShaderImpl::compileShaders() {
   DEBUG_METHOD();
   // Create the shaders
   GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   const char * vertex = vertexShader.c_str();
   const char * fragment = fragmentShader.c_str();

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

   enumerateUniforms();
   enumerateAttributes();
}

void PShaderImpl::enumerateUniforms() {
   DEBUG_METHOD();
   GLint size; // size of the variable
   GLenum type; // type of the variable (float, vec3 or mat4, etc)

   const GLsizei bufSize = 64; // maximum name length
   GLchar name[bufSize]; // variable name in GLSL
   GLsizei length; // name length

   GLint count;
   glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

   for (int i = 0; i < count; i++) {
      glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name);
      uniformLocation[name] = glGetUniformLocation(programID, name);
   }
}

void PShaderImpl::enumerateAttributes() {
   DEBUG_METHOD();
   GLint size; // size of the variable
   GLenum type; // type of the variable (float, vec3 or mat4, etc)

   const GLsizei bufSize = 64; // maximum name length
   GLchar name[bufSize]; // variable name in GLSL
   GLsizei length; // name length

   GLint count;
   glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &count);

   for (int i = 0; i < count; i++) {
      glGetActiveAttrib(programID, (GLuint)i, bufSize, &length, &size, &type, name);
      attribLocation[name] = glGetAttribLocation(programID, name);
   }

}

void PShaderImpl::releaseShaders() {
   DEBUG_METHOD();
   if ( programID ) {
      glDeleteProgram( programID );
      programID = 0;
   }
}

template <>
struct fmt::formatter<PShaderImpl> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const PShaderImpl& v, FormatContext& ctx) {
       return format_to(ctx.out(), "programID={:<4} vertexShader={:<25} fragmentShader={:<25} numUniforms={:4} numAttributes={:4}",
                        v.programID, (void*)&v.vertexShader, (void*)&v.fragmentShader, v.uniformLocation.size(), v.attribLocation.size());
    }
};

PShader::PShader(GLuint parent, const char *vertSource, const char *fragSource)
   : impl( std::make_shared<PShaderImpl>(parent, vertSource, fragSource) ) {
   shaderHandles().push_back( impl );
}

PShader::PShader(GLuint parent, const char *fragSource)
   : impl( std::make_shared<PShaderImpl>(parent, defaultVertexShader, fragSource) ) {
   shaderHandles().push_back( impl );
}

PShader::PShader(GLuint parent)
   : impl( std::make_shared<PShaderImpl>(parent, defaultVertexShader, defaultFragmentShader) ) {
   shaderHandles().push_back( impl );
}

void PShader::compileShaders() {
   impl->compileShaders();
}

void PShader::set_uniforms() {
   impl->set_uniforms();
}

void PShader::set(const char *uniform, float value) {
   impl->set( uniform, value );
}

void PShader::set(const char *uniform, float v1, float v2) {
   impl->set( uniform, v1, v2 );
}

void PShader::set(const char *uniform, float v1, float v2, float v3) {
   impl->set( uniform, v1, v2, v3 );
}

GLuint PShader::getAttribLocation(const char *attribute) const {
   return impl->getAttribLocation( attribute );
}

GLuint PShader::getUniformLocation(const char *uniform) const {
   return impl->getUniformLocation( uniform );
}

void PShader::useProgram() {
   impl->useProgram();
}

GLuint PShader::getProgramID() const {
   return impl->programID;
}

void PShader::init() {
}

void PShader::close() {
   PShader_releaseAllShaders();
}

gl::uniform PShader::get_uniform(const std::string &uniform_name) const {
   return {*this, uniform_name};
}

gl::attribute PShader::get_attribute(const std::string &attribute_name) const {
   return {*this, attribute_name};
}
