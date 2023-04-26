#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_opengl_shaders.h"
#include "processing_math.h"
#include "processing_color.h"
#include "processing_pshape.h"
#include "processing_pimage.h"
#include "processing_pfont.h"
#include "processing_enum.h"

class PGraphics;

class gl_context {
public:
   const int CAPACITY = 65536;

   std::vector<PVector> vbuffer;
   std::vector<PVector> nbuffer;
   std::vector<PVector> cbuffer;
   std::vector<float> xbuffer;
   std::vector<unsigned short> ibuffer;
   gl_context() {
      vbuffer.reserve(CAPACITY);
      nbuffer.reserve(CAPACITY);
      cbuffer.reserve(CAPACITY);
      xbuffer.reserve(CAPACITY);
      ibuffer.reserve(CAPACITY);
   }

   void reserve(int n_vertices, PGraphics &pg) {
      if (n_vertices > CAPACITY) {
         abort();
      } else {
         int new_size = n_vertices + ibuffer.size();
         if (new_size >= CAPACITY) {
            flush(pg);
         }
      }

   }

   void flush(PGraphics &pg);


};

class PGraphics {
public:
   GLuint bufferID;
   GLuint localFboID;
   GLuint depthBufferID;
   GLuint whiteTextureID;
   GLuint programID;
   GLuint currentTextureID = 0;
   int nextTextureID = 2;
   int currentTextureWidth;
   int currentTextureHeight;
   gl_context glc;

   color stroke_color = WHITE;
   color fill_color = WHITE;
   color tint_color = WHITE;

   int stroke_weight = 1;
   int line_end_cap = ROUND;
   int ellipse_mode = CENTER;
   int rect_mode = CORNER;
   int image_mode = CORNER;

   int width;
   int height;

   PFont currentFont;
   int xTextAlign;
   int yTextAlign;

   float xsphere_ures = 30;
   float xsphere_vres = 30;

   bool xSmoothing = true;
   PShape _shape;
   std::vector<Uint32> pixels;

   std::array<float,3> xambientLight;
   std::array<float,3> xdirectionLightColor;
   std::array<float,3> xdirectionLightVector;

   GLuint AmbientLight;
   GLuint DirectionLightColor;
   GLuint DirectionLightVector;

   Eigen::Matrix4f projection_matrix; // Default is identity
   Eigen::Matrix4f view_matrix; // Default is identity

   GLuint Pmatrix;
   GLuint Vmatrix;
   GLuint uSampler;

   GLuint index_buffer_id;
   GLuint vertex_buffer_id;
   GLuint coords_buffer_id;
   GLuint colors_buffer_id;
   GLuint normal_buffer_id;
   GLuint vertex_attrib_id;
   GLuint coords_attrib_id;
   GLuint colors_attrib_id;
   GLuint normal_attrib_id;

   std::vector<Eigen::Matrix4f> matrix_stack;
   Eigen::Matrix4f move_matrix; // Default is identity

   GLuint Mmatrix;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) {
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(depthBufferID, x.depthBufferID);
      std::swap(programID, x.programID);
      std::swap(currentTextureID, x.currentTextureID);
      std::swap(nextTextureID, x.nextTextureID);

      std::swap(stroke_color, x.stroke_color);
      std::swap(fill_color, x.fill_color);
      std::swap(tint_color, x.tint_color);

      std::swap(stroke_weight, x.stroke_weight);
      std::swap(line_end_cap, x.line_end_cap);
      std::swap(ellipse_mode, x.ellipse_mode);
      std::swap(rect_mode, x.rect_mode);
      std::swap(image_mode, x.image_mode);

      std::swap(width, x.width);
      std::swap(height, x.height);

      std::swap(currentFont, x.currentFont);
      std::swap(xTextAlign, x.xTextAlign);
      std::swap(yTextAlign, x.yTextAlign);
      std::swap(xsphere_ures, x.xsphere_ures);
      std::swap(xsphere_vres, x.xsphere_vres);
      std::swap(xSmoothing, x.xSmoothing);
      std::swap(_shape, x._shape);
      std::swap(pixels, x.pixels);

      std::swap(xambientLight, x.xambientLight);
      std::swap(xdirectionLightColor, x.xdirectionLightColor);
      std::swap(xdirectionLightVector, x.xdirectionLightVector);
      std::swap(AmbientLight, x.AmbientLight);
      std::swap(DirectionLightColor, x.DirectionLightColor);
      std::swap(DirectionLightVector, x.DirectionLightVector);
      std::swap(projection_matrix, x.projection_matrix);
      std::swap(view_matrix, x.view_matrix);
      std::swap(Pmatrix, x.Pmatrix);
      std::swap(Vmatrix, x.Vmatrix);
      std::swap(uSampler, x.uSampler);
      std::swap(index_buffer_id, x.index_buffer_id);
      std::swap(vertex_buffer_id, x.vertex_buffer_id);
      std::swap(coords_buffer_id, x.coords_buffer_id);
      std::swap(colors_buffer_id, x.colors_buffer_id);
      std::swap(normal_buffer_id, x.normal_buffer_id);
      std::swap(vertex_attrib_id, x.vertex_attrib_id);
      std::swap(coords_attrib_id, x.coords_attrib_id);
      std::swap(colors_attrib_id, x.colors_attrib_id);
      std::swap(normal_attrib_id, x.normal_attrib_id);
      std::swap(matrix_stack, x.matrix_stack);
      std::swap(move_matrix, x.move_matrix);
      std::swap(Mmatrix, x.Mmatrix);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x){
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(depthBufferID, x.depthBufferID);
      std::swap(programID, x.programID);
      std::swap(currentTextureID, x.currentTextureID);
      std::swap(nextTextureID, x.nextTextureID);

      std::swap(stroke_color, x.stroke_color);
      std::swap(fill_color, x.fill_color);
      std::swap(tint_color, x.tint_color);

      std::swap(stroke_weight, x.stroke_weight);
      std::swap(line_end_cap, x.line_end_cap);
      std::swap(ellipse_mode, x.ellipse_mode);
      std::swap(rect_mode, x.rect_mode);
      std::swap(image_mode, x.image_mode);

      std::swap(width, x.width);
      std::swap(height, x.height);

      std::swap(currentFont, x.currentFont);
      std::swap(xTextAlign, x.xTextAlign);
      std::swap(yTextAlign, x.yTextAlign);
      std::swap(xsphere_ures, x.xsphere_ures);
      std::swap(xsphere_vres, x.xsphere_vres);
      std::swap(xSmoothing, x.xSmoothing);
      std::swap(_shape, x._shape);
      std::swap(pixels, x.pixels);

      std::swap(xambientLight, x.xambientLight);
      std::swap(xdirectionLightColor, x.xdirectionLightColor);
      std::swap(xdirectionLightVector, x.xdirectionLightVector);
      std::swap(AmbientLight, x.AmbientLight);
      std::swap(DirectionLightColor, x.DirectionLightColor);
      std::swap(DirectionLightVector, x.DirectionLightVector);

      std::swap(projection_matrix, x.projection_matrix);
      std::swap(view_matrix, x.view_matrix);
      std::swap(Pmatrix, x.Pmatrix);
      std::swap(Vmatrix, x.Vmatrix);
      std::swap(uSampler, x.uSampler);
      std::swap(index_buffer_id, x.index_buffer_id);
      std::swap(vertex_buffer_id, x.vertex_buffer_id);
      std::swap(coords_buffer_id, x.coords_buffer_id);
      std::swap(colors_buffer_id, x.colors_buffer_id);
      std::swap(normal_buffer_id, x.normal_buffer_id);
      std::swap(vertex_attrib_id, x.vertex_attrib_id);
      std::swap(coords_attrib_id, x.coords_attrib_id);
      std::swap(colors_attrib_id, x.colors_attrib_id);
      std::swap(normal_attrib_id, x.normal_attrib_id);

      std::swap(matrix_stack, x.matrix_stack);
      std::swap(move_matrix, x.move_matrix);
      std::swap(Mmatrix, x.Mmatrix);
      return *this;
   }

   PGraphics() {
      localFboID = 0;
      bufferID = 0;
      depthBufferID = 0;
   }

   ~PGraphics() {
      if (localFboID)
         glDeleteFramebuffers(1, &localFboID);
      // if (bufferID)
      //    glDeleteTextures(1, &bufferID);
      // if (depthBufferID)
      //    glDeleteRenderbuffers(1, &depthBufferID);
   }

   PGraphics(int z_width, int z_height, int mode, bool fb = false) {
      // Initialize GLEW
      glewExperimental = true; // Needed for core profile
      if (glewInit() != GLEW_OK) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glew init error\n");
         abort();
      }

      if (!glewIsSupported("GL_EXT_framebuffer_object")) {
         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "framebuffer object is not supported, you cannot use it\n");
         abort();
      }

      width = z_width;
      height = z_height;

      // create the texture array
      glGenTextures(1, &bufferID);
      glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);
      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      // set texture parameters
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (fb) {
         // Use main framebuffer
         localFboID = 0;
      } else {
         // Create a framebuffer object
         glGenFramebuffers(1, &localFboID);
         glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
         glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, bufferID, 0, 1);

         glGenRenderbuffers(1, &depthBufferID);
         glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

         // Attach the depth buffer to the framebuffer object
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
//         glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, bufferID, 0, 2);
         auto err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if (err != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr,"%d\n",err);
         }
    }

      // Create a white OpenGL texture, this will be the default texture if we don't specify any coords
      whiteTextureID = 0;
      GLubyte white[4] = { 255, 255, 255, 255 };
      GLubyte red[4] = { 255, 0, 0, 255 };
      GLubyte green[4] = { 0, 255, 0, 255 };
      GLubyte blue[4] = { 0, 0, 255, 255 };
      glClearTexImage(bufferID, whiteTextureID, GL_RGBA, GL_UNSIGNED_BYTE, white);
      glClearTexImage(bufferID, 1, GL_RGBA, GL_UNSIGNED_BYTE, red);
      glClearTexImage(bufferID, 2, GL_RGBA, GL_UNSIGNED_BYTE, green);
      glClearTexImage(bufferID, 3, GL_RGBA, GL_UNSIGNED_BYTE, blue);
      glClearTexImage(bufferID, 4, GL_RGBA, GL_UNSIGNED_BYTE, red);
      glClearTexImage(bufferID, 5, GL_RGBA, GL_UNSIGNED_BYTE, green);
      glClearTexImage(bufferID, 6, GL_RGBA, GL_UNSIGNED_BYTE, blue);
      glClearTexImage(bufferID, 7, GL_RGBA, GL_UNSIGNED_BYTE, blue);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);

      programID = LoadShaders(Shaders3D());
      glUseProgram(programID);

      AmbientLight = glGetUniformLocation(programID, "ambientLight");
      DirectionLightColor = glGetUniformLocation(programID, "directionLightColor");
      DirectionLightVector = glGetUniformLocation(programID, "directionLightVector");

      uSampler = glGetUniformLocation(programID, "myTextures");

      int textureUnitIndex = 0;
      glUniform1i(uSampler,0);
      glActiveTexture(GL_TEXTURE0 + textureUnitIndex);

      // Get a handle for our "MVP" uniform
      Pmatrix = glGetUniformLocation(programID, "Pmatrix");
      Vmatrix = glGetUniformLocation(programID, "Vmatrix");


      glGenBuffers(1, &index_buffer_id);
      glGenBuffers(1, &vertex_buffer_id);
      glGenBuffers(1, &coords_buffer_id);
      glGenBuffers(1, &colors_buffer_id);
      glGenBuffers(1, &normal_buffer_id);
      vertex_attrib_id = glGetAttribLocation(programID, "position");
      normal_attrib_id = glGetAttribLocation(programID, "normal");
      coords_attrib_id = glGetAttribLocation(programID, "coords");
      colors_attrib_id = glGetAttribLocation(programID, "colors");

      Mmatrix = glGetUniformLocation(programID, "Mmatrix");

      move_matrix = Eigen::Matrix4f::Identity();

      textFont( createFont("DejaVuSans.ttf",12));
      noLights();
      camera();
      perspective();

      background(WHITE);
   }

   void saveFrame( std::string fileName = "frame-####.png" ) {
      static int counter = 0;
      int c = counter;
      std::size_t pos = fileName.rfind('#');
      while (pos != std::string::npos) {
         fileName[pos] = '0' + (c % 10);
         c /= 10;
         pos = fileName.rfind('#', pos - 1);
      }
      PImage::saveFrame( localFboID, width, height, fileName );
      counter++;
   }
   void pushMatrix() {
      matrix_stack.push_back(move_matrix);
   }

   void popMatrix() {
      flush();
      move_matrix = matrix_stack.back();
      matrix_stack.pop_back();
   }

   void translate(float x, float y, float z=0) {
      flush();
      move_matrix = move_matrix * TranslateMatrix(PVector{x,y,z});
   }

   void transform(Eigen::Matrix4f transform_matrix) {
      flush();
      move_matrix = move_matrix * transform_matrix;
   }

   void scale(float x, float y,float z = 1) {
      flush();
      move_matrix = move_matrix * ScaleMatrix(PVector{x,y,z});
   }

   void scale(float x) {
      flush();
      scale(x,x,x);
   }

   void rotate(float angle, PVector axis) {
      flush();
      move_matrix = move_matrix * RotateMatrix(angle,axis);
   }

   void rotate(float angle) {
      flush();
      move_matrix = move_matrix * RotateMatrix(angle,PVector{0,0,1});
   }

   void rotateY(float angle) {
      flush();
      move_matrix = move_matrix * RotateMatrix(angle,PVector{0,1,0});
   }

   void rotateX(float angle) {
      flush();
      move_matrix = move_matrix * RotateMatrix(angle,PVector{1,0,0});
   }

   Eigen::Matrix4f get_projection_matrix(float fov, float a, float near, float far) {
      float f = 1 / tan(0.5 * fov);
      float rangeInv = 1.0 / (near - far);
      float A = (near + far) * rangeInv;
      float B = near * far * rangeInv * 2;
      Eigen::Matrix4f ret = Eigen::Matrix4f{
         { f/a,  0,  0,  0 },
         {   0,  f,  0,  0 },
         {   0,  0,  A,  B },
         {   0,  0, -1,  0 }
      };
      return ret;
   }

   void ortho(float left, float right, float bottom, float top, float near, float far) {
      float tx = -(right + left) / (right - left);
      float ty = -(top + bottom) / (top - bottom);
      float tz = -(far + near) / (far - near);

      projection_matrix = Eigen::Matrix4f{
         { 2/(right-left),               0,              0,  tx },
         {              0,  2/(top-bottom),              0,  ty },
         {              0,               0, -2/(far - near), tz },
         {              0,               0,              0,   1 }
      };
   }

   void ortho(float left, float right, float bottom, float top) {
      ortho(left, right, bottom, top, bottom*2, top*2);
   }

   void ortho() {
      ortho(-width / 2.0, width / 2.0, -height / 2.0, height / 2.0);
   }

   void perspective(float angle, float aspect, float minZ, float maxZ) {
      flush();
      projection_matrix = get_projection_matrix(angle, aspect, minZ, maxZ);
   }

   void perspective() {
      float fov = PI/3.0;
      float cameraZ = (height/2.0) / tan(fov/2.0);
      perspective( fov, (float)width/(float)height, cameraZ/10.0,  cameraZ*10.0);
   }

   void camera( float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ ) {

      Eigen::Vector3f center = Eigen::Vector3f{centerX, centerY, centerZ};
      Eigen::Vector3f eye =  Eigen::Vector3f{eyeX,eyeY,eyeZ};
      Eigen::Vector3f _up = Eigen::Vector3f{upX,upY,upZ};

      Eigen::Vector3f forward = (center - eye).normalized();
      Eigen::Vector3f side = forward.cross(_up).normalized();
      Eigen::Vector3f up = side.cross(forward).normalized();

      Eigen::Matrix4f view{
         {    side[0],     side[1],     side[2], 0.0f},
         {      up[0],       up[1],       up[2], 0.0f},
         {-forward[0], -forward[1], -forward[2], 0.0f},
         {       0.0f,        0.0f,        0.0f, 1.0f} };

      Eigen::Matrix4f translate{
         {1.0,    0,     0,    -eyeX} ,
         {0,    1.0,     0,    -eyeY},
         {0,      0,   1.0,    -eyeZ},
         {0.0f, 0.0f,  0.0f,    1.0f} };

      flush();
      // Translate the camera to the origin
      view_matrix = view * translate;
   }

   void camera() {
      camera(width / 2.0, height / 2.0, (height / 2.0) / tan(PI * 30.0 / 180.0),
             width / 2.0, height / 2.0, 0, 0, 1, 0);
   }

   void directionalLight(float r, float g, float b, float nx, float ny, float nz) {
      flush();
      xdirectionLightColor = {r/255.0f, r/255.0f, r/255.0f};
      xdirectionLightVector = {nx, ny, nz};
      glUniform3fv(DirectionLightColor, 1, xdirectionLightColor.data() );
      glUniform3fv(DirectionLightVector, 1,xdirectionLightVector.data() );
   }

   void ambientLight(float r, float g, float b) {
      flush();
      xambientLight = { r/255.0f, g/255.0f, b/255.0f };
      glUniform3fv(AmbientLight, 1, xambientLight.data() );
   }

   void lights() {
      ambientLight(128, 128, 128);
      directionalLight(128, 128, 128, 0, 0, -1);
      //lightFalloff(1, 0, 0);
      //lightSpecular(0, 0, 0);
   };

   void noLights() {
      ambientLight(255.0, 255.0, 255.0);
      directionalLight(0.0,0.0,0.0, 0.0,0.0,-1.0);
   }

   void textFont(PFont font) {
      currentFont = font;
   }

   void textAlign(int x, int y) {
      xTextAlign = x;
      yTextAlign = y;
   }

   void textAlign(int x) {
      xTextAlign = x;
   }

   void textSize(int size) {
      currentFont = createFont(currentFont.name, size);
   }


   void text(std::string text, float x, float y, float twidth=-1, float theight=-1) {
      int texture = getNextTextureID();
      currentFont.render_text(bufferID, texture, text, fill_color, twidth, theight);

      // this works well enough for the Letters.cc example but it's not really general
      if ( xTextAlign == CENTER ) {
         x = x - twidth / 2;
      }
      if ( yTextAlign == CENTER ) {
         y = y - theight / 2;
      }

      float xrange = (1.0 * twidth) / width;
      float yrange = (1.0 * theight) / height;
      drawTexturedQuad(PVector{x,y},PVector{x+twidth,y},PVector{x+twidth,y+theight}, PVector{x,y+theight},
                       xrange, yrange, texture, WHITE);
   }

   void text(char c, float x, float y, float twidth = -1, float theight = -1) {
      std::string s(&c,1);
      text(s,x,y,twidth,theight);
   }

   void background(float r, float g, float b) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      auto color = flatten_color_mode(r,g,b,color::scaleA);
      // Set clear color
      glClearColor(color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);
      // Clear screen
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   void background(color c) {
      background(c.r,c.g,c.b);
   }

   void background(float gray) {
      if (color::mode == HSB) {
         background(0,0,gray);
      } else {
         background(gray,gray,gray);
      }
   }

   class GL_FLOAT_buffer {
      GLuint buffer_id = 0;
      GLuint attribId;
   public:
      GL_FLOAT_buffer(GLuint buffer_id, GLuint programID, const void *data, int size, GLuint attribId, int count, int stride, void* offset) {
         if (size > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
            glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, GL_STREAM_DRAW);
            glVertexAttribPointer(
               attribId,                         // attribute
               count,                                // size
               GL_FLOAT,                         // type
               GL_FALSE,                         // normalized?
               stride,                           // stride
               offset                     // array buffer offset
               );
            glEnableVertexAttribArray(attribId);
         }
      }
      ~GL_FLOAT_buffer() {
      }
   };

   void drawTrianglesDirect( const std::vector<PVector> &vertices,
                       const std::vector<PVector> &normals,
                       const std::vector<PVector> &coords,
                       const std::vector<float> &colors,
                       const std::vector<unsigned short> &indices,
                       int count ) {

      // Create a vertex array object (VAO)
      GLuint VAO;
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      GL_FLOAT_buffer vertex( vertex_buffer_id, programID, vertices.data(), vertices.size() * 3,
                              vertex_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer normal( normal_buffer_id, programID, normals.data(),  normals.size() * 3,
                              normal_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer coord(  coords_buffer_id, programID, coords.data(),   coords.size()  * 3,
                              coords_attrib_id, 3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer color(  colors_buffer_id, programID, colors.data(),   colors.size(),
                              colors_attrib_id, 4, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);
      glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      // Unbind the buffer objects and VAO
      glDeleteVertexArrays(1, &VAO);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
   }

   void drawTriangles( const std::vector<PVector> &vertices,
                       const std::vector<PVector> &normals,
                       const std::vector<PVector> &coords,
                       const std::vector<unsigned short> &indices,
                       color color) {
      if (indices.size() == 0) abort();

      glc.reserve( vertices.size(), *this );

      auto starti = glc.vbuffer.size();

      glc.vbuffer.insert(glc.vbuffer.end(), vertices.begin(), vertices.end());
      glc.cbuffer.insert(glc.cbuffer.end(), coords.begin(),   coords.end());
      glc.nbuffer.insert(glc.nbuffer.end(), normals.begin(),  normals.end());

      for (auto vertex : vertices) {
         glc.xbuffer.push_back( color.r / 255.0f );
         glc.xbuffer.push_back( color.g / 255.0f );
         glc.xbuffer.push_back( color.b / 255.0f );
         glc.xbuffer.push_back( color.a / 255.0f );
      }

      for (auto index : indices) {
         glc.ibuffer.push_back( starti + index  );
      }
   }

   void flush() {
      glc.flush(*this);
   }

   void noTexture() {
      currentTextureID = 0;
   }

   void texture(PImage &img) {
      currentTextureID = getNextTextureID();
      img.load_texture_to_layer( currentTextureID );
      currentTextureWidth = img.width;
      currentTextureHeight = img.height;
   }

   void imageMode(int iMode) {
      image_mode = iMode;
   }

   void tint(float r,float g,  float b, float a) {
      tint_color = flatten_color_mode(r,g,b,a);
   }

   void tint(float r,float g, float b) {
      tint(r,g,b,color::scaleA);
   }

   void tint(float r,float a) {
      if (color::mode == HSB) {
         tint(0,0,r,a);
      } else {
         tint(r,r,r,a);
      }
   }
   void tint(float r) {
      if (color::mode == HSB) {
         tint(r,0,0,color::scaleA);
      } else {
         tint(r,r,r,color::scaleA);
      }
   }

   void tint(color c) {
      tint(c.r,c.g,c.b,c.a);
   }

   void noTint() {
      tint_color = WHITE;
   }

   void box(float w, float h, float d) {
      w = w / 2;
      h = h / 2;
      d = d / 2;
      std::vector<PVector> vertices = {
         // Front face
         { -w, -h, d},
         {w, -h, d},
         {w, h, d},
         {-w, h, d},

         // Back face
         {-w, -h, -d},
         {-w, h, -d},
         {w, h, -d},
         {w, -h, -d},

         // Top face
         {-w, h, -d},
         {-w, h, d},
         {w, h, d},
         {w, h, -d},

         // Bottom face
         {  -w, -h, -d},
         {  w, -h, -d},
         {  w, -h, d},
         {  -w, -h, d},

         // Right face
         {  w, -h, -d},
         {  w, h, -d},
         {  w, h, d},
         {  w, -h, d},

         // Left face
         {  -w, -h, -d},
         {-w, -h, d},
         { -w, h, d},
         { -w, h, -d},
      };

      float z = currentTextureID;
      float xrange = (1.0 * currentTextureWidth) / width;
      float yrange = (1.0 * currentTextureHeight) / height;
      std::vector<PVector> coords = {
         // Front
         {0.0,  0.0, z},
         {xrange,  0.0, z},
         {xrange,  yrange, z},
         {0.0,  yrange, z},
         // Back
         {0.0,  0.0, z},
         {xrange,  0.0, z},
         {xrange,  yrange, z},
         {0.0,  yrange, z},
         // Top
         {0.0,  0.0, z},
         {xrange,  0.0, z},
         {xrange,  yrange, z},
         {0.0,  yrange, z},
         // Bottom
         {0.0,  0.0, z},
         {xrange,  0.0, z},
         {xrange,  yrange, z},
         {0.0,  yrange,z },
         // Right
         {0.0,  0.0,z },
         {xrange,  0.0,z },
         {xrange,  yrange,z },
         {0.0,  yrange, z},
         // Left
         {0.0,  0.0,z},
         {xrange,  0.0,z},
         {xrange,  yrange,z},
         {0.0,  yrange,z},
      };

      std::vector<PVector> normals = {
         // Front
         {0.0,  0.0,  1.0},
         {0.0,  0.0,  1.0},
         {0.0,  0.0,  1.0},
         {0.0,  0.0,  1.0},

         // Back
         {0.0,  0.0, -1.0},
         {0.0,  0.0, -1.0},
         {0.0,  0.0, -1.0},
         {0.0,  0.0, -1.0},

         // Top
         {0.0,  1.0,  0.0},
         {0.0,  1.0,  0.0},
         {0.0,  1.0,  0.0},
         {0.0,  1.0,  0.0},

         // Bottom
         {0.0, -1.0,  0.0},
         {0.0, -1.0,  0.0},
         {0.0, -1.0,  0.0},
         {0.0, -1.0,  0.0},

         // Right
         {1.0,  0.0,  0.0},
         {1.0,  0.0,  0.0},
         {1.0,  0.0,  0.0},
         {1.0,  0.0,  0.0},

         // Left
         {-1.0,  0.0,  0.0},
         {-1.0,  0.0,  0.0},
         {-1.0,  0.0,  0.0},
         {-1.0,  0.0,  0.0}
      };

      std::vector<unsigned short>  triangles = {
         0,1,2, 0,2,3, 4,5,6, 4,6,7,
         8,9,10, 8,10,11, 12,13,14, 12,14,15,
         16,17,18, 16,18,19, 20,21,22, 20,22,23
      };

      if (currentTextureID) {
         drawTriangles(vertices, normals, coords, triangles, tint_color);
      } else {
         drawTriangles(vertices, normals, coords, triangles, fill_color);
      }
   };

   void box(float size) {
      box(size, size, size);
   }

   void sphereDetail(float ures, float vres) {
      xsphere_ures = ures;
      xsphere_vres = vres;
   }

   void sphereDetail(float res) {
      sphereDetail(res, res);
   }

   void sphere(float radius) {

      std::vector<PVector> vertices;
      std::vector<PVector> normals;
      std::vector<PVector> coords;

      float latStep = M_PI / xsphere_ures;
      float lonStep = 2 * M_PI / xsphere_vres;

      for (int i = 0; i <= xsphere_ures; i++) {
         float lat = i * latStep;
         float cosLat = std::cos(lat);
         float sinLat = std::sin(lat);

         for (int j = 0; j <= xsphere_vres; j++) {
            float lon = j * lonStep;
            float cosLon = std::cos(lon);
            float sinLon = std::sin(lon);

            float x = sinLat * cosLon;
            float y = sinLat * sinLon;
            float z = cosLat;

            normals.push_back( {x,y,z} );
            vertices.push_back( { x * radius, y * radius, z * radius} );
         }
      }

      std::vector<unsigned short> indices;
      for (int i = 0; i < xsphere_ures; i++) {
         for (int j = 0; j < xsphere_vres; j++) {
            int idx0 = i * (xsphere_vres+1) + j;
            int idx1 = idx0 + 1;
            int idx2 = (i+1) * (xsphere_vres+1) + j;
            int idx3 = idx2 + 1;
            indices.push_back(idx0);
            indices.push_back(idx2);
            indices.push_back(idx1);
            indices.push_back(idx1);
            indices.push_back(idx2);
            indices.push_back(idx3);
         }
      }
      if (currentTextureID) {
         drawTriangles(vertices, normals,coords, indices, tint_color);
      } else {
         drawTriangles(vertices, normals,coords, indices, fill_color);
      }
   }

   int getNextTextureID() {
      if (nextTextureID == 8) {
         flush();
      }
      int textureID = nextTextureID;
      nextTextureID++;
      return textureID;
   }

   void image(PGraphics &gfx, int x, int y) {
      float left = x;
      float right = x + gfx.width;
      float top = y;
      float bottom = y + gfx.height;
      int texture = getNextTextureID();
      glBindTexture(GL_TEXTURE_2D_ARRAY, gfx.bufferID);
      glCopyImageSubData(gfx.bufferID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1,
                         bufferID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture,
                         gfx.width, gfx.height, 1);
      glBindTexture(GL_TEXTURE_2D_ARRAY, bufferID);
      float xrange = (1.0 * gfx.width) / width;
      float yrange = (1.0 * gfx.height) / height;
      drawTexturedQuad( {left, top},
                        {right,top},
                        {right, bottom},
                        {left, bottom},
                        xrange,yrange, texture, tint_color);
   }

   void image(PImage &pimage, float left, float top, float right, float bottom) {
      if ( image_mode == CORNER ) {
         float iwidth = right;
         float iheight = bottom;
         right = left + iwidth;
         bottom = top + iheight;
      } else if ( image_mode == CENTER ) {
         float iwidth = right;
         float iheight = bottom;
         left = left - ( iwidth / 2.0 );
         top = top - ( iheight / 2.0 );
         right = left + iwidth;
         bottom = top + iheight;
      }
      int texture = getNextTextureID();
      pimage.load_texture_to_layer(texture);
      float xrange = (1.0 * pimage.width) / width;
      float yrange = (1.0 * pimage.height) / height;

      drawTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom}, xrange,yrange,
                     texture, tint_color);
   }

   void image(PImage &pimage, float x, float y) {
      if ( image_mode == CORNER ) {
         image( pimage, x, y, pimage.width, pimage.height );;
      } else if ( image_mode == CORNERS ) {
         image( pimage, x, y, x + pimage.width, y + pimage.height );;
      } else   if (image_mode == CENTER) {
         image( pimage, x, y, pimage.width, pimage.height );
      } else {
         abort();
      }
   }

   void background(PImage &bg) {
      image(bg,0,0);
   }

   void loadPixels() {
      pixels.resize(width*height);
      // Read the pixel data from the framebuffer into the array
      glGetTextureSubImage(bufferID, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.size(), pixels.data());
   }

   void updatePixels() {
      // Write the pixel data to the framebuffer
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      // _pixels.clear();
      // pixels = NULL;
   }

   // ----
   // Begin shapes managed by Pshape.
   // ----
   void fill(float r,float g,  float b, float a) {
      fill_color = flatten_color_mode(r,g,b,a);
   }

   void fill(float r,float g, float b) {
      fill(r,g,b,color::scaleA);
   }

   void fill(float r,float a) {
      if (color::mode == HSB) {
         fill(0,0,r,a);
      } else {
         fill(r,r,r,a);
      }
   }

   void fill(float r) {
      if (color::mode == HSB) {
         fill(0,0,r,color::scaleA);
      } else {
         fill(r,r,r,color::scaleA);
      }
   }

   void fill(class color color) {
      fill(color.r,color.g,color.b,color.a);
   }

   void fill(class color color, float a) {
      fill(color.r,color.g,color.b,a);
   }

   void stroke(float r,float g,  float b, float a) {
      stroke_color = flatten_color_mode(r,g,b,a);
   }

   void stroke(float r,float g, float b) {
      stroke(r,g,b,color::scaleA);
   }

   void stroke(float r,float a) {
      if (color::mode == HSB) {
         stroke(0,0,r,a);
      } else {
         stroke(r,r,r,a);
      }
   }

   void rect(int x, int y, int _width, int _height) {
      PShape pshape = createRect(x,y,_width,_height);
      shape( pshape );
   }

   void stroke(float r) {
      if (color::mode == HSB) {
         stroke(r,0,0,color::scaleA);
      } else {
         stroke(r,r,r,color::scaleA);
      }
   }

   void stroke(color c) {
      stroke(c.r,c.g,c.b,c.a);
   }

   void strokeWeight(int x) {
      stroke_weight = x;
   }

   void noStroke() {
      stroke_color = {0,0,0,0};
   }

   void noFill() {
      fill_color = {0,0,0,0};
   }

   void ellipseMode(int mode) {
      ellipse_mode = mode;
   }

   void printMatrix4f(const Eigen::Matrix4f& mat) {
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
   }

   void drawTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, float xrange, float yrange, int textureLayer, color tint) {

      std::vector<PVector> vertices{ p0, p1, p2, p3 };
      float z = (float)textureLayer;
      std::vector<PVector> coords {
         { 0.0f, 0.0f,z},
         { xrange, 0.0f,z},
         { xrange, yrange,z},
         { 0.0f, yrange,z},
      };
      std::vector<PVector> normals;

      std::vector<unsigned short>  indices = {
         0,1,2, 0,2,3,
      };

      drawTriangles( vertices, normals, coords, indices, tint );
   }


   PLine drawLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) const {
      PLine l1{ p1, p2 };
      PLine l2{ p2, p3 };
      PLine low_l1 = l1.offset(-half_weight);
      PLine high_l1 = l1.offset(half_weight);
      PLine low_l2 = l2.offset(-half_weight);
      PLine high_l2 = l2.offset(half_weight);
      return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
   }

   PShape drawLinePoly(int points, const PVector *p, int weight, bool closed)  {
      PLine start;
      PLine end;

      PShape triangle_strip;
      triangle_strip.beginShape(TRIANGLE_STRIP);

      float half_weight = weight / 2.0;
      if (closed) {
         start = drawLineMitred(p[points-1], p[0], p[1], half_weight );
         end = start;
      } else {
         PVector normal = (p[1] - p[0]).normal();
         normal.normalize();
         normal.mult(half_weight);
         start = {  p[0] + normal, p[0] - normal };
         normal = (p[points-1] - p[points-2]).normal();
         normal.normalize();
         normal.mult(half_weight);
         end = { p[points-1] + normal, p[points-1] - normal };
      }

      triangle_strip.vertex( start.start );
      triangle_strip.vertex( start.end );

      for (int i =0; i<points-2;++i) {
         PLine next = drawLineMitred(p[i], p[i+1], p[i+2], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }
      if (closed) {
         PLine next = drawLineMitred(p[points-2], p[points-1], p[0], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }

      triangle_strip.vertex( end.start );
      triangle_strip.vertex( end.end );

      triangle_strip.endShape(CLOSE);
      return triangle_strip;
   }

   PShape drawRoundLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      int NUMBER_OF_VERTICES=16;

      float start_angle = PVector{p2.x-p1.x,p2.y-p1.y}.heading() + HALF_PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(p1.x + cos(i + start_angle) * weight/2, p1.y + sin(i+start_angle) * weight/2);
      }

      start_angle += PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(p2.x + cos(i+start_angle) * weight/2, p2.y + sin(i+start_angle) * weight/2);
      }
      shape.endShape(CLOSE);
      return shape;
   }

   PShape drawLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      shape.vertex(p1 + normal);
      shape.vertex(p1 - normal);
      shape.vertex(p2 - normal);
      shape.vertex(p2 + normal);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape drawCappedLine(PVector p1, PVector p2, int weight) const {

      PShape shape;
      shape.beginShape(POLYGON);
      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      PVector end_offset = PVector{p2.x-p1.x,p2.y-p1.y};
      end_offset.normalize();
      end_offset.mult(weight/2.0);

      shape.vertex(p1 + normal - end_offset);
      shape.vertex(p1 - normal - end_offset);
      shape.vertex(p2 - normal + end_offset);
      shape.vertex(p2 + normal + end_offset);

      shape.endShape(CLOSE);
      return shape;
   }

   void _line(PShape &triangles, PVector p1, PVector p2, int weight) const {

      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      PVector v[] = {
         p1 + normal,
         p1 - normal,
         p2 - normal,
         p2 + normal,
      };

      triangles.vertex( v[0] );
      triangles.vertex( v[1] );
      triangles.vertex( v[2] );

      triangles.vertex( v[0] );
      triangles.vertex( v[2] );
      triangles.vertex( v[3] );
   }

   PShape drawTriangleStrip(int points, const PVector *p,int weight) {
      PShape triangles;
      triangles.beginShape(TRIANGLES);
      _line(triangles, p[0], p[1], weight);
      for (int i=2;i<points;++i) {
         _line(triangles, p[i-1], p[i], weight);
         _line(triangles, p[i], p[i-2], weight);
      }
      triangles.endShape();
      return triangles;
   }

   void shape_stroke(PShape &pshape, float x, float y, float swidth, float sheight, color color) {
      if (color.a == 0)
         return;
      switch( pshape.style ) {
      case POINTS:
      {
         for (auto z : pshape.vertices ) {
            shape_fill( _createEllipse(z.x, z.y, stroke_weight, stroke_weight, CENTER),0,0,0,0,color );
         }
         break;
      }
      case POLYGON:
      {
         if (pshape.vertices.size() > 2 ) {
            if (pshape.type == OPEN_SKIP_FIRST_VERTEX_FOR_STROKE) {
               shape_fill( drawLinePoly( pshape.vertices.size() - 1, pshape.vertices.data() + 1, stroke_weight, false),
                           0,0,0,0,color );
            } else {
               shape_fill( drawLinePoly( pshape.vertices.size(), pshape.vertices.data(), stroke_weight, pshape.type == CLOSE),
                           0,0,0,0,color );
            }
         } else if (pshape.vertices.size() == 2) {
            switch(line_end_cap) {
            case ROUND:
               shape_fill( drawRoundLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), 0,0,0,0,color );
               break;
            case PROJECT:
               shape_fill( drawCappedLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), 0,0,0,0,color );
               break;
            case SQUARE:
               shape_fill( drawLine(pshape.vertices[0], pshape.vertices[1], stroke_weight), 0,0,0,0,color );
               break;
            default:
               abort();
            }
         } else if (pshape.vertices.size() == 1) {
            shape_fill( _createEllipse(pshape.vertices[0].x, pshape.vertices[0].y, stroke_weight, stroke_weight, CENTER),0,0,0,0,color );
         }
         break;
      }
      case TRIANGLE_STRIP:
      {
         shape_fill( drawTriangleStrip( pshape.vertices.size(), pshape.vertices.data(), stroke_weight),0,0,0,0,color );
         break;
      }
      case TRIANGLES:
         break;
      default:
         abort();
         break;
      }
   }

   void shape_fill(const PShape &pshape, float x, float y, float swidth, float sheight, color color) {
      std::vector<PVector> normals;
      std::vector<PVector> coords;
      if (color.a == 0)
         return;
      switch( pshape.style ) {
      case POINTS:
         break;
      case POLYGON:
      {
         if (pshape.vertices.size() > 2) {
            if (pshape.indices.size() == 0) {
               std::vector<PVector> triangles = triangulatePolygon({pshape.vertices.begin(),pshape.vertices.end()});
               std::vector<unsigned short> indices;
               for (int i = 0; i < triangles.size(); i ++ ){
                  indices.push_back(i);
               }
               drawTriangles(triangles, normals, coords, indices, color );
            } else {
               drawTriangles( pshape.vertices, normals, coords,pshape.indices,  color );
            }
         }
      }
      break;
      case TRIANGLES:
         if (pshape.indices.size() == 0) {
            std::vector<unsigned short> indices;
            for (int i = 0; i < pshape.vertices.size(); i ++ ){
               indices.push_back(i);
            }
            drawTriangles(  pshape.vertices, normals, coords, indices, color );
         } else {
            drawTriangles(  pshape.vertices, normals, coords, pshape.indices, color );
         }
         break;
      case TRIANGLE_STRIP:
      {
         if (pshape.indices.size() != 0) { abort();}
         std::vector<unsigned short> indices;
         for (int i = 0; i < pshape.vertices.size()-2; i ++ ){
            indices.push_back(i);
            indices.push_back(i+1);
            indices.push_back(i+2);
         }
         drawTriangles(  pshape.vertices, normals, coords, indices, color );
      }
      break;
      default:
         abort();
      }
   }

   void shape(PShape &pshape, float x, float y, float swidth, float sheight) {
      pushMatrix();
      translate(x,y);
      scale(1,1); // Need to fix this properly
      transform( pshape.shape_matrix );
      if ( pshape.style == GROUP ) {
         for (auto &&child : pshape.children) {
            shape(child,0,0,0,0);
         }
      } else {
         shape_fill(pshape, x,y,swidth,sheight,fill_color);
         shape_stroke(pshape, x,y,swidth,sheight, stroke_color);
      }
      popMatrix();
   }

   void shape(PShape &pshape) {
      shape(pshape,0,0,0,0);
   }

   void ellipse(float x, float y, float width, float height) {
      PShape pshape = createEllipse(x, y, width, height);
      shape( pshape );
   }

   void ellipse(float x, float y, float radius) {
      ellipse(x,y,radius,radius);
   }

   void arc(float x, float y, float width, float height, float start, float stop, int mode = DEFAULT) {
      PShape pshape = createArc(x, y, width, height, start, stop, mode);
      shape( pshape );
   }

   void strokeCap(int cap) {
      line_end_cap = cap;
   }

   void line(float x1, float y1, float x2, float y2) {
      PShape pshape = createLine( x1, y1, x2, y2);
      shape( pshape );
   }

   void line(float x1, float y1, float z1, float x2, float y2, float z2) {
      abort();
   }

   void line(PVector start, PVector end) {
      line(start.x,start.y, end.x,end.y);
   }

   void line(PLine l) {
      line(l.start, l.end);
   }

   void point(float x, float y) {
      PShape pshape = createPoint(x, y);
      shape( pshape );
   }

   void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
      PShape pshape = createQuad(x1, y1, x2, y2, x3, y3, x4, y4);
      shape( pshape );
   }

   void triangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape pshape = createTriangle( x1, y1, x2, y2, x3, y3 );
      shape( pshape );
   }

   void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape pshape = createBezier(x1, y1, x2, y2, x3, y3, x4, y4);
      shape( pshape );
   }

   void beginShape(int points = POLYGON) {
      _shape = PShape();
      _shape.beginShape(points);
   }

   void vertex(float x, float y, float z = 0.0) {
      _shape.vertex(x, y, z);
   }

   void bezierVertex(float x2, float y2, float x3, float y3, float x4, float y4) {
      _shape.bezierVertex(x2,y2, x3,y3,x4,y4);
   }

   void endShape(int type = OPEN) {
      _shape.endShape(type);
      shape(_shape, 0,0,0,0);
   }

   void rectMode(int mode){
      rect_mode = mode;
   }

   PShape createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape bezier;
      bezier.beginShape();
      bezier.vertex(x1, y1);
      bezier.bezierVertex(x2, y2, x3, y3, x4, y4);
      bezier.endShape(OPEN);
      return bezier;
   }

   PShape createRect(float x, float y, float width, float height) {
      if (rect_mode == CORNERS) {
         width = width - x;
         height = height - y;
      } else if (rect_mode == CENTER) {
         x = x - width / 2;
         y = y - height / 2;
      } else if (rect_mode == RADIUS) {
         width *= 2;
         height *= 2;
         x = x - width / 2;
         y = y - height / 2;
      }
      PShape shape;
      shape.beginShape(POLYGON);
      shape.vertex(x,y);
      shape.vertex(x+width,y);
      shape.vertex(x+width,y+height);
      shape.vertex(x,y+height);
      shape.indices = { 0,1,2,0,2,3 };
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
      PShape shape;
      shape.beginShape(POLYGON);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.vertex(x4, y4);
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createLine(float x1, float y1, float x2, float y2) {
      PShape shape;
      shape.beginShape(POLYGON);
      shape.vertex(x1,y1);
      shape.vertex(x2,y2);
      shape.endShape(OPEN);
      return shape;
   }

   PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      PShape shape;
      shape.beginShape(TRIANGLES);
      shape.vertex(x1, y1);
      shape.vertex(x2, y2);
      shape.vertex(x3, y3);
      shape.indices = { 0,1,2 };
      shape.endShape(CLOSE);
      return shape;
   }

   float sincos[32][2] = {
      { 1.000000, -0.000000 },
      { 0.980785, 0.195090 },
      { 0.923880, 0.382683 },
      { 0.831470, 0.555570 },
      { 0.707107, 0.707107 },
      { 0.555570, 0.831470 },
      { 0.382683, 0.923880 },
      { 0.195090, 0.980785 },
      { 0.000000, 1.000000 },
      { -0.195090, 0.980785 },
      { -0.382683, 0.923880 },
      { -0.555570, 0.831470 },
      { -0.707107, 0.707107 },
      { -0.831470, 0.555570 },
      { -0.923880, 0.382683 },
      { -0.980785, 0.195090 },
      { -1.000000, -0.000000 },
      { -0.980785, -0.195090 },
      { -0.923880, -0.382683 },
      { -0.831470, -0.555570 },
      { -0.707107, -0.707107 },
      { -0.555570, -0.831470 },
      { -0.382683, -0.923880 },
      { -0.195091, -0.980785 },
      { -0.000000, -1.000000 },
      { 0.195090, -0.980785 },
      { 0.382683, -0.923880 },
      { 0.555570, -0.831470 },
      { 0.707107, -0.707107 },
      { 0.831469, -0.555570 },
      { 0.923880, -0.382684 },
      { 0.980785, -0.195090 } };

   PVector fast_ellipse_point(const PVector &center, int index, float xradius, float yradius) {
      return PVector( center.x + xradius * sincos[index][0],
                      center.y + yradius * sincos[index][1],
                      center.z);
   }

   PVector ellipse_point(const PVector &center, int index, float start, float end, float xradius, float yradius) {
      float angle = map( index, 0, 32, start, end);
      return PVector( center.x + xradius * sin(-angle + HALF_PI),
                      center.y + yradius * cos(-angle + HALF_PI),
                      center.z);
   }

   PShape createUnitCircle(int NUMBER_OF_VERTICES = 32) {
      PShape shape;
      shape.beginShape(POLYGON);
      for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
         shape.vertex( ellipse_point( {0,0,0}, i, 0, TWO_PI, 1.0, 1.0 ) );
      }
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createEllipse(float x, float y, float width, float height) {
      return _createEllipse(x,y,width,height,ellipse_mode);
   }

   PShape _createEllipse(float x, float y, float width, float height, int ellipse_mode) {
      switch (ellipse_mode) {
      case CENTER:
         break;
      case RADIUS:
         width = width * 2.0;
         height = height * 2.0;
         break;
      case CORNER:
         x = x - width / 2.0;
         y = y - height / 2.0;
         break;
      case CORNERS:
         width = width - x;
         height = height - y;
         break;
         height;
      default:
         abort();
      }
      int NUMBER_OF_VERTICES=32;
      PShape shape;
      shape.beginShape(POLYGON);
      shape.vertex( fast_ellipse_point( {x,y}, 0, width / 2.0, height /2.0) );
      for(int i = 1; i < NUMBER_OF_VERTICES-1; ++i) {
         shape.vertex( fast_ellipse_point( {x,y}, i, width / 2.0, height /2.0) );
         shape.indices.push_back( 0 );
         shape.indices.push_back( i );
         shape.indices.push_back( i+1 );
      }
      shape.vertex( fast_ellipse_point( {x,y}, NUMBER_OF_VERTICES-1, width / 2.0, height /2.0) );
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createArc(float x, float y, float width, float height, float start,
                    float stop, int mode) {

      if (ellipse_mode != RADIUS) {
         width /=2;
         height /=2;
      }
      int strokeMode, fillMode;
      switch( mode ) {
      case DEFAULT:
         strokeMode = OPEN_SKIP_FIRST_VERTEX_FOR_STROKE;
         fillMode = PIE;
         break;
      case PIE:
         strokeMode = CLOSE;
         fillMode = PIE;
         break;
      case CHORD:
         strokeMode = CLOSE;
         fillMode = CHORD;
         break;
      case OPEN:
         strokeMode = OPEN;
         fillMode = CHORD;
         break;
      default:
         abort();
      }

      PShape shape;
      shape.beginShape(POLYGON);
      int NUMBER_OF_VERTICES=32;
      if ( fillMode == PIE ) {
         shape.vertex(x,y);
      }
      for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
         shape.vertex( ellipse_point( {x,y}, i, start, stop, width, height ) );
      }
      shape.vertex( ellipse_point( {x,y}, 32, start, stop, width, height ) );
      shape.endShape(strokeMode);
      return shape;
   }

   PShape createPoint(float x, float y) {
      PShape shape;
      shape.beginShape(POINTS);
      shape.vertex(x,y);
      shape.endShape();
      return shape;
   }



// ----
// End shapes managed by Pshape.
// ----



   void noSmooth() {
      // Doesn't yet apply to actual graphics
      xSmoothing = false;
   }

   void beginDraw() {}
   void endDraw() {}

   void draw_main() {
      flush();
      // Reset to default view settings to draw next frame and
      // draw the texture from the PGraphics flat.
      noLights();
      move_matrix = Eigen::Matrix4f::Identity();

      Eigen::Matrix4f new_projection_matrix = TranslateMatrix(PVector{-1,-1,0}) * ScaleMatrix(PVector{2.0f/width, 2.0f/height,1.0});

      // For drawing the main screen we need to flip the texture and remove any tint
      std::vector<PVector> vertices{
         {0.0f,       0.0f+height},
         {0.0f+width ,0.0f+height},
         {0.0f+width, 0.0f},
         {0.0f,       0.0f}};

      float z = 1.0;
      std::vector<PVector> coords {
         { 0.0f, 0.0f, z},
         { 1.0f, 0.0f, z},
         { 1.0f, 1.0f, z},
         { 0.0f, 1.0f, z},
      };
      std::vector<PVector> normals;

      std::vector<unsigned short>  indices = {
         0,1,2, 0,2,3,
      };
      std::vector<float> colors {
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
      };

      // bind the real frame buffer
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      // Clear the color and depth buffers
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
      glUniformMatrix4fv(Pmatrix, 1,false, new_projection_matrix.data());
      glUniformMatrix4fv(Vmatrix, 1,false, move_matrix.data());
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      drawTrianglesDirect( vertices, normals, coords, colors, indices, indices.size());

      // Clear the depth buffer but not what was drawn for the next frame
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      glClear(GL_DEPTH_BUFFER_BIT);
   }

   PGraphics createGraphics(int width, int height, int mode = P2D) {
      PGraphics pg{ width, height, mode };
      pg.view_matrix = view_matrix;
      pg.projection_matrix = projection_matrix;
      return pg;
   }

};


void gl_context::flush(PGraphics &pg) {
   glUniformMatrix4fv(pg.Mmatrix, 1,false, pg.move_matrix.data());
   glUniformMatrix4fv(pg.Pmatrix, 1,false, pg.projection_matrix.data());
   glUniformMatrix4fv(pg.Vmatrix, 1,false, pg.view_matrix.data());

   pg.nextTextureID = 2;

   glBindFramebuffer(GL_FRAMEBUFFER, pg.localFboID);
   glBindTexture(GL_TEXTURE_2D_ARRAY, pg.bufferID);

   pg.drawTrianglesDirect( vbuffer, nbuffer, cbuffer, xbuffer, ibuffer, ibuffer.size() );
   vbuffer.clear();
   nbuffer.clear();
   cbuffer.clear();
   ibuffer.clear();
   xbuffer.clear();
}

#endif
