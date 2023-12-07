// LINKLIB: box2d

#include <stdio.h>
#include <box2d/box2d.h>

b2Vec2 gravity(0.0f, -10.0f);
b2World world(gravity);

float timeStep = 1.0f / 60.0f;
int32 velocityIterations = 8;
int32 positionIterations = 3;

class Body {
   enum { RECT, CIRCLE };
   int type;
   PVector size;
   bool dynamic;
   color c;
   float angle;

   Body(int t, PVector c, PVector s, bool d, color x) :
      type(t), center(c), size(s), dynamic(d), c(x) {}

public:
   PVector center;
   b2Body *body;

   static Body make_dynamic_circle(PVector c, float size, color x) {
      Body a( CIRCLE, c, {size,size}, true, x);

      b2BodyDef bodyDef;
      bodyDef.type = b2_dynamicBody;
      bodyDef.position.Set(a.center.x, a.center.y);
      a.body = world.CreateBody(&bodyDef);

      b2CircleShape circle;
      circle.m_p.Set(0, 0);
      circle.m_radius = size / 2.0f;

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &circle;
      fixtureDef.density = 1.0f;
      fixtureDef.friction = 0.3f;

      a.body->CreateFixture(&fixtureDef);
      return a;
   }

   static Body make_dynamic_rectangle(PVector c, PVector s, color x ) {
      Body a( RECT, c, s, true, x);

      b2BodyDef bodyDef;
      bodyDef.type = b2_dynamicBody;
      bodyDef.position.Set(a.center.x, a.center.y);
      a.body = world.CreateBody(&bodyDef);

      b2PolygonShape dynamicBox;
      dynamicBox.SetAsBox(a.size.x/2.0f, a.size.y/2.0f);

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &dynamicBox;
      fixtureDef.density = 1.0f;
      fixtureDef.friction = 0.3f;

      a.body->CreateFixture(&fixtureDef);
      return a;
   }

   static Body make_static_rectangle(PVector c, PVector s, color x ) {
      Body a( RECT, c, s, false, x);

      b2BodyDef groundBodyDef;
      groundBodyDef.position.Set(a.center.x, a.center.y);

      a.body = world.CreateBody(&groundBodyDef);

      b2PolygonShape groundBox;
      groundBox.SetAsBox(a.size.x/2.0f, a.size.y/2.0f);

      a.body->CreateFixture(&groundBox, 0.0f);
      return a;
   }

   void update() {
      angle = body->GetAngle();
      b2Vec2 position = body->GetPosition();
      center.x = position.x;
      center.y = position.y;
   }

   void draw() const {
      noStroke();
      fill(c);
      rectMode(CENTER);
      pushMatrix();
      translate(center.x,center.y);
      rotate(angle);
      translate(-center.x,-center.y);
      if ( type == CIRCLE )
         ellipse(center.x,center.y,size.x,size.y);
      else
         rect(center.x,center.y,size.x,size.y);
      popMatrix();
      noFill();
      stroke(WHITE);
      strokeWeight(0.1);
      auto z = center + PVector::fromAngle(angle) * size.x;
      line(center.x,center.y, z.x,z.y);
   }
};

class Joint {
   b2Joint* distanceJoint;
   Body& b1;
   Body& b2;

 public:
   Joint(Body &body1, Body &body2) : b1(body1), b2(body2) {
      b2DistanceJointDef jointDef;
      jointDef.Initialize(b1.body, b2.body,  b1.body->GetWorldCenter(), b2.body->GetWorldCenter() );
      jointDef.collideConnected = false;
      b2Joint* distanceJoint = world.CreateJoint(&jointDef);
   }

   void draw() const {
      stroke(YELLOW);
      line(b1.center,b2.center);
   }

};

std::vector<Body> bodies;
std::vector<Joint> joints;

void setup() {
   size(640,480);
   bodies.push_back(Body::make_static_rectangle(  {0,-10}, {100,20}, WHITE));
   bodies.push_back(Body::make_dynamic_circle(    {-5,60}, 10,       RED));
   bodies.push_back(Body::make_dynamic_rectangle( {0,40},  {10,10},  GREEN));
   bodies.push_back(Body::make_dynamic_rectangle( {5,20},  {10,10},  BLUE));
   joints.push_back(Joint(bodies[1], bodies[2]));
   joints.push_back(Joint(bodies[2], bodies[3]));
}

void draw() {
   background(BLACK);
   translate(width/2,height/2);
   scale(5,-5);
   for (auto &b : bodies) {
      b.update();
      b.draw();
   }
   for (auto &j : joints) {
      j.draw();
   }
   world.Step(timeStep, velocityIterations, positionIterations);
}

void mousePressed() {
   for (auto &b : bodies) {
      b.body->ApplyLinearImpulseToCenter(b2Vec2{random(500)-250,1000}, true);
      b.body->ApplyTorque(random(50000), true);
   }
}
