PFont font;

void setup() {
   size(1000, 1000);
   font = createFont("SourceCodePro-Regular.ttf", 200);
   textFont( font );
}

void draw() {
   background(BLACK);
   fill(RED);
   text("Fordham", 0, 200);
   shape(font.render_as_pshape("Fordham"), 0, 200 );
   tint(BLUE);
   image(font.render_as_pimage("Fordham"), 0 ,400);
}
