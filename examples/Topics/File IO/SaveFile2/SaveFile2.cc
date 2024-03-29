/**
 * SaveFile 2
 *
 * This file a PrintWriter object to write data continuously to a file
 * while the mouse is pressed. When a key is pressed, the file closes
 * itself and the program is stopped.
 */

PWriter output;

void setup()
{
  size(200, 200);
  // Create a new file in the sketch directory
  output = createWriter("positions.txt");
  frameRate(12);
}

void draw()
{
  if (mousePressedb) {
    point(mouseX, mouseY);
    // Write the coordinate to a file with a
    // "\t" (TAB character) between each entry
    output.println(fmt::format("{}\t{}", mouseX, mouseY));
  }
}

void keyPressed() { // Press a key to save the data
  output.flush(); // Write the remaining data
  output.close(); // Finish the file
  exit(); // Stop the program
}
