/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/gpu_system.h"
#include "ashura/engine/image_system.h"
#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

// App Unit (AU)
inline constexpr i32 AU_UNIT = 128 * 64;

inline constexpr f32 AU_SCALE = 1 / (f32) AU_UNIT;

static_assert((AU_UNIT % 64) == 0,
              "App Unit needs to be in 26.6 Fractional Unit");

static_assert((AU_UNIT / 64) >= 64,
              "App Unit needs to be at least 64 26.6 Fractional Units");

constexpr f32 au_to_px(i32 au, f32 base)
{
  return au * AU_SCALE * base;
}

constexpr f32x2 au_to_px(i32x2 au, f32 base)
{
  return f32x2{au_to_px(au.x(), base), au_to_px(au.y(), base)};
}

constexpr Str to_str(FontLoadErr err)
{
  switch (err)
  {
    case FontLoadErr::OutOfMemory:
      return "OutOfMemory"_str;
    case FontLoadErr::DecodeFailed:
      return "DecodeFailed"_str;
    case FontLoadErr::FaceNotFound:
      return "FaceNotFound"_str;
    case FontLoadErr::UnsupportedFormat:
      return "UnsupportedFormat"_str;
    case FontLoadErr::InvalidPath:
      return "InvalidPath"_str;
    case FontLoadErr::IoErr:
      return "IoErr"_str;
    default:
      return "Unidentified"_str;
  }
}

inline void format(fmt::Sink sink, fmt::Spec, FontLoadErr const & err)
{
  sink(to_str(err));
}

/// @brief Glyph Metrics. expressed on an AU_UNIT scale
/// @param bearing offset from cursor baseline to start drawing glyph from (au)
/// @param descent distance from baseline to the bottom of the glyph (au)
/// @param advance advancement of the cursor after drawing this glyph (au)
/// @param extent glyph extent
struct GlyphMetrics
{
  i32x2 bearing = {};
  i32   advance = 0;
  i32x2 extent  = {};
};

struct ResolvedFontMetrics
{
  f32 ascent  = 0;
  f32 descent = 0;
  f32 advance = 0;

  constexpr f32 height() const
  {
    return ascent + descent;
  }
};

/// @brief Normalized font metrics
/// @param ascent  maximum ascent of the font's glyphs (au)
/// @param descent maximum descent of the font's glyphs (au)
/// @param advance maximum advance of the font's glyphs (au)
struct FontMetrics
{
  i32 ascent  = 0;
  i32 descent = 0;
  i32 advance = 0;

  constexpr i32 height() const
  {
    return ascent + descent;
  }

  constexpr ResolvedFontMetrics resolve(f32 font_height) const
  {
    return ResolvedFontMetrics{.ascent  = au_to_px(ascent, font_height),
                               .descent = au_to_px(descent, font_height),
                               .advance = au_to_px(advance, font_height)};
  }
};

/// @param later atlas layer this glyph belongs to
/// @param area area in the atlas this glyph's cache data is placed
/// @param uv normalized texture coordinates of this glyph in the layer
struct AtlasGlyph
{
  bool16 has_color = false;
  u16    layer     = 0;
  RectU  area      = {};
  CRect  uv        = {};
};

struct CpuFontAtlas
{
  i32             font_height = 0;
  u32x2           extent      = {};
  u32             num_layers  = 0;
  Vec<AtlasGlyph> glyphs      = {};
  Vec<u8>         channels    = {};

  ImageLayerSpan<u8, 1> span() const
  {
    return ImageLayerSpan<u8, 1>{
      .channels = channels, .extent = extent, .layers = num_layers};
  }
};

struct GpuFontAtlas
{
  Vec<TextureIndex> textures    = {};
  ImageId           image       = ImageId::None;
  i32               font_height = 0;
  u32x2             extent      = {};
  Vec<AtlasGlyph>   glyphs      = {};

  constexpr u32 num_layers() const
  {
    return size32(textures);
  }
};

enum class FontId : u64
{
  None = U64_MAX
};

/// @param postscript_name ASCII. i.e. RobotoBold
/// @param family_name ASCII. i.e. Roboto
/// @param style_name ASCII. i.e. Bold
/// @param replacement_glyph glyph for the replacement glyph 0xFFFD if
/// found, otherwise glyph index 0
/// @param ellipsis_glyph glyph for the ellipsis character '…'
/// @param font_height font height at which the this atlas was rendered (px)
/// @param cpu_atlas cpu font atlas if loaded
/// @param gpu_atlas gpu font atlas if loaded
struct FontInfo
{
  Str                          label             = {};
  bool                         has_color         = false;
  Str                          postscript_name   = {};
  Str                          family_name       = {};
  Str                          style_name        = {};
  Span<GlyphMetrics const>     glyphs            = {};
  u32                          replacement_glyph = 0;
  u32                          space_glyph       = 0;
  u32                          ellipsis_glyph    = 0;
  FontMetrics                  metrics           = {};
  Option<CpuFontAtlas const &> cpu_atlas         = none;
  Option<GpuFontAtlas const &> gpu_atlas         = none;
};

// [ ] font variations?; the user would resolve them; how to represent variations? same metrics or different ones?

typedef struct IFont * Font;

struct IFont
{
  virtual ~IFont()        = default;
  virtual FontInfo info() = 0;
};

}    // namespace ash
