#include "processing.h"
#include "processing_debug.h"
#include <vector>
#include <map>
#include <string>
#include <array>
#include <fmt/core.h>

#include "glad/glad.h"
#include "processing_opengl_shader.h"
#include "processing_opengl.h"
#include "processing_opengl_texture.h"

#undef DEBUG_METHOD
#define DEBUG_METHOD() do {} while (false)

namespace gl {

   shader_t::shader_t(const char *vertex, const char *fragment) {
      // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

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
      programID = glCreateProgram();
      glAttachShader(programID, VertexShaderID);
      glAttachShader(programID, FragmentShaderID);
      glLinkProgram(programID);

      // Check the program
      glGetProgramiv(programID, GL_LINK_STATUS, &Result);
      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if ( InfoLogLength > 0 ){
         std::vector<char> ProgramErrorMessage(InfoLogLength+1);
         glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
         fmt::print("{}\n", &ProgramErrorMessage[0]);
      }

      glDetachShader(programID, VertexShaderID);
      glDetachShader(programID, FragmentShaderID);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);
   }

   shader_t::~shader_t() {
      if (programID) {
         glDeleteProgram(programID);
      }
   }

   void shader_t::set_uniforms() const {
      DEBUG_METHOD();
      for (const auto& [id, value] : uniforms1f) {
         gl::uniform loc = get_uniform( id.c_str() );
         loc.set( value );
      }
      for (const auto& [id, value] : uniforms2fv) {
         gl::uniform loc = get_uniform( id.c_str() );
         loc.set( value );
      }
      for (const auto& [id, value] : uniforms3fv) {
         gl::uniform loc = get_uniform( id.c_str() );
         loc.set( value );
      }
      for (const auto& [id, value] : uniforms2i) {
         gl::uniform loc = get_uniform( id.c_str() );
         loc.set( value );
      }
      for (const auto& [id, value] : uniforms4i) {
         gl::uniform loc = get_uniform( id.c_str() );
         loc.set( value );
      }
      // TODO fix to make sure we don't collides with
      // other textures.
      int i = 15;
      for (auto& [id, value] : uniformsSampler) {
         gl::uniform loc = get_uniform( id.c_str() );
         glActiveTexture(GL_TEXTURE0 + i);
         glBindTexture(GL_TEXTURE_2D, value->get_id());
         loc.set( i-- );
      }
   }

   void shader_t::set(const char *id, texture_ptr textureID) {
      DEBUG_METHOD();
      uniformsSampler[id] = textureID;
   }

   void shader_t::set(const char *id, float value) {
      DEBUG_METHOD();
      uniforms1f[id] = value;
   }

   void shader_t::set(const char *id, float v1, float v2) {
      DEBUG_METHOD();
      uniforms2fv[id] = {v1,v2};
   }

   void shader_t::set(const char *id, float v1, float v2, float v3) {
      DEBUG_METHOD();
      uniforms3fv[id] = {v1, v2, v3};
   }

   void shader_t::set(const char *id, int v1, int v2) {
      DEBUG_METHOD();
      uniforms2i[id] = {v1, v2};
   }

   void shader_t::set(const char *id, int v1, int v2, int v3, int v4){
      DEBUG_METHOD();
      uniforms4i[id] = {v1, v2, v3, v4};
   }

} // namespace gl
