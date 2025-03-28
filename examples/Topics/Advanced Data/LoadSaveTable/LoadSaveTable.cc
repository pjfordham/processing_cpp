/**
 * Loading Tabular Data
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use loadTable()
 * to retrieve data from a CSV file and make objects
 * from that data.
 *
 * Here is what the CSV looks like:
 *
 x,y,diameter,name
 160,103,43.19838,Happy
 372,137,52.42526,Sad
 273,235,61.14072,Joyous
 121,179,44.758068,Melancholy
 */

#include "Bubble.h"


void loadData();


// An Array of Bubble objects
std::vector<Bubble> bubbles;
// A Table object
Table table;

void setup() {
  size(640, 360);
  loadData();
}

void draw() {
  background(255);
  // Display all bubbles
  for (Bubble &b : bubbles) {
    b.display();
    b.rollover(mouseX, mouseY);
  }

  textAlign(LEFT);
  fill(0);
  text("Click to add bubbles.", 10, height-10);
}

void loadData() {
  // Load CSV file into a Table object
  // "header" option indicates the file has a header row
  table = loadTable("data.csv", "header");

  // The size of the array of Bubble objects is determined by the total number of rows in the CSV
  bubbles.clear();

  // You can access iterate over all the rows in a table
  for (TableRow &row : table.rows()) {
    // You can access the fields via their column name (or index)
    float x = row.getFloat("x");
    float y = row.getFloat("y");
    float d = row.getFloat("diameter");
    std::string n = row.getString("name");
    // Make a Bubble object out of the data read
    bubbles.emplace_back(x, y, d, n);
  }
}

void mousePressed() {
  // Create a new row
  TableRow &row = table.addRow();
  // Set the values of that row
  row.setFloat("x", mouseX);
  row.setFloat("y", mouseY);
  row.setFloat("diameter", random(40, 80));
  row.setString("name", "Blah");

  // If the table has more than 10 rows
  if (table.getRowCount() > 10) {
    // Delete the oldest row
    table.removeRow(0);
  }

  // Writing the CSV back to the same file
  saveTable(table, "data.csv");
  // And reloading it
  loadData();
}

