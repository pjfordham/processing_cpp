#ifndef PROCESSING_PFONT_H
#define PROCESSING_PFONT_H

#include <string>
#include <vector>
#include "processing_pshape.h"
#include "processing_pimage.h"
#include "processing_utils.h"

class PFontImpl;

class PFont {
   friend PFontImpl;
   std::shared_ptr<PFontImpl> impl;
public:
   static std::vector<std::string> list();

   static void init();

   static void close();

   PFont();

   ~PFont();

   PFont(const char *name_, int size_);

   const char *getName() const;

   PShape render_as_pshape(std::string_view text) const;
   PImage render_as_pimage(std::string_view text);

   float textAscent() const;
   float textDescent() const;
   float textWidth(std::string_view text);
};

extern PFont currentFont;

inline PFont createFont(const char *name, int size) {
   return {name, size};
}

void textFont(PFont font);
void textSize(int size);

MAKE_GLOBAL(textAscent, currentFont);
MAKE_GLOBAL(textDescent, currentFont);
MAKE_GLOBAL(textWidth, currentFont);

#endif
