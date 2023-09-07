void setup() {
   size(640, 480);
}

float offset[3] = { 0, TWO_PI / 3, 2 * TWO_PI / 3 };
float x = 0, y = 0;
float inc = TWO_PI/ 120.0;
float f = 2.0;

void drawBond(PVector start, PVector end, float noise_amplitude) {
   beginShape(LINES);
   for (float i = 0 ; i<= 1 ; i+=(1.0/20.0)) {
      PVector v = lerp( start, end, i );
      float noise = random(-noise_amplitude,noise_amplitude);
      PVector normal = PLine{start,end}.normal();
      vertex(v + normal * noise);
   }
   endShape();
}

void draw() {
   background(BLACK);
   lights();
   translate(width/2, height/2);
   noStroke();

   PVector pos[3] = {
      { cosf(x+offset[0]), sinf(y+f*offset[0]), -sinf(y+f*offset[1]) * 0.5f },
      { cosf(x+offset[1]), sinf(y+f*offset[1]),  sinf(y+f*offset[2]) * 0.5f },
      { cosf(x+offset[2]), sinf(y+f*offset[2]), -sinf(y+f*offset[0]) * 0.5f },
   };

   scale(100);

   fill(RED);
   pushMatrix();
   translate(pos[0]);
   sphere(0.5);
   popMatrix();

   fill(GREEN);
   pushMatrix();
   translate(pos[1]);
   sphere(0.5);
   popMatrix();

   fill(BLUE);
   pushMatrix();
   translate(pos[2]);
   sphere(0.5);
   popMatrix();

   stroke(YELLOW);
   strokeWeight(0.03);
   noFill();
   drawBond(pos[0],pos[1],0.15);
   drawBond(pos[1],pos[2],0.15);
   drawBond(pos[2],pos[0],0.15);

   x+=inc;
   y+=f*inc;
}
