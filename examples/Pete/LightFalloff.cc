void setup() {
   size(400, 400, P3D);
}

void draw() {
   //lights();
   noStroke();
   background(0);
   ambientLight(0,0,0);
   directionalLight(0, 0, 0, 200, 200, 200);
   lightFalloff(1.0, 0.5, 0.0);
   pointLight(15, 25, 15, mouseX, 200, 200);
   beginShape();
   vertex(0, 0, 0);
   vertex(400, 0, -400);
   vertex(400, 400, -400);
   vertex(0, 400, 0);
   endShape(CLOSE);
}
