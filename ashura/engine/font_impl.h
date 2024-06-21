#pragma once

#include "ashura/engine/font.h"

extern "C"
{
#include "freetype/freetype.h"
#include "freetype/ftsystem.h"
#include "hb-ft.h"
#include "hb.h"
}

namespace ash
{

struct FontImpl
{
  AllocatorImpl allocator            = default_allocator;
  char         *postscript_name      = {};        // ASCII. i.e. RobotoBold
  usize         postscript_name_size = 0;         // ASCII. i.e. RobotoBold
  char         *family_name          = {};        // ASCII. i.e. Roboto
  usize         family_name_size     = 0;         // ASCII. i.e. Roboto
  char         *style_name           = {};        // ASCII. i.e. Bold
  usize         style_name_size      = 0;         // ASCII. i.e. Bold
  hb_blob_t    *hb_blob              = nullptr;
  hb_face_t    *hb_face              = nullptr;
  hb_font_t    *hb_font              = nullptr;
  FT_Library    ft_lib               = nullptr;
  FT_Face       ft_face              = nullptr;
  u32           face                 = 0;
  char         *font_data            = nullptr;
  u32           font_data_size       = 0;
  u32           num_glyphs           = 0;
  u32           replacement_glyph    = 0;
  u32           ellipsis_glyph       = 0;
  u32           space_glyph          = 0;
  Glyph        *glyphs               = nullptr;
  FontMetrics   metrics              = {};
};

}        // namespace ash