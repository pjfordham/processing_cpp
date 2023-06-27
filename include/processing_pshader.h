#ifndef PROCESSING_PSHADER_H
#define PROCESSING_PSHADER_H

#include <map>
#include <string>
#include <array>

typedef unsigned int GLuint;

class PShader {
   static const char *defaultVertexShader;
   static const char *defaultFragmentShader;

   std::map<GLuint, float> uniforms1f;
   std::map<GLuint, std::array<float,2>> uniforms2fv;
   std::map<GLuint, std::array<float,3>> uniforms3fv;

   std::string vertexShader;
   std::string fragmentShader;

   GLuint programID;
public:

   PShader(const PShader& other) = delete;
   PShader& operator=(const PShader& other) = delete;

   PShader(PShader&& other) noexcept : programID( 0 ) {
      *this = std::move(other);
   }

   PShader& operator=(PShader&& other) noexcept {
      std::swap(uniforms1f, other.uniforms1f);
      std::swap(uniforms2fv, other.uniforms2fv);
      std::swap(uniforms3fv, other.uniforms3fv);
      std::swap(vertexShader, other.vertexShader);
      std::swap(fragmentShader, other.fragmentShader);
      std::swap(programID, other.programID);
      return *this;
   }

   ~PShader();

   void compileShaders();

public:
   PShader(GLuint parent, const char *vertSource, const char *fragSource) :
      vertexShader( vertSource ) , fragmentShader( fragSource ), programID( 0 ) {
   }

   PShader(GLuint parent, const char *fragSource) : PShader( 0, defaultVertexShader, fragSource ) {
   }

   PShader() : PShader( 0, defaultVertexShader, defaultFragmentShader ) {
   }

   PShader(GLuint parent) : PShader() {
   }

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);

   GLuint getAttribLocation(const char *attribute);

   GLuint getUniformLocation(const char *uniform);

   void useProgram();

};


#endif
