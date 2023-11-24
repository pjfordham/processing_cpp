#include "processing_pfont.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "processing_math.h"
#include "processing_pimage.h"

#include <filesystem>
#include <map>
#include <optional>
#include <fmt/core.h>

static FT_Library ft;

static PShape buildPShapeFromFace(FT_Face face, char x);

bool operator<(const PFont&a, const PFont &b) {
   if (a.name != b.name) {
      return a.name < b.name;
   } else {
      return a.size < b.size;
   }
}

struct Font {
   FT_Face face;
   std::map<char,PShape> glyphs;
   std::map<char,float> m_advance;

   PShape &glyph( char x ) {
      if (glyphs.find(x) == glyphs.end()) {
         glyphs[x] = buildPShapeFromFace( face, x );
         m_advance[x] = face->glyph->advance.x;
      }
      return glyphs[x];
   }

   int em_size() {
      return face->units_per_EM;
   }

   float advance(char x) const {
      return m_advance.at(x);
   }

   PVector getKerning(char prev, char next) const {
      FT_Vector kerning;
      FT_Get_Kerning( face,
                      FT_Get_Char_Index(face,prev),
                      FT_Get_Char_Index(face,next),
                      FT_KERNING_DEFAULT, &kerning);
      return { (float)kerning.x, (float)kerning.y };
   }

};


static std::map<std::string, std::string> fontFileMap;
static std::map<PFont, Font> fontMap;

static void bezierVertexQuadratic(PVector control, PVector anchor2, std::vector<PVector> &out) {
   float anchor1_x = out.back().x;
   float anchor1_y = out.back().y;
   for (float t = 0.01; t <= 1; t += 0.01) {
      // Compute the Bezier curve points
      float x = bezierPointQuadratic( anchor1_x, control.x, anchor2.x, t );
      float y = bezierPointQuadratic( anchor1_y, control.y, anchor2.y, t );
      out.emplace_back( x, y );
   }
}

static PShape buildPShapeFromFace(FT_Face face, char c) {
   // Load and unpack FT glyph outline data
   if (FT_Load_Char(face, c, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE))
   {
      fmt::print("Failed to load glyph for character: {}\n",c);
      abort();
   }
   FT_Outline outline = face->glyph->outline;
   FT_Vector* points = outline.points;
   char* tags = outline.tags;
   int numPoints = outline.n_points;

   // Create our PShape for this glyph
   PShape glyph_shape;
   glyph_shape.beginShape(POLYGON);
   int vertex_index = 0;
   for (int i = 0 ; i < outline.n_contours ; i ++) {
      std::optional<PVector> prev_control;
      std::vector<PVector> contour;

      for (; vertex_index <= outline.contours[i] ; ++vertex_index) {
         // Check if it's an 'on' point (start or end of a contour)
         PVector pos{(float)points[vertex_index].x, -(float)points[vertex_index].y};
         bool anchor = tags[vertex_index] & 1;
         if (anchor && !prev_control) {
            contour.push_back(pos);
         } else if (anchor && prev_control) {
            bezierVertexQuadratic( prev_control.value(), pos, contour);
            prev_control.reset();
         } else if (!anchor && !prev_control) {
            prev_control = pos;
         } else { // if (!anchor && prev_control)
            PVector anchor = ( pos + prev_control.value() ) / 2;
            bezierVertexQuadratic( prev_control.value(), anchor, contour);
            prev_control = pos;
         }
      }
      if (prev_control) {
         bezierVertexQuadratic( prev_control.value(), contour[0], contour );
      }

      glyph_shape.beginContour();
      for (auto &v : contour ) {
         glyph_shape.vertex( v );
      }
      glyph_shape.endContour();
   }
   glyph_shape.endShape();
   return glyph_shape;
}


void PFont::init() {
   FT_Error err = FT_Init_FreeType(&ft);
   if (err != 0) {
      fmt::print("Failed to initialize FreeType\n");
      abort();
   }
   FT_Int major, minor, patch;
   FT_Library_Version(ft, &major, &minor, &patch);
}


void PFont::close() {
   for (auto font : fontMap) {
      FT_Done_Face(font.second.face);
   }
   FT_Done_FreeType(ft);
}

static void search_directory(const std::string& path, const std::string& suffix) {
   for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
      if (!entry.is_directory() && entry.path().extension() == suffix) {
         fontFileMap[entry.path().filename().string()] = entry.path().string();
      }
   }
}

std::vector<std::string>  PFont::list() {
   std::filesystem::path path_to_search = "/usr/share/fonts/truetype";
   std::string suffix_to_find = ".ttf";
   search_directory(path_to_search, suffix_to_find);
   path_to_search = "data";
   search_directory(path_to_search, suffix_to_find);
   std::vector<std::string> keys;
   for (auto const& [key, value] : fontFileMap) {
      keys.push_back(key);
   }
   if (keys.size() == 0) {
      abort();
   };
   return keys;
}


PFont::PFont(const char *name_, int size_) : name(name_), size(size_) {
   if (fontFileMap.size() == 0) {
      PFont::list();
   }
   auto fontPath = fontFileMap[name].c_str();
   if (fontMap.count(*this) == 0) {
      FT_Face face;
      if (FT_New_Face(ft, fontPath, 0, &face) != 0) {
         fmt::print("Failed to load face\n");
         fmt::print("FT_New_Face failed: {},{}\n", name, size);
         abort();
      }
      if (FT_Set_Pixel_Sizes(face,0, size)) {
         fmt::print("Failed to set size\n");
         abort();
      }
      fontMap[*this] = { face };
   }

   return;
}

PShape PFont::render_as_pshape(std::string_view text) const {
   auto font = fontMap[*this];
   PShape group;
   group.beginShape(GROUP);
   // TODO: This translate draws the font to the right and underneath
   // the point requested like other draw commands.
   group.translate( 0, size, 0);
   // Font is font.em_size() virtual units tall and
   // we want it to be size units tall.
   float scale_factor = (0.0 + size) / font.em_size();
   group.scale( scale_factor );
   float x = 0;
   float y = 0;
   for ( auto c : text ) {
      if (c == '\n') {
         x = 0;
         y += font.face->size->metrics.height/64.0 / ( scale_factor );
      } else {
         auto shape = font.glyph(c);
         shape.translate( x, y, 0 );
         group.addChild( shape );
         x += font.advance(c);
      }
   }
   group.endShape();
   return group;
}

float PFont::textAscent() const {
   auto font = fontMap[*this];
   auto face = font.face;
   return face->size->metrics.ascender / 64.0;
}

float PFont::textDescent() const {
   auto font = fontMap[*this];
   auto face = font.face;
   return face->size->metrics.descender / 64.0;
}

float PFont::textWidth(std::string_view text) const {
  auto image = render_as_pimage(text);
  return image.width;
}

PImage PFont::render_as_pimage(std::string_view text) const {
   auto font = fontMap[*this];
   auto face = font.face;

   // Get the width and height of the bitmap
   int width = 0;
   int height = 0;
   {
      int x = 0; // Current X position
      int y = 0; // Current Y position

      for (size_t i = 0; text[i] != '\0'; i++) {
         if (text[i] == '\n') {
            x = 0;
            y += face->size->metrics.height;
         } else {
            FT_Load_Char(face, text[i], FT_LOAD_RENDER);
            auto baseline_offset = (face->size->metrics.ascender);

            int oX = (x/64) + face->glyph->bitmap_left;
            int oY = (baseline_offset/64) + (y/64) - face->glyph->bitmap_top;

            int destY = oY + face->glyph->bitmap.rows;
            height = std::max( destY, height );
            int destX = oX + face->glyph->bitmap.width;
            width = std::max( destX, width );

            x += face->glyph->advance.x;
         }
      }
   }

   PImage image = createImage(width, height, 0);

   // Makesure texture is clear
   std::fill( image._pixels(), image._pixels() + (width * height), 0);

   int x = 0; // Current X position
   int y = 0; // Current Y position

   for (size_t i = 0; text[i] != '\0'; i++) {
      if (text[i] == '\n') {
         x = 0;
         y += face->size->metrics.height;
      } else {
         FT_Load_Char(face, text[i], FT_LOAD_RENDER);

         auto baseline_offset = (face->size->metrics.ascender);

         int oX = (x/64) + face->glyph->bitmap_left;
         int oY = (baseline_offset/64) + (y/64) - face->glyph->bitmap_top;

         // Render the text into the PImage
         for (int row = 0; row < face->glyph->bitmap.rows; row++) {
            int destY = oY + row;
            for (int col = 0; col < face->glyph->bitmap.width; col++) {
               int destX = oX + col;
               int srcIndex = col + row * face->glyph->bitmap.pitch;
               int destIndex = destX + destY * width;
               if (destX < width && destY < height) {
                  image.pixels[destIndex] = color( 255,255,255, face->glyph->bitmap.buffer[srcIndex]);
               }
            }
         }

         // Advance to the next position
         x += face->glyph->advance.x;
      }
   }
   return image;
}
