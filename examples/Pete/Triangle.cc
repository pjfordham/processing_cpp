void setup() {
   size(640,480);
   background(0);
}

void mousePressed() {
   fmt::print("{},{}\n", mouseX, mouseY);
}

void draw() {

   stroke(WHITE);
   beginShape(TRIANGLES);

   // clockwise
   fill(RED);
   vertex(100, 100);
   fill(GREEN);
   vertex(250, 200);
   fill(BLUE);
   vertex(50, 200);
   endShape();
   
   // counterclockwise
   beginShape(TRIANGLES);
   fill(BLUE);
   vertex(50, 400);
   fill(GREEN);
   vertex(250, 400);
   fill(RED);
   vertex(100, 300);

   endShape();

    beginShape(QUADS);

    // clockwise
    fill(RED);
    vertex(300, 100);
    fill(GREEN);
    vertex(550, 200);
    fill(BLUE);
    vertex(400, 275);
    fill(YELLOW);
    vertex(325, 200);
    endShape();

    // // counterclockwise
    beginShape(QUADS);
    fill(RED);
    vertex(300, 300);
    fill(BLUE);
    vertex(325, 400);
    fill(GREEN);
    vertex(400, 475);
    fill(YELLOW);
    vertex(550, 400);

    endShape();
   // beginShape(QUAD_STRIP);

   // // clockwise
   // fill(RED);
   // vertex(300,200);
   // fill(GREEN);
   // vertex(300,50);
   // fill(BLUE);
   // vertex(400,250);
   // fill(YELLOW);
   // vertex(425,125);
   // fill(CYAN);
   // vertex(525,250);
   // fill(MAGENTA);
   // vertex(475,75);
   // fill(WHITE);
   // vertex(600,150);
   // fill(BLUE);
   // vertex(575,100);
   // endShape();

   // // // counterclockwise
   // beginShape(QUAD_STRIP);
   // fill(GREEN);
   // vertex(300,250);
   // fill(RED);
   // vertex(300,400);

   // fill(YELLOW);
   // vertex(425,325);
   // fill(BLUE);
   // vertex(400,450);

   // fill(MAGENTA);
   // vertex(475,275);
   // fill(CYAN);
   // vertex(525,450);

   // fill(BLUE);
   // vertex(575,300);
   // fill(WHITE);
   // vertex(600,350);

   // endShape();
}
