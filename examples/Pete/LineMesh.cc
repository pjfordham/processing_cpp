float r = 0;
void setup() { size(640, 480); }
#include <glm/gtc/matrix_transform.hpp>

void line3D(PShape &mesh, PVector p1, PVector p2, float weight1, float weight2, color color1, color color2 ) {

   glm::vec3 end = p2;
   glm::vec3 start = p1;
   int segments = 32;
   PVector direction = end - start;
   float length = direction.mag();
   glm::vec3 axis = direction.normalize();
   glm::vec3 normal = direction.normal();

   unsigned short j = mesh.getCurrentIndex();
   // Generate vertices around the circumference
   for (int i = 0; i <= segments; ++i) {
      float angle = (glm::two_pi<float>() / static_cast<float>(segments)) * i;

      // Rotate circlePoint to align with the line
      glm::mat3 rotation = glm::rotate(glm::mat4(1), angle, axis);

      mesh.fill(color1);
      mesh.vertex(start + rotation * normal * weight1 / 2.0f);
      mesh.fill(color2);
      mesh.vertex(end + rotation * normal * weight2 / 2.0f);
   }
   bool reverse = false;
   for (int i = 0; i < (segments * 2); i++ ){
       if (reverse) {
          mesh.index( j + i + 2 );
          mesh.index( j + i + 1 );
          mesh.index( j + i );
       } else {
          mesh.index( j + i );
          mesh.index( j + i + 1 );
          mesh.index( j + i + 2 );
       }
       reverse = !reverse;
    }
   for(int i = 1; i < segments; i++) {
      mesh.index( j + 0 );
      mesh.index( j + i * 2 );
      mesh.index( j + i * 2 + 2 );
   }
   for(int i = 1; i < segments; i++) {
      mesh.index( j + 1 );
      mesh.index( j + i * 2 + 1 );
      mesh.index( j + i * 2 + 3 );
   }
}

PShape drawLineMesh(PVector p1, PVector p2, float weight1, float weight2, color color1, color color2, const PMatrix &transform ) {
   PShape mesh;
   mesh.beginShape(TRIANGLES);
   mesh.transform( transform );
   mesh.noStroke();

   line3D(mesh, p1, p2, weight1, weight2, color1, color2);

   mesh.endShape();
   // Return the generated vertices and indices
   return mesh;
}

void draw() {
   translate(width/2.0, height/2.0);
   background(BLACK);
   PVector a{-width/4.0f,-height/4.0f};
   PVector b{width/4.0f,height/4.0f};
   noFill();
   stroke(RED);
   line(a,b);
   glm::vec3 axis = (b - a).normalize();
   rotate(r += 0.01,axis);
   rotateY(r += 0.01);
   PShape mesh = drawLineMesh( a, b, 100, 50, BLUE, WHITE, PMatrix::Identity() );
   shape(mesh );
}
