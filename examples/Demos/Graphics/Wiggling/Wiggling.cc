// Press 'w' to start wiggling, space to restore
// original positions.

PShape cube;
float cubeSize = 320;
float circleRad = 100;
int circleRes = 40;
float noiseMag = 1;

boolean wiggling = false;

void createCube();
void restoreCube();
void createFaceWithHole(PShape face);
void restoreFaceWithHole(PShape face);

void setup() {
  size(1024, 768, P3D);

  createCube();
}

void draw() {
  background(0);

  translate(width/2, height/2);
  rotateX(frameCount * 0.01f);
  rotateY(frameCount * 0.01f);

  shape(cube);

  if (wiggling) {
    PVector pos;
    for (int i = 0; i < cube.getChildCount(); i++) {
      PShape face = cube.getChild(i);
      for (int j = 0; j < face.getVertexCount(); j++) {
        pos = face.getVertex(j, pos);
        pos.x += random(-noiseMag/2, +noiseMag/2);
        pos.y += random(-noiseMag/2, +noiseMag/2);
        pos.z += random(-noiseMag/2, +noiseMag/2);
        face.setVertex(j, pos.x, pos.y, pos.z);
      }
    }
  }

  // if (frameCount % 60 == 0) println(frameRate);
}

void keyPressed() {
   fmt::print("{}\n",key);
   if (key == 'w') {
    wiggling = !wiggling;
  } else if (key == ' ') {
    restoreCube();
  } else if (key == '1') {
    cube.setStrokeWeight(1);
  } else if (key == '2') {
    cube.setStrokeWeight(5);
  } else if (key == '3') {
    cube.setStrokeWeight(10);
  }
}

void createCube() {
  cube = createGroup();

  // Create all faces at front position
  for (int i = 0; i < 6; i++) {
    PShape face = createGroup();
    createFaceWithHole(face);
    cube.addChild(face);
  }

  // Rotate all the faces to their positions

  // Front face - already correct

  // Back face
  cube.getChild(1).rotateY(radians(180));

  // Right face
  cube.getChild(2).rotateY(radians(90));

  // Left face
  cube.getChild(3).rotateY(radians(-90));

  // Top face
  cube.getChild(4).rotateX(radians(90));

  // Bottom face
  cube.getChild(5).rotateX(radians(-90));
}

void createFaceWithHole(PShape face) {
  face.beginShape(POLYGON);
  face.stroke(255, 0, 0);
  face.fill(255);

  // Draw main shape Clockwise
  face.vertex(-cubeSize/2, -cubeSize/2, +cubeSize/2);
  face.vertex(+cubeSize/2, -cubeSize/2, +cubeSize/2);
  face.vertex(+cubeSize/2, +cubeSize/2, +cubeSize/2);
  face.vertex(-cubeSize / 2, +cubeSize / 2, +cubeSize / 2);

  // Draw contour (hole) Counter-Clockwise
  face.beginContour();
  for (int i = 0; i < circleRes; i++) {
    float angle = TWO_PI * i / circleRes;
    float x = circleRad * sin(angle);
    float y = circleRad * cos(angle);
    float z = +cubeSize/2;
    face.vertex(x, y, z);
  }
  face.endContour();

  face.endShape(CLOSE);
}

void restoreCube() {
  // Rotation of faces is preserved, so we just reset them
  // the same way as the "front" face and they will stay
  // rotated correctly
  for (int i = 0; i < 6; i++) {
    PShape face = cube.getChild(i);
    restoreFaceWithHole(face);
  }
}

void restoreFaceWithHole(PShape face) {
  face.setVertex(0, -cubeSize/2, -cubeSize/2, +cubeSize/2);
  face.setVertex(1, +cubeSize/2, -cubeSize/2, +cubeSize/2);
  face.setVertex(2, +cubeSize/2, +cubeSize/2, +cubeSize/2);
  face.setVertex(3, -cubeSize/2, +cubeSize/2, +cubeSize/2);
  for (int i = 0; i < circleRes; i++) {
    float angle = TWO_PI * i / circleRes;
    float x = circleRad * sin(angle);
    float y = circleRad * cos(angle);
    float z = +cubeSize/2;
    face.setVertex(4 + i, x, y, z);
  }
}
