#include <iostream>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <utility>
#include <optional>

// Initialize FreeType library
FT_Library ft;
// Load font
FT_Face face;

std::vector<std::vector<std::vector<std::pair<PVector, int>>>> glyphs;

void setup() {
   if (FT_Init_FreeType(&ft))
   {
      std::cerr << "Failed to initialize FreeType" << std::endl;
      abort();
   }

   if (FT_New_Face(ft, "./data/SourceCodePro-Regular.ttf", 0, &face))
   {
      std::cerr << "Failed to load font" << std::endl;
      abort();
   }

   // Set font size
   FT_Set_Pixel_Sizes(face, 0, 48);

   // Load characters as vector graphics
   for (int i = 32; i < 128; i++)
   {
      std::vector<std::vector<std::pair<PVector, int>>> region;
      if (FT_Load_Char(face, i, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE))
      {
         std::cerr << "Failed to load glyph for character: " << char(i) << std::endl;
         abort();
      }

      // Access outline data
      FT_Outline outline = face->glyph->outline;
      FT_Vector* points = outline.points;
      char* tags = outline.tags;
      int numPoints = outline.n_points;

      int x = 0;
      for (int i = 0 ; i < outline.n_contours ; i ++) {
         std::vector<std::pair<PVector,int>> vpoints;
         for (int j = x; j <= outline.contours[i] ; j++) {
            // Check if it's an 'on' point (start or end of a contour)
            vpoints.push_back( { { (float)points[x].x, 800 - (float)points[x].y}, (int)tags[x]} );
            x++;
         }
         region.push_back( vpoints );
      }
      glyphs.push_back(region);
   }

   // Clean up
   FT_Done_Face(face);
   FT_Done_FreeType(ft);
   size(1000, 1000);
   frameRate(1);
}

int index = 0;
void draw() {
   background(BLACK);
   stroke(WHITE);

   text(fmt::format("{}", index),0,0);
   auto regions = glyphs[index];

   {
      std::vector<PVector> p_control;
      std::vector<PVector> p_anchor;
      std::vector<PShape> holes;
      bool drawn = false;;
      for (auto &vpoints  : regions ) {

         std::optional<PVector> prev_control;
         PShape glyph;
         glyph.fill(WHITE);
         glyph.noStroke();
         glyph.beginShape();
         for (auto &[pos, tag] : vpoints ) {
            bool anchor = tag & 1;
            if (anchor && !prev_control) {
               p_anchor.push_back(pos);
               glyph.vertex(pos);
            } else if (anchor && prev_control) {
               p_anchor.push_back(pos);
               glyph.bezierVertex(prev_control.value(), prev_control.value(), pos);
               prev_control.reset();
            } else if (!anchor && !prev_control) {
               p_control.push_back(pos);
               prev_control = pos;
            } else { // if (!anchor && prev_control)
               PVector anchor = ( pos + prev_control.value() ) / 2;
               p_anchor.push_back( anchor );
               glyph.bezierVertex( prev_control.value(), prev_control.value(), anchor);
               prev_control = pos;
               p_control.push_back( pos );
            }
         }
         if (prev_control) {
            glyph.bezierVertex(prev_control.value(), prev_control.value(), vpoints[0].first );
         }
         glyph.endShape(CLOSE);

         if (!glyph.isClockwise() && !drawn) {
            holes.push_back(glyph);
         } else if (glyph.isClockwise() && !drawn) {
            drawn = true;
            shape(glyph);
            for (auto hole : holes ) {
               hole.setFill(BLACK);
               shape(hole);
            }
            holes.clear();
         } else if (!glyph.isClockwise() && drawn) {
            glyph.setFill(BLACK);
            shape(glyph);
         } else if (glyph.isClockwise() && drawn) {
            shape(glyph);
         } else {
            abort();
         }
      }
      for (auto hole : holes ) {
         hole.setFill(BLACK);
         shape(hole);
      }
      noStroke();
      fill(BLUE);
      for (auto point :  p_anchor ) {
         ellipse(point.x, point.y, 5, 5);
      }
      fill(RED);
      for (auto point :  p_control ) {
         ellipse(point.x, point.y, 5, 5);
      }
   }
   index = (index + 1) % glyphs.size();
}
