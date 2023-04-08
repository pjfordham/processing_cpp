/**
 * Line Segments.
 *
 * Project a line to have a thickness and
 * work out how the ends should interct cleanly.
*/

void setup() {
  size(640, 360);

}

void draw() {
  background(0);

  PVector p1{50,height/2};
  PVector p2{mouseX,mouseY};
  PVector p3{width-50,height/2};

  PLine a{p1, p2};
  PLine b{p2, p3};

  strokeWeight(10);
  stroke(255, 160);
  line(a);
  line(b);

  PLine low_a = a.offset(-50);
  PLine high_a = a.offset(50);

  PLine low_b = b.offset(-50);
  PLine high_b = b.offset(50);

  strokeWeight(1);

  stroke(255,0,0, 160);
  line(low_a);
  line(low_b);

  stroke(0,255,0, 160);
  line(high_a);
  line(high_b);

  PVector i_upper = high_a.intersect(high_b);
  stroke(RED);
  strokeWeight(5);
  point(i_upper.x,i_upper.y);

  PVector i_lower = low_a.intersect(low_b);
  stroke(RED);
  strokeWeight(5);
  point(i_lower.x,i_lower.y);
}
