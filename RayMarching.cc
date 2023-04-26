/**
 * 2D Ray Marching
 * by Dan Shiffman.
 *
 * This example was based The Coding Train ray marching live stream
 */


bool offScreen(PVector v) {
   return v.x < 0 || v.x > width || v.y < 0 || v.y > height;
}

float signedDistance(PVector a, PVector b, float r) {
   return  PVector::dist(a, b) - r;
}

class Boundary {
public:
   PVector pos;
   PVector vel;
   float r;
   Boundary() {};
   Boundary(float x, float y, float radius_)
      : pos(x, y), vel(PVector::random2D()), r(radius_) {}

   void update() {
      pos.add(vel);
      if (pos.x < r || pos.x > width - r) {
         vel.x *= -1;
      }
      if (pos.y < r || pos.y > height - r) {
         vel.y *= -1;
      }
   }

   void highlight() {
      strokeWeight(4);
      fill(0, 100, 255, 100);
      stroke(255);
      ellipse(pos.x, pos.y, r * 2);
   }

   void show() {
      noFill();
      strokeWeight(1);
      stroke(255);
      ellipse(pos.x, pos.y, r * 2);
   }
};

class Ray {
public:
   PVector pos;
   float angle;
   Ray() {}
   Ray(float x, float y, float angle_ ) : pos(x,y), angle(angle_) {}
 
   void rotate(float offset) {
      angle += offset;
   }

   void march(std::vector<Boundary> &stuff) {
      PVector current = pos;
      Boundary *closest = nullptr;
      while (true) {
         float record = 200000.0; // not Infinity
         closest = nullptr;
         for (auto &circle : stuff) {
            float d = signedDistance(current, circle.pos, circle.r);
            if (d < record) {

               record = d;

               closest = &circle;

            }

         }
         if (record < 1) {
            // glow.push(current);
            break;
         }

         auto v = PVector::fromAngle(angle);
         v.setMag(record);
         strokeWeight(1);

         pushMatrix();

         stroke(255, 0, 200);

         noFill();

         translate(current.x, current.y);

         ellipse(0, 0, record * 2);

         popMatrix();

         current.add(v);

         if (offScreen(current)) {

            closest = nullptr;

            break;

         }

      }

      if (closest) {

         closest->highlight();

      }

      stroke(0, 0, 255);

      strokeWeight(4);

      line(pos.x, pos.y, current.x, current.y);

      fill(0, 255, 0);

      ellipse(pos.x, pos.y, 16);

      ellipse(current.x, current.y, 16);

      //show(record);

   }

   // show(radius) {

   //  push();

   //  stroke(255, 0, 200);

   //  noFill();

   //  translate(pos.x, pos.y);

   //  const v = p5.Vector.fromAngle(angle);

   //  v.setMag(radius);

   //  pop();

   // }

};

std::vector<Boundary> stuff;
std::vector<PVector> glow;

Ray ray;

void setup() {
   size(640, 480);

   for (int i = 0; i < 7 ; i++) {

      int r = random(10, 50);

      int x = random(r, width - r);

      int y = random(r, height - r);

      stuff.push_back(Boundary(x, y, r));

  }

 ray = Ray(width / 2, height / 2, 0);

}

void draw() {
   background(0);
   for (auto &s : stuff) {
      s.show();
      s.update();
   }

   ray.march(stuff);
   ray.rotate(0.01);


   for (auto &v : glow) {
      fill(255, 200);
      stroke(255);
      ellipse(v.x, v.y, 4);
   }
}
