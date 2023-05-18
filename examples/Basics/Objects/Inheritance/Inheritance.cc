/**
 * Inheritance
 *
 * A class can be defined using another class as a foundation. In object-oriented
 * programming terminology, one class can inherit fi elds and methods from another.
 * An object that inherits from another is called a subclass, and the object it
 * inherits from is called a superclass. A subclass extends the superclass.
 */

class Spin {
public:
  float x, y, speed;
  float angle = 0.0;
  Spin() {}
  Spin(float xpos, float ypos, float s) {
    x = xpos;
    y = ypos;
    speed = s;
  }
  void update() {
    angle += speed;
  }
};

class SpinArm : public Spin {
public:
  SpinArm() {}
  SpinArm(float x, float y, float s) : Spin( x, y, s) {
  }
  void display() {
    strokeWeight(1);
    stroke(0);
    pushMatrix();
    translate(x, y);
    angle += speed;
    rotate(angle);
    line(0, 0, 165, 0);
    popMatrix();
  }
};

class SpinSpots : public Spin {
public:
  float dim;
  SpinSpots() {}
  SpinSpots(float x, float y, float s, float d) : Spin( x, y, s) {
    dim = d;
  }
  void display() {
    noStroke();
    pushMatrix();
    translate(x, y);
    angle += speed;
    rotate(angle);
    ellipse(-dim/2, 0, dim, dim);
    ellipse(dim/2, 0, dim, dim);
    popMatrix();
  }
};

SpinSpots spots;
SpinArm arm;

void setup() {
  size(640, 360);
  arm = SpinArm(width/2, height/2, 0.01);
  spots = SpinSpots(width/2, height/2, -0.02, 90.0);
}

void draw() {
  background(204);
  arm.update();
  arm.display();
  spots.update();
  spots.display();
}
