PFont font;

void setup() {
   size(1000, 1000);
   font = createFont("SourceCodePro-Regular.ttf", 200);
   textFont( font );
}

void draw() {
   background(BLACK);
   stroke(GREEN);
   text("Fordham", 0, 0);
   shape(font.render_as_pshape("Fordham"), 0, 200 );
   image(font.render_as_pimage("Fordham"), 0 ,400);
}
