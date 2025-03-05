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
      in int mindex;
      uniform mat4 PVmatrix;
      uniform mat4 Mmatrix[16];
      uniform mat3 Nmatrix[16];

      uniform int lightCount;
      uniform vec4 lightPosition[8];
      uniform vec3 lightNormal[8];
      uniform vec3 lightAmbient[8];
      uniform vec3 lightDiffuse[8];
      uniform vec3 lightSpecular[8];
      uniform vec3 lightFalloff[8];
      uniform vec2 lightSpot[8];

      uniform vec3 eye;

      in vec3 position;
      in vec4 color;
      in vec3 normal;
      in vec2 texCoord;

      in vec4 ambient;
      in vec4 specular;
      in vec4 emissive;
      in float shininess;

      in int tunit;

      flat out int vertTindex;
      out vec3 vertNormal;
      out vec4 vertPosition;

      out vec4 vertColor;
      out vec4 backVertColor;
      out vec4 vertTexCoord;

      const float zero_float = 0.0;
      const float one_float = 1.0;
      const vec3 zero_vec3 = vec3(0);

      float falloffFactor(vec3 lightPos, vec3 vertPos, vec3 coeff) {
         vec3 lpv = lightPos - vertPos;
         vec3 dist = vec3(one_float);
         dist.z = dot(lpv, lpv);
         dist.y = sqrt(dist.z);
         return one_float / dot(dist, coeff);
      }

      float spotFactor(vec3 lightPos, vec3 vertPos, vec3 lightNorm, float minCos, float spotExp) {
         vec3 lpv = normalize(lightPos - vertPos);
         vec3 nln = -one_float * lightNorm;
         float spotCos = dot(nln, lpv);
         return spotCos <= minCos ? zero_float : pow(spotCos, spotExp);
      }

      float lambertFactor(vec3 lightDir, vec3 vecNormal) {
         return max(zero_float, dot(lightDir, vecNormal));
      }

      float blinnPhongFactor(vec3 lightDir, vec3 vertPos, vec3 vecNormal, float shine) {
         vec3 np = normalize(vertPos);
         vec3 ldp = normalize(lightDir - np);
         return pow(max(zero_float, dot(ldp, vecNormal)), shine);
      }

      void main()
      {
          mat4 M = Mmatrix[mindex];
          mat3 N = Nmatrix[mindex];
          vertPosition = M * vec4(position,1.0);
          vertNormal = normalize(N * normal);
          vertTexCoord = vec4(texCoord,1.0,1.0);
          vertTindex = tunit;
          vertColor = color;

          gl_Position = PVmatrix * vertPosition;

          vec3 ecVertex = vertPosition.xyz;
          vec3 ecNormal = vertNormal;
          vec3 ecNormalInv = -vertNormal;

          // Light calculations
          vec3 totalAmbient = vec3(0, 0, 0);

          vec3 totalFrontDiffuse = vec3(0, 0, 0);
          vec3 totalFrontSpecular = vec3(0, 0, 0);

          vec3 totalBackDiffuse = vec3(0, 0, 0);
          vec3 totalBackSpecular = vec3(0, 0, 0);

          for (int i = 0; i < 8; i++) {
             if (lightCount == i) break;

            vec3 lightPos = lightPosition[i].xyz;
            bool isDir = lightPosition[i].w < one_float;
            float spotCos = lightSpot[i].x;
            float spotExp = lightSpot[i].y;

            vec3 lightDir;
            float falloff;
            float spotf;

            if (isDir) {
               falloff = one_float;
               lightDir = -one_float * lightNormal[i];
            } else {
               falloff = falloffFactor(lightPos, ecVertex, lightFalloff[i]);
               lightDir = normalize(lightPos - ecVertex);
            }

            spotf = spotExp > zero_float ? spotFactor(lightPos, ecVertex, lightNormal[i],
                                           spotCos, spotExp)
                                 : one_float;
            if (any(greaterThan(lightAmbient[i], zero_vec3))) {
               totalAmbient       += lightAmbient[i] * falloff;
            }

            // need to deal with direction to eye here
            if (any(greaterThan(lightDiffuse[i], zero_vec3))) {
               totalFrontDiffuse  += lightDiffuse[i] * falloff * spotf *
                                     lambertFactor(lightDir, ecNormal);
               totalBackDiffuse   += lightDiffuse[i] * falloff * spotf *
                                     lambertFactor(lightDir, ecNormalInv);
            }

            if (any(greaterThan(lightSpecular[i], zero_vec3))) {
               totalFrontSpecular += lightSpecular[i] * falloff * spotf *
                                     blinnPhongFactor(lightDir, ecVertex - eye, ecNormal, shininess);
               totalBackSpecular  += lightSpecular[i] * falloff * spotf *
                                     blinnPhongFactor(lightDir, ecVertex - eye, ecNormalInv, shininess);
            }
         }

         // Calculating final color as result of all lights (plus emissive term).
         // Transparency is determined exclusively by the diffuse component.
         vertColor = vec4(totalAmbient, 0) * ambient +
                     vec4(totalFrontDiffuse, 1) * color +
                     vec4(totalFrontSpecular, 0) * specular +
                     vec4(emissive.rgb, 0);

         backVertColor = vec4(totalAmbient, 0) * ambient +
                         vec4(totalBackDiffuse, 1) * color +
                         vec4(totalBackSpecular, 0) * specular +
                         vec4(emissive.rgb, 0);
}
)glsl";

static const char *defaultFragmentShader = R"glsl(
      #version 400

      uniform sampler2D texture[16];
      flat in int vertTindex;

      in vec4 vertColor;
      in vec4 backVertColor;
      in vec4 vertTexCoord;

      out vec4 fragColor;

      void main() {

         if ( vertTindex == -1 ) { // It's a circle
            vec2 pos = vertTexCoord.xy;
            vec2 centre = vec2(0.5,0.5);
            if (distance(pos,centre) > 0.5)
               discard;
            else
               fragColor = gl_FrontFacing ? vertColor : backVertColor;
         } else {
            fragColor = texture2D(texture[vertTindex], vertTexCoord.st) * (gl_FrontFacing ? vertColor : backVertColor);
         }
      }

)glsl";

class PShaderImpl {

   std::map<std::string, PImage> uniformsSampler;
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

   void set(const char *uniform, PImage image);

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
   for (auto& [id, value] : uniformsSampler) {
      if (value.isDirty()) {
         value.updatePixels();
      }
      auto textureID = value.getTextureID();
      shader.set(id.c_str(), textureID);
   }
   shader.set_uniforms();
}

void PShaderImpl::set(const char *id, PImage img) {
   DEBUG_METHOD();
   uniformsSampler[id] = img;
}

void PShaderImpl::set(const char *id, float value) {
   DEBUG_METHOD();
   shader.set(id, value);
}

void PShaderImpl::set(const char *id, float v1, float v2) {
   DEBUG_METHOD();
   shader.set(id,v1,v2);
}

void PShaderImpl::set(const char *id, float v1, float v2, float v3) {
   DEBUG_METHOD();
   shader.set(id, v1, v2, v3);
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

void PShader::set(const char *uniform, PImage image) {
   impl->set( uniform, image );
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
