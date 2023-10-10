#include "Geometry.h"

extern std::vector<PShape> models;
extern float scaleFactor;

inline void readPDB(std::string filename) {
  std::vector<std::string> strLines;

  float xmin, xmax, ymin, ymax, zmin, zmax;

  std::string xstr, ystr, zstr;
  float x, y, z;
  int res, res0;
  int nmdl;
  std::string atstr, resstr;

  PShape model;
  std::vector<PVector> atoms;
  std::vector<HashMap> residues;
  HashMap residue;
  PVector v;
  std::string s;
  strLines = loadStrings(filename);

  models.clear();

  xmin = ymin = zmin = 10000;
  xmax = ymax = zmax = -10000;

  res0 = -1;
  nmdl = -1;
  for (int i = 0; i < strLines.size(); i++) {
    s = strLines[i];

    if (startsWith(s, "MODEL") || (startsWith(s,"ATOM") && res0 == -1)) {
      nmdl++;

      res0 = -1;

      atoms.clear();
      residues.clear();
    }

    if (startsWith(s,"ATOM")) {
      atstr = substr(s, 12, 15);
      atstr = trim(atstr);
      resstr = substr(s, 22, 26);
      resstr = trim(resstr);
      res = std::stoi(resstr);

      xstr = substr(s, 30, 37);
      xstr = trim(xstr);
      ystr = substr(s, 38, 45);
      ystr = trim(ystr);
      zstr = substr(s, 46, 53);
      zstr = trim(zstr);

      x = scaleFactor * std::stof(xstr);
      y = scaleFactor * std::stof(ystr);
      z = scaleFactor * std::stof(zstr);
      v = PVector(x, y, z);

      xmin = min(xmin, x);
      xmax = max(xmax, x);

      ymin = min(ymin, y);
      ymax = max(ymax, y);

      zmin = min(zmin, z);
      zmax = max(zmax, z);

      atoms.push_back(v);

      if (res0 != res) {
        if (!residue.empty()) residues.push_back(residue);
        residue.clear();
      }
      residue[atstr] = v;

      res0 = res;
    }

    if (startsWith(s, "ENDMDL") || startsWith(s, "TER")) {
      if (!residue.empty()) residues.push_back(residue);

      createRibbonModel(residues, model, models);
      float rgyr = calculateGyrRadius(atoms);

      res0 = -1;
      residue.clear();
      atoms.clear();
      residues.clear();
    }
  }

  if (!residue.empty()) {
     if (!residue.empty()) residues.push_back(residue);

    createRibbonModel(residues, model, models);
    float rgyr = calculateGyrRadius(atoms);

    atoms.clear();
    residues.clear();
  }

  // Centering models at (0, 0, 0).
  float dx = -0.5f * (xmin + xmax);
  float dy = -0.5f * (ymin + ymax);
  float dz = -0.5f * (zmin + zmax);
  for (int n = 0; n < models.size(); n++) {
    model = models.at(n);
    model.translate(dx, dy, dz);
  }

  fmt::print("Loaded PDB file with {} models.\n", models.size());
}
