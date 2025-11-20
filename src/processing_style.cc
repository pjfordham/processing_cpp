#include "processing_style.h"

bool flat_style_t::operator<(const flat_style_t& other) const {
   // --- style_enabled ---
   if (style_enabled != other.style_enabled)
      return style_enabled < other.style_enabled;

   // --- fill ---
   if (fill_enabled != other.fill_enabled)
      return fill_enabled < other.fill_enabled;
   if (fill_enabled) {
      if (fill_color != other.fill_color)
         return fill_color < other.fill_color;
   }

   // --- draw_normals ---
   if (draw_normals != other.draw_normals)
      return draw_normals < other.draw_normals;

   // --- stroke ---
   if (stroke_enabled != other.stroke_enabled)
      return stroke_enabled < other.stroke_enabled;
   if (stroke_enabled) {
      if (stroke_color != other.stroke_color)
         return stroke_color < other.stroke_color;
      if (stroke_weight != other.stroke_weight)
         return stroke_weight < other.stroke_weight;
      if (stroke_end_cap != other.stroke_end_cap)
         return stroke_end_cap < other.stroke_end_cap;
   }

   // --- ambient ---
   if (ambient_enabled != other.ambient_enabled)
      return ambient_enabled < other.ambient_enabled;
   if (ambient_enabled) {
      if (ambient_color != other.ambient_color)
         return ambient_color < other.ambient_color;
   }

   // --- texture ---
   if (texture_enabled != other.texture_enabled)
      return texture_enabled < other.texture_enabled;
   if (texture_enabled) {
      if (texture_img != other.texture_img)
         return texture_img < other.texture_img;
      if (texture_tint != other.texture_tint)
         return texture_tint < other.texture_tint;
      if (texture_mode != other.texture_mode)
         return texture_mode < other.texture_mode;
   }

   // --- lighting ---
   if (specular != other.specular)
      return specular < other.specular;

   if (emissive != other.emissive)
      return emissive < other.emissive;

   if (shininess != other.shininess)
      return shininess < other.shininess;

   return false; // equal
}


flat_style_t::flat_style_t() :
   style_enabled(true),
   fill_enabled(true),
   fill_color(WHITE),
   draw_normals(false),
   stroke_enabled(true),
   stroke_color(BLACK),
   stroke_weight(1.0),
   stroke_end_cap(ROUND),
   ambient_enabled(false),
   texture_enabled(false),
   texture_tint(WHITE),
   texture_mode(IMAGE),
   specular(BLACK),
   emissive(BLACK),
   shininess(0)
{}

flat_style_t::flat_style_t(const style_t& s) :
   style_enabled(*s.style_enabled),
   fill_enabled(*s.fill_enabled),
   fill_color(*s.fill_color),
   draw_normals(*s.draw_normals),
   stroke_enabled(*s.stroke_enabled),
   stroke_color(*s.stroke_color),
   stroke_weight(*s.stroke_weight),
   stroke_end_cap(*s.stroke_end_cap),
   ambient_enabled(*s.ambient_enabled),
   ambient_color(*s.ambient_color),
   texture_enabled(*s.texture_enabled),
   texture_img( texture_enabled ? *s.texture_img : PImage()),
   texture_tint(*s.texture_tint),
   texture_mode(*s.texture_mode),
   specular(*s.specular),
   emissive(*s.emissive),
   shininess(*s.shininess)
{}


flat_style_t style_t::resolve_style(const flat_style_t &parent_style) const {
   flat_style_t local_style;
   const style_t &style = *this;
   
#define RESOLVE(x) do { local_style.x = style.x ? style.x.value() : parent_style.x; } while (false)

      RESOLVE(style_enabled);

      if (!local_style.style_enabled) {
         flat_style_t x = parent_style;
         x.style_enabled = false;
         if (style.texture_enabled && style.texture_enabled.value() && style.texture_img == PImage::circle() ) {
            // TODO: We cant regenerate the stroke for this circle becuase
            // it was never genereated in the SVG parsing. We explicitly
            // use a geometry with _NOSTROKE
            x.texture_enabled = true;
            x.texture_img = style.texture_img.value();
            x.texture_tint = parent_style.fill_color;
            x.texture_mode = style.texture_mode.value();
         }
         return x;
      }

      RESOLVE(fill_enabled);
      RESOLVE(fill_color);
      RESOLVE(draw_normals);
      RESOLVE(stroke_enabled);
      RESOLVE(stroke_color);
      RESOLVE(stroke_weight);
      RESOLVE(stroke_end_cap);
      RESOLVE(ambient_enabled);
      if (local_style.ambient_enabled)
         RESOLVE(ambient_color);
      else
         local_style.ambient_color = BLACK;
      RESOLVE(texture_enabled);
      if (local_style.texture_enabled)
         RESOLVE(texture_img);
      RESOLVE(texture_tint);
      RESOLVE(texture_mode);
      RESOLVE(specular);
      RESOLVE(emissive);
      RESOLVE(shininess);
      return local_style;


}
