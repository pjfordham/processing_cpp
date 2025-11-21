#include <utility>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "processing_opengl.h"
#include "processing_opengl_framebuffer.h"
#include "processing_debug.h"
#include "processing_task_queue.h"

#include <glm/gtc/type_ptr.hpp>

#undef DEBUG_METHOD
#undef DEBUG_METHOD_MESSAGE
#define DEBUG_METHOD() do {} while (false)
#define DEBUG_METHOD_MESSAGE(x) do {} while (false)

progschj::ThreadPool renderThread(1);

static bool enable_debug = false;

namespace gl {

   class VAO_t {
      GLuint vao = 0;
      GLuint indexId = 0;
      GLuint vertexId = 0;
      GLuint materialId= 0;
   public:
      friend struct fmt::formatter<VAO_t>;

      std::vector<vertex_t> vertices;
      std::vector<material_t> materials;
      std::vector<unsigned short> indices;
      std::vector<texture_t_ptr> textures;
      std::vector<glm::mat4> transforms;

      VAO_t() noexcept;

      VAO_t(const VAO_t& x) noexcept;

      VAO_t(VAO_t&& x) noexcept;

      VAO_t& operator=(const VAO_t&) = delete;

      VAO_t& operator=(VAO_t&& other) noexcept;

      void bind();
      void bind( attribute_t Position, attribute_t Normal, attribute_t Color,
                 attribute_t Coord,    attribute_t TUnit,  attribute_t MIndex,
                 attribute_t Ambient,  attribute_t Specular, attribute_t Emissive, attribute_t Shininess);
      int hasTexture(texture_t_ptr texture);
      void loadBuffers() const;
      void draw() const;
      void debugPrint() const;
      ~VAO_t();
   };

   int scene_t::blendMode(int b ) {
      return std::exchange(currentBlendMode,b);
   }

   void scene_t::hint(int type) {
      switch(type) {
      case DISABLE_DEPTH_TEST:
         depth_test = false;
         break;
      case ENABLE_DEPTH_TEST:
         depth_test = true;
         break;
      case DISABLE_DEPTH_MASK:
         depth_mask = false;
         break;
      case ENABLE_DEPTH_MASK:
         depth_mask = true;
         break;
      default:
         break;
      }
   }

   scene_t::scene_t() {}

   void scene_t::setup( const shader_t &shader) {
      LightCount = shader.get_uniform("lightCount");
      LightPosition = shader.get_uniform("lightPosition");
      LightNormal = shader.get_uniform("lightNormal");
      LightAmbient = shader.get_uniform("lightAmbient");
      LightDiffuse = shader.get_uniform("lightDiffuse");
      LightSpecular = shader.get_uniform("lightSpecular");
      LightFalloff = shader.get_uniform("lightFalloff");
      LightSpot = shader.get_uniform("lightSpot");
      PVmatrix = shader.get_uniform("PVmatrix");
      Eye = shader.get_uniform("eye");
   }

   float scene_t::screenX(float x, float y, float z) const {
      glm::vec4 in = { x, y, z, 1 };
      return (projection_matrix * (view_matrix * in)).x;
   }

   float scene_t::screenY(float x, float y, float z) const {
      glm::vec4 in = { x, y, z, 1 };
      return (projection_matrix * (view_matrix * in)).y;
   }

   void scene_t::set() {
      PVmatrix.set( projection_matrix * view_matrix  );
      Eye.set( glm::vec3(glm::inverse(view_matrix)[3]));
      if ( lights.size() == 0 ) {
         // setup flat light
         LightCount.set( 1 );
         LightPosition.set( glm::vec4{ 0,0,0,0} );
         LightNormal.set( glm::vec3{ 0,0,0} );
         LightAmbient.set( glm::vec3{ 1,1,1} );
         LightDiffuse.set( glm::vec3{0,0,0} );
         LightSpecular.set( glm::vec3{0,0,0} );
         LightFalloff.set( glm::vec3{1,0,0} );
         LightSpot.set( glm::vec2{0,0} );
      } else {
         LightCount.set( (int) lights.size()  );
         std::vector<glm::vec4> position;
         std::vector<glm::vec3> normal, ambient, diffuse, specular, falloff;
         std::vector<glm::vec2> spot;
         for (const auto &light : lights){
            position.push_back( light.position );
            normal.push_back( light.normal );
            ambient.push_back( light.ambient );
            diffuse.push_back( light.diffuse );
            specular.push_back( light.specular );
            falloff.push_back( light.falloff );
            spot.push_back( light.spot );
         }
         LightPosition.set( position );
         LightNormal.set( normal );
         LightAmbient.set( ambient );
         LightDiffuse.set( diffuse );
         LightSpecular.set( specular );
         LightFalloff.set( falloff );
         LightSpot.set( spot );
      }

      glEnable(GL_BLEND);
      switch (currentBlendMode) {
      case BLEND:
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBlendEquation(GL_FUNC_ADD);
         break;
      case ADD:
         glBlendFunc(GL_SRC_ALPHA, GL_ONE);
         glBlendEquation(GL_FUNC_ADD);
         break;
      case SUBTRACT:
         glBlendFunc(GL_SRC_ALPHA, GL_ONE);
         glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
         break;
      case DARKEST:
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBlendEquation(GL_MIN);
         break;
      case LIGHTEST:
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBlendEquation(GL_MAX);
         break;
      case DIFFERENCE:
         // Not supported
         glDisable(GL_BLEND);
         break;
      case EXCLUSION:
         glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
         glBlendEquation(GL_FUNC_ADD);
         break;
      case MULTIPLY:
         glBlendFunc(GL_DST_COLOR, GL_ZERO);
         glBlendEquation(GL_FUNC_ADD);
         break;
      case SCREEN:
         glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
         glBlendEquation(GL_FUNC_ADD);
         break;
      case REPLACE:
         glDisable(GL_BLEND);
         break;
      default:
         abort();
      }

      glDepthFunc(GL_LEQUAL);
      if (depth_test) {
         glEnable(GL_DEPTH_TEST);
      } else {
         glDisable(GL_DEPTH_TEST);
      }
      if (depth_mask) {
         glDepthMask(GL_TRUE);
      } else {
         glDepthMask(GL_FALSE);
      }
   }

   void scene_t::setProjectionMatrix( const glm::mat4 &PV ) {
      projection_matrix = PV;
   }

   void scene_t::setViewMatrix( const glm::mat4 &PV ) {
      glm::mat4 flipY = {
         { 1.0f,  0.0f, 0.0f, 0.0f },
         { 0.0f, -1.0f, 0.0f, 0.0f },
         { 0.0f,  0.0f, 1.0f, 0.0f } ,
         { 0.0f,  0.0f, 0.0f, 1.0f } };
      view_matrix = flipY * PV ;
   }

   void scene_t::pushAmbientLight( glm::vec3 color ) {
      lights.emplace_back( light_t{{0.0f,0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
   }

   void scene_t::pushDirectionalLight( glm::vec3 color, glm::vec3 vector, glm::vec3 specular ) {
      lights.emplace_back( light_t{{0.0f,0.0f,0.0f,0.0f}, vector, {0,0,0}, color, specular, {1.0f,0.0f,0.0f}, {0.0f,0.0f}} );
   }

   void scene_t::pushPointLight( glm::vec3 color, glm::vec4 position, glm::vec3 specular, glm::vec3 falloff) {
      lights.emplace_back( light_t{position, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, color, specular, falloff, {0.0f,0.0f}} );
   }

   void scene_t::pushSpotLight(  glm::vec3 color, glm::vec4 position, glm::vec3 direction,  glm::vec3 specular, glm::vec3 falloff, glm::vec2 spot) {
      lights.emplace_back( light_t{position, direction, {0.0f,0.0f,0.0f}, color, specular, falloff, spot });
   }

   void scene_t::clearLights() {
      lights.clear();
   }

   void scene_t::flatLight() {
      clearLights();
      pushAmbientLight(glm::vec3{1.0f,1.0f,1.0f});
   }

   bool scene_t::anyLights() const {
      return lights.size() != 0;
   }

   void renderDirect( framebuffer_t &fb, batch_t_ptr batch, const glm::mat4 &transform, scene_t scene, const shader_t &shader ) {
      renderThread.enqueue( [&fb, &shader, batch, transform, scene] () mutable {
         fb.bind();
         shader.bind();
         uniform_t uSampler = shader.get_uniform("texture");
         uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

         scene.setup( shader );
         batch->setup( shader );
         shader.set_uniforms();
         scene.set();
         batch->bind();
         batch->draw( transform );
      } );
   }

   void frame_t::background(color_t b) {
      c = true;
      background_ = b;
   }

   void frame_t::add(batch_t_ptr b, scene_t sc, const shader_t &sh) {
      geometries.emplace_back( b, sc, sh );
   }

   void frame_t::clear() {
      c = false;
      geometries.clear();
   }

   void frame_t::render(framebuffer_t &fb) {

      // Stop the main thread getting multiple frames ahead of the render thread.
      renderThread.wait_until_nothing_in_flight();

      renderThread.enqueue( [c=c,&fb, background_=background_,geo=geometries]  {
         fb.bind();
         if (c) {
            fb.clear(background_.r, background_.g, background_.b, background_.a);
         }
         for (auto g : geo) {
            // Add flat shader optimization
            g.shader.bind();

            uniform_t uSampler = g.shader.get_uniform("texture");
            uSampler.set( std::vector<int>{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} );

            g.scene.setup( g.shader );
            g.batch->setup( g.shader );

            g.shader.set_uniforms();
            g.scene.set();
            g.batch->bind();
            g.batch->_load();
            g.batch->draw();
            g.batch->clear();
         }
      });

      geometries.clear();
      c = false;
   }

   void batch_t::setup( const shader_t &shader ) {
      Position = shader.get_attribute("position");
      Normal = shader.get_attribute("normal");
      Color = shader.get_attribute("color");
      Coord = shader.get_attribute("texCoord");
      TUnit = shader.get_attribute("tunit");
      MIndex = shader.get_attribute("mindex");
      Mmatrix = shader.get_uniform("Mmatrix");
      Nmatrix  = shader.get_uniform("Nmatrix");
      TexOffset = shader.get_uniform("texOffset");
      Ambient = shader.get_attribute("ambient");
      Specular = shader.get_attribute("specular");
      Emissive = shader.get_attribute("emissive");
      Shininess = shader.get_attribute("shininess");
   }

   void batch_t::setupTextures(VAO_t_ptr draw) {
      std::vector<glm::vec2> textureOffsets(16);
      for ( int i = 0; i < draw->textures.size() ; ++i ) {
         auto &img = draw->textures[i];
         if (img != texture_t::circle()) {
            // Set this here so get_width and get_height don't mess up
            // previously bound textures.
            glActiveTexture(GL_TEXTURE0 + i);
            textureOffsets[i] = glm::vec2(1.0 / img->_get_width(), 1.0 / img->_get_height());
            img->bind();
         }
      }
      TexOffset.set(textureOffsets);
   }

   void batch_t::draw( const glm::mat4 &transform ) {

      if (enable_debug) {
         fmt::print("### FLAT GEOMETRY DUMP START ###\n");
         int i = 0;
         for (auto &vao : vaos) {
            fmt::print("\n### GEOMETRY DUMP VAO {}   ###\n",i++);
            vao->debugPrint();
         }
         fmt::print("\n### GEOMETRY DUMP END   ###\n");
      }

      std::vector<glm::mat4> transforms;
      std::vector<glm::mat3> normals;
      transforms.push_back(transform);
      normals.emplace_back( glm::transpose(glm::inverse(transform)) );
      Mmatrix.set( transforms );
      Nmatrix.set( normals );

      for (auto &draw: vaos ) {
         setupTextures( draw );
         draw->draw();
      }
   }

   void batch_t::vertices(const std::vector<vertex_t> &vertices, const std::vector<material_t> &materials, const std::vector<unsigned short> &indices, const glm::mat4 &transform_, bool flatten_transforms, std::optional<texture_t_ptr> texture_, std::optional<color_t> override ) {
      DEBUG_METHOD();

      if (texture_ == texture_t::circle()) {
         uses_circles = true;
      } else {
         if (texture_) {
            uses_textures = true;
         } else {
            texture_ = texture_t::blank();
         }
      }

      // Do this better and share somehow with shader and texture unit init
      // code.
      // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);

      const int MaxTextureImageUnits = 15; // keep one spare
      const int MaxTransformsPerBatch = 16;

      if (vertices.size() > 65536)
         abort();

      glm::mat4 I = glm::identity<glm::mat4>();
      const glm::mat4 &transform =
         flatten_transforms ? I : transform_;

      if (vaos.size() == 0 || vaos.back()->vertices.size() + vertices.size() > 65536) {
         vaos.emplace_back(std::make_shared<VAO_t>());
         vaos.back()->transforms.push_back( transform );
         vaos.back()->textures.push_back(texture_.value());
      }
      // At this point vaos has a back and that back has a transform and a texture.
      // It also has enough capacity for these triangles.

      if ( transform != vaos.back()->transforms.back()) {
         if (vaos.back()->transforms.size() == MaxTransformsPerBatch) {
            vaos.emplace_back(std::make_shared<VAO_t>());
            vaos.back()->textures.push_back(texture_.value());
         }
         vaos.back()->transforms.push_back(transform);
      }

      // Try to reused existing textures if they are the same. Hopefully the
      // search will be minimal cost since N<15 but we could use a hashmap
      // if need-be. Could apply to transforms too but comparision cost is
      // much higher.
      int tunit;
      if(texture_ == texture_t::circle()) {
         tunit = -1;
      } else {
         auto &vec = vaos.back()->textures;
         auto i = std::find(vec.begin(), vec.end(), texture_.value());

         if ( i == vec.end() ) {
            if (vec.size() == MaxTextureImageUnits) {
               vaos.emplace_back(std::make_shared<VAO_t>());
               vaos.back()->transforms.push_back( transform );
            }
            // Old version of vec might have been invalidated.
            auto &vec = vaos.back()->textures;
            vec.push_back(texture_.value());
            tunit = vec.size() - 1;
         } else {
            tunit = i - vec.begin();
         }
      }

      auto &vao = *(vaos.back());
      int currentM = vao.transforms.size() - 1;
      int offset = vao.vertices.size();

      for (const auto &v : vertices) {
         vao.vertices.emplace_back(
            flatten_transforms ? transform_ * glm::vec4(v.position,1.0) : v.position,
            v.normal,
            v.coord,
            override.value_or(v.fill),
            tunit,
            currentM);
      }

      for (const auto &m : materials ) {
         vao.materials.push_back( m );
      }

      for (const auto index : indices) {
         vao.indices.push_back( offset + index );
      }
   }

   void VAO_t::debugPrint() const {
      for (const auto &m : transforms) {
         fmt::print("{}\n",m);
      }
      fmt::print("Vertices: {}\n", vertices.size() );
      for ( int i = 0; i < vertices.size(); ++i ) {
         fmt::print("{:3}: {} {}\n", i, vertices[i], materials[i]);
      }
      fmt::print("Triangles: {}\n", indices.size() );
      for ( int i = 0; i < indices.size(); i+=3 ) {
         fmt::print("{},{},{} ", indices[i], indices[i+1], indices[i+2]);
      }
    }

   void batch_t::draw() {
      if (enable_debug) {
         fmt::print("### GEOMETRY DUMP START ###\n");
         int i = 0;
         for (auto &vao : vaos) {
            fmt::print("\n### GEOMETRY DUMP VAO {}   ###\n",i++);
            vao->debugPrint();
         }
         fmt::print("\n### GEOMETRY DUMP END   ###\n");
      }

      for (auto &draw: vaos ) {
         std::vector<glm::mat3> normals;
         for ( const auto &transform : draw->transforms ) {
            normals.emplace_back( glm::transpose(glm::inverse(transform)) );
         }

         Mmatrix.set( draw->transforms );
         Nmatrix.set(normals);

         setupTextures( draw );
         draw->draw();
      }
   }

   size_t batch_t::size() {
      return vaos.size();
   }

   void batch_t::bind() {
      for (auto &draw: vaos ) {
         draw->bind( Position, Normal, Color, Coord, TUnit, MIndex, Ambient, Specular, Emissive, Shininess );
      }
   }

   void batch_t::_load() {
         for (auto &draw: vaos ) {
            draw->loadBuffers();
         }
   }

   void batch_t::load() {
      renderThread.enqueue( [&] {
         _load();
      } );
      renderThread.wait_until_nothing_in_flight();
   }

   bool batch_t::usesCircles() const {
      return uses_circles;
   }

   bool batch_t::usesTextures() const {
      return uses_textures;
   }

   void batch_t::clear() {
      vaos.clear();
   }

   void shader_t::bind() const {
      glUseProgram(programID);
   }

   VAO_t::VAO_t() noexcept {
      DEBUG_METHOD();
      vertices.reserve(65536);
      materials.reserve(65536);
      indices.reserve(65536);
      textures.reserve(16);
      transforms.reserve(16);
      renderThread.enqueue( [&] {
         // glGenVertexArrays(1, &vao);
         glGenBuffers(1, &indexId);
         glGenBuffers(1, &vertexId);
         glGenBuffers(1, &materialId);
      } );
   }
   
   VAO_t::VAO_t(const VAO_t &that) noexcept {
      DEBUG_METHOD();
      // Needs to be here for code to compile but should never actually happen becuase only
      // compiled shapes have VAOs and we never copy them.
      fmt::print(stderr,"Error tried to copy construct a VAO. This should never happen.");
      abort();
   }

   VAO_t::VAO_t(VAO_t&& x) noexcept : VAO_t() {
      DEBUG_METHOD();
      *this = std::move(x);
   }

   VAO_t& VAO_t::operator=(VAO_t&& other) noexcept {
      DEBUG_METHOD();
      std::swap(vao, other.vao);
      std::swap(indexId, other.indexId);
      std::swap(vertexId, other.vertexId);
      std::swap(materialId, other.materialId);
      std::swap(vertices, other.vertices);
      std::swap(indices, other.indices);
      std::swap(materials, other.materials);
      std::swap(textures, other.textures);
      std::swap(transforms, other.transforms);
      return *this;
   }

   void VAO_t::bind() {
      DEBUG_METHOD();
      if (!vao)
         glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
      glBindBuffer(GL_ARRAY_BUFFER, vertexId);
   }

   void VAO_t::bind( attribute_t Position, attribute_t Normal, attribute_t Color,
                     attribute_t Coord,  attribute_t TUnit, attribute_t MIndex,
                     attribute_t Ambient,  attribute_t Specular, attribute_t Emissive, attribute_t Shininess) {
      DEBUG_METHOD();

      bind();
      Position.bind_vec3( sizeof(vertex_t), (void*)offsetof(vertex_t,position) );
      Normal.bind_vec3( sizeof(vertex_t),  (void*)offsetof(vertex_t,normal));
      Coord.bind_vec2( sizeof(vertex_t), (void*)offsetof(vertex_t,coord));
      Color.bind_vec4( sizeof(vertex_t), (void*)offsetof(vertex_t,fill));
      TUnit.bind_int( sizeof(vertex_t), (void*)offsetof(vertex_t,tunit));
      MIndex.bind_int( sizeof(vertex_t), (void*)offsetof(vertex_t,mindex));

      glBindBuffer(GL_ARRAY_BUFFER, materialId);
      Ambient.bind_vec4( sizeof(material_t), (void*)offsetof(material_t, ambient) );
      Specular.bind_vec4( sizeof(material_t), (void*)offsetof(material_t, specular) );
      Emissive.bind_vec4( sizeof(material_t), (void*)offsetof(material_t, emissive) );
      Shininess.bind_float( sizeof(material_t), (void*)offsetof(material_t, shininess) );

      glBindVertexArray(0);
   }

   template <typename T>
   static void loadBufferData(GLenum target, GLint bufferId, const std::vector<T> &data, GLenum usage) {
      glBindBuffer(target, bufferId);
      glBufferData(target, data.size() * sizeof(T), data.data(), usage);
   }

   void VAO_t::loadBuffers() const {
      DEBUG_METHOD();
      loadBufferData(GL_ARRAY_BUFFER, vertexId, vertices, GL_STREAM_DRAW);
      loadBufferData(GL_ARRAY_BUFFER, materialId, materials, GL_STREAM_DRAW);
      loadBufferData(GL_ELEMENT_ARRAY_BUFFER, indexId, indices, GL_STREAM_DRAW);
   }

   void VAO_t::draw() const {
      DEBUG_METHOD();
      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
      glBindVertexArray(0);
   }

   VAO_t::~VAO_t() {
      DEBUG_METHOD();
      // renderThread.enqueue( [&] {
         if (vao) {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &vao);
         }
         if (indexId)
            glDeleteBuffers(1, &indexId);
         if (vertexId)
            glDeleteBuffers(1, &vertexId);
         if (materialId)
            glDeleteBuffers(1, &materialId);
      // } );
   }

   int VAO_t::hasTexture(texture_t_ptr texture) {
      auto it = std::find(textures.begin(), textures.end(), texture);
      if (it == textures.end())
         return -1;
      else
         return std::distance(textures.begin(), it) ;
   }

   attribute_t::attribute_t(GLuint shaderID, const std::string &attribute) {
      id = glGetAttribLocation(shaderID, attribute.c_str());
      shaderId = shaderID;
   }

   void attribute_t::bind_vec2(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute_t::bind_vec3(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute_t::bind_vec4(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute_t::bind_int(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribIPointer( id, 1, GL_INT, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   void attribute_t::bind_float(std::size_t stride, void *offset) {
      if ( id != -1 ) {
         glVertexAttribPointer( id, 1, GL_FLOAT, GL_FALSE, stride, (void*)offset );
         glEnableVertexAttribArray(id);
      }
   }

   uniform_t::uniform_t(GLuint programID, const std::string &uniform) {
      id = glGetUniformLocation(programID, uniform.c_str());
   }

   void uniform_t::set(float value) const {
      if ( id != -1 )
         glUniform1f(id,value);
   }

   void uniform_t::set(int value) const {
      if ( id != -1 )
         glUniform1i(id,value);
   }

   void uniform_t::set(const std::array<int,2> &value) const {
      if ( id != -1 )
         glUniform2i(id, value[0], value[1] );
   }

   void uniform_t::set(const std::array<int,4> &value) const {
      if ( id != -1 )
         glUniform4i(id, value[0], value[1], value[2], value[3] );
   }

   void uniform_t::set(const glm::vec2 &value) const {
      if ( id != -1 )
         glUniform2fv(id, 1, glm::value_ptr(value) );
   }

   void uniform_t::set(const glm::vec3 &value) const {
      if ( id != -1 )
         glUniform3fv(id, 1, glm::value_ptr(value) );
   }

   void uniform_t::set(const glm::vec4 &value) const {
      if ( id != -1 )
         glUniform4fv(id, 1, glm::value_ptr(value) );
   }

   void uniform_t::set(const std::vector<int> &value) const {
      if ( id != -1 )
         glUniform1iv(id,value.size(),value.data());
   }

   void uniform_t::set(const std::vector<glm::vec2> &value) const {
      if ( id != -1 )
         glUniform2fv(id, value.size(), glm::value_ptr(value[0]) );
   }

   void uniform_t::set(const std::vector<glm::vec3> &value) const {
      if ( id != -1 )
         glUniform3fv(id, value.size(), glm::value_ptr(value[0]) );
   }

   void uniform_t::set(const std::vector<glm::vec4> &value) const {
      if ( id != -1 )
         glUniform4fv(id, value.size(), glm::value_ptr(value[0]) );
   }

   void uniform_t::set(const std::vector<glm::mat4> &value) const {
      if ( id != -1 )
         glUniformMatrix4fv(id, value.size(), GL_FALSE, glm::value_ptr(value[0]) );
   }

   void uniform_t::set(const std::vector<glm::mat3> &value) const {
      if ( id != -1 )
         glUniformMatrix3fv(id, value.size(), GL_FALSE, glm::value_ptr(value[0]) );
   }

   void uniform_t::set(const glm::mat4 &value) const {
      if ( id != -1 )
         glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(value) );
   }

   void enumerateUniforms(GLuint programID) {
      GLint size; // size of the variable
      GLenum type; // type of the variable (float, vec3 or mat4, etc)

      const GLsizei bufSize = 64; // maximum name length
      GLchar name[bufSize]; // variable name in GLSL
      GLsizei length; // name length

      GLint count;
      glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

      for (int i = 0; i < count; i++) {
         glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name);
         // uniformLocation[name] = glGetUniformLocation(programID, name);
      }
   }

   void enumerateAttributes(GLuint programID) {
      GLint size; // size of the variable
      GLenum type; // type of the variable (float, vec3 or mat4, etc)

      const GLsizei bufSize = 64; // maximum name length
      GLchar name[bufSize]; // variable name in GLSL
      GLsizei length; // name length

      GLint count;
      glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &count);

      for (int i = 0; i < count; i++) {
         glGetActiveAttrib(programID, (GLuint)i, bufSize, &length, &size, &type, name);
         // attribLocation[name] = glGetAttribLocation(programID, name);
      }

   }

} // namespace gl

template <>
struct fmt::formatter<gl::VAO_t> {
   // Format the MyClass object
   template <typename ParseContext>
   constexpr auto parse(ParseContext& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const gl::VAO_t& v, FormatContext& ctx) {
      return format_to(ctx.out(), "VAO:{:2} VID:{:2} IID:{:2} V{:8} I{:8} Tx{:2} Tr{:2}",
                       v.vao, v.vertexId, v.indexId,
                       v.vertices.size(), v.indices.size(), v.textures.size(), v.transforms.size());
   }
};
