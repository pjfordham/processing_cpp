#include "processing_pshader.h"
#include <vector>
#include <fmt/core.h>

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

PShader::~PShader() {
   if (programID) {
      glDeleteProgram(programID);
   }
}

void PShader::set_uniforms() {
   for (const auto& [id, value] : uniforms1f) {
      glUniform1f(id,value);
   }
   for (const auto& [id, value] : uniforms2fv) {
      glUniform2fv(id,1,value.data());
   }
   for (const auto& [id, value] : uniforms3fv) {
      glUniform3fv(id,1,value.data());
   }
}

void PShader::set(const char *uniform, float value) {
   GLuint id = glGetUniformLocation(programID, uniform);
   uniforms1f[id] = value;
}

void PShader::set(const char *uniform, float v1, float v2) {
   std::array<float,2> vec = {v1,v2};
   GLuint id = glGetUniformLocation(programID, uniform);
   uniforms2fv[id] = vec;
}

void PShader::set(const char *uniform, float v1, float v2, float v3) {
   std::array<float,3> vec = {v1,v2,v3};
   GLuint id = glGetUniformLocation(programID, uniform);
   uniforms3fv[id] = vec;
}

GLuint PShader::getAttribLocation(const char *attribute) {
   return glGetAttribLocation(programID, attribute);;
}

GLuint PShader::getUniformLocation(const char *uniform) {
   return glGetUniformLocation(programID, uniform);
}

void PShader::useProgram() {
   glUseProgram(programID);
}

void PShader::compileShaders() {
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
}

const char *PShader::defaultVertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec3 normal;
      in vec3 coords;
      in vec4 colors;
      in int mindex;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix[16];
      out vec3 vTexture;
      out vec4 vColor;
      out vec3 vNormal;
      out vec4 vPosition;

      void main()
      {
          mat4 M = Mmatrix[mindex];
          vPosition = M * vec4(position,1.0);
          vNormal = normalize((M * (vec4(position,1.0) + vec4(normal,0.0))) - vPosition).xyz;
          vTexture = coords;
          vColor = colors;

          gl_Position = Pmatrix * Vmatrix * vPosition;
          gl_Position.y = -gl_Position.y;
       }
)glsl";

const char *PShader::defaultFragmentShader = R"glsl(
      #version 330
      in vec3 vTexture;
      in vec3 vNormal;
      in vec4 vColor;
      in vec4 vPosition;
      out vec4 fragColor;
      uniform sampler2DArray myTextures;
      uniform vec3 ambientLight;
      uniform vec3 directionLightColor;
      uniform vec3 directionLightVector;
      uniform vec3 pointLightColor;
      uniform vec3 pointLightPosition;
      uniform vec3 pointLightFalloff;
      void main()
      {
          vec3 pointLightDirection = vPosition.xyz - pointLightPosition;

          float d = length(pointLightDirection);
          float pointLightIntensity = 1 / ( pointLightFalloff.x + pointLightFalloff.y * d + pointLightFalloff.z * d * d);
          float pointLight = max(dot(vNormal, -pointLightDirection), 0.0) * pointLightIntensity;

          float directional = max(dot(vNormal, -directionLightVector), 0.0);
          vec3 vLighting = ambientLight + (directionLightColor * directional) + (pointLightColor * pointLight );

          vec4 texelColor;
          if ( vTexture.z == 8 ) { // It's a circle
              vec2 pos = vTexture.xy;
              vec2 centre = vec2(0.5,0.5);
               if (distance(pos,centre) > 0.5)
                   discard;
               else
                   texelColor = vec4(1.0,1.0,1.0,1.0);
          } else {
              texelColor = texture(myTextures, vTexture);
          }
          fragColor = vec4(vLighting,1.0) * vColor * texelColor;
      }
)glsl";
