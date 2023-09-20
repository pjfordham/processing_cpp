#ifndef PROCESSING_PFONT_H
#define PROCESSING_PFONT_H

#include <string>
#include <vector>
#include "processing_pshape.h"
#include "processing_pimage.h"

class PFont {

public:
   const char *name;
   int size;

   static std::vector<std::string> list();

   static void init();

   static void close();

   PFont() : name(nullptr), size(0) {}

   // We can safely copy and don't have to delete as the
   // map gets garbage collected at the end of the program
   PFont(const char *name_, int size_);

   PShape render_as_pshape(std::string_view text) const;
   PImage render_as_pimage(std::string_view text) const;

   float textAscent() const;
   float textDescent() const;
   float textWidth(std::string_view text) const;
};


inline PFont createFont(const char *name, int size) {
   return {name, size};
}

#endif
