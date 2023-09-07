///
 void setup() {
   size(640, 640);
}
 ///

void draw() {
   background( MAGENTA );
   fill( BLUE );
   ellipse(100,100,150,150);
   fill (WHITE);
   ellipse(100,100,50,50);
   fill( YELLOW );
   beginShape( TRIANGLES );
   vertex(200,200);
   vertex(300,200);
   vertex(200,300);
   endShape( CLOSE );
   fill( GREEN );
   beginShape( POLYGON );
   vertex(mouseX,mouseY);
   vertex(500,400);
   vertex(500,500);
   vertex(400,500);
   endShape( CLOSE );
   fill( 0, 127, 0 );
   rect( 300,500,200,100);
}
