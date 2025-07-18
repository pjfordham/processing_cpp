/**
 * Thread function example
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use thread() to spawn
 * a process that happens outside of the main animation thread.
 *
 * When thread() is called, the draw() loop will continue while
 * the code inside the function passed to thread() will operate
 * in the background.
 *
 */

#include <thread>
#include <atomic>

// This sketch will load data from all of these URLs in a separate thread
std::vector<std::string> urls = {
  "http://processing.org",
  "http://www.processing.org/exhibition/",
  "http://www.processing.org/reference/",
  "http://www.processing.org/reference/libraries",
  "http://www.processing.org/reference/tools",
  "http://www.processing.org/reference/environment",
  "http://www.processing.org/learning/",
  "http://www.processing.org/learning/basics/",
  "http://www.processing.org/learning/topics/",
  "http://www.processing.org/learning/gettingstarted/",
  "http://www.processing.org/download/",
  "http://www.processing.org/shop/",
  "http://www.processing.org/about/"
};

// This will keep track of whether the thread is finished
std::atomic<boolean> finished = false;
// And how far along
float percent = 0;

// A variable to keep all the data loaded
std::string allData;
std::jthread myThread;

void loadData();

void setup() {
  size(640, 360);
  // Spawn the thread!
  myThread = std::jthread(loadData);
}

void draw() {
  background(0);

  // If we're not finished draw a "loading bar"
  // This is so that we can see the progress of the thread
  // This would not be necessary in a sketch where you wanted to load data in the background
  // and hide this from the user, allowing the draw() loop to simply continue
  if (!finished) {
    stroke(255);
    noFill();
    rect(width/2-150, height/2, 300, 10);
    fill(255);
    // The size of the rectangle is mapped to the percentage completed
    float w = map(percent, 0, 1, 0, 300);
    rect(width/2-150, height/2, w, 10);
    textSize(16);
    textAlign(CENTER);
    fill(255);
    text("Loading", width/2, height/2+30);
  }
  else {
    if (myThread.joinable())
       myThread.join();
    // The thread is complete!
    textAlign(CENTER);
    textSize(24);
    fill(255);
    text("Finished loading. Click the mouse to load again.", width/2, height/2);
  }
}

void mousePressed() {
   if (finished) {
      // The thread is not completed
      finished = false;
      myThread = std::jthread(loadData);
   }
}

void loadData() {
  // Reset the data to empty
  allData = "";

  // Look at each URL
  // This example is doing some highly arbitrary things just to make it take longer
  // If you had a lot of data parsing you needed to do, this can all happen in the background
  for (int i = 0; i < urls.size(); i++) {
    std::vector<std::string> lines = loadStrings(urls[i]);
    // Demonstrating some arbitrary text splitting, joining, and sorting to make the thread take longer
    std::string allTxt = join(lines, " ");
    std::vector<std::string> words = splitTokens(allTxt, "\t+\n <>=\\-!@#$%^&*(),.;:/?\"\'");
    for (int j = 0; j < words.size(); j++) {
      words[j] = trim( words[j] );
      words[j] = toLowerCase(words[j]);
    }
    std::sort(words.begin(), words.end());
    allData += join(words, " ");
    percent = float(i)/urls.size();;
  }

  std::vector<std::string> words = split(allData," ");
  sort(words.begin(), words.end());
  allData = join(words, " ");

  // The thread is completed!
  finished = true;
}

