#ifndef PROCESSING_PSHADER_H
#define PROCESSING_PSHADER_H

#include <memory>

typedef unsigned int GLuint;
class PShaderImpl;

class PShader {
   std::shared_ptr<PShaderImpl> impl;
public:

   static void init();
   static void close();

   PShader(GLuint parent, const char *vertSource, const char *fragSource);
   PShader(GLuint parent, const char *fragSource);
   PShader(GLuint parent);
   PShader();

   void compileShaders();

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);

   GLuint getAttribLocation(const char *attribute);

   GLuint getUniformLocation(const char *uniform);

   void useProgram();

};


#endif
