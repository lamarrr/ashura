/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_context.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Font_T *Font;

// 1024px per pt.
constexpr i32 PT_UNIT = 1024;

constexpr f32 pt_to_px(i32 pt, f32 base)
{
  return pt / (f32) PT_UNIT * base;
}

constexpr Vec2 pt_to_px(Vec2I pt, f32 base)
{
  return Vec2{pt_to_px(pt.x, base), pt_to_px(pt.y, base)};
}

enum class FontStatus : u8
{
  Loaded         = 0,
  Loading        = 1,
  DecodingFailed = 2,
  FaceNotFound   = 3,
  OutOfMemory    = 4,
};

/// @param bearing offset from cursor baseline to start drawing glyph from (pt)
/// @param descent distance from baseline to the bottom of the glyph (pt)
/// @param advance advancement of the cursor after drawing this glyph (pt)
/// @param extent glyph extent
struct GlyphMetrics
{
  Vec2I bearing = {};
  i32   descent = 0;
  i32   advance = 0;
  Vec2I extent  = {};
};

/// @param ascent  maximum ascent of the font's glyphs (pt)
/// @param descent maximum descent of the font's glyphs (pt)
/// @param advance maximum advance of the font's glyphs (pt)
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
  gfx::Rect area  = {};
  Vec2      uv[2] = {};
};

/// @param postscript_name ASCII. i.e. RobotoBold
/// @param family_name ASCII. i.e. Roboto
/// @param style_name ASCII. i.e. Bold
/// @param replacement_glyph glyph for the replacement glyph 0xFFFD if
/// found, otherwise glyph index 0
/// @param ellipsis_glyph glyph for the ellipsis character '…'
/// @param font_height font height at which the this atlas was rendered (px)
struct FontInfo
{
  Span<char const>  postscript_name   = {};
  Span<char const>  family_name       = {};
  Span<char const>  style_name        = {};
  Span<Glyph const> glyphs            = {};
  u32               replacement_glyph = 0;
  u32               space_glyph       = 0;
  u32               ellipsis_glyph    = 0;
  FontMetrics       metrics           = {};
};

struct FontAtlas
{
  i32             font_height = 0;
  Vec2U           extent      = {};
  u32             num_layers  = 0;
  Vec<AtlasGlyph> glyphs      = {};
  Vec<u8>         channels    = {};

  ImageLayerSpan<u8, 1> span() const
  {
    return ImageLayerSpan<u8, 1>{.channels = to_span(channels),
                                 .width    = extent.x,
                                 .height   = extent.y,
                                 .layers   = num_layers};
  }

  void uninit()
  {
    glyphs.uninit();
    channels.uninit();
  }
};

struct FontAtlasResource
{
  gfx::Image          image    = nullptr;
  Vec<gfx::ImageView> views    = {};
  Vec<u32>            textures = {};
  Vec<AtlasGlyph>     glyphs   = {};
  gfx::Format         format   = gfx::Format::Undefined;

  void init(RenderContext &c, FontAtlas const &atlas,
            AllocatorImpl const &allocator);
  void release(RenderContext &c);

  void uninit()
  {
    image = nullptr;
    views.uninit();
    textures.uninit();
    glyphs.uninit();
    format = gfx::Format::Undefined;
  }
};

/// @param face font face to use
Result<Font, FontStatus> load_font(Span<u8 const> encoded, u32 face,
                                   AllocatorImpl const &allocator);

FontInfo get_font_info(Font font);

void destroy_font(Font font);

/// @brief rasterize the font at the specified font height. Note: raster is
/// stored as alpha values.
/// @note rasterizing mutates the font's internal data, not thread-safe
/// @param font_height the font height at which the texture should be rasterized
/// at (px)
bool rasterize_font(Font font, u32 font_height, FontAtlas &atlas,
                    AllocatorImpl const &allocator);

}        // namespace ash
