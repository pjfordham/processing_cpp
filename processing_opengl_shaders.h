#ifndef PROCESSING_OPENGL_SHADERS_H
#define PROCESSING_OPENGL_SHADERS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include <tuple>

#include <fmt/core.h>

inline std::tuple<const char*, const char*, const char*> Shaders3D() {

  const char *vertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec3 normal;
      in vec2 coords;
      in vec4 colors;
      uniform vec3 ambientLight;
      uniform vec3 directionLightColor;
      uniform vec3 directionLightVector;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix;
      out vec3 vLighting;
      out vec2 vTexture;
      out vec4 vColor;
      void main()
      {
          vec4 Mposition = Mmatrix * vec4(position,1.0);
          vec3 Mnormal = normalize((Mmatrix * (vec4(position,1.0) + vec4(normal,0.0))) - Mposition).xyz;

          gl_Position = Pmatrix * Vmatrix * Mposition;
          float directional = max(dot(Mnormal, -directionLightVector), 0.0);
          vLighting = ambientLight + (directionLightColor * directional);
          vTexture = coords;
          vColor = colors;
       }
)glsl";

  const char *geometryShader = NULL;

  const char *fragmentShader = R"glsl(
      #version 330
      in vec2 vTexture;
      in vec3 vLighting;
      in vec4 vColor;
      out vec4 fragColor;
      uniform sampler2D uSampler;
      void main()
      {
          vec4 texelColor = texture2D(uSampler, vTexture);
          fragColor = vec4(vLighting,1.0) * vColor * texelColor;
      }
)glsl";
   return { vertexShader, geometryShader, fragmentShader };
}

inline GLuint LoadShaders(std::tuple<const char*, const char*, const char*> in){

   auto [vertexShader, geometryShader, fragmentShader] = in;

   // Create the shaders
   GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
   GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   // fmt::print("Compiling vertex shader\n");
   glShaderSource(VertexShaderID, 1, &vertexShader , NULL);
   glCompileShader(VertexShaderID);

   if (geometryShader) {
      // fmt::print("Compiling geometry shader\n");
      glShaderSource(GeometryShaderID, 1, &geometryShader , NULL);
      glCompileShader(GeometryShaderID);
   }
   // fmt::print("Compiling fragment shader\n");
   glShaderSource(FragmentShaderID, 1, &fragmentShader , NULL);
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

   if (geometryShader) {
      // Check Geometry Shader
      glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
         glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
         fmt::print("{}\n", &GeometryShaderErrorMessage[0]);
      }
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
   // fmt::print("Linking program\n");
   GLuint ProgramID = glCreateProgram();
   glAttachShader(ProgramID, VertexShaderID);
   if (geometryShader) {
      glAttachShader(ProgramID, GeometryShaderID);
   }
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
   glDetachShader(ProgramID, GeometryShaderID);
   glDetachShader(ProgramID, FragmentShaderID);

   glDeleteShader(VertexShaderID);
   if (geometryShader) {
      glDeleteShader(GeometryShaderID);
   }
   glDeleteShader(FragmentShaderID);

   return ProgramID;
}

#endif
