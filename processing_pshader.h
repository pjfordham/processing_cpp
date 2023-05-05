#ifndef PROCESSING_PSHADER_H
#define PROCESSING_PSHADER_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include <tuple>
#include <map>

#include <fmt/core.h>
#include "processing_enum.h"

class PShader {
public:
   static const char *defaultVertexShader;
   static const char *defaultFragmentShader;

   std::string vertexShader;
   std::string fragmentShader;
   GLuint ProgramID;

   PShader(const PShader& other) = delete;
   PShader& operator=(const PShader& other) = delete;

   PShader(PShader&& other) noexcept {
      std::swap(vertexShader, other.vertexShader);
      std::swap(fragmentShader, other.fragmentShader);
      std::swap(ProgramID, other.ProgramID);
   }

   PShader& operator=(PShader&& other) noexcept {
      std::swap(vertexShader, other.vertexShader);
      std::swap(fragmentShader, other.fragmentShader);
      std::swap(ProgramID, other.ProgramID);
      return *this;
   }

   ~PShader() {
      if (ProgramID) {
         glDeleteProgram(ProgramID);
      }
   }

   GLuint compileShaders() {
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
      ProgramID = glCreateProgram();
      glAttachShader(ProgramID, VertexShaderID);
      glAttachShader(ProgramID, FragmentShaderID);
      glLinkProgram(ProgramID);

      // Check the program
      glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
      glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> ProgramErrorMessage(InfoLogLength+1);
         glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
         fmt::print("{}\n", &ProgramErrorMessage[0]);
      }

      glDetachShader(ProgramID, VertexShaderID);
      glDetachShader(ProgramID, FragmentShaderID);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);
      return ProgramID;
   }

public:
   PShader(GLuint parent, const char *vertSource, const char *fragSource) :
      vertexShader( vertSource ) , fragmentShader( fragSource ), ProgramID( 0 ) {
   }

   PShader(GLuint parent, const char *fragSource) : PShader( 0, defaultVertexShader, fragSource ) {
   }

   PShader() : PShader( 0, defaultVertexShader, defaultFragmentShader ) {
   }

   PShader(GLuint parent) : PShader() {
   }

   std::map<GLuint, float> uniforms1f;
   std::map<GLuint, std::array<float,2>> uniforms2fv;

   void set_uniforms() {
      for (const auto& [id, value] : uniforms1f) {
         glUniform1f(id,value);
      }
      for (const auto& [id, value] : uniforms2fv) {
         glUniform2fv(id,1,value.data());
      }
   }

   void set(const char *uniform, float value) {
      GLuint id = glGetUniformLocation(ProgramID, uniform);
      uniforms1f[id] = value;
   }

   void set(const char *uniform, float v1, float v2) {
      std::array<float,2> vec = {v1,v2};
      GLuint id = glGetUniformLocation(ProgramID, uniform);
      uniforms2fv[id] = vec;
   }
};


const char *PShader::defaultVertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec3 normal;
      in vec3 coords;
      in vec4 colors;
      in int mindex;
      uniform vec3 ambientLight;
      uniform vec3 directionLightColor;
      uniform vec3 directionLightVector;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix[16];
      out vec3 vLighting;
      out vec3 vTexture;
      out vec4 vColor;

      void main()
      {
          mat4 M = Mmatrix[mindex];
          vec4 Mposition = M * vec4(position,1.0);
          vec3 Mnormal = normalize((M * (vec4(position,1.0) + vec4(normal,0.0))) - Mposition).xyz;

          gl_Position = Pmatrix * Vmatrix * Mposition;
          gl_Position.y = -gl_Position.y;
          float directional = max(dot(Mnormal, -directionLightVector), 0.0);
          vLighting = ambientLight + (directionLightColor * directional);
          vTexture = coords;
          vColor = colors;
       }
)glsl";

const char *PShader::defaultFragmentShader = R"glsl(
      #version 330
      in vec3 vTexture;
      in vec3 vLighting;
      in vec4 vColor;
      out vec4 fragColor;
      uniform sampler2DArray myTextures;
      void main()
      {
          vec4 texelColor = texture(myTextures, vTexture);
          fragColor = vec4(vLighting,1.0) * vColor * texelColor;
      }
)glsl";

#endif
