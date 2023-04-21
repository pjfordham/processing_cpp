/**
 * Penrose Snowflake L-System
 * by Geraldine Sarmiento.
 *
 * This example was based on Patrick Dwyer's L-System class.
 */

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



class PenroseSnowflakeLSystem : public LSystem {

public:
  std::string ruleF;

  PenroseSnowflakeLSystem() {
    axiom = "F3-F3-F3-F3-F";
    ruleF = "F3-F3-F45-F++F3-F";
    startLength = 450.0;
    theta = radians(18);
    reset();
  }

  void useRule(std::string r_) {
    rule = r_;
  }

  void useAxiom(std::string a_) {
    axiom = a_;
  }

  void useLength(float l_) {
    startLength = l_;
  }

  void useTheta(float t_) {
    theta = radians(t_);
  }

  void reset() {
    production = axiom;
    drawLength = startLength;
    generations = 0;
  }

  void render() {
    translate(width, height);
    int repeats = 1;

    steps += 3;
    if (steps > production.length()) {
      steps = production.length();
    }

    for (int i = 0; i < steps; i++) {
      char step = production[i];
      if (step == 'F') {
        for (int j = 0; j < repeats; j++) {
          line(0,0,0, -drawLength);
          translate(0, -drawLength);
        }
        repeats = 1;
      }
      else if (step == '+') {
        for (int j = 0; j < repeats; j++) {
          rotate(theta);
        }
        repeats = 1;
      }
      else if (step == '-') {
        for (int j =0; j < repeats; j++) {
          rotate(-theta);
        }
        repeats = 1;
      }
      else if (step == '[') {
        pushMatrix();
      }
      else if (step == ']') {
        popMatrix();
      }
      else if ( (step >= 48) && (step <= 57) ) {
        repeats += step - 48;
      }
    }
  }


  std::string iterate(std::string prod_, std::string rule_) {
    std::string newProduction;
    for (int i = 0; i < prod_.length(); i++) {
      char step = production[i];
      if (step == 'F') {
        newProduction = newProduction + ruleF;
      }
      else {
        newProduction = newProduction + step;
      }
    }
    drawLength = drawLength * 0.4;
    generations++;
    return newProduction;
  }

};

PenroseSnowflakeLSystem ps;

void setup() {
  size(640, 360);
  stroke(255);
  noFill();
  ps = PenroseSnowflakeLSystem();
  ps.simulate(4);
}

void draw() {
  background(0);
  ps.render();
}
