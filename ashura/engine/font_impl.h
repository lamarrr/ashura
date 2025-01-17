/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/std/types.h"

extern "C"
{
#include "freetype/freetype.h"
#include "hb.h"
}

namespace ash
{

struct FontImpl : Font
{
  static constexpr u32 MAX_NAME_SIZE = 256;

  using Name = InplaceVec<char, MAX_NAME_SIZE>;

  FontId id = FontId::Invalid;

  Vec<char> label;

  Vec<char> font_data;

  /// @brief Postscript name, name of the font face, ASCII. i.e. RobotoBold
  Name postscript_name;

  /// @brief Font family name, ASCII. i.e. Roboto
  Name family_name;

  /// @brief Font family style name, ASCII. i.e. Bold
  Name style_name;

  hb_blob_t * hb_blob;

  hb_face_t * hb_face;

  hb_font_t * hb_font;

  FT_Library ft_lib;

  FT_Face ft_face;

  u32 face;

  Vec<GlyphMetrics> glyphs;

  u32 replacement_glyph;

  u32 ellipsis_glyph;

  u32 space_glyph;

  FontMetrics metrics;

  Option<CpuFontAtlas> cpu_atlas = none;

  Option<GpuFontAtlas> gpu_atlas = none;

  FontImpl(FontId id, Vec<char> label, Vec<char> font_data,
           Name postscript_name, Name family_name, Name style_name,
           hb_blob_t * hb_blob, hb_face_t * hb_face, hb_font_t * hb_font,
           FT_Library ft_lib, FT_Face ft_face, u32 face,
           Vec<GlyphMetrics> glyphs, u32 replacement_glyph, u32 ellipsis_glyph,
           u32 space_glyph, FontMetrics metrics) :
    id{id},
    label{std::move(label)},
    font_data{std::move(font_data)},
    postscript_name{std::move(postscript_name)},
    family_name{std::move(family_name)},
    style_name{std::move(style_name)},
    hb_blob{hb_blob},
    hb_face{hb_face},
    hb_font{hb_font},
    ft_lib{ft_lib},
    ft_face{ft_face},
    face{face},
    glyphs{std::move(glyphs)},
    replacement_glyph{replacement_glyph},
    ellipsis_glyph{ellipsis_glyph},
    space_glyph{space_glyph},
    metrics{metrics}
  {
  }

  FontImpl(FontImpl const &) = delete;

  FontImpl(FontImpl &&) = delete;

  FontImpl & operator=(FontImpl const &) = delete;

  FontImpl & operator=(FontImpl &&) = delete;

  virtual ~FontImpl() override
  {
    gpu_atlas.unwrap_none("GPU font atlas has not been unloaded"_str);
    hb_font_destroy(hb_font);
    hb_face_destroy(hb_face);
    hb_blob_destroy(hb_blob);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
  }

  virtual FontInfo info() override
  {
    FontInfo info{.id                = id,
                  .label             = label,
                  .postscript_name   = postscript_name,
                  .family_name       = family_name,
                  .style_name        = style_name,
                  .glyphs            = glyphs,
                  .replacement_glyph = replacement_glyph,
                  .space_glyph       = space_glyph,
                  .ellipsis_glyph    = ellipsis_glyph,
                  .metrics           = metrics};

    if (cpu_atlas.is_some())
    {
      info.cpu_atlas = cpu_atlas.value();
    }

    if (gpu_atlas.is_some())
    {
      info.gpu_atlas = gpu_atlas.value();
    }

    return info;
  }
};

}    // namespace ash
