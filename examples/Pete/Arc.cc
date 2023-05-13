/**
 * Arc.
 * by Peter Fordham.
 *
 * Exercise the arc function in it's various modes.
 */

void setup() {
   size(400, 400);
}

float i = PI + QUARTER_PI;
float inc = TWO_PI/600.0;

void draw() {
   background(BLACK);

   stroke(WHITE);
   strokeWeight(5);
   fill(YELLOW);
   arc(100, 100, 150, 150, 0, HALF_PI);

   noFill();
   arc(100, 100, 180, 180, HALF_PI, PI);
   arc(100, 100, 140, 140, PI, PI+QUARTER_PI);
   arc(100, 100, 100, 100, PI+QUARTER_PI, TWO_PI);

   fill(RED);
   arc(300, 100, 150, 150, 0, i, OPEN);
   fill(GREEN);
   arc(100, 300, 170, 170, 0, i, CHORD);
   fill(BLUE);
   arc(300, 300, 190, 190, 0, i, PIE);
   i = fmod(i + inc, TWO_PI);
}
