/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/errors.h"
#include "ashura/engine/font.h"
#include "ashura/engine/font_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "freetype/freetype.h"
#include "hb.h"
}

namespace ash
{

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
  Allocator                    allocator_;
  RWLock                       rw_lock_;
  SparseVec<FontId, Dyn<Font>> fonts_;
  FileSys                      file_sys_;
  ImageSys                     image_sys_;
  Scheduler                    scheduler_;

  explicit FontSysImpl(Allocator allocator, FileSys file_sys,
                       ImageSys image_sys, Scheduler scheduler) :
    allocator_{allocator},
    rw_lock_{},
    fonts_{allocator},
    file_sys_{file_sys},
    image_sys_{image_sys},
    scheduler_{scheduler}
  {
  }

  FontSysImpl(FontSysImpl const &)             = delete;
  FontSysImpl(FontSysImpl &&)                  = delete;
  FontSysImpl & operator=(FontSysImpl const &) = delete;
  FontSysImpl & operator=(FontSysImpl &&)      = delete;
  ~FontSysImpl();

  virtual void init() override;

  virtual void shutdown() override;

  Result<Dyn<Font>, SysErr> decode_(Str label, Span<u8 const> encoded,
                                          u32 face);

  /// @brief Rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  Result<CpuFontAtlas, SysErr> rasterize(Font font, u32 font_height);

  Future<Result<GpuFontAtlas, SysErr>> upload_atlas_to_gpu_(Str                label,
                                            Rc<CpuFontAtlas const *> cpu_atlas);

  virtual Dyn<TextLayoutBuffer>
    create_layout_buffer(Allocator allocator) override;

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout, TextLayoutBuffer buffer,
                           Allocator scratch) override;

  virtual Future<Result<FontId, SysErr>>
    load_from_memory(Str label, RcBlob8 encoded, u32 font_height,
                     u32 face) override;

  virtual Future<Result<FontId, SysErr>>
    load_from_path(Str label, Str path, u32 font_height, u32 face) override;

  virtual FontInfo get(FontId id) override;

  virtual Option<FontInfo> get(Str label) override;

  void unload_(FontId id);

  virtual void unload(FontId id) override;
};

}    // namespace ash
