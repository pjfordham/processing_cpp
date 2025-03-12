#ifndef PROCESSING_OPENGL_SHADER_H
#define PROCESSING_OPENGL_SHADER_H

#include <vector>
#include <map>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "processing_opengl_texture.h"

typedef unsigned int GLuint;
typedef int GLint;

namespace gl {

    class attribute {
      GLint id = -1;
      GLint shaderId = -1;
   public:
      attribute() {}
      attribute(GLuint shaderID, const std::string &attribute);
      void bind_vec2(std::size_t stride, void *offset);
      void bind_vec3(std::size_t stride, void *offset);
      void bind_vec4(std::size_t stride, void *offset);
      void bind_int(std::size_t stride, void *offset);
      void bind_float(std::size_t stride, void *offset);
   };

   class uniform {
      GLint id = -1;
   public:
      uniform() {}
      uniform(GLuint shaderID, const std::string &uniform);
      void set(float value) const;
      void set(int value) const;
      void set(const glm::vec2 &value) const;
      void set(const glm::vec3 &value) const;
      void set(const glm::vec4 &value) const;
      void set(const std::vector<int> &value) const;
      void set(const std::vector<glm::vec2> &value) const;
      void set(const std::vector<glm::vec3> &value) const;
      void set(const std::vector<glm::vec4> &value) const;
      void set(const std::vector<glm::mat4> &value) const;
      void set(const std::vector<glm::mat3> &value) const;
      void set(const glm::mat4 &value) const;
   };

   class shader_t {
      std::map<std::string, glm::vec3>   uniforms3fv;
      std::map<std::string, glm::vec2>   uniforms2fv;
      std::map<std::string, float>       uniforms1f;
      std::map<std::string, texture_ptr> uniformsSampler;

   public:
      bool operator!=(const shader_t &other) {
         return programID != other.programID;
      }
      GLuint programID = 0;
      shader_t() {}
      shader_t(const char *vertSource, const char *fragSource);
      ~shader_t();
      void bind() const;
      uniform get_uniform(const std::string &uniform_name) const {
         return {programID, uniform_name};
      }
      attribute get_attribute(const std::string &attribute_name) const {
         return {programID, attribute_name};
      }
      void set_uniforms() const;
      void set(const char *id, gl::texture_ptr textureID);
      void set(const char *id, float value);
      void set(const char *id, float v1, float v2);
      void set(const char *id, float v1, float v2, float v3);

   };

} // namespace gl

#endif // PROCESSING_OPENGL_SHADER_H
