#pragma once

#include "ashura/engine/font.h"

extern "C"
{
#include "freetype/freetype.h"
#include "freetype/ftsystem.h"
#include "harfbuzz/hb-ft.h"
#include "harfbuzz/hb.h"
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
  u32           selected_face        = 0;
  char         *font_data            = nullptr;
  u32           font_data_size       = 0;
  u32           num_glyphs           = 0;
  u32           replacement_glyph    = 0;
  u32           ellipsis_glyph       = 0;
  u32           space_glyph          = 0;

  // RASTERIZED ATLAS INFO
  u32             font_height  = 0;
  Glyph          *glyphs       = nullptr;
  u8             *atlas        = nullptr;
  u64             atlas_size   = 0;
  u32             atlas_extent = 0;
  u32             num_layers   = 0;
  f32             ascent       = 0;
  f32             descent      = 0;
  f32             advance      = 0;
  gfx::Format     format       = gfx::Format::Undefined;
  gfx::Image      image        = nullptr;
  gfx::ImageView *views        = nullptr;
};

bool load_font_from_memory(FontImpl *f, Span<u8 const> encoded_data, u32 face);

bool load_font_from_file(FontImpl *f, AllocatorImpl const &allocator,
                         Span<char const> path, u32 face);

void destroy_font(FontImpl *f, gfx::DeviceImpl const &d);

bool render_font_atlas(FontImpl &f, i32 font_height,
                       Span<UnicodeRange const> ranges,
                       AllocatorImpl const     &allocator);

gfx::Status upload_font_to_device(FontImpl &f, RenderContext &c,
                                  AllocatorImpl const &allocator);

}        // namespace ash