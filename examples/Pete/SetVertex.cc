PShape group, pshape, pshape2;
PImage img;

void mousePressed() {
   pshape.setTexture( img );
   pshape.setStroke(RED);
   pshape2.setFill(false);
}

void setup() {
   size(600, 600);

   textureMode( NORMAL );
   img = loadImage("berlin-1.jpg");

   pshape = createShape();
   pshape.noFill();
   pshape.beginShape( POLYGON );
   pshape.vertex(400,400,0,0,0);
   pshape.vertex(500,400,0,1,0);
   pshape.vertex(500,500,0,1,1);
   pshape.vertex(400,500,0,0,1);
   pshape.endShape( CLOSE );

   pshape2 = createShape();
   pshape2.fill( YELLOW );
   pshape2.beginShape( POLYGON );
   pshape2.vertex(100,100);
   pshape2.vertex(200,100);
   pshape2.vertex(200,200);
   pshape2.vertex(100,200);
   pshape2.endShape( CLOSE );

   group = createGroup();
   group.addChild(pshape);
   group.addChild(pshape2);
}

void draw() {
   background( WHITE );
   pshape.setVertex( 0, {(float)mouseX, (float)mouseY} );
   pshape2.setVertex( 2, {(float)mouseX, (float)mouseY} );
   shape(group);
}
