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
/// @bearing: offset from cursor baseline to start drawing glyph from
/// @descent: distance from baseline to the bottom of the glyph
/// @advance: advancement of the cursor after drawing this glyph
/// @extent: glyph extent
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
/// @is_valid: if the glyph was found in the font and loaded
// successfully
/// @is_needed: if the texture is a texture that is needed. i.e. if the
/// unicode ranges are empty then this is always true,
/// otherwise it is set to true if the config unicode ranges
/// contains it, note that special glyphs like replacement
/// unicode codepoint glyph (0xFFFD) will always be true
/// @metrics: normalized font metrics
/// @bin: atlas layer this glyph belongs to
/// @offset, extent: area in the atlas this glyph's cache data is placed
/// @uv0, uv1: normalized texture coordinates of this
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

/// @postscript_name: ASCII. i.e. RobotoBold
/// @family_name: ASCII. i.e. Roboto
/// @style_name: ASCII. i.e. Bold
/// @replacement_glyph: glyph for the replacement glyph 0xFFFD if
/// found, otherwise glyph index 0
/// @ellipsis_glyph: glyph for the ellipsis character '…'
/// @font_height: font height at which the this atlas was rendered
/// @ascent: normalized maximum ascent of the font's glyphs
/// @descent: normalized maximum descent of the font's glyphs
/// @advance: normalized maximum advance of the font's glyphs
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

/// @name: name to use in font matching
/// @path: local file system path of the typeface resource
/// @face: font face to use
/// @font_height: the height at which the texture is cached at
/// @ranges: if set only the specified unicode ranges will be loaded,
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
