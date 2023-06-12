class LSystem
{
public:
  int steps = 0;

  std::string axiom;
  std::string rule;
  std::string production;

  float startLength;
  float drawLength;
  float theta;

  int generations;

  LSystem() {
    axiom = "F";
    rule = "F+F-F";
    startLength = 90.0;
    theta = radians(120.0);
    reset();
  }

  void reset() {
    production = axiom;
    drawLength = startLength;
    generations = 0;
  }

  int getAge() {
    return generations;
  }

  void render() {
    translate(width/2, height/2);
    steps += 5;
    if (steps > production.length()) {
      steps = production.length();
    }
    for (int i = 0; i < steps; i++) {
      char step = production[i];
      if (step == 'F') {
        rect(0, 0, -drawLength, -drawLength);
        noFill();
        translate(0, -drawLength);
      }
      else if (step == '+') {
        rotate(theta);
      }
      else if (step == '-') {
        rotate(-theta);
      }
      else if (step == '[') {
        pushMatrix();
      }
      else if (step == ']') {
        popMatrix();
      }
    }
  }

  void simulate(int gen) {
    while (getAge() < gen) {
      production = iterate(production, rule);
    }
  }

  std::string find_replace( std::string str, std::string toReplace, std::string replaceWith ) {

    fprintf(stderr, "IN ) %s %s %s\n", str.c_str(), toReplace.c_str(),
            replaceWith.c_str());
    // Find the first occurrence of the substring
    size_t pos = str.find(toReplace);

    // Loop until no more occurrences are found
    while (pos != std::string::npos) {
      // Replace the substring with the replacement string
      str.replace(pos, toReplace.length(), replaceWith);

      // Find the next occurrence of the substring
      pos = str.find(toReplace, pos + replaceWith.length());
    }
    fprintf(stderr, "OUT) %s\n", str.c_str());
    return str;
  }

  virtual std::string iterate(std::string prod_, std::string rule_) {
    drawLength = drawLength * 0.6;
    generations++;
    std::string newProduction = prod_;
    newProduction = find_replace(newProduction, "F", rule_);
    return newProduction;
  }
};

