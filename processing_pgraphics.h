#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_math.h"
#include "processing_color.h"
#include "processing_pshape.h"
#include "processing_pimage.h"

extern GLuint programID;

enum {
   P2D, P3D
};

struct DrawingMode {
   int stroke_weight = 1;
   int line_end_cap = ROUND;
   int ellipse_mode = DIAMETER;
   int rect_mode = CORNER;
};

struct ColorMode {
   color stroke_color{255,255,255,255};
   color fill_color{255,255,255,255};
};


class PGraphics {
public:
   GLuint bufferID;
   GLuint localFboID;
   GLuint depthBufferID;
   GLuint whiteTextureID;

   DrawingMode dm{};
   ColorMode cm{};

   int gfx_width, gfx_height;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) {
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(depthBufferID, x.depthBufferID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(cm, x.cm);
      std::swap(dm, x.dm);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x){
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(depthBufferID, x.depthBufferID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(cm, x.cm);
      std::swap(dm, x.dm);
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
      if (bufferID)
         glDeleteTextures(1, &bufferID);
      if (depthBufferID)
         glDeleteRenderbuffers(1, &depthBufferID);
   }
   PGraphics(int width, int height, int mode, bool fb = false) {
      if (fb) {
         // Use main framebuffer
         localFboID = 0;
      } else {
         // Create a framebuffer object
         glGenFramebuffers(1, &localFboID);
         glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      }
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glDepthFunc(GL_LEQUAL);
      glEnable(GL_DEPTH_TEST);
      // Create a renderbuffer for the depth buffer
      if (!fb) {
         glGenRenderbuffers(1, &depthBufferID);
         glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

         // Attach the depth buffer to the framebuffer object
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
      }

      gfx_width = width;
      gfx_height = height;

      // Create a white OpenGL texture
      unsigned int white = 0xFFFFFFFF;
      glGenTextures(1, &whiteTextureID);
      glBindTexture(GL_TEXTURE_2D, currentTextureID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, &white);

      if (!fb) {
         // Create a texture to render to
         glGenTextures(1, &bufferID);
         glBindTexture(GL_TEXTURE_2D, bufferID);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferID, 0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
      GL_FLOAT_buffer(const void *data, int size, const char *attrib, int count, int stride, void* offset) {
         if (size > 0) {
            glGenBuffers(1, &buffer_id);
            glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
            glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data, GL_STATIC_DRAW);

            attribId = glGetAttribLocation(programID, attrib);
            glEnableVertexAttribArray(attribId);
            glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
            glVertexAttribPointer(
               attribId,                         // attribute
               count,                                // size
               GL_FLOAT,                         // type
               GL_FALSE,                         // normalized?
               stride,                           // stride
               offset                     // array buffer offset
               );
         }
      }
      ~GL_FLOAT_buffer() {
         if (buffer_id) {
            glDeleteBuffers(1, &buffer_id);
         }
      }
   };

   void drawGeometry( const std::vector<PVector> &vertices,
                      const std::vector<PVector> &normals,
                      const std::vector<PVector> &coords,
                      const std::vector<unsigned short> &indices,
                      GLuint element_type,
                      GLuint frame_buffer_ID,
                      color color) {

      glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_ID);

      if (currentTextureID) {
         glBindTexture(GL_TEXTURE_2D, currentTextureID);
         GLuint uSampler = glGetUniformLocation(programID, "uSampler");
         int textureUnitIndex = 0;
         glUniform1i(uSampler,0);
         glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
      } else {
         glBindTexture(GL_TEXTURE_2D, whiteTextureID);
      }

      extern GLuint Color;
      float color_vec[] = {
         color.r/255.0f,
         color.g/255.0f,
         color.b/255.0f,
         color.a/255.0f };
      glUniform4fv(Color, 1, color_vec);

      // Create a vertex array object (VAO)
      GLuint VAO;
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);

      GL_FLOAT_buffer vertex( vertices.data(), vertices.size() * 3, "position", 3,
                              sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer normal( normals.data(),  normals.size() * 3,  "normal",   3, sizeof(PVector), (void*)offsetof(PVector,x));
      GL_FLOAT_buffer coord(  coords.data(),   coords.size() * 3,   "coords",   2, sizeof(PVector), (void*)offsetof(PVector,x));

      if ( indices.size() > 0 ) {
         GLuint indexbuffer;
         glGenBuffers(1, &indexbuffer);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

         glDrawElements(element_type, indices.size(), GL_UNSIGNED_SHORT, 0);

         glDeleteBuffers(1, &indexbuffer);

         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      } else {
         glDrawArrays(element_type, 0, vertices.size());
      }

      // Unbind the buffer objects and VAO
      glDeleteVertexArrays(1, &VAO);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
   }

   GLuint currentTextureID = 0;

   void noTexture() {
      if (currentTextureID) {
         glDeleteTextures(1, &currentTextureID);
         currentTextureID = 0;
         glBindTexture(GL_TEXTURE_2D, whiteTextureID);
      }
   }
   void texture(PImage &img) {
      noTexture();
      auto newSurface = img.surface;
      // Create an OpenGL texture from the SDL_Surface
      glGenTextures(1, &currentTextureID);
      glBindTexture(GL_TEXTURE_2D, currentTextureID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newSurface->w, newSurface->h, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, newSurface->pixels);
   }

   int image_mode = CORNER;
   void imageMode(int iMode) {
      image_mode = iMode;
   }

   color tint_color = WHITE;
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

      std::vector<PVector> coords = {
         // Front
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
         // Back
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
         // Top
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
         // Bottom
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
         // Right
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
         // Left
         {0.0,  0.0},
         {1.0,  0.0},
         {1.0,  1.0},
         {0.0,  1.0},
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
         drawGeometry(vertices, normals, coords, triangles, GL_TRIANGLES, localFboID, tint_color);
      }else {
         drawGeometry(vertices, normals, coords, triangles, GL_TRIANGLES, localFboID, cm.fill_color);
      }
   };

   void box(float size) {
      box(size, size, size);
   }

   float xsphere_ures = 30;
   float xsphere_vres = 30;

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
      drawGeometry(vertices, normals,coords, indices, GL_TRIANGLES, localFboID, cm.fill_color);
   }

   void image(const PImage &pimage, float left, float top, float right, float bottom) {
      if ( image_mode == CORNER ) {
         float width = right;
         float height = bottom;
         right = left + width;
         bottom = top + height;
      } else if ( image_mode == CENTER ) {
         float width = right;
         float height = bottom;
         left = left - ( width / 2.0 );
         top = top - ( height / 2.0 );
         right = left + width;
         bottom = top + height;
      }
      glTexturedQuad({left,top},{right,top},{right,bottom}, {left,bottom}, pimage.surface, localFboID, tint_color);
   }

   void image(const PImage &pimage, float x, float y) {
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

   void background(const PImage &bg) {
      image(bg,0,0);
   }

   std::vector<Uint32> pixels;
   void loadPixels() {
      pixels.resize(gfx_width*gfx_height);
      glBindTexture(GL_TEXTURE_2D, bufferID);
      // Read the pixel data from the framebuffer into the array
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
   }

   void updatePixels() {
      // Write the pixel data to the framebuffer
      glBindTexture(GL_TEXTURE_2D, bufferID);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gfx_width, gfx_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
      // _pixels.clear();
      // pixels = NULL;
   }

   // ----
   // Begin shapes managed by Pshape.
   // ----
   void fill(float r,float g,  float b, float a) {
      cm.fill_color = flatten_color_mode(r,g,b,a);
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
      cm.stroke_color = flatten_color_mode(r,g,b,a);
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
      dm.stroke_weight = x;
   }

   void noStroke() {
      cm.stroke_color = {0,0,0,0};
   }

   void noFill() {
      cm.fill_color = {0,0,0,0};
   }

   void ellipseMode(int mode) {
      dm.ellipse_mode = mode;
   }

   void printMatrix4f(const Eigen::Matrix4f& mat) {
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3));
      printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
   }

   void glTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, float xrange, float yrange, GLuint textureID, GLuint frame_buffer_ID, color tint) {

      std::vector<PVector> vertices{ p0, p1, p2, p3 };
      std::vector<PVector> coords {
         { 0.0f, 0.0f},
         { xrange, 0.0f},
         { xrange, yrange},
         { 0.0f, yrange},
      };
      std::vector<PVector> normals;

      std::vector<unsigned short>  indices = {
         0,1,2, 0,2,3,
      };

      auto tempID = currentTextureID;
      currentTextureID = textureID;
      drawGeometry( vertices, normals, coords, indices, GL_TRIANGLES, frame_buffer_ID, tint );
      currentTextureID = tempID;
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   unsigned int next_power_of_2(unsigned int v) {
      v--;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      v++;
      return v;
   }

   void glTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, SDL_Surface *surface, GLuint frame_buffer_ID, color tint) {
      int newWidth = next_power_of_2(surface->w);
      int newHeight = next_power_of_2(surface->h);

      SDL_Surface* newSurface = SDL_CreateRGBSurface(surface->flags, newWidth, newHeight,
                                                     surface->format->BitsPerPixel,
                                                     surface->format->Rmask,
                                                     surface->format->Gmask,
                                                     surface->format->Bmask,
                                                     surface->format->Amask);
      if (newSurface == NULL) {
         abort();
      }

      // clear new surface with a transparent color and blit existing surface to it
      SDL_FillRect(newSurface, NULL, SDL_MapRGBA(newSurface->format, 0, 0, 0, 0));
      SDL_BlitSurface(surface, NULL, newSurface, NULL);

      // Calculate extends of texture to use
      float xrange = (1.0 * surface->w) / newWidth;
      float yrange = (1.0 * surface->h) / newHeight;

      // Create an OpenGL texture from the SDL_Surface
      GLuint textureID;
      glGenTextures(1, &textureID);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newSurface->w, newSurface->h, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, newSurface->pixels);

      glBindTexture(GL_TEXTURE_2D, 0);
      glTexturedQuad(p0,p1,p2,p3,xrange, yrange, textureID, frame_buffer_ID, tint);

      glDeleteTextures(1, &textureID);
      SDL_FreeSurface(newSurface);


   }
   PLine glLineMitred(PVector p1, PVector p2, PVector p3, float half_weight) const {
      PLine l1{ p1, p2 };
      PLine l2{ p2, p3 };
      PLine low_l1 = l1.offset(-half_weight);
      PLine high_l1 = l1.offset(half_weight);
      PLine low_l2 = l2.offset(-half_weight);
      PLine high_l2 = l2.offset(half_weight);
      return { high_l1.intersect(high_l2), low_l1.intersect(low_l2) };
   }

   PShape glLinePoly(int points, const PVector *p, int weight, bool closed)  {
      PLine start;
      PLine end;

      PShape triangle_strip;
      triangle_strip.beginShape(TRIANGLE_STRIP);

      float half_weight = weight / 2.0;
      if (closed) {
         start = glLineMitred(p[points-1], p[0], p[1], half_weight );
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
         PLine next = glLineMitred(p[i], p[i+1], p[i+2], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }
      if (closed) {
         PLine next = glLineMitred(p[points-2], p[points-1], p[0], half_weight);
         triangle_strip.vertex( next.start );
         triangle_strip.vertex( next.end );
      }

      triangle_strip.vertex( end.start );
      triangle_strip.vertex( end.end );

      triangle_strip.endShape(closed ? CLOSE : OPEN);
      return triangle_strip;
   }

   // only used by glTriangleStrip as mitred line probably wouldn't work.
   void glLine(PShape &triangles, PVector p1, PVector p2, int weight) const {

      PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
      normal.normalize();
      normal.mult(weight/2.0);

      triangles.vertex(p1 + normal);
      triangles.vertex(p1 - normal);
      triangles.vertex(p2 + normal);

      triangles.vertex(p2 + normal);
      triangles.vertex(p2 - normal);
      triangles.vertex(p1 - normal);

   }

   PShape glTriangleStrip(int points, const PVector *p,int weight) {
      PShape triangles;
      triangles.beginShape(TRIANGLES);
      glLine(triangles, p[0], p[1], weight);
      for (int i=2;i<points;++i) {
         glLine(triangles, p[i-1], p[i], weight);
         glLine(triangles, p[i], p[i-2], weight);
      }
      triangles.endShape();
      return triangles;
   }

   void shape_stroke(PShape &pshape, float x, float y, float width, float height, color color) {
      if (color.a == 0)
         return;
      switch( pshape.style ) {
      case POINTS:
      {
         for (auto z : pshape.vertices ) {
            PShape xshape = createEllipse(z.x, z.y, dm.stroke_weight, dm.stroke_weight);
            shape_fill( xshape,0,0,0,0,color );
         }
         break;
      }
      case POLYGON:
      {
         PShape xshape = glLinePoly( pshape.vertices.size(), pshape.vertices.data(), dm.stroke_weight, pshape.type == CLOSE);
         shape_fill( xshape,0,0,0,0,color );
         break;
      }
      case TRIANGLE_STRIP:
      {
         PShape xshape = glTriangleStrip( pshape.vertices.size(), pshape.vertices.data(), dm.stroke_weight);
         shape_fill( xshape,0,0,0,0,color );
         break;
      }
      case TRIANGLES:
         break;
      default:
         abort();
         break;
      }
   }

   void draw_vertices( std::vector<PVector> &vertices, GLuint element_type, color color) {

      std::vector<PVector> normals;
      std::vector<PVector> coords;
      std::vector<unsigned short> indicies;
      return drawGeometry( vertices, normals, coords, indicies, element_type, localFboID, color );

   }

   void shape_fill(PShape &pshape, float x, float y, float width, float height, color color) {
      if (color.a == 0)
         return;
      switch( pshape.style ) {
      case POINTS:
         break;
      case POLYGON:
      {
         std::vector<PVector> triangles = triangulatePolygon({pshape.vertices.begin(),pshape.vertices.end()});
         draw_vertices( triangles, GL_TRIANGLES, color );
      }
      break;
      case TRIANGLES:
         draw_vertices( pshape.vertices, GL_TRIANGLES, color );
         break;
      case TRIANGLE_STRIP:
         draw_vertices( pshape.vertices, GL_TRIANGLE_STRIP, color );
         break;
      default:
         abort();
      }
   }

   void shape(PShape &pshape, float x, float y, float width, float height) {
      pushMatrix();
      translate(x,y);
      scale(1,1); // Need to fix this properly
      transform( pshape.shape_matrix );
      if ( pshape.style == GROUP ) {
         for (auto &&child : pshape.children) {
            shape(child,0,0,0,0);
         }
      } else {
         shape_fill(pshape, x,y,width,height,cm.fill_color);
         shape_stroke(pshape, x,y,width,height, cm.stroke_color);
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
      dm.line_end_cap = cap;
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

   PShape _shape;

   void beginShape(int points = POLYGON) {
      _shape = PShape();
      _shape.beginShape(points);
   }

   void vertex(float x, float y, float z = 0.0) {
      _shape.vertex(x, y, z);
   }

   void endShape(int type = OPEN) {
      _shape.endShape(type);
      shape(_shape, 0,0,0,0);
   }

   void rectMode(int mode){
      dm.rect_mode = mode;
   }



   PShape createBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape bezier;
      bezier.beginShape(POLYGON);
      for (float t = 0; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float t_ = 1 - t;
         float x = t_ * t_ * t_ * x1 + 3 * t_ * t_ * t * x2 + 3 * t_ * t * t * x3 + t * t * t * x4;
         float y = t_ * t_ * t_ * y1 + 3 * t_ * t_ * t * y2 + 3 * t_ * t * t * y3 + t * t * t * y4;
         bezier.vertex(x, y);
      }
      bezier.endShape(OPEN);
      return bezier;
   }


   PShape createRect(float x, float y, float width, float height) {
      if (dm.rect_mode == CORNERS) {
         width = width - x;
         height = height - y;
      } else if (dm.rect_mode == CENTER) {
         x = x - width / 2;
         y = y - height / 2;
      } else if (dm.rect_mode == RADIUS) {
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
      shape.endShape(CLOSE);
      return shape;
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
      if (dm.ellipse_mode != RADIUS) {
         width /=2;
         height /=2;
      }
      int NUMBER_OF_VERTICES=32;
      PShape shape;
      shape.beginShape(POLYGON);
      for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
         shape.vertex( ellipse_point( {x,y}, i, 0, TWO_PI, width, height ) );
      }
      shape.endShape(CLOSE);
      return shape;
   }

   PShape createArc(float x, float y, float width, float height, float start,
                    float stop, int mode = DEFAULT) {

      if (dm.ellipse_mode != RADIUS) {
         width /=2;
         height /=2;
      }
      PShape shape;
      shape.beginShape(POLYGON);
      int NUMBER_OF_VERTICES=32;
      if ( mode == PIE ) {
         shape.vertex(x,y);
      }
      for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
         shape.vertex( ellipse_point( {x,y}, i, start, stop, width, height ) );
      }
      shape.vertex( ellipse_point( {x,y}, 32, start, stop, width, height ) );
      shape.endShape(CLOSE);
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

   bool xSmoothing = true;

   void noSmooth() {
      // Doesn't yet apply to actual graphics
      xSmoothing = false;
   }

   void beginDraw() {}
   void endDraw() {}

   void draw_main() {
      // For drawing the main screen we need to flip the texture and remove any tintint
      glTexturedQuad(
         {0.0f,           0.0f+gfx_height},
         {0.0f+gfx_width ,0.0f+gfx_height},
         {0.0f+gfx_width, 0.0f},
         {0.0f,           0.0f},
         1.0,1.0, bufferID, 0, WHITE);
   }

   void draw(float x, float y) {
      glTexturedQuad( {x, y},
                      {x+gfx_width,y},
                      {x+gfx_width,y+gfx_height},
                      {x,y+gfx_height},
                      1.0,1.0, bufferID, localFboID, tint_color);
   }
};


PGraphics createGraphics(int width, int height, int mode = P2D) {
   return { width, height, mode };
}

extern PGraphics g;

void image(PGraphics &gfx, int x, int y) {
   gfx.draw(x,y);
}


#endif
