/**
 * Geometry
 * by Marius Watz.
 *
 * Using sin/cos lookup tables, blends colors, and draws a series of
 * rotating arcs on the screen.
*/

// Trig lookup tables borrowed from Toxi; cryptic but effective.
std::vector<float> sinLUT;
std::vector<float> cosLUT;
float SINCOS_PRECISION=1.0;
int SINCOS_LENGTH= int((360.0/SINCOS_PRECISION));

// System data
boolean dosave=false;
int num;
std::vector<float> pt;
std::vector<color> c;
std::vector<int> style;

color colorBlended(float fract, float r, float g, float b, float r2, float g2, float b2, float a);
void arcLine(float x,float y,float deg,float rad,float w);
void arcLineBars(float x,float y,float deg,float rad,float w);
void zarc(float x,float y,float deg,float rad,float w);

void setup() {
  size(1024, 768, P3D);
  background(255);

  // Fill the tables
  for (int i = 0; i < SINCOS_LENGTH; i++) {
    sinLUT.push_back( sinf(i*DEG_TO_RAD*SINCOS_PRECISION) );
    cosLUT.push_back( cosf(i*DEG_TO_RAD*SINCOS_PRECISION) );
  }

  num = 150;
  pt.resize(6*num); // rotx, roty, deg, rad, w, speed
  style.resize(num); // render style
  c.resize(num); // color

  // Set up arc shapes
  int index=0;
  float prob;
  for (int i=0; i<num; i++) {
    pt[index++] = random(PI*2); // Random X axis rotation
    pt[index++] = random(PI*2); // Random Y axis rotation

    pt[index++] = random(60,80); // Short to quarter-circle arcs
    if(random(100)>90) pt[index]=(int)random(8,27)*10;

    pt[index++] = int(random(2,50)*5); // Radius. Space them out nicely

    pt[index++] = random(4,32); // Width of band
    if(random(100)>90) pt[index]=random(40,60); // Width of band

    pt[index++] = radians(random(5,30))/5; // Speed of rotation

    // get colors
    prob = random(100);
    if(prob<30) c[i]=colorBlended(random(1), 255,0,100, 255,0,0, 210);
    else if(prob<70) c[i]=colorBlended(random(1), 0,153,255, 170,225,255, 210);
    else if(prob<90) c[i]=colorBlended(random(1), 200,255,0, 150,255,0, 210);
    else c[i]=color(255,255,255, 220);

    if(prob<50) c[i]=colorBlended(random(1), 200,255,0, 50,120,0, 210);
    else if(prob<90) c[i]=colorBlended(random(1), 255,100,0, 255,255,0, 210);
    else style[i*2]=color(255,255,255, 220);

    style[i]=(int)(random(100))%3;
  }
}

void draw() {

  background(0);

  int index=0;
  translate(width/2, height/2, 0);
  rotateX(PI/6);
  rotateY(PI/6);

  for (int i = 0; i < num; i++) {
    pushMatrix();

    rotateX(pt[index++]);
    rotateY(pt[index++]);

    if(style[i]==0) {
       stroke(c[i]);
      noFill();
      strokeWeight(1);
      arcLine(0,0, pt[index],pt[index+1],pt[index+2]);
      index += 3;
    }
    else if(style[i]==1) {
      fill(c[i]);
      noStroke();
      arcLineBars(0,0, pt[index],pt[index+1],pt[index+2]);
      index += 3;
    }
    else {
      fill(c[i]);
      noStroke();
      zarc(0,0, pt[index],pt[index+1],pt[index+2]);
      index += 3;
    }

    // increase rotation
    pt[index-5]+=pt[index]/10;
    pt[index-4]+=pt[index++]/20;

    popMatrix();
  }
}


// Get blend of two colors
color colorBlended(float fract,
float r, float g, float b,
float r2, float g2, float b2, float a) {
  r2 = (r2 - r);
  g2 = (g2 - g);
  b2 = (b2 - b);
  return color(r + r2 * fract, g + g2 * fract, b + b2 * fract, a);
}


// Draw arc line
void arcLine(float x,float y,float deg,float rad,float w) {
  int a=(int)(min<float>(deg/SINCOS_PRECISION,SINCOS_LENGTH-1));
  int numlines=(int)(w/2);

  for (int j=0; j<numlines; j++) {
    beginShape();
    for (int i=0; i<a; i++) {
      vertex(cosLUT[i]*rad+x,sinLUT[i]*rad+y);
    }
    endShape();
    rad += 2;
  }
}

// Draw arc line with bars
void arcLineBars(float x,float y,float deg,float rad,float w) {
  int a = int((min<float>(deg/SINCOS_PRECISION,SINCOS_LENGTH-1)));
  a /= 4;

  beginShape(QUADS);
  for (int i=0; i<a; i+=4) {
    vertex(cosLUT[i]*(rad)+x,sinLUT[i]*(rad)+y);
    vertex(cosLUT[i]*(rad+w)+x,sinLUT[i]*(rad+w)+y);
    vertex(cosLUT[i+2]*(rad+w)+x,sinLUT[i+2]*(rad+w)+y);
    vertex(cosLUT[i+2]*(rad)+x,sinLUT[i+2]*(rad)+y);
  }
  endShape();
}

// Draw solid arc
void zarc(float x,float y,float deg,float rad,float w) {
  int a = int(min<float>(deg/SINCOS_PRECISION,SINCOS_LENGTH-1));
  beginShape(QUAD_STRIP);
  for (int i = 0; i < a; i++) {
    vertex(cosLUT[i]*(rad)+x,sinLUT[i]*(rad)+y);
    vertex(cosLUT[i]*(rad+w)+x,sinLUT[i]*(rad+w)+y);
  }
  endShape();
}
