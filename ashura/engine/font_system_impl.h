/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/errors.h"
#include "ashura/engine/font.h"
#include "ashura/engine/font_system.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "freetype/freetype.h"
#include "hb.h"
}

namespace ash
{
// [ ] make systems MT-safe?

struct FontImpl final : IFont
{
  static constexpr u32 MAX_NAME_SIZE = 256;

  using Name = InplaceVec<char, MAX_NAME_SIZE>;

  Vec<char> label;

  Vec<char> font_data;

  bool has_color;

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

  FontImpl(Vec<char> label, Vec<char> font_data, bool has_color,
           Name postscript_name, Name family_name, Name style_name,
           hb_blob_t * hb_blob, hb_face_t * hb_face, hb_font_t * hb_font,
           FT_Library ft_lib, FT_Face ft_face, u32 face,
           Vec<GlyphMetrics> glyphs, u32 replacement_glyph, u32 ellipsis_glyph,
           u32 space_glyph, FontMetrics metrics) :
    label{std::move(label)},
    font_data{std::move(font_data)},
    has_color{has_color},
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

  virtual ~FontImpl() override;

  virtual FontInfo info() override;
};

struct FontSysImpl final : IFontSys
{
  Allocator            allocator_;
  SparseVec<Dyn<Font>> fonts_;
  Vec<TextSegment>     segments_;
  hb_buffer_t *        hb_buffer_;

  explicit FontSysImpl(Allocator allocator, hb_buffer_t * hb_buffer) :
    allocator_{allocator},
    fonts_{allocator},
    segments_{allocator},
    hb_buffer_{hb_buffer}
  {
  }

  FontSysImpl(FontSysImpl const &)             = delete;
  FontSysImpl(FontSysImpl &&)                  = delete;
  FontSysImpl & operator=(FontSysImpl const &) = delete;
  FontSysImpl & operator=(FontSysImpl &&)      = delete;
  ~FontSysImpl();

  virtual void shutdown() override;

  Result<Dyn<Font>, FontLoadErr> decode_(Str label, Span<u8 const> encoded,
                                         u32 face = 0);

  virtual Result<> rasterize(Font font, u32 font_height) override;

  FontId upload_(Dyn<Font> font);

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout) override;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_memory(Vec<char> label, Vec<u8> encoded, u32 font_height,
                     u32 face = 0) override;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_path(Vec<char> label, Str path, u32 font_height,
                   u32 face = 0) override;

  virtual FontInfo get(FontId id) override;

  virtual Option<FontInfo> get(Str label) override;

  virtual void unload(FontId id) override;
};

}    // namespace ash
