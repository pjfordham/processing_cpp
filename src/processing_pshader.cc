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

static const char *flatVertexShader = R"glsl(
      #version 400
      in vec3 position;
      in vec4 color;
      in int mindex;
      uniform mat4 PVmatrix;
      uniform mat4 Mmatrix[16];
      out vec4 vertColor;

      void main()
      {
          gl_Position = PVmatrix * Mmatrix[mindex] * vec4(position,1.0);
          vertColor = color;
       }
)glsl";

static const char *flatFragmentShader = R"glsl(
      #version 400
      in vec4 vertColor;
      out vec4 fragColor;
      void main()
      {
          fragColor = vertColor;
      }
)glsl";

static const char *defaultVertexShader = R"glsl(
      #version 400
      in vec3 position;
      in vec3 normal;
      in vec2 texCoord;
      in int tunit;
      in vec4 color;
      in int mindex;
      uniform mat4 PVmatrix;
      uniform mat4 Mmatrix[16];
      flat out int vertTindex;
      out vec4 vertTexCoord;
      out vec4 vertColor;
      out vec3 vertNormal;
      out vec4 vertPosition;

      void main()
      {
          mat4 M = Mmatrix[mindex];
          vertPosition = M * vec4(position,1.0);
          vertNormal = normalize((M * (vec4(position,1.0) + vec4(normal,0.0))) - vertPosition).xyz;
          vertTexCoord = vec4(texCoord,1.0,1.0);
          vertTindex = tunit;
          vertColor = color;

          gl_Position = PVmatrix * vertPosition;
       }
)glsl";

static const char *defaultFragmentShader = R"glsl(
      #version 400
      in vec4 vertTexCoord;
      in vec3 vertNormal;
      in vec4 vertColor;
      in vec4 vertPosition;
      flat in int vertTindex;
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
              vec3 pointLightDirection = vertPosition.xyz - pointLightPosition[i];
              float d = length(pointLightDirection);
              float pointLightIntensity = 1.0 / (pointLightFalloff.x + pointLightFalloff.y * d + pointLightFalloff.z * d * d);
              float pointLight = max(dot(vertNormal, normalize(-pointLightDirection)), 0.0) * pointLightIntensity;
              totalPointLight += pointLightColor[i] * pointLight;
         }

          float directional = max(dot(vertNormal, -directionLightVector), 0.0);
          vec3 vLighting = ambientLight + (directionLightColor * directional) + totalPointLight;

          vec4 texelColor;
          if ( vertTindex == -1 ) { // It's a circle
              vec2 pos = vertTexCoord.xy;
              vec2 centre = vec2(0.5,0.5);
               if (distance(pos,centre) > 0.5)
                   discard;
               else
                   texelColor = vec4(1.0,1.0,1.0,1.0);
          } else {
              texelColor =  texture(myTextures[vertTindex], vertTexCoord.xy);
          }
          fragColor = vec4(vLighting,1.0) * vertColor * texelColor;
      }
)glsl";

class PShaderImpl {

   std::map<std::string, glm::vec3> uniforms3fv;
   std::map<std::string, glm::vec2> uniforms2fv;
   std::map<std::string, float>     uniforms1f;

   gl::shader_t shader;
public:

   ~PShaderImpl();
   PShaderImpl(GLuint parent, const char *vertSource, const char *fragSource);

   const gl::shader_t &getShader() {
      return shader;
   }

   void releaseShaders();

   void bind();

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);

   friend struct fmt::formatter<PShaderImpl>;

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
   : shader(vertSource, fragSource) {
   DEBUG_METHOD();
}

PShaderImpl::~PShaderImpl() {
   DEBUG_METHOD();
}

void PShaderImpl::bind() {
   DEBUG_METHOD();
   shader.bind();
}

void PShaderImpl::set_uniforms() {
   DEBUG_METHOD();
   for (const auto& [id, value] : uniforms1f) {
      gl::uniform loc = shader.get_uniform( id.c_str() );
      loc.set( value );
   }
   for (const auto& [id, value] : uniforms2fv) {
      gl::uniform loc = shader.get_uniform( id.c_str() );
      loc.set( value );
   }
   for (const auto& [id, value] : uniforms3fv) {
      gl::uniform loc = shader.get_uniform( id.c_str() );
      loc.set( value );
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



void PShaderImpl::releaseShaders() {
   DEBUG_METHOD();
   shader = {};
}

template <>
struct fmt::formatter<PShaderImpl> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const PShaderImpl& v, FormatContext& ctx) {
       return format_to(ctx.out(), "nothing");
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

const gl::shader_t &PShader::getShader() const {
   return impl->getShader();
}

void PShader::set_uniforms() {
   impl->set_uniforms();
}

void PShader::bind() {
   impl->bind();
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

void PShader::init() {
}

void PShader::close() {
   PShader_releaseAllShaders();
}

PShader flatShader() {
   return {0, flatVertexShader, flatFragmentShader };
};

PShader loadShader() {
   return { 0, defaultVertexShader, defaultFragmentShader };
}

PShader loadShader(const char *fragShader) {
   using namespace std::literals;

   std::ifstream inputFile("data/"s + fragShader);

   if (!inputFile.is_open()) {
      abort();
   }

   std::stringstream buffer;
   buffer << inputFile.rdbuf();

   inputFile.close();

   return { 0, defaultVertexShader, buffer.str().c_str() };
}

PShader loadShader(const char *fragShader, const char *vertShader) {
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

   return { 0, buffer2.str().c_str(), buffer.str().c_str() };
}
