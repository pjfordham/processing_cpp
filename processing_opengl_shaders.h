#ifndef PROCESSING_OPENGL_SHADERS_H
#define PROCESSING_OPENGL_SHADERS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>

#include <fmt/core.h>

inline GLuint LoadShaders(){

   const char *vertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec3 normal;
      in vec3 color;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix;
      out vec3 gColor;
      out vec3 gLighting;
      void main()
      {
          gl_Position = Pmatrix * Vmatrix * Mmatrix * vec4(position, 1.0);
          gColor = color;
          vec3 ambientLight = vec3(0.3, 0.3, 0.3);
          vec3 directionalLightColor = vec3(1.0, 1.0, 1.0);
          vec3 directionalVector = normalize(vec3(0.0, 1.0, 0));
          vec4 transformedNormal = normalize(Mmatrix * vec4(normal, 1.0));
          float directional = max(dot(transformedNormal.xyz, directionalVector), 0.0);
         gLighting = ambientLight + (directionalLightColor * directional);
      }
)glsl";

   const char *geometryShader = R"glsl(
      #version 330
      in vec3 gLighting[];
      in vec3 gColor[];
      layout(triangles) in;
      layout(triangle_strip, max_vertices=6) out;
      out vec3 vLighting;
      out vec3 vColor;

      void main() {
          for(int i = 0; i < 3; i++) {
              gl_Position = gl_in[i].gl_Position;
              vLighting = gLighting[i];
              vColor = gColor[i];
              EmitVertex();
          }
          EndPrimitive();

          //for(int i = 0; i < 3; i++) {
          //    gl_Position = gl_in[i].gl_Position + vec4(45.0, 45.0, 0.0, 0.0);
          //    vLighting = gLighting[i];
          //    vColor = vec3(1.0-gColor[i][0],1.0-gColor[i][1],1.0-gColor[i][2]);
          //    EmitVertex();
          //}
          //EndPrimitive();
      }
)glsl";

   const char *fragmentShader = R"glsl(
      #version 330
      in vec3 vColor;
      in vec3 vLighting;
      out vec4 fragColor;
      void main()
      {
          fragColor = vec4(vLighting * vColor, 1.0);
      }
)glsl";

   // Create the shaders
   GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
   GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   // fmt::print("Compiling vertex shader\n");
   glShaderSource(VertexShaderID, 1, &vertexShader , NULL);
   glCompileShader(VertexShaderID);

   // fmt::print("Compiling geometry shader\n");
   glShaderSource(GeometryShaderID, 1, &geometryShader , NULL);
   glCompileShader(GeometryShaderID);

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

   // Check Geometry Shader
   glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
   glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   if ( InfoLogLength > 0 ){
      std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
      fmt::print("{}\n", &GeometryShaderErrorMessage[0]);
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
   glAttachShader(ProgramID, GeometryShaderID);
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
   glDeleteShader(GeometryShaderID);
   glDeleteShader(FragmentShaderID);

   return ProgramID;
}

#endif
