
// Custom Cube Class

class Cube {
public:
  // Position, velocity vectors
  PVector position;
  PVector velocity;
  // Also using PVector to hold rotation values for 3 axes
  PVector rotation;

  // Vertices of the cube
  std::vector<PVector> vertices;
  // width, height, depth
  float w, h, d;

  // colors for faces of cube
  std::vector<color> quadBG;

  Cube(float w, float h, float d) {
    this->w = w;
    this->h = h;
    this->d = d;

    quadBG.resize(6);
    // Colors are hardcoded
    quadBG[0] = color(0.0f);
    quadBG[1] = color(51.0f);
    quadBG[2] = color(102.0f);
    quadBG[3] = color(153.0f);
    quadBG[4] = color(204.0f);
    quadBG[5] = color(255.0f);

    // Start in center
    // Random velocity vector
    velocity = PVector::random3D();
    // Random rotation
    rotation = PVector(random(40, 100), random(40, 100), random(40, 100));

    vertices.resize(24);
    // cube composed of 6 quads
    //front
    vertices[0] = PVector(-w/2, -h/2, d/2);
    vertices[1] = PVector(w/2, -h/2, d/2);
    vertices[2] = PVector(w/2, h/2, d/2);
    vertices[3] = PVector(-w/2, h/2, d/2);
    //left
    vertices[4] = PVector(-w/2, -h/2, d/2);
    vertices[5] = PVector(-w/2, -h/2, -d/2);
    vertices[6] = PVector(-w/2, h/2, -d/2);
    vertices[7] = PVector(-w/2, h/2, d/2);
    //right
    vertices[8] = PVector(w/2, -h/2, d/2);
    vertices[9] = PVector(w/2, -h/2, -d/2);
    vertices[10] = PVector(w/2, h/2, -d/2);
    vertices[11] = PVector(w/2, h/2, d/2);
    //back
    vertices[12] = PVector(-w/2, -h/2, -d/2);
    vertices[13] = PVector(w/2, -h/2, -d/2);
    vertices[14] = PVector(w/2, h/2, -d/2);
    vertices[15] = PVector(-w/2, h/2, -d/2);
    //top
    vertices[16] = PVector(-w/2, -h/2, d/2);
    vertices[17] = PVector(-w/2, -h/2, -d/2);
    vertices[18] = PVector(w/2, -h/2, -d/2);
    vertices[19] = PVector(w/2, -h/2, d/2);
    //bottom
    vertices[20] = PVector(-w/2, h/2, d/2);
    vertices[21] = PVector(-w/2, h/2, -d/2);
    vertices[22] = PVector(w/2, h/2, -d/2);
    vertices[23] = PVector(w/2, h/2, d/2);
  }

  // Cube shape itself
  void drawCube() {
    // Draw cube
    for (int i=0; i<6; i++) {
      fill(quadBG[i]);
      beginShape(QUADS);
      for (int j=0; j<4; j++) {
        vertex(vertices[j+4*i].x, vertices[j+4*i].y, vertices[j+4*i].z);
      }
      endShape();
    }
  }

  // Update location
  void update( float bounds ) {
    position.add(velocity);

    // Check wall collisions
    if (position.x > bounds/2 || position.x < -bounds/2) {
      velocity.x*=-1;
    }
    if (position.y > bounds/2 || position.y < -bounds/2) {
      velocity.y*=-1;
    }
    if (position.z > bounds/2 || position.z < -bounds/2) {
      velocity.z*=-1;
    }
  }


  // Display method
  void display() {
    pushMatrix();
    translate(position.x, position.y, position.z);
    rotateX(frameCount*PI/rotation.x);
    rotateY(frameCount*PI/rotation.y);
    rotateZ(frameCount*PI/rotation.z);
    noStroke();
    drawCube(); // Farm out shape to another method
    popMatrix();
  }
};

