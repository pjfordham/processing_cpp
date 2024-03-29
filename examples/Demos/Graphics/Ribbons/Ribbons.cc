// Ribbons, by Andres Colubri
// ArcBall class by Ariel, V3ga and Robert Hodgin (flight404)
// This sketch loads 3D atomic coordinates of a protein molecule
// from a file in PDB format (http://www.pdb.org/) and displays
// the structure using a ribbon representation.

std::string pdbFile = "4HHB.pdb"; // PDB file to read
//String pdbFile = "1CBS.pdb";
//String pdbFile = "2POR.pdb";

// Some parameters to control the visual appearance:
float scaleFactor = 10;          // Size factor
int renderMode = 1;             // 0 = lines, 1 = flat ribbons
int ribbonDetail = 4;           // Ribbon detail: from 1 (lowest) to 4 (highest)
float helixDiam = 10;           // Helix diameter.
std::array<int,3> ribbonWidth = {10, 7, 2}; // Ribbon widths for helix, strand and coil
color ribbonColor = color(0, 102, 153, 255); // Ribbon color

// All the molecular models read from the PDB file (it could contain more than one)
std::vector<PShape> models;

#include "ArcBall.h"
#include "PDB.h"

Arcball arcball;

void setup() {
  size(1024, 768, P3D);

  arcball = Arcball(width/2, height/2, 600);
  readPDB(pdbFile);
}

void draw() {
  background(0);

  // ambient(80);
  lights();

  translate(width/2, height/2, 200);
  arcball.run();

  for (int i = 0; i < models.size(); i++) {
    shape(models.at(i));
  }
}

void mousePressed(){
  arcball.mousePressed();
}

void mouseDragged(){
  arcball.mouseDragged();
}
