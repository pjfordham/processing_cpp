/**
 * Arc.
 * by Peter Fordham.
 *
 * Exercise the arc function in it's various modes.
 */

void setup() {
   size(400, 400);
}

void draw() {
   background(220);

   stroke(0);
   fill(255);
   arc(100, 100, 150, 150, 0, HALF_PI);

   noFill();
   arc(100, 100, 160, 160, HALF_PI, PI);
   arc(100, 100, 170, 170, PI, PI+QUARTER_PI);
   arc(100, 100, 180, 180, PI+QUARTER_PI, TWO_PI);

   fill(255);
   arc(300, 100, 150, 150, 0, PI+QUARTER_PI, OPEN);
   arc(100, 300, 170, 170, 0, PI+QUARTER_PI, CHORD);
   arc(300, 300, 190, 190, 0, PI+QUARTER_PI, PIE);
}
