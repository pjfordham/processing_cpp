#ifndef PROCESSING_PFONT_H
#define PROCESSING_PFONT_H

#include <string>
#include <vector>

struct SDL_Surface;
typedef struct _TTF_Font TTF_Font;

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

   SDL_Surface *render_text(const std::string &text);

};


inline PFont createFont(const char *name, int size)  {
  return {name, size};
}

// #include <freetype2/ft2build.h>
// #include FT_FREETYPE_H

// FT_Library ft;
// FT_Face face;

// class PFont_FreeType {
// public:
//    static void init() {

//       // Initialize FreeType
//       FT_Error err = FT_Init_FreeType(&ft);
//       if (err != 0) {
//          printf("Failed to initialize FreeType\n");
//          exit(EXIT_FAILURE);
//       }
//       FT_Int major, minor, patch;
//       FT_Library_Version(ft, &major, &minor, &patch);
//       // printf("FreeType's version is %d.%d.%d\n", major, minor, patch);

//       // Load the TrueType font file
//       FT_New_Face(ft, "./SourceCodePro-Regular.ttf", 0, &face);
//       if (err != 0) {
//          printf("Failed to load face\n");
//          exit(EXIT_FAILURE);
//       }

//       // Set the character size
//       FT_Set_Char_Size(face, 0, 16*64, 300, 300);
//       if (err != 0) {
//          printf("Failed to set char size\n");
//          exit(EXIT_FAILURE);
//       }

//       // Load the glyph outline data
//       FT_Load_Char(face, 'A', FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
//       if (err != 0) {
//          printf("Failed to load glyph outlie data\n");
//          exit(EXIT_FAILURE);
//       }

//       std::vector<PVector> gvertices;
//       std::vector<char> gtags;

//       // Convert the glyph outline data into vertices
//       for (int i = 0; i < face->glyph->outline.n_points; i++) {
//          FT_Vector vec = face->glyph->outline.points[i];
//          gvertices.emplace_back(PVector{vec.x / 64.0f , vec.y / 64.0f} );
//          gtags.emplace_back(  face->glyph->outline.tags[i] );
//       }

//       // Render the vertices using OpenGL
//       // glEnableClientState(GL_VERTEX_ARRAY);
//       // glVertexPointer(2, GL_FLOAT, 0, &gvertices[0]);
//       // glDrawArrays(GL_TRIANGLES, 0, gvertices.size() / 2);
//       // glDisableClientState(GL_VERTEX_ARRAY);

//       return;
//    }

//    static void close() {
//       // Cleanup
//       FT_Done_Face(face);
//       FT_Done_FreeType(ft);
//       return;
//    };

// };

#endif
