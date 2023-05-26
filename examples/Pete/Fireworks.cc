#include <algorithm> // for remove_if

class Particle {
public:
  PVector pos;
  PVector vel;
  PVector acc;
  Particle(PVector pos_, PVector vel_) : pos(pos_), vel(vel_), acc(0,0) {
  }
  void update() {
    pos = pos + vel;
    vel = vel + acc;
    acc = PVector{0,0};
  }

  void push(PVector push) {
    acc = acc + push;
  }

  PVector getPos() const {
    return pos;
  }
  bool offScreen() const {
    return pos.y > height;
  }

  bool fallingDown() const {
    return vel.y > 0;
  }
};

class Firework : public Particle {
  bool exploded = false;
public:
  Firework(PVector pos, PVector vel) : Particle(pos, vel) {
  }

  bool shouldExplode() {
    if ( fallingDown() && !exploded ) {
      exploded = true;
      return true;
    }
    return false;
  }

  void draw() const {
    if (!exploded) {
      stroke(WHITE);
      strokeWeight(2);
      point(pos.x, pos.y);
    }

  }
};

class Point : public Particle {
  color cfill;

public:
  Point(PVector pos, PVector vel, color fill_) : Particle(pos, vel), cfill(fill_) {
  }

  void draw() const {
    stroke(cfill);
    strokeWeight(1);
    point(pos.x, pos.y);
  }
};

std::vector<Firework> fireworks;
std::vector<Point> points;

void setup() {
  size(640, 480);
  fireworks.emplace_back( PVector(50+random(width-100), height), PVector(0, -27 + random(5)) );
}

void draw() {
  background(0);
  fill(255);
  strokeWeight(2);
  for( auto &&firework : fireworks) {
    firework.draw();
    firework.push( PVector(0,1) );
    firework.update();
    if ( firework.shouldExplode() ) {
      color c= random( { RED, GREEN, BLUE, YELLOW } );
      for (int i = 0; i < 15 ; i++ ) {
        points.emplace_back( firework.getPos(), PVector::fromAngle(random(TWO_PI)) * random(9,10), c );
      }
      fireworks.emplace_back( PVector(50+random(width-100), height), PVector(0, -27 + random(5)) );
    }
  }
  for( auto &&point : points) {
    point.draw();
    point.push( PVector(0,1) );
    point.update();
  }

  fireworks.erase(std::remove_if(fireworks.begin(), fireworks.end(), [](const Firework &i) {
    return i.offScreen();
  }), fireworks.end());
  points.erase(std::remove_if(points.begin(), points.end(), [](const Point &i) {
    return i.offScreen();
  }), points.end());
}
