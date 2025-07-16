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
      float x = 0, y = 0, z = 0;

      if( lineType == "newmtl" ) {
         std::string name;
         lineSS >> name;
         currentName = name;
      } else if( lineType == "illum" ) {
         int light;
         lineSS >> light;
         materials[currentName].illum = light;
      } else if( lineType == "Ka" ) {
         lineSS >> x >> y >> z;
         materials[currentName].ambientColor = {x,y,z,1};
      } else if( lineType == "Kd" ) {
         lineSS >> x >> y >> z;
         materials[currentName].diffuseColor = {x,y,z,1};
      } else if( lineType == "Ks" ) {
         lineSS >> x >> y >> z;
         materials[currentName].specularColor = {x,y,z,1};
      } else if( lineType == "#" || lineType == ""  ) {
      } else if( lineType == "Ns" ) {
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
