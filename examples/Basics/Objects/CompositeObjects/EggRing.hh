#include "Egg.hh"
#include "Ring.hh"

class EggRing {
public:

  Egg ovoid;
  Ring circle;

  EggRing() {}

  EggRing(float x, float y, float t, float sp) {
    ovoid = Egg(x, y, t, sp);
    circle.start(x, y - sp/2);
  }

  void transmit() {
    ovoid.wobble();
    ovoid.display();
    circle.grow();
    circle.display();
    if (circle.on == false) {
      circle.on = true;
    }
  }
};
