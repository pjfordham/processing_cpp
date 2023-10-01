/**
 * SaveFile 1
 *
 * Saving files is a useful way to store data so it can be viewed after a
 * program has stopped running. The saveStrings() function writes an array
 * of strings to a file, with each string written to a new line. This file
 * is saved to the sketch's folder.
 */

std::vector<int> x,y;

void setup()
{
  size(200, 200);
}

void draw()
{
  background(204);
  stroke(0);
  noFill();
  beginShape();
  for (int i = 0; i < x.size(); i++) {
    vertex(x[i], y[i]);
  }
  endShape();
  // Show the next segment to be added
  if (x.size() >= 1) {
    stroke(255);
    line(mouseX, mouseY, x[x.size()-1], y[x.size()-1]);
  }
}

void mousePressed() { // Click to add a line segment
   x.push_back( mouseX );
   y.push_back( mouseY);
}

void keyPressed() { // Press a key to save the data
  std::vector<std::string> lines;
  for (int i = 0; i < x.size(); i++) {
     lines.push_back( fmt::format("{}\t{}", x[i], y[i]) );
  }
  saveStrings("lines.txt", lines);
  exit(); // Stop the program
}

