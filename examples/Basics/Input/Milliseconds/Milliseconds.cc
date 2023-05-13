/**
 * Milliseconds.
 *
 * A millisecond is 1/1000 of a second.
 * Processing keeps track of the number of milliseconds a program has run.
 * By modifying this number with the modulo(%) operator,
 * different patterns in time are created.
 */

float scl;

void setup() {
  size(640, 360);
  noStroke();
  scl = width/20;
}

void draw() {
  for (int i = 0; i < scl; i++) {
    colorMode(RGB, (i+1) * scl * 10);
    fill(fmod(millis(),((i+1) * scl * 10)));
    rect(i*scl, 0, scl, height);
  }
}
