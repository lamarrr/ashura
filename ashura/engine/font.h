#pragma once

#include "ashura/gfx/gfx.h"
#include "ashura/std/types.h"
#include "ashura/std/unicode.h"

namespace ash
{

typedef struct Font_T *Font;

enum class FontStatus : u8
{
  Loaded          = 0,
  Loading         = 1,
  LoadFailed      = 2,
  DecodingFailed  = 3,
  FaceNotFound    = 4,
  OutOfHostMemory = 5
};

/// Metrics are normalized
/// @param bearing offset from cursor baseline to start drawing glyph from
/// @param descent distance from baseline to the bottom of the glyph
/// @param advance advancement of the cursor after drawing this glyph
/// @param extent glyph extent
struct GlyphMetrics
{
  Vec2 bearing = {0, 0};
  f32  descent = 0;
  f32  advance = 0;
  Vec2 extent  = {0, 0};
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
  Vec2U        offset    = {};
  Vec2U        extent    = {};
  Vec2         uv0       = {};
  Vec2         uv1       = {};
};

/// @param postscript_name ASCII. i.e. RobotoBold
/// @param family_name ASCII. i.e. Roboto
/// @param style_name ASCII. i.e. Bold
/// @param replacement_glyph glyph for the replacement glyph 0xFFFD if
/// found, otherwise glyph index 0
/// @param ellipsis_glyph glyph for the ellipsis character '…'
/// @param font_height font height at which the this atlas was rendered
/// @param ascent normalized maximum ascent of the font's glyphs
/// @param descent normalized maximum descent of the font's glyphs
/// @param advance normalized maximum advance of the font's glyphs
struct FontInfo
{
  Span<char const>           postscript_name   = {};
  Span<char const>           family_name       = {};
  Span<char const>           style_name        = {};
  Span<Glyph const>          glyphs            = {};
  u32                        replacement_glyph = 0;
  u32                        space_glyph       = 0;
  u32                        ellipsis_glyph    = 0;
  u32                        font_height       = 0;
  f32                        ascent            = 0;
  f32                        descent           = 0;
  f32                        advance           = 0;
  Vec2U                      extent            = {};
  u32                        num_layers        = 0;
  gfx::Image                 image             = nullptr;
  Span<gfx::ImageView const> image_views       = {};
  u32                        texture           = 0;
};

/// @param name name to use in font matching
/// @param path local file system path of the typeface resource
/// @param face font face to use
/// @param font_height the height at which the texture is cached at
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
  u32                      font_height = 20;
  Span<UnicodeRange const> ranges      = {};
};

/// TODO(lamarrr): structure and dependency order between render-context and
/// this
struct FontManager
{
  virtual void                         init()                          = 0;
  virtual Font                         add_font(FontInfo const &info)  = 0;
  virtual Font                         get_font(Span<char const> name) = 0;
  virtual Result<FontInfo, FontStatus> get_info(Font font)             = 0;
  virtual void                         uninit()                        = 0;
};

}        // namespace ash
