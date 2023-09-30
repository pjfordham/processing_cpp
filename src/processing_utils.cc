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

std::vector<std::string> splitTokens(std::string_view input, std::string_view delimiters) {
   std::vector<std::string> tokens;
   size_t startPos = 0;
   size_t foundPos;

   while ((foundPos = input.find_first_of(delimiters, startPos)) != std::string::npos) {
      // Extract the token from startPos to foundPos
      std::string_view token = input.substr(startPos, foundPos - startPos);

      // Add the token to the vector
      tokens.emplace_back(token);

      // Update the start position to the character after the found delimiter
      startPos = foundPos + 1;
   }

   // Add the last token (or the whole string if no delimiter is found)
   std::string_view token = input.substr(startPos);
   tokens.emplace_back(token);

   return tokens;
}

std::string join(const std::vector<std::string>& strings, std::string_view separator) {
   std::string result;

   for (size_t i = 0; i < strings.size(); ++i) {
      result += strings[i];

      // Add the separator if not the last element
      if (i < strings.size() - 1) {
         result += separator;
      }
   }

   return result;
}

std::string toLowerCase(std::string_view input) {

   std::string output(input);
   for (char &c : output) {
      c = std::tolower(c);
   }

   return output;
}
