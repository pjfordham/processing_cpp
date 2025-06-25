
class Vertlet {
public:
   PVector position_current;
   PVector position_old;
   PVector acceleration;
   float radius;
   color fill_color;

   Vertlet() {}
   
   Vertlet(PVector position, float radius_, color color_) {
      position_current = position_old = position;
      radius = radius_;
      fill_color = color_;
      acceleration = {0,0};
   }

   void update_position( float dt ) {
      auto velocity = position_current - position_old;
      position_old = position_current;
      position_current = position_current + velocity + acceleration * dt * dt;
      acceleration = { 0, 0};
   }

   void accelerate( PVector a ) {
      acceleration = acceleration + a;
   }

   void draw() {
      noStroke();
      fill( fill_color );
      ellipse( position_current.x, position_current.y, 2*radius, 2*radius );
   }

   static void draw_constraint() {
      PVector cpos(500,500);
      float cradius = 500;
      noStroke();
      fill(BLUE);
      ellipse(500, 500, 2*500, 2*500);
   }

   void apply_constraint() {
      PVector cpos(500,500);
      float cradius = 500;
      PVector to_obj = position_current - cpos;
      float dist = to_obj.mag();
      if ( dist > (cradius - radius) ) {
         PVector n = to_obj / dist;
         position_current = cpos + n * (cradius - radius);
      }
   }

   void solve_collisions( std::vector<Vertlet> &balls, int skip ) {
      for( int i = skip+1 ; i < balls.size(); i++ ) {
         auto collider = balls[i];
         PVector collision_axis = position_current - collider.position_current;
         float dist = collision_axis.mag();
         float overlap = ( radius + collider.radius) - dist;
         if ( overlap > 0 ) {
            PVector n = collision_axis / dist;
            position_current = position_current + n * (collider.radius/radius) * overlap;
            collider.position_current = collider.position_current - n * (1-collider.radius/radius) * overlap;
         }
      }
   }
};

std::vector<Vertlet> balls;

void setup() {
   size(1000, 1000);
}

void draw() {
   background(BLACK);
   noStroke();
   fill(WHITE);

   if ( (frameCount % 30) == 0 && balls.size() < 100) {
      auto size = 10 + random(20);
      auto color = RANDOM_COLOR();
      PVector pos{700,300};
      balls.emplace_back( pos, size, color);
   }

   Vertlet::draw_constraint();

   int skip = 0;
   for (auto &ball : balls) {
      // for (int i = 0; i< 1; i++) {
         ball.accelerate({0,1000});
         ball.update_position(1.0/60);
         ball.apply_constraint();
         ball.solve_collisions( balls, skip );
         // }
      skip++ ;
   }

   for (auto &ball : balls) {
      ball.draw();
   }
}
