#pragma once

#include "ashura/engine/render_context.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/types.h"
#include "ashura/std/unicode.h"

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
  Loaded            = 0,
  Loading           = 1,
  LoadFailed        = 2,
  DecodingFailed    = 3,
  FaceNotFound      = 4,
  OutOfHostMemory   = 5,
  OutOfDeviceMemory = 6
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

/// see:
/// https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
/// NOTE: using stubs enables us to perform fast constant lookups of glyph
/// indices by ensuring the array is filled and sorted by glyph index from 0 ->
/// nglyphs_found_in_font-1
/// @param is_valid if the glyph was found in the font and loaded
// successfully
/// @param is_needed if the texture is a texture that is needed. i.e. if the
/// unicode ranges are empty then this is always true,
/// otherwise it is set to true if the config unicode ranges
/// contains it, note that special glyphs like replacement
/// unicode codepoint glyph (0xFFFD) will always be true
/// @param metrics normalized font metrics
/// @param bin atlas layer this glyph belongs to
/// @param offset, extent: area in the atlas this glyph's cache data is placed
/// @param uv0, uv1: normalized texture coordinates of this
/// glyph in the atlas bin
struct Glyph
{
  bool         is_valid  = false;
  bool         is_needed = false;
  GlyphMetrics metrics   = {};
  u32          layer     = 0;
  gfx::Rect    area      = {};
  Vec2         uv[2]     = {};
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
  Span<char const>           postscript_name   = {};
  Span<char const>           family_name       = {};
  Span<char const>           style_name        = {};
  Span<Glyph const>          glyphs            = {};
  u32                        replacement_glyph = 0;
  u32                        space_glyph       = 0;
  u32                        ellipsis_glyph    = 0;
  i32                        font_height       = 0;
  FontMetrics                metrics           = {};
  Vec2U                      extent            = {};
  u32                        num_layers        = 0;
  gfx::Image                 image             = nullptr;
  gfx::Format                format            = gfx::Format::B8G8R8A8_UNORM;
  Span<gfx::ImageView const> image_views       = {};
  Span<u32 const>            textures          = {};
};

/// @param name name to use in font matching
/// @param path local file system path of the typeface resource
/// @param face font face to use
/// @param font_height the font height at which the texture should be cached at
/// (px)
/// @param ranges if set only the specified unicode ranges will be loaded,
/// otherwise all glyphs in the font will be loaded. Note that this
/// means during font ligature glyph substitution where scripts
/// might change, if the replacement glyph is not in the unicode
/// range, it won't result in a valid glyph.
struct FontDesc
{
  Span<char const>         name        = {};
  Span<char const>         path        = {};
  u32                      face        = 0;
  i32                      font_height = 20;
  Span<UnicodeRange const> ranges      = {};
};

Result<Font, FontStatus> load_font(Span<u8 const> encoded, u32 face,
                                   Span<UnicodeRange const> ranges,
                                   AllocatorImpl const     &allocator);

void destroy_font(Font f, RenderContext &c);

void rasterize_font(Font f, i32 font_height, AllocatorImpl const &allocator);

void upload_font_to_device(Font f, RenderContext &c,
                           AllocatorImpl const &allocator);

FontInfo get_font_info(Font f);

}        // namespace ash
