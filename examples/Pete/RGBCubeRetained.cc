/**
 * RGB Cube.
 *
 * The three primary colors of the additive color model are red, green, and blue.
 * This RGB color cube displays smooth transitions between these colors.
 */

float xmag, ymag = 0;
float newXmag, newYmag = 0;
PShape cubes, cube, cube1, cube2;

void setup()  {
  size(640, 360, P3D);
  noStroke();
  colorMode(RGB, 1);
  cube = createShape();
  cube.beginShape(QUADS);

  cube.fill(0, 1, 1); cube.vertex(-1,  1,  1);
  cube.fill(1, 1, 1); cube.vertex( 1,  1,  1);
  cube.fill(1, 0, 1); cube.vertex( 1, -1,  1);
  cube.fill(0, 0, 1); cube.vertex(-1, -1,  1);

  cube.fill(1, 1, 1); cube.vertex( 1,  1,  1);
  cube.fill(1, 1, 0); cube.vertex( 1,  1, -1);
  cube.fill(1, 0, 0); cube.vertex( 1, -1, -1);
  cube.fill(1, 0, 1); cube.vertex( 1, -1,  1);

  cube.fill(1, 1, 0); cube.vertex( 1,  1, -1);
  cube.fill(0, 1, 0); cube.vertex(-1,  1, -1);
  cube.fill(0, 0, 0); cube.vertex(-1, -1, -1);
  cube.fill(1, 0, 0); cube.vertex( 1, -1, -1);

  cube.fill(0, 1, 0); cube.vertex(-1,  1, -1);
  cube.fill(0, 1, 1); cube.vertex(-1,  1,  1);
  cube.fill(0, 0, 1); cube.vertex(-1, -1,  1);
  cube.fill(0, 0, 0); cube.vertex(-1, -1, -1);

  cube.fill(0, 1, 0); cube.vertex(-1,  1, -1);
  cube.fill(1, 1, 0); cube.vertex( 1,  1, -1);
  cube.fill(1, 1, 1); cube.vertex( 1,  1,  1);
  cube.fill(0, 1, 1); cube.vertex(-1,  1,  1);

  cube.fill(0, 0, 0); cube.vertex(-1, -1, -1);
  cube.fill(1, 0, 0); cube.vertex( 1, -1, -1);
  cube.fill(1, 0, 1); cube.vertex( 1, -1,  1);
  cube.fill(0, 0, 1); cube.vertex(-1, -1,  1);

  cube.endShape();
  cube1 = cube.copy();

  cube2 = cube1.copy();
  cube1.translate(3.5,0,0);
  cube2.translate(-3.5,0,0);

  cubes = createGroup();
  cubes.addChild( cube );
  cubes.addChild( cube1 );
  cubes.addChild( cube2 );

}

void draw()  {
  background(0.5);

  pushMatrix();
  translate(width/2, height/2, -30);

  newXmag = (float)mouseX/float(width) * TWO_PI;
  newYmag = (float)mouseY/float(height) * TWO_PI;

  float diff = xmag-newXmag;
  if (abs(diff) >  0.01) {
    xmag -= diff/4.0;
  }

  diff = ymag-newYmag;
  if (abs(diff) >  0.01) {
    ymag -= diff/4.0;
  }

  cube.resetMatrix();
  cube.rotateX(-ymag);
  cube.rotateY(-xmag);

  cube1.rotateX(0.01);
  cube2.rotateY(0.01);

  scale(50);

  shape(cubes);

  popMatrix();
}
