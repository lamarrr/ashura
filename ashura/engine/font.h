/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/gpu_context.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/dyn.h"
#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

// App Unit (AU)
constexpr i32 AU_UNIT = 128 * 64;

static_assert((AU_UNIT % 64) == 0,
              "App Unit needs to be in 26.6 Fractional Unit");

static_assert((AU_UNIT / 64) >= 64,
              "App Unit needs to be at least 64 26.6 Fractional Units");

constexpr f32 au_to_px(i32 au, f32 base)
{
  return (au / (f32) AU_UNIT) * base;
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

constexpr Span<char const> to_string(FontErr err)
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
  return push(ctx, spec, to_string(err));
}

}        // namespace fmt

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

/// @param ascent  maximum ascent of the font's glyphs (au)
/// @param descent maximum descent of the font's glyphs (au)
/// @param advance maximum advance of the font's glyphs (au)
struct FontMetrics
{
  i32 ascent  = 0;
  i32 descent = 0;
  i32 advance = 0;
};

/// @param is_valid if the glyph was found in the font and loaded
// successfully
/// @param metrics normalized font metrics
/// @param bin atlas layer this glyph belongs to
/// @param offset, extent: area in the atlas this glyph's cache data is placed
/// @param uv0, uv1: normalized texture coordinates of this
/// glyph in the atlas bin
struct Glyph
{
  bool         is_valid = false;
  GlyphMetrics metrics  = {};
};

/// @param rasterized if the glyph was rasterized
struct AtlasGlyph
{
  u32       layer = 0;
  gpu::Rect area  = {};
  Vec2      uv[2] = {};
};

typedef struct CpuFontAtlas CpuFontAtlas;
typedef struct GpuFontAtlas GpuFontAtlas;

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
  Span<char const>             postscript_name   = {};
  Span<char const>             family_name       = {};
  Span<char const>             style_name        = {};
  Span<Glyph const>            glyphs            = {};
  u32                          replacement_glyph = 0;
  u32                          space_glyph       = 0;
  u32                          ellipsis_glyph    = 0;
  FontMetrics                  metrics           = {};
  Option<CpuFontAtlas const *> cpu_atlas         = None;
  Option<GpuFontAtlas const *> gpu_atlas         = None;
};

struct Font
{
  static Result<Dyn<Font *>, FontErr> decode(Span<u8 const> encoded,
                                             u32            face      = 0,
                                             AllocatorImpl  allocator = {});

  /// @brief rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  virtual Result<> rasterize(u32 font_height, AllocatorImpl allocator) = 0;

  virtual FontInfo info() = 0;

  virtual void upload_to_device(GpuContext & c, AllocatorImpl allocator) = 0;

  virtual void unload_from_device(GpuContext & c) = 0;

  virtual ~Font() = default;
};

}        // namespace ash
