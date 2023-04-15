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

class PGraphics {
public:
   GLuint bufferID;
   GLuint localFboID;

   DrawingMode dm;
   ColorMode cm;

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
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createRect(x,y,_width,_height).draw(cm);
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

   void ellipse(float x, float y, float width, float height) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createEllipse(x, y, width, height).draw(cm);
   }

   void ellipse(float x, float y, float radius) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createEllipse(x, y, radius, radius).draw(cm);
   }

   void arc(float x, float y, float width, float height, float start, float stop, int mode = DEFAULT) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createArc(x, y, width, height, start, stop, mode).draw(cm);
   }

   void strokeCap(int cap) {
      dm.line_end_cap = cap;
   }

   void line(float x1, float y1, float x2, float y2) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createLine( x1, y1, x2, y2).draw(cm);
   }

   void line(float x1, float y1, float z1, float x2, float y2, float z2) {
      abort();
   }


   void line(PVector start, PVector end) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      line(start.x,start.y, end.x,end.y);
   }

   void line(PLine l) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      line(l.start, l.end);
   }

   void point(float x, float y) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createPoint(x, y).draw(cm);
   }

   void quad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createQuad(x1, y1, x2, y2, x3, y3, x4, y4).draw(cm);
   }

   void triangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      createTriangle( x1, y1, x2, y2, x3, y3 ).draw(cm);
   }

   void shape(PShape shape, float x, float y, float width, float height) {
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      pushMatrix();
      translate(x,y);
      scale(1,1); // Need to fix this properly
      shape.draw(cm);
      popMatrix();
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
      _shape.endShape(type,dm);
      glBindFramebuffer(GL_FRAMEBUFFER, localFboID);
      _shape.draw(cm);
   }

   void rectMode(int mode){
      dm.rect_mode = mode;
   }

   void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
      PShape bezier;
      bezier.beginShape();
      for (float t = 0; t <= 1; t += 0.01) {
         // Compute the Bezier curve points
         float t_ = 1 - t;
         float x = t_ * t_ * t_ * x1 + 3 * t_ * t_ * t * x2 + 3 * t_ * t * t * x3 + t * t * t * x4;
         float y = t_ * t_ * t_ * y1 + 3 * t_ * t_ * t * y2 + 3 * t_ * t * t * y3 + t * t * t * y4;
         bezier.vertex(x, y);
      }
      bezier.endShape(OPEN, dm);
      bezier.draw(cm);
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
   shape.endShape(CLOSE, dm);
   return shape;
}

PShape createQuad( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 ) {
   PShape shape;
   shape.beginShape(POLYGON);
   shape.vertex(x1, y1);
   shape.vertex(x2, y2);
   shape.vertex(x3, y3);
   shape.vertex(x4, y4);
   shape.endShape(CLOSE, dm);
   return shape;
}


PShape createLine(float x1, float y1, float x2, float y2) {
   PVector p[] = {{x1,y1},{x1,y1},{x2,y2},{x2,y2}};

   float half_stroke = dm.stroke_weight/2.0;

   PShape shape;
   shape.stroke_only = true;
   shape.beginShape(POLYGON);

   PVector direction = PVector{x2-x1,y2-y1};
   PVector normal = direction.normal();
   normal.normalize();
   normal.mult(half_stroke);

   if (dm.line_end_cap == ROUND ) {
      int NUMBER_OF_VERTICES=16;

      float start_angle = direction.heading() + HALF_PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(x1 + cos(i + start_angle) * half_stroke,
                      y1 + sin(i + start_angle) * half_stroke);
      }

      start_angle += PI;

      for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
         shape.vertex(x2 + cos(i + start_angle) * half_stroke,
                      y2 + sin(i + start_angle) * half_stroke);
      }
   } else {
      p[0].add(normal);
      p[1].sub(normal);
      p[2].sub(normal);
      p[3].add(normal);

      if (dm.line_end_cap == PROJECT) {
         direction.normalize();
         direction.mult(half_stroke);
         p[0].sub(direction);
         p[1].sub(direction);
         p[2].add(direction);
         p[3].add(direction);
      }

      shape.vertex( p[0] );
      shape.vertex( p[1] );
      shape.vertex( p[2] );
      shape.vertex( p[3] );
   }
   shape.endShape(CLOSE,dm);
   return shape;
}

PShape createTriangle( float x1, float y1, float x2, float y2, float x3, float y3 ) {
   PShape shape;
   shape.beginShape(TRIANGLE_STRIP);
   shape.vertex(x1, y1);
   shape.vertex(x2, y2);
   shape.vertex(x3, y3);
   shape.endShape(CLOSE,dm);
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
   if ( mode == DEFAULT || mode == PIE ) {
      shape.vertex(x,y);
   }
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      shape.vertex( ellipse_point( {x,y}, i, start, stop, width, height ) );
   }
   shape.vertex( ellipse_point( {x,y}, 32, start, stop, width, height ) );
   shape.endShape(CLOSE,dm);
   return shape;
   // NEED to tweak outline see Arc.cc
   // int NUMBER_OF_VERTICES=32;
   // std::vector<PVector> vertexBuffer;
   // if ( mode == PIE ) {
   //    vertexBuffer.push_back(center);
   // }
   // for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
   //    vertexBuffer.push_back( ellipse_point( center, i, start, end, xradius, yradius ) );
   // }
   // vertexBuffer.push_back( ellipse_point( center, 32, start, end, xradius, yradius ) );
   // if ( mode == CHORD || mode == PIE ) {
   //    vertexBuffer.push_back( vertexBuffer[0] );
   // }
   // glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}

PShape createEllipse(float x, float y, float width, float height) {
   static PShape unitCircle = createUnitCircle();
   PShape ellipse;
   ellipse.borrowVAO( unitCircle );
   if (dm.ellipse_mode != RADIUS) {
      width /=2;
      height /=2;
   }
   ellipse.style = TRIANGLE_FAN;
   ellipse.translate(x,y);
   ellipse.scale(width,height);

   if ( PShape::stroke_on() ) {
      PShape group;
      group.beginShape(GROUP);

      PShape shape;
      shape.beginShape(LINES);
      shape.stroke_only = true;
      for(int i = 0; i < 32; ++i) {
         shape.vertex( ellipse_point( {x,y,0}, i, 0, TWO_PI, width, height ) );
      }
      shape.endShape(CLOSE,dm);

      group.addChild( std::move(ellipse) );
      group.addChild( std::move(shape) );
      group.endShape(OPEN,dm);
      return group;
   }
   return ellipse;
}

PShape createPoint(float x, float y) {
   static PShape unitCircle = createUnitCircle();
   PShape shape;
   shape.borrowVAO( unitCircle );
   shape.style = TRIANGLE_FAN;
   shape.translate(x,y);
   shape.scale(dm.stroke_weight,dm.stroke_weight);
   shape.stroke_only = true;
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
