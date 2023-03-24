#ifndef PROCESSING_OPENGL_H
#define PROCESSING_OPENGL_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "processing_math.h"
#include "processing_earclipping.h"

#include <fmt/core.h>

bool anything_drawn = false;


unsigned int next_power_of_2(unsigned int v)
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;
   return v;
}

extern GLuint programID;
extern GLuint flatTextureShader;
extern Eigen::Matrix4f move_matrix; // Default is identity
extern Eigen::Matrix4f projection_matrix; // Default is identity
extern Eigen::Matrix4f view_matrix; // Default is identity

void printMatrix4f(const Eigen::Matrix4f& mat) {
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3));
    printf("[ %8.4f, %8.4f, %8.4f, %8.4f ]\n", mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
}

void glTexturedQuad(PVector p0, PVector p1, PVector p2, PVector p3, SDL_Surface *surface) {
   anything_drawn = true;

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

   GLuint uSampler = glGetUniformLocation(flatTextureShader, "uSampler");

   int textureUnitIndex = 0;
   glUniform1i(uSampler,0);
   glActiveTexture(GL_TEXTURE0 + textureUnitIndex);

   Eigen::Vector4f vert0 = projection_matrix * view_matrix * move_matrix * Eigen::Vector4f{p0.x,p0.y,0,1};
   Eigen::Vector4f vert1 = projection_matrix * view_matrix * move_matrix * Eigen::Vector4f{p1.x,p1.y,0,1};
   Eigen::Vector4f vert2 = projection_matrix * view_matrix * move_matrix * Eigen::Vector4f{p2.x,p2.y,0,1};
   Eigen::Vector4f vert3 = projection_matrix * view_matrix * move_matrix * Eigen::Vector4f{p3.x,p3.y,0,1};

   std::vector<float> vertices{
      vert0[0],  vert0[1], 0.0f,
      vert1[0],  vert1[1], 0.0f,
      vert2[0],  vert2[1], 0.0f,
      vert3[0],  vert3[1], 0.0f,
   };

   std::vector<float> coords{
      0.0f, 0.0f,
      xrange, 0.0f,
      xrange, yrange,
      0.0f, yrange,
   };

   std::vector<unsigned short>  indices = {
      0,1,2, 0,2,3,
   };

   GLuint localVAO;
   // Create a vertex array object (VAO)
   glGenVertexArrays(1, &localVAO);
   glBindVertexArray(localVAO);

   GLuint indexbuffer;
   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   GLuint coordsbuffer;
   glGenBuffers(1, &coordsbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, coordsbuffer);
   glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(float), coords.data(), GL_STATIC_DRAW);


   GLuint attribId = glGetAttribLocation(flatTextureShader, "position");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(flatTextureShader, "coords");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, coordsbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      2,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glUseProgram(flatTextureShader);
   glBindVertexArray(localVAO);
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
   glBindVertexArray(0);
   glBindTexture(GL_TEXTURE_2D, 0);

   glDeleteTextures(1, &textureID);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);

   // Unbind the buffer objects and VAO
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   SDL_FreeSurface(newSurface);

   glUseProgram(programID);

}

extern GLuint programID;

void glEllipseDump();

void glFilledElement(GLuint element_type, int points, PVector *p, SDL_Color color) {
   anything_drawn = true;
   glEllipseDump();

   std::vector<float> colors;
   std::vector<float> vertices;
   std::vector<unsigned short> indices;

   for (int i = 0; i< points; ++i ) {
      colors.push_back(color.r / 255.0);
      colors.push_back(color.g / 255.0);
      colors.push_back(color.b / 255.0);
      colors.push_back(color.a / 255.0);
      vertices.push_back(p[i].x);
      vertices.push_back(p[i].y);
      vertices.push_back(p[i].z);
      indices.push_back(i);
   };

   GLuint VAO;
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   GLuint indexbuffer;
   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

   GLuint colorbuffer;
   glGenBuffers(1, &colorbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
   glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);

   GLuint attribId = glGetAttribLocation(programID, "position");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(programID, "color");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      4,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glDrawElements(element_type, indices.size(), GL_UNSIGNED_SHORT, 0);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);
   glDeleteBuffers(1, &colorbuffer);

   // Unbind the buffer objects and VAO
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);


}

void glFilledTriangleStrip(int points, PVector *p, SDL_Color color) {
   glFilledElement(GL_TRIANGLE_STRIP, points, p, color);
}

void glFilledPoly(int points, PVector *p, SDL_Color color) {
   std::vector<PVector> triangles = triangulatePolygon({p,p+points});
   glFilledElement(GL_TRIANGLES,triangles.size(), triangles.data(),color);
}

void glFilledTriangleFan(int points, PVector *p, SDL_Color color) {
   glFilledElement(GL_TRIANGLE_FAN,points,p,color);
}


void _glRoundLine(PVector p1, PVector p2, SDL_Color color, int weight) {

   // Compute direction vector of line
   PVector direction = p2 - p1;
   direction.normalize();

   // Compute first orthogonal vector
   PVector z_axis(0.0, 0.0, 1.0);
   PVector orthogonal1 = direction.cross(z_axis);
   orthogonal1.normalize();

   // Compute second orthogonal vector
   PVector orthogonal2 = direction.cross(orthogonal1);
   orthogonal2.normalize();

   if (orthogonal1 == PVector{0.0,0.0,0.0} ) {
      orthogonal1 = PVector{1.0, 0.0, 0.0};
      orthogonal2 = PVector{0.0, 1.0, 0.0};
   }

   // Compute dimensions of cuboid
   float length = weight * 1;

   // Construct vertices of cuboid
   std::vector<PVector> vertices(8);
   vertices[0] = p1 - orthogonal1 * length - orthogonal2 * length;
   vertices[1] = p1 + orthogonal1 * length - orthogonal2 * length;
   vertices[2] = p1 + orthogonal1 * length + orthogonal2 * length;
   vertices[3] = p1 - orthogonal1 * length + orthogonal2 * length;
   vertices[4] = p2 - orthogonal1 * length - orthogonal2 * length;
   vertices[5] = p2 + orthogonal1 * length - orthogonal2 * length;
   vertices[6] = p2 + orthogonal1 * length + orthogonal2 * length;
   vertices[7] = p2 - orthogonal1 * length + orthogonal2 * length;


   std::vector<PVector> vertexBuffer;

   vertexBuffer = { vertices[0], vertices[1],vertices[2],vertices[3] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   vertexBuffer = { vertices[4], vertices[5],vertices[6],vertices[7] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   vertexBuffer = { vertices[0], vertices[1],vertices[4],vertices[5] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   vertexBuffer = { vertices[2], vertices[3],vertices[6],vertices[7] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   vertexBuffer = { vertices[1], vertices[5],vertices[6],vertices[2] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );

   vertexBuffer = { vertices[0], vertices[4],vertices[3],vertices[7] };
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );


}

void glRoundLine(PVector p1, PVector p2, SDL_Color color, int weight) {

   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   int NUMBER_OF_VERTICES=16;
   std::vector<PVector> vertexBuffer;

   float start_angle = PVector{p2.x-p1.x,p2.y-p1.y}.get_angle() + HALF_PI;

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(p1.x + cos(i + start_angle) * weight/2, p1.y + sin(i+start_angle) * weight/2);
   }

   start_angle += PI;

   for(float i = 0; i < PI; i += TWO_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(p2.x + cos(i+start_angle) * weight/2, p2.y + sin(i+start_angle) * weight/2);
   }

   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );
}

void glCappedLine(PVector p1, PVector p2, SDL_Color color, int weight) {

   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   PVector end_offset = PVector{p2.x-p1.x,p2.y-p1.y};
   end_offset.normalize();
   end_offset.mult(weight/2.0);

   PVector p[] = {p1,p1,p2,p2};
   p[0].add(normal);
   p[0].sub(end_offset);

   p[1].sub(normal);
   p[1].sub(end_offset);

   p[2].sub(normal);
   p[2].add(end_offset);

   p[3].add(normal);
   p[3].add(end_offset);

   glFilledTriangleFan(4, p, color);

}

void glLine(PVector p1, PVector p2, SDL_Color color, int weight) {

   PVector normal = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   normal.normalize();
   normal.mult(weight/2.0);

   PVector p[] = {p1,p1,p2,p2};
   p[0].add(normal);
   p[1].sub(normal);
   p[2].sub(normal);
   p[3].add(normal);
   glFilledTriangleFan(4, p, color);

}

void glLines(int points, PVector *p, SDL_Color color, int weight) {
   for (int i =1; i<points;++i) {
      glLine(p[i-1], p[i], color, weight);
   }
}

void glTriangleStrip(int points, PVector *p, SDL_Color color,int weight) {
   for (int i =2; i<points;++i) {
      glLine(p[i-2], p[i-1], color, weight);
      glLine(p[i-1], p[i], color, weight);
      glLine(p[i], p[i-2], color, weight);
   }
}

void glTriangleFan(int points, PVector *p, SDL_Color color,int weight) {
   glLine( p[0], p[1], color, weight );
   for (int i =2; i<points;++i) {
      glLine( p[i-1], p[i], color, weight);
      glLine( p[0], p[i], color, weight);
   }
}

void glClosedLinePoly(int points, PVector *p, SDL_Color color, int weight) {
   glLines(points, p, color, weight);
   glLine(p[points-1], p[0], color, weight);
}

void glLinePoly(int points, PVector *p, SDL_Color color, int weight) {
   glLines(points, p, color, weight);
}

extern GLuint circleID;

std::vector<float> circle_fill_color;
std::vector<float> circle_stroke_color;
std::vector<float> circle_vertices;
std::vector<float> circle_radius;
std::vector<float> circle_stroke_weight;
std::vector<unsigned short> circle_indices;

void glEllipseDump() {
   anything_drawn = true;

   GLuint VAO;
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   GLuint vertexbuffer;
   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, circle_vertices.size() * sizeof(float), circle_vertices.data(), GL_STATIC_DRAW);

   GLuint radiusbuffer;
   glGenBuffers(1, &radiusbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, radiusbuffer);
   glBufferData(GL_ARRAY_BUFFER, circle_radius.size() * sizeof(float), circle_radius.data(), GL_STATIC_DRAW);

   GLuint strokeweightbuffer;
   glGenBuffers(1, &strokeweightbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, strokeweightbuffer);
   glBufferData(GL_ARRAY_BUFFER, circle_stroke_weight.size() * sizeof(float), circle_stroke_weight.data(), GL_STATIC_DRAW);

   GLuint indexbuffer;
   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, circle_indices.size() * sizeof(unsigned short), circle_indices.data(), GL_STATIC_DRAW);

   GLuint fillcolorbuffer;
   glGenBuffers(1, &fillcolorbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, fillcolorbuffer);
   glBufferData(GL_ARRAY_BUFFER, circle_fill_color.size() * sizeof(float), circle_fill_color.data(), GL_STATIC_DRAW);

   GLuint strokecolorbuffer;
   glGenBuffers(1, &strokecolorbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, strokecolorbuffer);
   glBufferData(GL_ARRAY_BUFFER, circle_stroke_color.size() * sizeof(float), circle_stroke_color.data(), GL_STATIC_DRAW);

   GLuint attribId = glGetAttribLocation(circleID, "radius");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, radiusbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      2,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(circleID, "position");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(circleID, "strokeWeight");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, strokeweightbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      1,                                // size
      GL_FLOAT,                           // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(circleID, "fillColor");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, fillcolorbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      4,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   attribId = glGetAttribLocation(circleID, "strokeColor");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, strokecolorbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      4,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glUseProgram(circleID);

   // Get a handle for our "MVP" uniform
   GLuint Pmatrix = glGetUniformLocation(circleID, "Pmatrix");
   GLuint Vmatrix = glGetUniformLocation(circleID, "Vmatrix");
   GLuint Mmatrix = glGetUniformLocation(circleID, "Mmatrix");

   // Send our transformation to the currently bound shader,
   glUniformMatrix4fv(Pmatrix, 1,false, projection_matrix.data());
   glUniformMatrix4fv(Vmatrix, 1,false, view_matrix.data());
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());

   glDrawElements(GL_POINTS, circle_indices.size(), GL_UNSIGNED_SHORT, 0);
   glUseProgram(programID);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);
   glDeleteBuffers(1, &fillcolorbuffer);
   glDeleteBuffers(1, &strokecolorbuffer);
   glDeleteBuffers(1, &strokeweightbuffer);
   glDeleteBuffers(1, &radiusbuffer);

   // Unbind the buffer objects and VAO
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   circle_fill_color.clear();
   circle_stroke_color.clear();
   circle_stroke_weight.clear();
   circle_vertices.clear();
   circle_radius.clear();
   circle_indices.clear();
}

void glEllipse( PVector center, float xradius, float yradius, SDL_Color fill_color, SDL_Color stroke_color, int stroke_weight ) {
   circle_stroke_color.insert(circle_stroke_color.end(), { stroke_color.r / 255.0f, stroke_color.g / 255.0f, stroke_color.b / 255.0f, stroke_color.a / 255.0f });
   circle_fill_color.insert(circle_fill_color.end(), { fill_color.r / 255.0f, fill_color.g / 255.0f, fill_color.b / 255.0f, fill_color.a / 255.0f });
   circle_vertices.insert(circle_vertices.end(),{ center.x, center.y, center.z });
   circle_radius.insert(circle_radius.end(),{ xradius, yradius });
   circle_stroke_weight.insert(circle_stroke_weight.end(),{ stroke_weight * 1.0f });
   circle_indices.insert(circle_indices.end(),{ (unsigned short) circle_indices.size() });
   return;
}

PVector ellipse_point(const PVector &center, int index, float start, float end, float xradius, float yradius) {
   float angle = map( index, 0, 32, start, end);
   return PVector( center.x + xradius * sin(-angle + HALF_PI),
                   center.y + yradius * cos(-angle + HALF_PI),
                   center.z);
}

enum { /*OPEN == 0,*/ CHORD = 1, PIE=2, DEFAULT=3 };

void glFilledArc( PVector center, float xradius, float yradius, float start, float end, SDL_Color color, int mode ) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   if ( mode == DEFAULT || mode == PIE ) {
      vertexBuffer.push_back(center);
   }
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      vertexBuffer.push_back( ellipse_point( center, i, start, end, xradius, yradius ) );
   }
   vertexBuffer.push_back( ellipse_point( center, 32, start, end, xradius, yradius ) );
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );
}

void glLineArc( PVector center, float xradius, float yradius, float start, float end, SDL_Color color, int weight, int mode) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   if ( mode == PIE ) {
      vertexBuffer.push_back(center);
   }
   for(int i = 0; i < NUMBER_OF_VERTICES; ++i) {
      vertexBuffer.push_back( ellipse_point( center, i, start, end, xradius, yradius ) );
   }
   vertexBuffer.push_back( ellipse_point( center, 32, start, end, xradius, yradius ) );
   if ( mode == CHORD || mode == PIE ) {
      vertexBuffer.push_back( vertexBuffer[0] );
   }
   glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}

#endif
