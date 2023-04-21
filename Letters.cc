#include "processing.h"

/**
 * Letters.
 *
 * Draws letters to the screen. This requires loading a font,
 * setting the font, and then drawing the letters.
 */

std::vector<std::string> fonts;
int i = 0;

void setup() {
  size(640, 600);
  background(0);

  fonts = PFont::list();

  frameRate(1);
  textAlign(CENTER, CENTER);
}

void draw() {
  background(0);

  // Create the font
  PFont f = createFont( fonts[ i++ % fonts.size()].c_str(), 24 );
  textFont(f);

  text(fonts[( (i++) % fonts.size())],width/2.0,570);
  // Set the left and top margin
  int margin = 10;
  translate(margin*4, margin*4);

  int gap = 46;
  int counter = 35;

  for (int y = 0; y < 480-gap; y += gap) {
    for (int x = 0; x < width-gap; x += gap) {

      char letter = char(counter);

      if (letter == 'A' || letter == 'E' || letter == 'I' || letter == 'O' || letter == 'U') {
        fill(255, 204, 0);
      }
      else {
        fill(255);
      }

      // Draw the letter to the screen
      text(letter, x, y);

      // Increment the counter
      counter++;
    }
  }
}
