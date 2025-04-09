/**
 * Ignore Styles.
 * Illustration by George Brower.
 *
 * Shapes are loaded with style information that tells them how
 * to draw (the color, stroke weight, etc.) The disableStyle()
 * method of PShape turns off this information. The enableStyle()
 * method turns it back on.
 */

PShape bot;

float zoom = 1.0;             // Current zoom level
float panX = 0, panY = 0;     // Current pan offset
float prevMouseX, prevMouseY; // Previous mouse position
boolean isDragging = false;

void setup() {
  size(640, 360);
  // The file "bot1.svg" must be in the data folder
  // of the current sketch to load successfully
  bot = loadShape("bot1.svg");
}

void draw() {
  background(102);
  translate(width / 2, height / 2);  // Move origin to center of screen
  scale(zoom);
  translate(-width / 2 + panX, -height / 2 + panY);

  // Draw left bot
  bot.disableStyle();  // Ignore the colors in the SVG
  fill(0, 102, 153);    // Set the SVG fill to blue
  stroke(255);          // Set the SVG fill to white
  shape(bot, 20, 25, 300, 300);

  // Draw right bot
  bot.enableStyle();
  shape(bot, 320, 25, 300, 300);
}


void mousePressed() {
//    fmt::print("{},{}\n", mouseX, mouseY);
   prevMouseX = mouseX;
   prevMouseY = mouseY;
   isDragging = true;
}

void mouseDragged() {
   if (isDragging) {
      // Pan the scene relative to zoom level
      float dx = (mouseX - prevMouseX) / zoom;
      float dy = (mouseY - prevMouseY) / zoom;
      panX += dx;
      panY += dy;

      prevMouseX = mouseX;
      prevMouseY = mouseY;
   }
}

void mouseReleased() {
    isDragging = false;
}

void mouseWheel(const MouseEvent &event) {
// Handle mouse wheel zooming (centered on cursor)
   float zoomFactor = 1.1; // Scaling factor
   float delta = event.getCount();

   // Compute new zoom level
   float newZoom = zoom * (delta > 0 ? 1 / zoomFactor : zoomFactor);

   // Get world coordinates of mouse before zoom
   float worldX = (mouseX - width / 2) / zoom - panX;
   float worldY = (mouseY - height / 2) / zoom - panY;

   // Apply new zoom level
   zoom = newZoom;

   // Adjust panning to keep mouse position consistent
   panX = (mouseX - width / 2) / zoom - worldX;
   panY = (mouseY - height / 2) / zoom - worldY;
}
