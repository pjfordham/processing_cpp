#ifndef PROCESSING_STYLE_H
#define PROCESSING_STYLE_H

#include <optional>
#include "processing_color.h"
#include "processing_pimage.h"

struct style_t;

struct flat_style_t {
   bool style_enabled;
   bool fill_enabled;
   color fill_color;

   bool draw_normals;
   bool stroke_enabled;
   color stroke_color;
   float stroke_weight;
   int stroke_end_cap;

   bool ambient_enabled;
   color ambient_color;

   bool texture_enabled;
   PImage texture_img;
   color texture_tint;
   int texture_mode;

   color specular;
   color emissive;
   float shininess;

   bool operator<(const flat_style_t& other) const;
   bool operator==(const flat_style_t &other) const = default;

   flat_style_t();
   flat_style_t(const style_t& s);
};

struct style_t {
   std::optional<bool> style_enabled;
   std::optional<bool> fill_enabled;
   std::optional<color> fill_color;

   std::optional<bool> draw_normals;
   std::optional<bool> stroke_enabled;
   std::optional<color> stroke_color;
   std::optional<float> stroke_weight;
   std::optional<int> stroke_end_cap;

   std::optional<bool> ambient_enabled;
   std::optional<color> ambient_color;

   std::optional<bool> texture_enabled;
   std::optional<PImage> texture_img;
   std::optional<color> texture_tint;
   std::optional<int> texture_mode;

   std::optional<color> specular;
   std::optional<color> emissive;
   std::optional<float> shininess;

   flat_style_t resolve_style(const flat_style_t &parent_style) const;
};

#endif


