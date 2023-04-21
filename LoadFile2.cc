/**
 * LoadFile 2
 *
 * This example loads a data file about cars. Each element is separated
 * with a tab and corresponds to a different aspect of each car. The file stores
 * the miles per gallon, cylinders, displacement, etc., for more than 400 different
 * makes and models. Press a mouse button to advance to the next group of entries.
 */

class Record {
public:
  std::string name;
  float mpg;
  int cylinders;
  float displacement;
  float horsepower;
  float weight;
  float acceleration;
  int year;
  float origin;
  Record() {}

  Record(std::vector<std::string> pieces) {
    name = pieces[0];
    mpg = std::stof(pieces[1]);
    cylinders = std::stoi(pieces[2]);
    displacement = std::stof(pieces[3]);
    horsepower = std::stof(pieces[4]);
    weight = std::stof(pieces[5]);
    acceleration = std::stof(pieces[6]);
    year = std::stoi(pieces[7]);
    origin = std::stof(pieces[8]);
  }
};

std::vector<Record> records;
std::vector<std::string> lines;
int recordCount;
PFont body;
int num = 15; // Display this many entries on each screen.
int startingEntry = 0;  // Display from this entry number

void setup() {
  size(640, 360);
  fill(255);
  noLoop();

  body = createFont("SourceCodePro-Regular.ttf",20);
  textFont(body);

  lines = loadStrings("cars2.tsv");
  for (int i = 0; i < lines.size(); i++) {
    auto pieces = split(lines[i], '\t'); // Load data into array
    if (pieces.size() == 9) {
      records.emplace_back( pieces );
      recordCount++;
    }
  }
}

void draw() {
  background(0);
  for (int i = 0; i < num; i++) {
    int thisEntry = startingEntry + i;
    if (thisEntry < recordCount) {
      text(std::to_string(thisEntry) + " > " + records[thisEntry].name, 20, 20 + i*20);
    }
  }
}

void mousePressed() {
  startingEntry += num;
  if (startingEntry > records.size()) {
    startingEntry = 0;  // go back to the beginning
  }
  redraw();
}
