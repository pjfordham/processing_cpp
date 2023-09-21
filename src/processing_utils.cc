#include "processing_utils.h"
#include <fstream>

void link(std::string_view link) {
   (void)!system(fmt::format("xdg-open {} >nul 2>nul",link).c_str());
}

std::vector<std::string> loadStrings(std::string_view fileName) {
   std::vector<std::string> lines;
   std::ifstream inputFile("data/"s.append(fileName));

   if (!inputFile.is_open()) {
      abort();
   }

   std::string line;
   while (std::getline(inputFile, line)) {
      lines.push_back(line);
   }

   inputFile.close();
   return lines;
}

std::vector<std::string> split(std::string_view str, char delimiter) {
   std::vector<std::string> result;
   std::string token;

   for (const char& c : str) {
      if (c == delimiter) {
         result.push_back(token);
         token.clear();
      } else {
         token += c;
      }
   }

   // Push back the last token, if any
   if (!token.empty()) {
      result.push_back(token);
   }

   return result;
}
