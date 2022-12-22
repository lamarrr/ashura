#pragma once

#include "freetype/freetype.h"
#include "hb.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

struct Text {
  std::string str;
  // std::string language;
  // hb_script_t script;
  hb_direction_t direction;
};

int main() {
  uint32_t font_size = 50;
  FT_Library ftlib;
  FT_Init_FreeType(&ftlib);

  auto fontpath =
      "C:\\Users\\Basit\\OneDrive\\Documents\\workspace\\oss\\ashura-"
      "assets\\fonts\\default.ttf";

  FT_Face ftface;
  FT_New_Face(ftlib, fontpath, 0, &ftface);
  FT_Set_Char_Size(ftface, 0, font_size * 64, 72, 72);

  float x = 0, y = 0;
  hb_blob_t *blob = hb_blob_create_from_file_or_fail(fontpath);
  hb_face_t *face = hb_face_create(blob, 0);
  hb_font_t *font = hb_font_create(face);

  /// != ! = == = = -> - >
  // ->
  // uint8_t str[] = {0xe6, 0x9c, 0xac, 0xe6, 0x97, 0xa5, 0}; // japanese
  // uint8_t str[] = {0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0}; // chinese
  // uint8_t str[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x87, 0} //
  // arabic
  uint8_t str[] = {0xd8, 0xb5, 0xd8, 0xa8, 0xd8, 0xa7, 0xd8, 0xad, 0x20, 0xd8,
                   0xa7, 0xd9, 0x84, 0xd8, 0xae, 0xd9, 0x8a, 0xd8, 0xb1, 0};
  // 3 codepoint represents space
  Text text{(char *)str, HB_DIRECTION_RTL};
  hb_buffer_t *buffer = hb_buffer_create();
  // CHECK(hb_buffer_allocation_successful(buffer));
  int xscale, yscale;
  hb_font_get_scale(font, &xscale, &yscale);

  hb_font_set_scale(font, 64 * font_size, 64 * font_size);

  std::cout << "xscale: " << xscale << " yscale: " << yscale << std::endl;

  hb_buffer_set_direction(buffer, text.direction);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", -1));

  hb_buffer_add_utf8(buffer, text.str.c_str(), text.str.size(), 0,
                     text.str.size());
  const hb_tag_t KERN_TAG = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
  const hb_tag_t LIGA_TAG =
      HB_TAG('l', 'i', 'g', 'a'); // standard ligature substitution
  const hb_tag_t CLIG_TAG =
      HB_TAG('c', 'l', 'i', 'g'); // contextual ligature substitution

  static hb_feature_t LIGATURE_OFF = {LIGA_TAG, 0, 0,
                                      std::numeric_limits<unsigned int>::max()};
  static hb_feature_t LIGATURE_ON = {LIGA_TAG, 1, 0,
                                     std::numeric_limits<unsigned int>::max()};
  static hb_feature_t KERNING_OFF = {KERN_TAG, 0, 0,
                                     std::numeric_limits<unsigned int>::max()};
  static hb_feature_t KERNING_ON = {KERN_TAG, 1, 0,
                                    std::numeric_limits<unsigned int>::max()};
  static hb_feature_t CLIG_OFF = {CLIG_TAG, 0, 0,
                                  std::numeric_limits<unsigned int>::max()};
  static hb_feature_t CLIG_ON = {CLIG_TAG, 1, 0,
                                 std::numeric_limits<unsigned int>::max()};

  hb_feature_t features[] = {LIGATURE_ON, CLIG_ON, KERNING_ON};
  hb_shape(font, buffer, features, std::size(features));

  unsigned int glyph_count;
  hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t *glyph_pos =
      hb_buffer_get_glyph_positions(buffer, &glyph_count);
  std::cout << "glyph count:" << glyph_count
            << " char size: " << text.str.size() << std::endl;

  for (int i = 0; i < glyph_count; ++i) {
    uint32_t codepoint = glyph_info[i].codepoint;
    // Glyph *glyph = lib->rasterize(face, glyphInfo[i].codepoint);
    // x_offset, y_offset ascent and descent from intended rendering position
    // stored 26.6 pixel format which divides by 64
    std::cout << "codepoint: " << codepoint
              << " x=" << glyph_pos[i].x_offset / 64.0f
              << ", y=" << glyph_pos[i].y_offset / 64.0f
              << ", xadvance: " << glyph_pos[i].x_advance / 64.0f
              << ", yadvance: " << glyph_pos[i].y_advance / 64.0f << std::endl;

    FT_Load_Glyph(ftface, codepoint, FT_LOAD_RENDER);
    FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);
    FT_Bitmap bitmap = ftface->glyph->bitmap;
    auto buff = bitmap.buffer;
    auto h = bitmap.rows;
    auto w = bitmap.width;

    std::cout
        << " width: " << ftface->glyph->metrics.width / 64.0f
        << " height: " << ftface->glyph->metrics.height / 64.0f
        << " horiBearingX: " << ftface->glyph->metrics.horiBearingX / 64.0f
        << " horiBearingY: " << ftface->glyph->metrics.horiBearingY / 64.0f
        << " vertBearingX: " << ftface->glyph->metrics.vertBearingX / 64.0f
        << " vertBearingY: " << ftface->glyph->metrics.vertBearingY / 64.0f
        << std::endl;
    std::cout << "w: " << w << " h: " << h << std::endl;
    std::cout << "xmin: " << ftface->bbox.xMin / 64.0f
              << " ymin: " << ftface->bbox.yMin / 64.0f
              << " xmax: " << ftface->bbox.xMax / 64.0f
              << " ymax: " << ftface->bbox.yMax / 64.0f << std::endl;

    for (size_t i = 0; i < h; i++) {
      for (size_t j = 0; j < w; j++) {
        int c = buff[i * w + j];
        char ch = ' ';
        if (c > 0 && c < 127)
          ch = '*';
        if (c > 127)
          ch = '#';
        std::cout << ch;
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    /* int twidth = pow(2, ceil(log(glyph->width) / log(2)));
     int theight = pow(2, ceil(log(glyph->height) / log(2)));

     auto tdata = new unsigned char[twidth * theight]();

     for (int iy = 0; iy < glyph->height; ++iy) {
       memcpy(tdata + iy * twidth, glyph->buffer + iy * glyph->width,
              glyph->width);
     }

     float s0 = 0.0;
     float t0 = 0.0;
     float s1 = (float)glyph->width / twidth;
     float t1 = (float)glyph->height / theight;
     float xa = (float)glyphPos[i].x_advance / 64;
     float ya = (float)glyphPos[i].y_advance / 64;
     float xo = (float)glyphPos[i].x_offset / 64;
     float yo = (float)glyphPos[i].y_offset / 64;
     float x0 = x + xo + glyph->bearing_x;
     float y0 = floor(y + yo + glyph->bearing_y);
     float x1 = x0 + glyph->width;
     float y1 = floor(y0 - glyph->height);
     */
  }

  // hb_font_destroy();
  // hb_buffer_destroy();
}
