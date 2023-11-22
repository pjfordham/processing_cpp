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
   text("Peter\nFordham", 0, 200);
   shape(font.render_as_pshape("Peter\nFordham"), 0, 800 );
   tint(BLUE);
   image(font.render_as_pimage("Peter\nFordham"), 0 ,400);
}
