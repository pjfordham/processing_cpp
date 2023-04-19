/**
 * BezierVertex.
 *
 *  Each call to bezierVertex() defines the position of two control points
 *  and one anchor point of a Bezier curve, adding a new segment to a line
 *  or shape. The first time bezierVertex() is used within a beginShape()
 *  call, it must be prefaced with a call to vertex() to set the first
 *  anchor point.
 */

void setup() {
   size(800, 400);
   stroke(BLACK);
   noFill();
}

void draw() {
   background(132);

   noFill();
   beginShape();
   vertex(120, 80);
   bezierVertex(320, 0, 320, 300, 120, 300);
   endShape(OPEN);

   fill(WHITE);
   beginShape();
   vertex(520, 80);
   bezierVertex(720, 0, 720, 300, 490, 300);
   bezierVertex(600, 320, 640, 100, 520, 80);
   endShape();

}
