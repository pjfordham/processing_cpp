#ifndef PROCESSING_PMATERIAL_H
#define PROCESSING_PMATERIAL_H

#include "processing_pimage.h"
#include <array>
#include <unordered_map>

struct PMaterial {
   std::array<float, 3> ambientColor;
   std::array<float, 3> diffuseColor;
   std::array<float, 3> specularColor;
   float specularExponent;
   float alpha;
   int illum;
   PImage texture;
};

extern std::unordered_map<std::string, PMaterial> materials;
void loadMaterials(const char *objPath);

#endif
