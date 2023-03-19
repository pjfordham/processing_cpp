#ifndef PROCESSING_OPENGL_SHADERS_H
#define PROCESSING_OPENGL_SHADERS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <vector>
#include <tuple>

#include <fmt/core.h>

inline std::tuple<const char*, const char*, const char*> CircleShaderFlat() {
  const char *vertexShader = R"glsl(
      #version 330
      in vec2 radius;
      in vec3 position;
      in vec4 color;
      out vec4 gColor;
      out vec2 gRadius;
      void main()
      {
          gl_Position = vec4(position, 1.0);
          gColor = color;
          gRadius = radius;
      }
)glsl";

  const char *geometryShader = R"glsl(
      #version 330
      #define PI_BY_32 (3.1415926535897932384626433832795/16.0)
      in vec4 gColor[];
      in vec2 gRadius[];
      layout(points) in;
      layout(triangle_strip, max_vertices=150) out;
      out vec4 vColor;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix;
      uniform vec4 strokeColor;
      uniform float strokeWeight;

      vec4 ellipse_point(vec4 center, int index, vec2 radius) {
          float angle = float(index) * PI_BY_32;
          return vec4( center.x + radius.x * sin(angle),
                       center.y + radius.y * cos(angle),
                       center.zw);
      }

      void main() {
           mat4 Tmatrix = Pmatrix * Vmatrix * Mmatrix;
           vec4 center = Tmatrix * gl_in[0].gl_Position;

           for(int i = 0; i < 32; i++) {
               gl_Position = center;
               vColor = gColor[0];
               EmitVertex();

               vColor = gColor[0];
               gl_Position = Tmatrix * ellipse_point(gl_in[0].gl_Position, i, gRadius[0]);
               EmitVertex();
           }

           gl_Position = center;
           vColor = gColor[0];
           EmitVertex();

           gl_Position = Tmatrix * ellipse_point(gl_in[0].gl_Position, 0, gRadius[0]);
           vColor = gColor[0];
           EmitVertex();

           EndPrimitive();

           if (strokeWeight > 0 && strokeColor != gColor[0]) {

               vec2 outerRadius = gRadius[0] + strokeWeight / 2.0;
               vec2 innerRadius = gRadius[0] - strokeWeight / 2.0;

               for(int i = 0; i <= 32; i++) {
                   gl_Position = Tmatrix * ellipse_point(gl_in[0].gl_Position, i, outerRadius);
                   vColor = strokeColor;
                   EmitVertex();

                   gl_Position = Tmatrix * ellipse_point(gl_in[0].gl_Position, i, innerRadius);
                   vColor = strokeColor;
                   EmitVertex();
               }

               EndPrimitive();
          }

      }
)glsl";

   const char *fragmentShader = R"glsl(
      #version 330
      in vec4 vColor;
      out vec4 fragColor;
      void main()
      {
          fragColor = vColor;
      }
)glsl";

   return { vertexShader, geometryShader, fragmentShader };
}

inline std::tuple<const char*, const char*, const char*> ShadersFlatTexture() {
   const char *vertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec2 coords;
      out vec2 vTexture;
      void main()
      {
          gl_Position = vec4(position, 1.0);
          vTexture = coords;
      }
)glsl";

   const char *geometryShader = NULL;

   const char *fragmentShader = R"glsl(
      #version 330
      in vec2 vTexture;
      uniform sampler2D uSampler;
      out vec4 fragColor;
      void main()
      {
          vec4 texelColor = texture2D(uSampler, vTexture);
          fragColor = vec4(texelColor.rgb, 1.0);
      }
)glsl";

   return { vertexShader, geometryShader, fragmentShader };
}

inline std::tuple<const char*, const char*, const char*> ShadersFlat() {
   const char *vertexShader = R"glsl(
      #version 330
      in vec3 position;
      in vec4 color;
      uniform mat4 Pmatrix;
      uniform mat4 Vmatrix;
      uniform mat4 Mmatrix;
      out vec4 gColor;
      void main()
      {
          gl_Position = Pmatrix * Vmatrix * Mmatrix * vec4(position, 1.0);
          gColor = color;
      }
)glsl";

   const char *geometryShader = R"glsl(
      #version 330
      in vec4 gColor[];
      layout(triangles) in;
      layout(triangle_strip, max_vertices=6) out;
      out vec4 vColor;

      void main() {
          for(int i = 0; i < 3; i++) {
              gl_Position = gl_in[i].gl_Position;
              vColor = gColor[i];
              EmitVertex();
          }
          EndPrimitive();
      }
)glsl";

   const char *fragmentShader = R"glsl(
      #version 330
      in vec4 vColor;
      out vec4 fragColor;
      void main()
      {
          fragColor = vColor;
      }
)glsl";

   return { vertexShader, geometryShader, fragmentShader };
}

inline std::tuple<const char*, const char*, const char*> Shaders3D() {

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
