/**
 * Tickle.
 *
 * The word "tickle" jitters when the cursor hovers over.
 * Sometimes, it can be tickled off the screen.
 */

std::string message = "tickle";
float x, y; // X and Y coordinates of text
float hr, vr;  // horizontal and vertical radius of the text

void setup() {
  size(640, 360);

  // Create the font
  textFont(createFont("SourceCodePro-Regular.ttf", 36));
  textAlign(CENTER, CENTER);

  hr = textWidth(message) / 2;
  vr = (textAscent() + textDescent()) / 2;
  noStroke();
  x = width / 2;
  y = height / 2;
}

void draw() {
  // Instead of clearing the background, fade it by drawing
  // a semi-transparent rectangle on top
  fill(204, 120);
  rect(0, 0, width, height);

  // If the cursor is over the text, change the position
  if (fabs(mouseX - x) < hr &&
      fabs(mouseY - y) < vr) {
    x += random(-5, 5);
    y += random(-5, 5);
  }
  fill(0);
  text("tickle", x, y);
  auto fimage = g.currentFont.render_as_pimage("tickle");
  g.drawPImageWithCPU(fimage, 10,10);
}
