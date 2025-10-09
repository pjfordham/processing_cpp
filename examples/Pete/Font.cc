PFont font;

void setup() {
   size(1000, 1300);
   font = createFont("SourceCodePro-Regular.ttf", 200);
   textFont( font );
   noLoop();
}

void draw() {
   background(BLACK);
   fill(RED);
   text("Peter 0\nFordham", 0, 200);
   tint(BLUE);
   image(font.render_as_pimage("Peter 0\nFordham"), 0 ,400);
   fill(WHITE);
   shape(font.render_as_pshape("Peter 0\nFordham"), 0, 800 );
}
