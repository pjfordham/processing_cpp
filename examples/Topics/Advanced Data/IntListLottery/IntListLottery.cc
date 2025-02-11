/**
 * IntList Lottery example
 * by Daniel Shiffman.
 *
 * This example demonstrates an IntList can be used to store a list of numbers.
 * While an array of integers serves a similar purpose it is of fixed size.  The
 * An IntList can easily have values added or deleted and it can also be
 * shuffled and sorted.  For lists of floats or Strings, you can use FloatList
 * and StringList.  For lists of objects, use ArrayList.
 *
 * In this example, three lists of integers are created.  One is a pool of numbers
 * that is shuffled and picked randomly from.  One is the list of "picked" numbers.
 * And one is a lottery "ticket" which includes 5 numbers that are trying to be matched.
 */
#include <deque>

// Three lists of integers
std::deque<int> lottery;
std::deque<int> results;
std::deque<int> ticket;

// Create a random number generator engine
std::mt19937 gen(millis());

void setup() {
  size(640, 360);
  frameRate(30);


  // Add 20 integers in order to the lottery list
  for (int i = 0; i < 20; i++) {
    lottery.push_back(i);
  }

  // Pick five numbers from the lottery list to go into the Ticket list
  for (int i = 0; i < 5; i++) {
    int index = int(random(lottery.size()));
    ticket.push_back(lottery[index]);
  }
}

void showList(std::deque<int> &list, float x, float y);

void draw() {
  background(51);

  // The shuffle() method randomly shuffles the order of the values in the list
  std::shuffle(lottery.begin(), lottery.end(), gen);

  // Call a method that will display the integers in the list at an x,y location
  showList(lottery, 16, 48);
  showList(results, 16, 100);
  showList(ticket, 16, 140);


  // This loop checks if the picked numbers (results)
  // match the ticket numbers
  for (int i = 0; i < results.size(); i++) {
    // Are the integers equal?
    if (results[i] == ticket[i]) {
      fill(0, 255, 0, 100);  // if so green
    } else {
      fill(255, 0, 0, 100);  // if not red
    }
    ellipse(16+i*32, 140, 24, 24);
  }


  // One every 30 frames we pick a new lottery number to go in results
  if (frameCount % 30 == 0) {
    if (results.size() < 5) {
      // Get the first value in the lottery list and remove it
      int val = lottery.front(); lottery.pop_front();
      // Put it in the results
      results.push_back(val);
    } else {
      // Ok we picked five numbers, let's reset
      for (int i = 0; i < results.size(); i++) {
        // Put the picked results back into the lottery
        lottery.push_back(results[i]);
      }
      // Clear the results and start over
      results.clear();
    }
  }
}

// Draw a list of numbers starting at an x,y location
void showList(std::deque<int> &list, float x, float y) {
  for (int i = 0; i < list.size(); i++) {
    // Use get() to pull a value from the list at the specified index
    int val = list[i];
    stroke(255);
    noFill();
    ellipse(x+i*32, y, 24, 24);
    textAlign(CENTER);
    fill(255);
    text(fmt::format("{}",val), x+i*32, y+6);
  }
}

