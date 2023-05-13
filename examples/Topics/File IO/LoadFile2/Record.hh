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
