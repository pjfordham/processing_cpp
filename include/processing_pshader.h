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

typedef unsigned int GLuint;
class PShaderImpl;

class PShader {
   std::shared_ptr<PShaderImpl> impl;
public:

   class Attribute {
      GLint id = -1;
      GLint shaderId = -1;
   public:
      Attribute() {}
      Attribute(const PShader &pshader, const std::string &attribute) {
         id = pshader.getAttribLocation( attribute.c_str() );
         shaderId = pshader.getProgramID();
      }

      void bind_vec2(std::size_t stride, void *offset) {
         if ( id != -1 ) {
            glVertexAttribPointer( id, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset );
            glEnableVertexAttribArray(id);
         }
      }

      void bind_vec3(std::size_t stride, void *offset) {
         if ( id != -1 ) {
            glVertexAttribPointer( id, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset );
            glEnableVertexAttribArray(id);
         }
      }

      void bind_vec4(std::size_t stride, void *offset) {
         if ( id != -1 ) {
            glVertexAttribPointer( id, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset );
            glEnableVertexAttribArray(id);
         }
      }

      void bind_int(std::size_t stride, void *offset) {
         if ( id != -1 ) {
            glVertexAttribIPointer( id, 1, GL_INT, stride, (void*)offset );
            glEnableVertexAttribArray(id);
         }
      }
   };

   class Uniform {
      GLint id = -1;

   public:
      Uniform() {}
      Uniform(const PShader &pshader, const std::string &uniform) {
         id = pshader.getUniformLocation( uniform.c_str() );
      }

      void set(float value) const {
         if ( id != -1 )
            glUniform1f(id,value);
      }

      void set(int value) const {
         if ( id != -1 )
            glUniform1i(id,value);
      }

      void set(const glm::vec2 &value) const {
         if ( id != -1 )
            glUniform2fv(id, 1, glm::value_ptr(value) );
      }

      void set(const glm::vec3 &value) const {
         if ( id != -1 )
            glUniform3fv(id, 1, glm::value_ptr(value) );
      }

      void set(const glm::vec4 &value) const {
         if ( id != -1 )
            glUniform4fv(id, 1, glm::value_ptr(value) );
      }

      void set(const std::vector<int> &value) const {
         if ( id != -1 )
            glUniform1iv(id,value.size(),value.data());
      }

      void set(const std::vector<glm::vec3> &value) const {
         if ( id != -1 )
            glUniform3fv(id, value.size(), glm::value_ptr(value[0]) );
      }

      void set(const std::vector<glm::mat4> &value) const {
         if ( id != -1 )
            glUniformMatrix4fv(id, value.size(), false, glm::value_ptr(value[0]) );
      }

      void set(const glm::mat4 &value) const {
         if ( id != -1 )
            glUniformMatrix4fv(id, 1, false, glm::value_ptr(value) );
      }
   };

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

   void compileShaders();

   Uniform get_uniform(const std::string &uniform_name) const {
      return {*this, uniform_name};
   }

   Attribute get_attribute(const std::string &attribute_name) const {
      return {*this, attribute_name};
   }

   void set_uniforms();

   void set(const char *uniform, float value);

   void set(const char *uniform, float v1, float v2);

   void set(const char *uniform, float v1, float v2, float v3);

   GLuint getAttribLocation(const char *attribute) const;

   GLuint getUniformLocation(const char *uniform) const;

   void useProgram();

   GLuint getProgramID() const ;

};


#endif
