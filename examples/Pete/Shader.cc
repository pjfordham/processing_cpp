#include "processing_pshader_builder.h"

PShader basic;

// GLSL_Output vTindex  { GLSL_Type::INT,  "vTindex",   GLSL_Interpolation::FLAT   };
// GLSL_Output vTexture { GLSL_Type::VEC2, "vTexture",  GLSL_Interpolation::SMOOTH };
// GLSL_Output vColor   { GLSL_Type::VEC4, "vColor",    GLSL_Interpolation::SMOOTH };
// GLSL_Output vNormal  { GLSL_Type::VEC3, "vNormal",   GLSL_Interpolation::SMOOTH };
// GLSL_Output vPosition{ GLSL_Type::VEC4, "vPosition", GLSL_Interpolation::SMOOTH };

void setup() {
   size(800, 600);

   GLSL_Input position{GLSL_Type::VEC3, "position", GLSL_Interpolation::NONE };
   GLSL_Input color {GLSL_Type::VEC4, "color", GLSL_Interpolation::NONE };
   GLSL_Output vertColor{GLSL_Type::VEC4, "vertColor", GLSL_Interpolation::SMOOTH };
   GLSL_Output gl_Position{GLSL_Type::VEC4, "gl_Position", GLSL_Interpolation::SMOOTH };
   GLSL_Uniform transformMatrix{GLSL_Type::MAT4, "transformMatrix",  1};
   GLSL_Function vec4{"vec4"};

   Shader basicVertex {{
         gl_Position = transformMatrix * vec4(position,1.0f) ,
         vertColor = color,
      }};

   GLSL_Input vertColor2{GLSL_Type::VEC4, "vertColor", GLSL_Interpolation::SMOOTH };
   GLSL_Output fragColor{GLSL_Type::VEC4, "fragColor", GLSL_Interpolation::NONE };

   Shader basicFragment {{
         fragColor = vertColor2
      }};

   std::string basicVertexShader = basicVertex.print();
   std::string basicFragmentShader = basicFragment.print();

   fmt::print("Vertex Shader:\n\n{}\n\n\nFragment Shader:\n\n{}\n", basicVertexShader, basicFragmentShader);
   basic = PShader(0, basicVertexShader.c_str(), basicFragmentShader.c_str());
   basic.compileShaders();
   noStroke();
   fill(0, 1);
   shader(basic);
}

void draw() {
   background(255);
   for (int i = 0; i < 50000; i++) {
      float x = random(width);
      float y = random(height);
      rect(x, y, 30, 30);
   }
}
