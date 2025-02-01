#ifndef PROCESSING_PMATERIAL_H
#define PROCESSING_PMATERIAL_H

#include "processing_opengl.h"
#include "processing_pimage.h"
#include <array>
#include <unordered_map>

struct PMaterial {
   gl::color ambientColor;
   gl::color diffuseColor;
   gl::color specularColor;
   gl::color emissiveColor;
   float specularExponent;
   float alpha;
   int illum;
   PImage texture;
};

extern std::unordered_map<std::string, PMaterial> materials;
void loadMaterials(const char *objPath);

#endif
