void setup() {
  size(400, 400, P3D);
  noLoop();
}

void draw() {
  background(255, 0, 0);
  ellipse(mouseX, mouseY, 100, 50);
  fmt::print("draw\n");
}

void keyPressed() {
  redraw();
}
