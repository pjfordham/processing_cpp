PShape group, pshape, pshape2;

void mousePressed() {
   pshape.setFill(RED);
   pshape2.setTint(BLUE);
}

void setup() {
   size(600, 600);

   pshape = createShape();
   pshape.fill( GREEN );
   pshape.beginShape( POLYGON );
   pshape.vertex(400,400);
   pshape.vertex(500,400);
   pshape.vertex(500,500);
   pshape.vertex(400,500);
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
