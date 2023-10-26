#ifndef PROCESSING_DEBUG_H
#define PROCESSING_DEBUG_H

#include <fmt/core.h>
#include <type_traits>
const char *const KNRM="\x1B[0m";
const char *const KRED="\x1B[1;31m";
const char *const KGRN="\x1B[1;32m";
const char *const KYEL="\x1B[1;33m";
const char *const KBLU="\x1B[34m";
const char *const KMAG="\x1B[35m";
const char *const KCYN="\x1B[36m";
const char *const KWHT="\x1B[37m";

template <typename T>
class Printer {
   T *ptr_;
   const char *name_;
   std::string x;

   std::string diff(std::string_view a, std::string_view b, std::string_view same,std::string_view diff ) {
      std::string z;
      for (int i = 0; i< b.length(); i++) {
         if (a[i] == b[i]) {
            z += b[i];
         } else {
            z+= fmt::format("{}{}{}", diff, b[i], same);
         }
      }
      return z+ KNRM;
   }

public:
   Printer(T *ptr, const char *name) : ptr_(ptr), name_(name) {
      x = fmt::format("{}",*ptr_);
      fmt::print(stderr, "{}{} Enter {:60}: {}{}\n", KGRN, (void*)ptr_, name_, x, KNRM);
   }
   ~Printer() {
      fmt::print(stderr, "{}{} Leave {:60}: {}{}\n", KRED, (void*)ptr_, name_, diff(x,fmt::format("{}",*ptr_), KRED, KCYN), KNRM);
   }
};

class FunctionPrinter {
   const char *name_;
public:
   FunctionPrinter(const char *name) : name_(name) {
      fmt::print(stderr, "{}Enter {}{}\n", KGRN, name_, KNRM);
   }
   ~FunctionPrinter() {
      fmt::print(stderr, "{}Leave {}{}\n", KRED, name_, KNRM);
   }
};

#define DEBUG_METHOD()  Printer<std::remove_reference_t<decltype(*this)>> unlikely_debug_token_name(this, __PRETTY_FUNCTION__)
#define DEBUG_METHOD_MESSAGE(x) fmt::print(stderr, "{}{} Durin {:60}: {}{}\n", KYEL, (void*)this, __PRETTY_FUNCTION__, x, KNRM);
#define DEBUG_FUNCTION()  FunctionPrinter unlikely_debug_token_name(__PRETTY_FUNCTION__)

#endif
