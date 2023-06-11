/**
 * ArrayList of objects
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use a Java ArrayList to store
 * a variable number of objects.  Items can be added and removed
 * from the ArrayList.
 *
 * Click the mouse to add bouncing balls.
 */

#include "Ball.h"

std::vector<Ball> balls;
int ballWidth = 48;

void setup() {
  size(640, 360);
  noStroke();

  // Start by adding one element
  balls.emplace_back(width/2, 0, ballWidth);
}

void draw() {
  background(255);

  // With an array, we say balls.length, with an ArrayList, we say balls.size()
  // The length of an ArrayList is dynamic
  // Notice how we are looping through the ArrayList backwards
  // This is because we are deleting elements from the list
  for (int i = balls.size()-1; i >= 0; i--) {
    // An ArrayList doesn't know what it is storing so we have to cast the object coming out
    Ball &ball = balls[i];
    ball.move();
    ball.display();
    if (ball.finished()) {
      // Items can be deleted with remove()
      balls.erase(balls.begin() + i);
    }

  }

}

void mousePressed() {
  // A new ball object is added to the ArrayList (by default to the end)
  balls.emplace_back(mouseX, mouseY, ballWidth);
}

