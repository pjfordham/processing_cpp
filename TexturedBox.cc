/**
 * Primitives 3D.
 *
 * Placing mathematically 3D objects in synthetic space.
 * The lights() method reveals their imagined dimension.
 * The box() and sphere() functions each have one parameter
 * which is used to specify their size. These shapes are
 * positioned using the translate() function.
 */

PImage img;
void setup() {
  size(640, 360, P3D); 
  background(0);
  img = loadImage("moon.jpg");
}

float a = 0;
void draw() {
  background(48);
  lights();

  noStroke();
  pushMatrix();
  translate(130, height/2, 0);
  rotateY(1.25 + a);
  rotateX(-0.4 + 0.3 * a);
  a += 0.01;
  tint(BLUE);
  texture(img);
  box(100);
  popMatrix();

  // noFill();
  stroke(255);
  pushMatrix();
  translate(500, height*0.35, -200);
  noTexture();
  sphere(mouseX);
  popMatrix();
}
