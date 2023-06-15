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

   //  float mouseX = width/4;
   // float mouseY =height/4;
   PVector p1{50.0f,height/2.0f};
   PVector p2{(float)mouseX,(float)mouseY};
   PVector p3{(float)width-mouseX,(float)height - mouseY};
   PVector p4{width-50.0f,height/2.0f};

   strokeWeight(10);
   noFill();
   stroke(255, 160);
   beginShape();
   vertex(p1.x,p1.y);
   vertex(p2.x,p2.y);
   vertex(p3.x,p3.y);
   vertex(p4.x,p4.y);
   endShape();

   std::vector<PLine> lines = { {p1, p2}, {p2, p3} , {p3, p4} };

   float offset = 50;

   for (int i = 0; i < lines.size() -1; ++i) {
      auto &a = lines[i];
      auto &b = lines[i+1];

      PLine low_a = a.offset(-offset);
      PLine high_a = a.offset(offset);

      PLine low_b = b.offset(-offset);
      PLine high_b = b.offset(offset);

      strokeWeight(1);

      stroke(GREEN);
      line(high_a);
      line(high_b);

      stroke(RED);
      line(low_a);
      line(low_b);

      PVector i_high = high_a.intersect(high_b);
      stroke(GREEN);
      strokeWeight(5);
      point(i_high.x,i_high.y);

      PVector i_low = low_a.intersect(low_b);
      stroke(RED);
      strokeWeight(5);
      point(i_low.x,i_low.y);
   }
}
