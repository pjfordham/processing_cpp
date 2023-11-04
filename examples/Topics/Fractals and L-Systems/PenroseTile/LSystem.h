class LSystem {
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
    startLength = 190.0;
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

  virtual void render() {
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

  std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
     size_t start_pos = 0;
     while((start_pos = str.find(from, start_pos)) != std::string::npos) {
       str.replace(start_pos, from.length(), to);
       start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
     }
     return str;
  }

  virtual std::string iterate(std::string prod_, std::string rule_) {
    drawLength = drawLength * 0.6;
    generations++;
    std::string newProduction = prod_;
    newProduction = ReplaceAll(newProduction,"F", rule_);
    return newProduction;
  }
};

