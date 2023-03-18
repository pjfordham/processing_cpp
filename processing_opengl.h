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

void glFilledElement(GLuint element_type, int points, PVector *p, SDL_Color color) {
   anything_drawn = true;

   std::vector<float> colors;
   std::vector<float> vertices;
   std::vector<float> normals;
   std::vector<unsigned short> indices;

   for (int i = 0; i< points; ++i ) {
      colors.push_back(color.r / 255.0);
      colors.push_back(color.g / 255.0);
      colors.push_back(color.b / 255.0);
      colors.push_back(color.a / 255.0);
      vertices.push_back(p[i].x);
      vertices.push_back(p[i].y);
      vertices.push_back(p[i].z);
      normals.push_back(1);
      normals.push_back(1);
      normals.push_back(1);
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

   GLuint normalbuffer;
   glGenBuffers(1, &normalbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
   glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

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

   attribId = glGetAttribLocation(programID, "normal");
   glEnableVertexAttribArray(attribId);
   glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
   glVertexAttribPointer(
      attribId,                         // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
      );

   glDrawElements(element_type, indices.size(), GL_UNSIGNED_SHORT, 0);

   glDeleteBuffers(1, &vertexbuffer);
   glDeleteBuffers(1, &indexbuffer);
   glDeleteBuffers(1, &normalbuffer);
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

   PVector end_offset = PVector{p2.x-p1.x,p2.y-p1.y}.normal();
   end_offset.mult(weight/2.0);

   PVector p[] = {p1,p1,p2,p2};
   p[0].add(normal);
   p[1].sub(normal);
   p[2].sub(normal);
   p[3].add(normal);
   p[0].add(end_offset);
   p[1].sub(end_offset);
   p[2].sub(end_offset);
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

void glLinePoly(int points, PVector *p, SDL_Color color, int weight) {
   glLines(points, p, color, weight);
   glLine(p[points-1], p[0], color, weight);
}

void glFilledEllipse( PVector center, float xradius, float yradius, SDL_Color color ) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   for(float i = 0; i < TWO_PI; i += TWO_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );
}

void glLineEllipse( PVector center, float xradius, float yradius, SDL_Color color, int weight) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   for(float i = 0; i < TWO_PI; i += TWO_PI / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   vertexBuffer.push_back(vertexBuffer[0]);
   glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}

void glFilledArc( PVector center, float xradius, float yradius, float start, float end, SDL_Color color ) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   vertexBuffer.emplace_back(center.x, center.y);
   for(float i = start; i < end; i += (end - start) / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   vertexBuffer.emplace_back(center.x + cos(end) * xradius, center.y + sin(end) * yradius);
   glFilledTriangleFan(vertexBuffer.size(), vertexBuffer.data(), color );
}

void glLineArc( PVector center, float xradius, float yradius, float start, float end, SDL_Color color, int weight) {
   int NUMBER_OF_VERTICES=32;
   std::vector<PVector> vertexBuffer;
   vertexBuffer.emplace_back(center.x, center.y);
   for(float i = start; i < end; i += (end - start) / NUMBER_OF_VERTICES){
      vertexBuffer.emplace_back(center.x + cos(i) * xradius, center.y + sin(i) * yradius);
   }
   vertexBuffer.emplace_back(center.x + cos(end) * xradius, center.y + sin(end) * yradius);
   glLines(vertexBuffer.size(),vertexBuffer.data(),color,weight);
}

#endif
