/**
 * Linear Image.
 *
 * Click and drag mouse up and down to control the signal.
 * Press and hold any key to watch the scanning.
 */

PImage img;
int direction = 1;

float signal;

void setup() {
  size(640, 360);
  stroke(255);
  img = loadImage("sea.jpg");
  img.loadPixels();
  loadPixels();
}

void draw() {
  if (signal > img.height-1 || signal < 0) {
    direction = direction * -1;
  }
  if (mousePressedb) {
    signal = abs(mouseY % img.height);
  }
  else {
    signal += (0.3*direction);
  }

  if (keyPressedb) {
    set(0, 0, img);
    line(0, signal, img.width, signal);
  }
  else {
    int signalOffset = int(signal)*img.width;
    for (int y = 0; y < img.height; y++) {
      std::copy(img.pixels + signalOffset,
                img.pixels + signalOffset + img.width,
                pixels + y*width );
    }
    updatePixels();
  }
}

