#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <fmt/core.h>

// This macro pulls the API a subobject into current scope.
#define MAKE_GLOBAL(method, instance) template<typename... Args> auto method(Args&&... args) { return instance.method(args...); }

using namespace std::literals;

const char TAB = '\t';

void link(std::string_view link);
std::vector<std::string> split(std::string_view str, char delimiter);
std::vector<std::string> loadStrings(std::string_view fileName);

template <typename T>
std::string nf(T num, int digits) {
   return fmt::format("{0:0{1}}", num, digits);
}

template <typename T>
std::vector<T> subset(const std::vector<T> &c, int start, int length) {
   return { c.begin() + start, c.begin() + start + length };
}

template<typename T>
void printArray(const std::vector<T> &data) {
   int i = 0;
   for ( auto &&item : data ) {
      fmt::print("[{}] {}\n",i++,item);
   }
}

#endif
