#ifndef PROCESSING_TRANSFORMS_H
#define PROCESSING_TRANSFORMS_H

#include <GL/glew.h>     // GLEW library header
#include <GL/gl.h>       // OpenGL header
#include <GL/glu.h>      // GLU header
#include <GL/glut.h>

#include "processing_math.h"

std::vector<Eigen::Matrix4f> matrix_stack;
Eigen::Matrix4f move_matrix; // Default is identity

GLuint Mmatrix;

void pushMatrix() {
   matrix_stack.push_back(move_matrix);
}

void popMatrix() {
   move_matrix = matrix_stack.back();
   matrix_stack.pop_back();
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void translate(float x, float y, float z=0) {
   move_matrix = move_matrix * TranslateMatrix(PVector{x,y,z});
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void transform(Eigen::Matrix4f transform_matrix) {
   move_matrix = move_matrix * transform_matrix;
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void scale(float x, float y,float z = 1) {
   move_matrix = move_matrix * ScaleMatrix(PVector{x,y,z});
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void scale(float x) {
   scale(x,x,x);
}

void rotate(float angle, PVector axis) {
   move_matrix = move_matrix * RotateMatrix(angle,axis);
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}


void rotate(float angle) {
   move_matrix = move_matrix * RotateMatrix(angle,PVector{0,0,1});
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void rotateY(float angle) {
   move_matrix = move_matrix * RotateMatrix(angle,PVector{0,1,0});
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

void rotateX(float angle) {
   move_matrix = move_matrix * RotateMatrix(angle,PVector{1,0,0});
   glUniformMatrix4fv(Mmatrix, 1,false, move_matrix.data());
}

#endif
