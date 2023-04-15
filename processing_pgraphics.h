#ifndef PROCESSING_PGRAPHICS_H
#define PROCESSING_PGRAPHICS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_math.h"
#include "processing_opengl.h"
#include "processing_color.h"
#include "processing_pshape.h"

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

   DrawingMode dm{};
   ColorMode cm{};

   int gfx_width, gfx_height;

   PGraphics(const PGraphics &x) = delete;

   PGraphics(PGraphics &&x) {
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(cm, x.cm);
      std::swap(dm, x.dm);
   }

   PGraphics& operator=(const PGraphics&) = delete;
   PGraphics& operator=(PGraphics&&x){
      std::swap(bufferID, x.bufferID);
      std::swap(localFboID, x.localFboID);
      std::swap(gfx_width, x.gfx_width);
      std::swap(gfx_height, x.gfx_height);
      std::swap(cm, x.cm);
      std::swap(dm, x.dm);
      return *this;
   }

   PGraphics() {
      localFboID = 0;
      bufferID = 0;
   }
   ~PGraphics() {
      if (localFboID)
         glDeleteFramebuffers(1, &localFboID);
      if (bufferID)
         glDeleteTextures(1, &bufferID);
   }
   PGraphics(int width, int height, int mode) {
      // Create a framebuffer object
      glGenFramebuffers(1, &localFboID);
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);

      if (mode == P3D) {
         // Create a renderbuffer for the depth buffer
         GLuint depthBufferID;
         glGenRenderbuffers(1, &depthBufferID);
         glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

         // Attach the depth buffer to the framebuffer object
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
      }

      gfx_width = width;
      gfx_height = height;

      // Create a texture to render to
      glGenTextures(1, &bufferID);
      glBindTexture(GL_TEXTURE_2D, bufferID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferID, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   }

   void background(float r, float g, float b) {
      anything_drawn = true;
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

   std::vector<Uint32> pixels;
   void loadPixels() {
      pixels.resize(gfx_width*gfx_height);
      glBindTexture(GL_TEXTURE_2D, bufferID);
      // Read the pixel data from the framebuffer into the array
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
   }

   void updatePixels() {
      anything_drawn = true;
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

   void shape_stroke(PShape &pshape, float x, float y, float width, float height, color color) {
      extern GLuint Color;
      float color_vec[] = {
         color.r / 255.0f,
         color.g / 255.0f,
         color.b / 255.0f,
         color.a / 255.0f };
      glUniform4fv(Color, 1, color_vec);
      switch( pshape.style ) {
      case POINTS:
      {
         for (auto z : pshape.vertices ) {
            PShape xshape = createRect(z.x, z.y, dm.stroke_weight, dm.stroke_weight);
            shape_fill( xshape,0,0,0,0,color );
         }
         break;
      }
      case POLYGON:
         break;
      case TRIANGLES:
         break;
      case TRIANGLE_STRIP:
         break;
      }
   }

   void shape_fill(PShape &pshape, float x, float y, float width, float height, color color) {
      extern GLuint Color;
      switch( pshape.style ) {
      case POINTS:
         break;
      case POLYGON:
      {
         float color_vec[] = {
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f };
         glUniform4fv(Color, 1, color_vec);
         std::vector<PVector> triangles = triangulatePolygon({pshape.vertices.begin(),pshape.vertices.end()});

         // Create a vertex array object (VAO)
         GLuint VAO;
         glGenVertexArrays(1, &VAO);
         glBindVertexArray(VAO);

         auto vertexbuffer_size = triangles.size();

         GLuint vertexbuffer;
         glGenBuffers(1, &vertexbuffer);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(float) * 3, triangles.data(), GL_STATIC_DRAW);

         GLuint attribId = glGetAttribLocation(programID, "position");
         glEnableVertexAttribArray(attribId);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glVertexAttribPointer(
            attribId,                         // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof(PVector),                  // stride
            (void*)offsetof(PVector,x)        // array buffer offset
            );

         glBindVertexArray(VAO);
         glDrawArrays(GL_TRIANGLES, 0, vertexbuffer_size);
         glBindVertexArray(0);

         glDeleteBuffers(1, &vertexbuffer);
         glDeleteVertexArrays(1, &VAO);
      }
      break;
      case TRIANGLES:
      {
         float color_vec[] = {
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f };
         glUniform4fv(Color, 1, color_vec);
         std::vector<PVector> &triangles = pshape.vertices;

         // Create a vertex array object (VAO)
         GLuint VAO;
         glGenVertexArrays(1, &VAO);
         glBindVertexArray(VAO);

         auto vertexbuffer_size = triangles.size();

         GLuint vertexbuffer;
         glGenBuffers(1, &vertexbuffer);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(float) * 3, triangles.data(), GL_STATIC_DRAW);

         GLuint attribId = glGetAttribLocation(programID, "position");
         glEnableVertexAttribArray(attribId);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glVertexAttribPointer(
            attribId,                         // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof(PVector),                  // stride
            (void*)offsetof(PVector,x)        // array buffer offset
            );

         glUniform4fv(Color, 1, color_vec);
         glBindVertexArray(VAO);
         glDrawArrays(GL_TRIANGLES, 0, vertexbuffer_size);
         glBindVertexArray(0);

         glDeleteBuffers(1, &vertexbuffer);
         glDeleteVertexArrays(1, &VAO);
      }
      break;
      case TRIANGLE_STRIP:
      {
         float color_vec[] = {
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f };
         glUniform4fv(Color, 1, color_vec);
         std::vector<PVector> &triangles = pshape.vertices;

         // Create a vertex array object (VAO)
         GLuint VAO;
         glGenVertexArrays(1, &VAO);
         glBindVertexArray(VAO);

         auto vertexbuffer_size = triangles.size();

         GLuint vertexbuffer;
         glGenBuffers(1, &vertexbuffer);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(float) * 3, triangles.data(), GL_STATIC_DRAW);

         GLuint attribId = glGetAttribLocation(programID, "position");
         glEnableVertexAttribArray(attribId);
         glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
         glVertexAttribPointer(
            attribId,                         // attribute
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            sizeof(PVector),                  // stride
            (void*)offsetof(PVector,x)        // array buffer offset
            );

         glUniform4fv(Color, 1, color_vec);
         glBindVertexArray(VAO);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexbuffer_size);
         glBindVertexArray(0);

         glDeleteBuffers(1, &vertexbuffer);
         glDeleteVertexArrays(1, &VAO);
      }
      break;
      default:
         abort();
      }
   }

   void shape(PShape &pshape, float x, float y, float width, float height) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
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
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
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
      PShape ellipse = createUnitCircle();
      ellipse.translate(x,y);
      ellipse.scale(width,height);
      return ellipse;
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

   void draw(float x, float y, bool flip=false) {
      glTexturedQuad( {x, y},
                      {x+gfx_width,y},
                      {x+gfx_width,y+gfx_height},
                      {x,y+gfx_height},
                      1.0,1.0, bufferID,flip );
      return;
   }
};


PGraphics createGraphics(int width, int height, int mode = P2D) {
   return { width, height, mode };
}

extern PGraphics g;

void image(PGraphics &gfx, int x, int y) {
   glBindFramebuffer(GL_FRAMEBUFFER, g.localFboID);
   gfx.draw(x,y);
}


#endif
