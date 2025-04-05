/**
 * Regular Expression example
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use matchAll() to create
 * a list of all matches of a given regex.
 *
 * Here we'll load the raw HTML from a URL and search for any
 * <a href=" "> links
 */

#include <regex>

// Our source url
std::string url = "https://processing.org";
// We'll store the results in an array
std::vector<std::string> links;

std::vector<std::string> loadLinks(std::string_view s);

void setup() {
  size(640, 360);
  // Load the links
  links = loadLinks(url);
}

void draw() {
  background(0);
  // Display the raw links
  fill(255);
  for (int i = 0; i < links.size(); i++) {
    text(links[i],10,16+i*16);
  }
}

std::vector<std::string> loadLinks(std::string_view s) {
  // Load the raw HTML
  std::vector<std::string> lines = loadStrings(s);
  // Put it in one big string
  std::string html = join(lines,"\n");

  // A wacky regex for matching a URL
  std::string regexStr = "<\\s*a\\s+href\\s*=\\s*\"(.*?)\"";

  std::regex regex(regexStr, std::regex::icase);

  // An array for the results
  std::vector<std::string> results;
  auto begin = std::sregex_iterator(html.begin(), html.end(), regex);
  auto end = std::sregex_iterator();

  // We want group 1 for each result
  for (auto i = begin; i != end; ++i) {
      results.push_back((*i)[1].str());
  }

  // Return the results
  return results;
}

