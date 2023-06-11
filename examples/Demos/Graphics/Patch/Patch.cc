// Bezier patch By Maritus Watz:
// http://www.openprocessing.org/sketch/57709
// Normal calculation added by Andres Colubri
// Direct port of sample code by Paul Bourke.
// Original code: http://paulbourke.net/geometry/bezier/

using VecMatrix = std::vector<std::vector<PVector>>;
void resize(VecMatrix &v, int w, int h) {
   v.resize(w);
   for (auto &q : v ) {
      q.resize(h);
   }
}


int ni=4, nj=5, RESI=ni*10, RESJ=nj*10;
VecMatrix outp, inp, normp;
boolean autoNormals = false;

void build();

void setup() {
  size(1024, 768, P3D);
  build();
}

void draw() {
  background(255);
  translate(width/2,height/2);
  lights();
  scale(0.9);
  rotateY(map(mouseX,0,width,-PI,PI));
  rotateX(map(mouseY,0,height,-PI,PI));

  noStroke();
  fill(255);
  for(int i=0; i<RESI-1; i++) {
    beginShape(QUAD_STRIP);
    for(int j=0; j<RESJ; j++) {
      if (!autoNormals) {
        normal(normp[i][j].x, normp[i][j].y, normp[i][j].z);
      }
      vertex(outp[i][j].x,outp[i][j].y,outp[i][j].z);
      vertex(outp[i+1][j].x,outp[i+1][j].y,outp[i+1][j].z);
    }
    endShape();
  }
}

void keyPressed() {
  if(key==' ') build();
  saveFrame("bezPatch.png");
}

double BezierBlend(int k, double mu, int n);
double DBezierBlend(int k, double mu, int n);

void build() {
  int i, j, ki, kj;
  double mui, muj, bi, bj, dbi, dbj;

  resize( outp, RESI, RESJ);
  resize( normp, RESI, RESJ);
  resize( inp, ni+1, nj+1);
  PVector uitang;
  PVector ujtang;

  for (i=0;i<=ni;i++) {
    for (j=0;j<=nj;j++) {
      inp[i][j] = PVector(i,j,random(-3,3));
    }
  }

  for (i=0;i<RESI;i++) {
    mui = i / (double)(RESI-1);
    for (j=0;j<RESJ;j++) {
      muj = j / (double)(RESJ-1);
      uitang.set(0, 0, 0);
      ujtang.set(0, 0, 0);
      for (ki=0;ki<=ni;ki++) {
        bi = BezierBlend(ki, mui, ni);
        dbi = DBezierBlend(ki, mui, ni);
        for (kj=0;kj<=nj;kj++) {
          bj = BezierBlend(kj, muj, nj);
          dbj = DBezierBlend(kj, muj, nj);
          outp[i][j].x += (inp[ki][kj].x * bi * bj);
          outp[i][j].y += (inp[ki][kj].y * bi * bj);
          outp[i][j].z += (inp[ki][kj].z * bi * bj);

          uitang.x += (inp[ki][kj].x * dbi * bj);
          uitang.y += (inp[ki][kj].y * dbi * bj);
          uitang.z += (inp[ki][kj].z * dbi * bj);

          ujtang.x += (inp[ki][kj].x * bi * dbj);
          ujtang.y += (inp[ki][kj].y * bi * dbj);
          ujtang.z += (inp[ki][kj].z * bi * dbj);
        }
      }
      outp[i][j].add(PVector(-ni/2,-nj/2,0));
      outp[i][j].mult(100);

      uitang.normalize();
      ujtang.normalize();
      normp[i][j] = uitang.cross(ujtang);
    }
  }
}

double BezierBlend(int k, double mu, int n) {
  int nn, kn, nkn;
  double blend=1;

  nn = n;
  kn = k;
  nkn = n - k;

  while (nn >= 1) {
    blend *= nn;
    nn--;
    if (kn > 1) {
      blend /= (double)kn;
      kn--;
    }
    if (nkn > 1) {
      blend /= (double)nkn;
      nkn--;
    }
  }
  if (k > 0)
    blend *= pow(mu, (double)k);
  if (n-k > 0)
    blend *= pow(1-mu, (double)(n-k));

  return(blend);
}

double DBezierBlend(int k, double mu, int n) {
  int nn, kn, nkn;
  double dblendf = 1;

  nn = n;
  kn = k;
  nkn = n - k;

  while (nn >= 1) {
    dblendf *= nn;
    nn--;
    if (kn > 1) {
      dblendf /= (double)kn;
      kn--;
    }
    if (nkn > 1) {
      dblendf /= (double)nkn;
      nkn--;
    }
  }

  double fk = 1;
  double dk = 0;
  double fnk = 1;
  double dnk = 0;
  if (k > 0) {
    fk = pow(mu, (double)k);
    dk = k*pow(mu, (double)k-1);
  }
  if (n-k > 0) {
    fnk = pow(1-mu, (double)(n-k));
    dnk = (k-n)*pow(1-mu, (double)(n-k-1));
  }
  dblendf *= (dk * fnk + fk * dnk);

  return(dblendf);
}
