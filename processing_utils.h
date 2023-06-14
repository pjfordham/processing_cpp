#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <fmt/core.h>

inline void link(const char *link) {
   (void)!system(fmt::format("xdg-open {} >nul 2>nul",link).c_str());
}

using namespace std::literals;
template <typename T>
std::string nf(T num, int digits) {
   return fmt::format("{0:0{1}}", num, digits);
}

template <typename T>
std::vector<T> subset(const std::vector<T> &c, int start, int length) {
   return { c.begin() + start, c.begin() + start + length };
}

const char TAB = '\t';
inline std::vector<std::string> split(const std::string& str, char delimiter) {
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

template<typename T> void printArray(const std::vector<T> &data) {
  int i = 0;
  for ( auto &&item : data ) {
     fmt::print("[{}] {}\n",i++,item);
  }
}


inline std::vector<std::string> loadStrings(const std::string& fileName) {
   std::vector<std::string> lines;
   std::ifstream inputFile(fileName);

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

// This macro pulls the API a subobject into current scope.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

#endif
