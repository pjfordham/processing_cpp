/**
 * CountingString example
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use a IntDict to store
 * a number associated with a String.  Java HashMaps can also
 * be used for this, however, this example uses the IntDict
 * class offered by Processing's data package for simplicity
 * and added functionality.
 *
 * This example uses the IntDict to perform a simple concordance
 * http://en.wikipedia.org/wiki/Concordance_(publishing)
 *
 */

// An IntDict pairs Strings with integers
IntDict concordance;

// The raw array of words in
std::vector<std::string> tokens;
int counter = 0;

void setup() {
  size(640, 360);

  // Load file and chop it up
  std::vector<std::string> lines = loadStrings("dracula.txt");
  std::string allText = toLowerCase(join(lines, " "));
  tokens = splitTokens(allText, " ,.?!:;[]-\"");

  // Create the font
  textFont(createFont("SourceCodePro-Regular.ttf", 24));
}

void draw() {
  background(51);
  fill(255);

  // Look at words one at a time
  if (counter < tokens.size()) {
    std::string s = tokens[counter];
    counter++;
    concordance.increment(s);
  }

  // x and y will be used to locate each word
  float x = 0;
  float y = 48;

  concordance.sortValues();

  auto keys = concordance.keyArray();

  // Look at each word
  for (std::string word : keys) {
    int count = concordance.get(word);

    // Only display words that appear 3 times
    if (count > 3) {
      // The size is the count
      int fsize = constrain(count, 0, 48);
      textSize(fsize);
      text(word, x, y);
      // Move along the x-axis
      x += textWidth(word + " ");
    }

    // If x gets to the end, move y
    if (x > width) {
      x = 0;
      y += 48;
      // If y gets to the end, we're done
      if (y > height) {
        break;
      }
    }
  }
}
