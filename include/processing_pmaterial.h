#ifndef PROCESSING_PMATERIAL_H
#define PROCESSING_PMATERIAL_H

#include "processing_opengl.h"
#include "processing_pimage.h"
#include <array>
#include <unordered_map>
#include <optional>

struct PMaterial {
   gl::color_t ambientColor;
   gl::color_t diffuseColor;
   gl::color_t specularColor;
   gl::color_t emissiveColor;
   float specularExponent;
   float alpha;
   int illum;
   std::optional<PImage> texture;
};

extern std::unordered_map<std::string, PMaterial> materials;
void loadMaterials(const char *objPath);

#endif
