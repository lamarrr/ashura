/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/dyn.h"
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

constexpr Vec2 au_to_px(Vec2I au, f32 base)
{
  return Vec2{au_to_px(au.x, base), au_to_px(au.y, base)};
}

enum class FontErr : u8
{
  None           = 0,
  DecodingFailed = 1,
  FaceNotFound   = 2,
  OutOfMemory    = 3
};

constexpr Span<char const> to_str(FontErr err)
{
  switch (err)
  {
    case FontErr::None:
      return "None"_str;
    case FontErr::DecodingFailed:
      return "DecodingFailed"_str;
    case FontErr::FaceNotFound:
      return "FaceNotFound"_str;
    case FontErr::OutOfMemory:
      return "OutOfMemory"_str;
    default:
      return "Unidentified"_str;
  }
}

namespace fmt
{

inline bool push(Context const & ctx, Spec const & spec, FontErr const & err)
{
  return push(ctx, spec, to_str(err));
}

}    // namespace fmt

/// @brief Glyph Metrics. expressed on an AU_UNIT scale
/// @param bearing offset from cursor baseline to start drawing glyph from (au)
/// @param descent distance from baseline to the bottom of the glyph (au)
/// @param advance advancement of the cursor after drawing this glyph (au)
/// @param extent glyph extent
struct GlyphMetrics
{
  Vec2I bearing = {};
  i32   advance = 0;
  Vec2I extent  = {};
};

/// @brief normalized font metrics
/// @param ascent  maximum ascent of the font's glyphs (au)
/// @param descent maximum descent of the font's glyphs (au)
/// @param advance maximum advance of the font's glyphs (au)
struct FontMetrics
{
  i32 ascent  = 0;
  i32 descent = 0;
  i32 advance = 0;
};

/// @param later atlas layer this glyph belongs to
/// @param area area in the atlas this glyph's cache data is placed
/// @param uv normalized texture coordinates of this glyph in the layer
struct AtlasGlyph
{
  u32       layer = 0;
  gpu::Rect area  = {};
  Vec2      uv[2] = {};
};

struct CpuFontAtlas
{
  i32             font_height = 0;
  Vec2U           extent      = {};
  u32             num_layers  = 0;
  Vec<AtlasGlyph> glyphs      = {};
  Vec<u8>         channels    = {};

  ImageLayerSpan<u8, 1> span() const
  {
    return ImageLayerSpan<u8, 1>{.channels = channels,
                                 .width    = extent.x,
                                 .height   = extent.y,
                                 .layers   = num_layers};
  }
};

struct GpuFontAtlas
{
  gpu::Image          image       = nullptr;
  Vec<gpu::ImageView> views       = {};
  Vec<TextureId>      textures    = {};
  i32                 font_height = 0;
  u32                 num_layers  = 0;
  Vec2U               extent      = {};
  Vec<AtlasGlyph>     glyphs      = {};
  gpu::Format         format      = gpu::Format::Undefined;
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
  Span<char const>              postscript_name   = {};
  Span<char const>              family_name       = {};
  Span<char const>              style_name        = {};
  Span<GlyphMetrics const>      glyphs            = {};
  u32                           replacement_glyph = 0;
  u32                           space_glyph       = 0;
  u32                           ellipsis_glyph    = 0;
  FontMetrics                   metrics           = {};
  OptionRef<CpuFontAtlas const> cpu_atlas         = none;
  OptionRef<GpuFontAtlas const> gpu_atlas         = none;
};

struct Font
{
  static Result<Dyn<Font *>, FontErr>
    decode(Span<u8 const> encoded, u32 face = 0, AllocatorImpl allocator = {});

  /// @brief rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  virtual Result<> rasterize(u32 font_height, AllocatorImpl allocator) = 0;

  virtual FontInfo info() = 0;

  virtual void upload_to_device(GpuSystem & c, AllocatorImpl allocator) = 0;

  virtual void unload_from_device(GpuSystem & c) = 0;

  virtual ~Font() = default;
};

}    // namespace ash
