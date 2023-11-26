#include "processing_pmaterial.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

std::unordered_map<std::string, PMaterial> materials;

void loadMaterials(const char *objPath) {
   using namespace std::literals;

   std::ifstream inputFile("data/"s + objPath);

   if (!inputFile.is_open()) {
      abort();
   }

   std::string currentName;
   std::string lineStr;
   while( std::getline( inputFile, lineStr ) ) {
      std::istringstream lineSS( lineStr );
      std::string lineType;
      lineSS >> lineType;

      if( lineType == "newmtl" ) {
         std::string name;
         lineSS >> name;
         currentName = name;
      } else if( lineType == "illum" ) {
         int light;
         lineSS >> light;
         materials[currentName].illum = light;
      } else if( lineType == "Ka" ) {
         float x = 0, y = 0, z = 0;
         lineSS >> x >> y >> z;
         materials[currentName].ambientColor = {x,y,z};
      } else if( lineType == "Kd" ) {
         float x = 0, y = 0, z = 0;
         lineSS >> x >> y >> z;
         materials[currentName].diffuseColor = {x,y,z};
      } else if( lineType == "Ks" ) {
         float x = 0, y = 0, z = 0;
         lineSS >> x >> y >> z;
         materials[currentName].specularColor = {x,y,z};
      } else if( lineType == "#" || lineType == ""  ) {
      } else if( lineType == "Ns" ) {
         float x = 0;
         lineSS >> x;
         materials[currentName].specularExponent = x;
      } else if( lineType == "map_Kd" ) {
         std::string name;
         lineSS >> name;
         materials[currentName].texture = loadImage( name );
      } else {
         fmt::print("Unrecognized material file line: {}\n", lineStr);
      }
   }
}
