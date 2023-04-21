/**
 * LoadFile 1
 *
 * Loads a text file that contains two numbers separated by a tab ('\t').
 * A new pair of numbers is loaded each frame and used to draw a point on the screen.
 */

std::vector<std::string> lines;
int index = 0;

void setup() {
  size(640, 360);
  background(0);
  stroke(255);
  frameRate(12);
  lines = loadStrings("positions.txt");
}

void draw() {
  if (index < lines.size()) {
    auto pieces = split(lines[index], ',');
    if (pieces.size() == 2) {
      // Scale the coordinates to match the size of the sketch window
      float x = map(std::stof(pieces[0]),0,120,0,width);
      float y = map(std::stof(pieces[1]),0,120,0,height);
      strokeWeight(2);
      point(x, y);
    }
    // Go to the next line for the next run through draw()
    index = index + 1;
  }
}
