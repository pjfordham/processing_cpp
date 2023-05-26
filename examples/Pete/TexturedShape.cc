/**
 * Primitives 3D.
 *
 * Placing mathematically 3D objects in synthetic space.
 * The lights() method reveals their imagined dimension.
 * The box() and sphere() functions each have one parameter
 * which is used to specify their size. These shapes are
 * positioned using the translate() function.
 */

//PImage img;
void setup() {
  size(640, 640, P3D); 
  background(0);
//  img = createImage(1280,1280,0);
}

float a = 0;
void draw() {
  background(48);

  noStroke();
  fill(BLUE);
  
  noTexture();
  ellipse(width/2,height/2, width, height);
}
