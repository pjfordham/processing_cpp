class Bullet {
public:
   PVector pos;
   PVector vel;
   color bcolor;
   void draw() const {
      noStroke();
      fill(bcolor);
      ellipse(pos.x,pos.y, 10,10);
   }
   void update() {
      pos += vel;
   }
   bool offScreen() const {
      return (pos.x < 0 || pos.x > width || pos.y < 0 || pos.y > height);
   }
};

std::vector<Bullet> bullets;

class Tank {
public:
   PShape geometry;
   color tcolor;

   PVector pos;
   float vel;
   float rotation;
   float angularVel;
   float spin;

   Tank(color _color, PVector _pos, float _rot):
      tcolor(_color), pos(_pos), rotation(_rot), angularVel(0.0f), spin(0.0f) {

      geometry = createShape();
      geometry.beginShape();
      // Construct tank geometry
      geometry.noStroke();
      geometry.fill(tcolor);
      geometry.vertex(0,4);
      geometry.vertex(3,4);
      geometry.vertex(3,5);
      geometry.vertex(2,5);
      geometry.vertex(2,7);
      geometry.vertex(8,7);
      geometry.vertex(8,5);
      geometry.vertex(6,5);
      geometry.vertex(6,2);
      geometry.vertex(8,2);
      geometry.vertex(8,0);
      geometry.vertex(2,0);
      geometry.vertex(2,2);
      geometry.vertex(3,2);
      geometry.vertex(3,3);
      geometry.vertex(0,3);
      geometry.endShape(CLOSE);
      geometry.scale(10);
      geometry.rotate(PI);
      geometry.translate(-4.5, -3.5);
   }

   enum keys {
      LEFT,
      RIGHT,
      FORWARD,
      FIRE
   };

   void keyPressed( keys key ) {
      switch (key) {
      case LEFT:
         angularVel = -.05;
         break;
      case RIGHT:
         angularVel = 0.05;
         break;
      case FORWARD:
         vel = 2;
         break;
      case FIRE:
         fire();
         break;
      }
   }

   void keyReleased( keys key ) {
      switch (key) {
      case LEFT:
         angularVel = 0;
         break;
      case RIGHT:
         angularVel = 0;
         break;
      case FORWARD:
         vel = 0;
         break;
      case FIRE:
         break;
      }
   }

   void collide() {
      for (auto &b : bullets ) {
         if ( std::abs(pos.x - b.pos.x) < 45 && std::abs(pos.y - b.pos.y) < 35 &&
              tcolor != b.bcolor) {
            // If we're hit by a bullet, move it offscreen to be deleted
            // and start spinnig, ending at a random orientation.
            b.pos = {-100,-100};
            spin = 3 * TWO_PI + random(TWO_PI);
            rotation += spin;
         }
      }
   }

   void fire() {
      bullets.emplace_back(Bullet{{pos.x,pos.y}, PVector::fromAngle(rotation)*5.0, tcolor});
   }

   void update() {
       // Move in direction pointing with current speed
      pos += PVector::fromAngle(rotation) * vel;

      // Rotate with current angular speed
      rotation += angularVel;
   };

   void draw() {
      // Spinning is only a visual effect for now
      if (spin <= 0) {
         spin = 0.0f;
      } else {
         spin = spin - (TWO_PI/10);
      }

      pushMatrix();
      translate(pos.x, pos.y);
      rotate( rotation + spin );
      shape( geometry );
      popMatrix();
   }
};

std::vector<Tank> tanks;

void setup() {
   size(640, 480);
   tanks.emplace_back(Tank(RED,  {7.0f*width/8.0f,height/2.0f}, PI));
   tanks.emplace_back(Tank(BLUE, {width/8.0f,height/2.0f},       0));
}

void draw() {
   background(BLACK);

   for (auto &t : tanks) {
      t.collide();
      t.draw();
      t.update();
   }

   for (auto &b : bullets ) {
      b.draw();
      b.update();
   }

   // Erase bullets that have gone offscreen
   bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](Bullet val) {
      return val.offScreen();
   }), bullets.end());

}

void keyPressed() {
   switch (keyCode) {
   case LEFT:
      tanks[0].keyPressed(Tank::LEFT);
      break;
   case RIGHT:
      tanks[0].keyPressed(Tank::RIGHT);
      break;
   case UP:
      tanks[0].keyPressed(Tank::FORWARD);
      break;
   case DOWN:
      tanks[0].keyPressed(Tank::FIRE);
      break;
   case 0:
      switch (key) {
      case 'a':
         tanks[1].keyPressed(Tank::LEFT);
         break;
      case 'd':
         tanks[1].keyPressed(Tank::RIGHT);
         break;
      case 'w':
         tanks[1].keyPressed(Tank::FORWARD);
         break;
      case 's':
         tanks[1].keyPressed(Tank::FIRE);
         break;
      };
   };
}

void keyReleased() {
   switch (keyCode) {
   case LEFT:
      tanks[0].keyReleased(Tank::LEFT);
      break;
   case RIGHT:
      tanks[0].keyReleased(Tank::RIGHT);
      break;
   case UP:
      tanks[0].keyReleased(Tank::FORWARD);
      break;
   case DOWN:
      tanks[0].keyReleased(Tank::FIRE);
      break;
   case 0:
      switch (key) {
      case 'a':
         tanks[1].keyReleased(Tank::LEFT);
         break;
      case 'd':
         tanks[1].keyReleased(Tank::RIGHT);
         break;
      case 'w':
         tanks[1].keyReleased(Tank::FORWARD);
         break;
      case 's':
         tanks[1].keyReleased(Tank::FIRE);
         break;
      };
   };
}

