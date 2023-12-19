#ifndef PROCESSING_PSHADER_H
#define PROCESSING_PSHADER_H

#include <memory>
#include "glad/glad.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <set>
#include <fmt/core.h>

class PShaderImpl;

namespace gl{
   class uniform;
   class attribute;
   class shader_t;
}

class PShader {
   std::shared_ptr<PShaderImpl> impl;
public:

   static void init();
   static void close();

   PShader(GLuint parent, const char *vertSource, const char *fragSource);
   PShader(GLuint parent, const char *fragSource);
   PShader(GLuint parent);
   PShader() { }

   bool operator==(const PShader &other) const {
      return this->impl == other.impl;
   }

   bool operator!=(const PShader &other) const {
      return this->impl != other.impl;
   }

   const gl::shader_t &getShader() const;

   gl::uniform get_uniform(const std::string &uniform_name) const;

   gl::attribute get_attribute(const std::string &attribute_name) const;

   void bind();

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);


};
PShader loadShader();
PShader loadShader(const char *fragShader);
PShader loadShader(const char *fragShader, const char *vertShader);

#endif
