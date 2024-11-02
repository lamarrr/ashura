/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/gpu_context.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Font_T *Font;

// App Unit (AU) = 1/1024 of a px
constexpr i32 AU_UNIT = 1024;

constexpr f32 au_to_px(i32 au, f32 base)
{
  return au / (f32) AU_UNIT * base;
}

constexpr Vec2 au_to_px(Vec2I au, f32 base)
{
  return Vec2{au_to_px(au.x, base), au_to_px(au.y, base)};
}

enum class FontDecodeError : u8
{
  None           = 0,
  DecodingFailed = 1,
  FaceNotFound   = 2,
  OutOfMemory    = 3
};

/// @param bearing offset from cursor baseline to start drawing glyph from (au)
/// @param descent distance from baseline to the bottom of the glyph (au)
/// @param advance advancement of the cursor after drawing this glyph (au)
/// @param extent glyph extent
struct GlyphMetrics
{
  Vec2I bearing = {};
  i32   descent = 0;
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

Result<Font, FontDecodeError>
    decode_font(Span<u8 const> encoded, u32 face = 0,
                AllocatorImpl allocator = default_allocator);

FontInfo get_font_info(Font font);

void uninit_font(Font font);

/// @brief rasterize the font at the specified font height. Note: raster is
/// stored as alpha values.
/// @note rasterizing mutates the font's internal data, not thread-safe
/// @param font_height the font height at which the texture should be rasterized
/// at (px)
bool rasterize_font(Font font, u32 font_height,
                    AllocatorImpl allocator = default_allocator);

void upload_font_to_device(Font font, GpuContext &c,
                           AllocatorImpl allocator = default_allocator);

void unload_font_from_device(Font font, GpuContext &c);

}        // namespace ash
