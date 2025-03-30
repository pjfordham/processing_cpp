// Based on code by GeneKao (https://github.com/GeneKao)

#include "Arcball.h"

boolean mousePressedOnParent = false;
Arcball arcball, arcball2;

class ChildApplet : public PSurface {
   void setup() {
    setTitle("Child sketch");
    size(400, 400, P3D);
    g.smooth();
    arcball2 = Arcball(this, 300);
   }

  void draw() {
    g.background(0);
    arcball2.run();
    if (mousePressedb) {
      g.fill(240, 0, 0);
      g.ellipse(mouseX, mouseY, 20, 20);
      g.fill(255);
      g.text("Mouse pressed on child.", 10, 30);
    } else {
      g.fill(255);
      g.ellipse(width/2, height/2, 20, 20);
    }

    g.box(100, 200, 100);
    if (mousePressedOnParent) {
      g.fill(255);
      g.text("Mouse pressed on parent", 20, 20);
    }
  }

   void mousePressed() {
    arcball2.mousePressed();
  }

   void mouseDragged() {
    arcball2.mouseDragged();
  }
};

ChildApplet child;


void setup() {
  size(320, 240, P3D);
  smooth();
  surface.setTitle("Main sketch");
  arcball = Arcball(&surface, 300);
}

void draw() {
  background(250);
  noTexture();
  arcball.run();
  if (mousePressedb) {
    fill(0);
    text("Mouse pressed on parent.", 10, 10);
    fill(0, 240, 0);
    ellipse(mouseX, mouseY, 60, 60);
    mousePressedOnParent = true;
  } else {
    fill(20);
    ellipse(width/2, height/2, 60, 60);
    mousePressedOnParent = false;
  }
  box(100);
  if (child.mousePressedb) {
    text("Mouse pressed on child.", 10, 30);
  }
}

void mousePressed() {
  arcball.mousePressed();
}

void mouseDragged() {
  arcball.mouseDragged();
}

