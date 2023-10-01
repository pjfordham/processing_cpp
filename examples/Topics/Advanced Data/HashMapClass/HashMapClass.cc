/**
 * HashMap example
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use a HashMap to store
 * a collection of objects referenced by a key. This is much like an array,
 * only instead of accessing elements with a numeric index, we use a String.
 * If you are familiar with associative arrays from other languages,
 * this is the same idea.
 *
 * A simpler example is CountingStrings which uses IntDict instead of
 * HashMap.  The Processing classes IntDict, FloatDict, and StringDict
 * offer a simpler way of pairing Strings with numbers or other Strings.
 * Here we use a HashMap because we want to pair a String with a custom
 * object, in this case a "Word" object that stores two numbers.
 *
 * In this example, words that appear in one book (Dracula) only are colored white
 * while words the other (Frankenstein) are colored black.
 */

#include "Word.h"

std::unordered_map<std::string, Word> words;  // HashMap object

void loadFile(std::string filename);

void setup() {
  size(640, 360);

  // Load two files
  loadFile("dracula.txt");
  loadFile("frankenstein.txt");

  // Create the font
  textFont(createFont("SourceCodePro-Regular.ttf", 24));
}

void draw() {
  background(126);

  // Show words
  for (auto& [key, w] : words) {
    if (w.qualify()) {
       w.display();
       w.move();
    }
  }
}

// Load a file
void loadFile(std::string filename) {
  std::vector<std::string> lines = loadStrings(filename);
  std::string allText = toLowerCase(join(lines, " "));
  std::vector<std::string> tokens = splitTokens(allText, " ,.?!:;[]-\"'");

  for (std::string &s : tokens) {
     // Is the word in the HashMap
    if (words.count(s) != 0) {
      // Get the word object and increase the count
      // We access objects from a HashMap via its key, the String
      Word &w = words[s];
      // Which book am I loading?
      if (filename.find("dracula") != std::string::npos) {
        w.incrementDracula();
      }
      else if (filename.find("frankenstein") != std::string::npos) {
        w.incrementFranken();
      }
    }
    else {
      // Otherwise make a new word
      Word w(s);
      // And add to the HashMap put() takes two arguments, "key" and "value"
      // The key for us is the String and the value is the Word object
      words[s] = w;
      if (filename.find("dracula") != std::string::npos) {
        w.incrementDracula();
      } else if (filename.find("frankenstein") != std::string::npos) {
        w.incrementFranken();
      }
    }
  }
}
