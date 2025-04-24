/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/font.h"
#include "ashura/engine/ids.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class TextDirection : u8
{
  LeftToRight = 0,
  RightToLeft = 1
};

enum class TextScript : u8
{
  None = 0x00,

  Inherited = 0x01,
  Common    = 0x02,
  Unknown   = 0x03,

  /// Unicode 1.1
  Arabic     = 0x04,
  Armenian   = 0x05,
  Bengali    = 0x06,
  Bopomofo   = 0x07,
  Cyrillic   = 0x08,
  Devanagari = 0x09,
  Georgian   = 0x0A,
  Greek      = 0x0B,
  Gujarati   = 0x0C,
  Gurmukhi   = 0x0D,
  Hangul     = 0x0E,
  Han        = 0x0F,
  Hebrew     = 0x10,
  Hiragana   = 0x11,
  Katakana   = 0x12,
  Kannada    = 0x13,
  Lao        = 0x14,
  Latin      = 0x15,
  Malayalam  = 0x16,
  Oriya      = 0x17,
  Tamil      = 0x18,
  Telugu     = 0x19,
  Thai       = 0x1A,

  /// Unicode 2.0
  Tibetan = 0x1B,

  /// Unicode 3.0
  Braille            = 0x1C,
  CanadianAboriginal = 0x1D,
  Cherokee           = 0x1E,
  Ethiopic           = 0x1F,
  Khmer              = 0x20,
  Mongolian          = 0x21,
  Myanmar            = 0x22,
  Ogham              = 0x23,
  Runic              = 0x24,
  Sinhala            = 0x25,
  Syriac             = 0x26,
  Thaana             = 0x27,
  Yi                 = 0x28,

  /// Unicode 3.1
  Deseret   = 0x29,
  Gothic    = 0x2A,
  OldItalic = 0x2B,

  /// Unicode 3.2
  Buhid    = 0x2C,
  Hanunoo  = 0x2D,
  Tagbanwa = 0x2E,
  Tagalog  = 0x2F,

  /// Unicode 4.0
  Cypriot  = 0x30,
  Limbu    = 0x31,
  LinearB  = 0x32,
  Osmanya  = 0x33,
  Shavian  = 0x34,
  TaiLe    = 0x35,
  Ugaritic = 0x36,

  /// Unicode 4.1
  Buginese    = 0x37,
  Coptic      = 0x38,
  Glagolitic  = 0x39,
  Kharoshthi  = 0x3A,
  SylotiNagri = 0x3B,
  NewTaiLue   = 0x3C,
  Tifinagh    = 0x3D,
  OldPersian  = 0x3E,

  /// Unicode 5.0
  Balinese   = 0x3F,
  Nko        = 0x40,
  PhagsPa    = 0x41,
  Phoenician = 0x42,
  Cuneiform  = 0x43,

  /// Unicode 5.1
  Carian     = 0x44,
  Cham       = 0x45,
  KayahLi    = 0x46,
  Lepcha     = 0x47,
  Lycian     = 0x48,
  Lydian     = 0x49,
  OlChiki    = 0x4A,
  Rejang     = 0x4B,
  Saurashtra = 0x4C,
  Sundanese  = 0x4D,
  Vai        = 0x4E,

  /// Unicode 5.2
  ImperialAramaic       = 0x4F,
  Avestan               = 0x50,
  Bamum                 = 0x51,
  EgyptianHieroglyphs   = 0x52,
  Javanese              = 0x53,
  Kaithi                = 0x54,
  TaiTham               = 0x55,
  Lisu                  = 0x56,
  MeeteiMayek           = 0x57,
  OldTurkic             = 0x58,
  InscriptionalPahlavi  = 0x59,
  InscriptionalParthian = 0x5A,
  Samaritan             = 0x5B,
  OldSouthArabian       = 0x5C,
  TaiViet               = 0x5D,

  /// Unicode 6.0
  Batak   = 0x5E,
  Brahmi  = 0x5F,
  Mandaic = 0x60,

  /// Unicode 6.1
  Chakma              = 0x61,
  MeroiticCursive     = 0x62,
  MeroiticHieroglyphs = 0x63,
  Miao                = 0x64,
  Sharada             = 0x65,
  SoraSompeng         = 0x66,
  Takri               = 0x67,

  /// Unicode 7.0
  CaucasianAlbanian = 0x68,
  BassaVah          = 0x69,
  Duployan          = 0x6A,
  Elbasan           = 0x6B,
  Grantha           = 0x6C,
  PahawhHmong       = 0x6D,
  Khojki            = 0x6E,
  LinearA           = 0x6F,
  Mahajani          = 0x70,
  Manichaean        = 0x71,
  MendeKikakui      = 0x72,
  Modi              = 0x73,
  Mro               = 0x74,
  OldNorthArabian   = 0x75,
  Nabataean         = 0x76,
  Palmyrene         = 0x77,
  PauCinHau         = 0x78,
  OldPermic         = 0x79,
  PsalterPahlavi    = 0x7A,
  Siddham           = 0x7B,
  Khudawadi         = 0x7C,
  Tirhuta           = 0x7D,
  WarangCiti        = 0x7E,

  /// Unicode 8.0
  Ahom                 = 0x7F,
  Hatran               = 0x80,
  AnatolianHieroglyphs = 0x81,
  OldHungarian         = 0x82,
  Multani              = 0x83,
  SignWriting          = 0x84,

  /// Unicode 9.0
  Adlam     = 0x85,
  Bhaiksuki = 0x86,
  Marchen   = 0x87,
  Newa      = 0x88,
  Osage     = 0x89,
  Tangut    = 0x8A,

  /// Unicode 10.0
  MasaramGondi    = 0x8B,
  Nushu           = 0x8C,
  Soyombo         = 0x8D,
  ZanabazarSquare = 0x8E,

  /// Unicode 11.0
  Dogra          = 0x8F,
  GunjalaGondi   = 0x90,
  Makasar        = 0x91,
  Medefaidrin    = 0x92,
  HanifiRohingya = 0x93,
  Sogdian        = 0x94,
  OldSogdian     = 0x95,

  /// Unicode 12.0
  Elymaic              = 0x96,
  NyiakengPuachueHmong = 0x97,
  Nandinagari          = 0x98,
  Wancho               = 0x99,

  /// Unicde 13.0
  Chorasmian        = 0x9A,
  DivesAkuru        = 0x9B,
  KhitanSmallScript = 0x9C,
  Yezidi            = 0x9D,

  /// Unicde 14.0
  CyproMinoan = 0x9E,
  OldUyghur   = 0x9F,
  Tangsa      = 0xA0,
  Toto        = 0xA1,
  Vithkuqi    = 0xA2,

  /// Unicde 15.1
  Kawi       = 0xA3,
  NagMundari = 0xA4
};

constexpr TextDirection level_to_direction(u8 level)
{
  return ((level & 0x1) == 0) ? TextDirection::LeftToRight :
                                TextDirection::RightToLeft;
}

struct TextHighlightStyle
{
  Vec4U8 color        = {};
  Vec4   corner_radii = Vec4::splat(1);
  f32    stroke       = 0;
  f32    thickness    = 1;
};

struct CaretStyle
{
  Vec4U8 color        = {};
  f32    thickness    = 1;
  Vec4   corner_radii = Vec4::splat(0.25F);

  constexpr bool is_none() const
  {
    return thickness == 0 || color == Vec4U8{};
  }
};

struct TextCursor
{
  isize span_ = 0;
  isize base_ = 0;

  constexpr TextCursor & selectx(Slice s)
  {
    base_ = s.offset;
    span_ = s.span;
    return *this;
  }

  constexpr isize caretx() const
  {
    return base_ + span_;
  }

  constexpr isize left_caretx() const
  {
    if (span_ > 0)
    {
      return base_;
    }
    else if (span_ < 0)
    {
      return base_ + span_;
    }
    else
    {
      return base_;
    }
  }

  constexpr isize right_caretx() const
  {
    if (span_ > 0)
    {
      return base_ + span_;
    }
    else if (span_ < 0)
    {
      return base_;
    }
    else
    {
      return base_;
    }
  }

  constexpr TextCursor & unselectx()
  {
    auto c = caretx();
    base_  = c;
    span_  = 0;
    return *this;
  }

  constexpr TextCursor & unselect_leftx()
  {
    auto c = left_caretx();
    base_  = c;
    span_  = 0;
    return *this;
  }

  constexpr TextCursor & unselect_rightx()
  {
    auto c = right_caretx();
    base_  = c;
    span_  = c;
    return *this;
  }

  constexpr bool has_selectionx() const
  {
    return span_ != 0;
  }

  constexpr TextCursor & normalizex(isize num_carets)
  {
    if (num_carets == 0)
    {
      base_ = 0;
      span_ = 0;
    }
    else
    {
      base_ = clamp(base_, (isize) 0, num_carets - 1);
      span_ = clamp(base_ + span_, (isize) 0, num_carets) - base_;
    }
    return *this;
  }

  constexpr Slice selectionx() const
  {
    if (span_ >= 0)
    {
      return Slice{(usize) base_, (usize) span_};
    }
    else
    {
      return Slice{(usize) (base_ + span_), (usize) (-span_)};
    }
  }

  constexpr TextCursor & span_by2x(isize distance)
  {
    span_ = distance;
    return *this;
  }

  constexpr isize spanx() const
  {
    return span_;
  }

  constexpr TextCursor & span_to2x(isize selection_pos)
  {
    span_ = (selection_pos - base_);
    return *this;
  }

  constexpr TextCursor & extend_selectionx(isize extension)
  {
    span_ += extension;
    return *this;
  }

  constexpr TextCursor & move_tox(isize pos)
  {
    base_ = pos;
    span_ = 0;
    return *this;
  }

  constexpr TextCursor & translatex(isize dist)
  {
    base_ += dist;
    return *this;
  }
};

/// @param font font to use to render the text
/// @param word_spacing px. additional word spacing, can be negative
/// @param line_height relative. multiplied by font_height
struct FontStyle
{
  FontId font         = FontId::None;
  f32    height       = 20;
  f32    line_height  = 1.2F;
  f32    word_spacing = 0;
};

/// @param shadow_scale relative. multiplied by font_height
/// @param shadow_offset offset from center of glyph
struct TextStyle
{
  f32           underline_thickness     = 0;
  f32           underline_offset        = 2.0F;
  f32           strikethrough_thickness = 0;
  f32           shadow_scale            = 0;
  Vec2          shadow_offset           = Vec2{0, 0};
  ColorGradient color                   = {};
  ColorGradient background              = {};
  ColorGradient underline               = {};
  ColorGradient strikethrough           = {};
  ColorGradient shadow                  = {};
  Vec4          corner_radii            = Vec4::splat(0.5F);
  void *        user_data               = nullptr;

  constexpr bool has_shadow() const
  {
    return shadow_scale != 0 && !shadow.is_transparent();
  }

  constexpr bool has_color() const
  {
    return !color.is_transparent();
  }
};

/// @brief A block of text to be laid-out, consists of multiple runs of text
/// spanning multiple paragraphs.
/// @param text utf-32-encoded text
/// @param runs end offset of each text run
/// @param fonts font style of each text run
/// @param direction base text direction
/// @param language base language to use for selecting opentype features to
/// use on the text, uses system default if not set
/// @param use_kerning use provided font kerning
/// @param use_ligatures use standard and contextual font ligature substitution
struct TextBlock
{
  Str32                 text          = {};
  Span<u32 const>       runs          = {};
  Span<FontStyle const> fonts         = {};
  f32                   font_scale    = 1;
  TextDirection         direction     = TextDirection::LeftToRight;
  Str                   language      = {};
  bool                  wrap          = true;
  bool                  use_kerning   = true;
  bool                  use_ligatures = true;
};

/// @param styles styles for each run in the source text, for each
/// `TextBlock::runs`
/// @param align_width width to align the text block to when rendering.
/// @param alignment alignment of the text to its base direction. -1 = line
/// start, +1 = line end, 0 = center of line, the direction depends on the
/// directionality of the line.
struct TextBlockStyle
{
  Span<TextStyle const> runs        = {};
  f32                   alignment   = 0;
  f32                   align_width = 0;
  TextHighlightStyle    highlight   = {};
  CaretStyle            caret       = {};
};

/// @param cluster unicode grapheme cluster within the text run
/// @param advance context-dependent horizontal-layout advance
/// @param offset context-dependent text shaping offset from normal font glyph
/// position, i.e. offset from GlyphMetrics::bearing
struct GlyphShape
{
  usize glyph   = 0;
  usize cluster = 0;
  i32   advance = 0;
  Vec2I offset  = {};
};

/// @param style the text/font style of the current run
/// @param script script of the current codepoint
/// @param paragraph_begin if this codepoint marks the beginning of a new
/// paragraph
/// @param paragraph_end if this codepoint marks the end of a paragraph
/// @param base_level the current paragraph's embedding level
/// @param level embedding level of the current codepoint in the paragraph
/// @param wrappable if this codepoint begins a wrappable text, i.e. has spaces
/// or tabs before it
struct TextSegment
{
  u32        style               = 0;
  TextScript script              = TextScript::None;
  bool       paragraph_begin : 1 = false;
  bool       paragraph_end   : 1 = false;
  bool       whitespace      : 1 = false;
  bool       tab             : 1 = false;
  bool       wrappable       : 1 = false;
  u8         base_level          = 0;
  u8         level               = 0;

  constexpr bool is_wrap_point() const
  {
    return whitespace | tab;
  }
};

struct ResolvedTextRunMetrics
{
  f32 advance = 0;
  f32 ascent  = 0;
  f32 descent = 0;

  constexpr f32 height() const
  {
    return ascent + descent;
  }
};

struct TextRunMetrics
{
  i32 advance = 0;
  i32 ascent  = 0;
  i32 descent = 0;

  constexpr i32 height() const
  {
    return ascent + descent;
  }

  constexpr ResolvedTextRunMetrics resolve(f32 font_height) const
  {
    return ResolvedTextRunMetrics{.advance = au_to_px(advance, font_height),
                                  .ascent  = au_to_px(ascent, font_height),
                                  .descent = au_to_px(descent, font_height)};
  }
};

enum class TextRunType : u8
{
  Char       = 0,
  WhiteSpace = 1,
  Tab        = 2
};

struct TextRun
{
  /// @brief Codepoints in the source text the run belongs to
  Slice codepoints = {};

  /// @brief Codepoints that created the paragraph,
  /// Only non-empty if the line is the first in the paragraph.
  /// `.span` is usually 0 (none), 1 ('\n') or 2 ('\r\n').
  Slice break_codepoints = {};

  /// @brief style in the list of specified text styles
  u32 style = 0;

  f32 font_height = 0;

  f32 line_height = 0;

  Slice glyphs = {};

  TextRunMetrics metrics = {};

  u8 base_level = 0;

  u8 level = 0;

  /// @brief if the run represents a break-opportunity as constrained by the max-width.
  bool wrappable = false;

  TextRunType type = TextRunType::Char;

  constexpr TextDirection base_direction() const
  {
    return level_to_direction(base_level);
  }

  constexpr TextDirection direction() const
  {
    return level_to_direction(level);
  }

  constexpr bool is_paragraph_begin() const
  {
    return !break_codepoints.is_empty();
  }

  constexpr Slice carets(Slice line_carets, Slice line_codepoints) const
  {
    return Slice{line_carets.offset +
                   (codepoints.offset - line_codepoints.offset),
                 codepoints.span};
  }

  constexpr bool is_spacing() const
  {
    return type == TextRunType::Tab || type == TextRunType::WhiteSpace;
  }
};

/// @param width width of the line
/// @param height maximum line height of all the runs on the line (px)
/// @param ascent maximum ascent of all the runs on the line (px)
/// @param descent maximum descent of all the runs on the line (px)
/// @param level base level of the line
struct LineMetrics
{
  f32 width   = 0;
  f32 height  = 0;
  f32 ascent  = 0;
  f32 descent = 0;
  u8  level   = 0;

  constexpr TextDirection direction() const
  {
    return level_to_direction(level);
  }
};

/// @brief
struct Line
{
  /// @brief codepoints in the line (excludes the preceding line-breaks if any).
  Slice codepoints = {};

  /// @brief line-break codepoints that began this line/paragraph (i.e. `n` or `\r\n`)
  Slice break_codepoints = {};

  /// @brief Logical Carets on the current line. If the Line is an RTL line,
  /// the first caret will be visually placed on the right
  Slice carets = {};

  Slice runs = {};

  LineMetrics metrics = {};
};

enum class CaretAlignment : isize
{
  LineStart = 0,
  LineEnd   = ISIZE_MAX
};

enum class LineAlignment : isize
{
  Top    = ISIZE_MIN,
  First  = 0,
  Bottom = ISIZE_MAX
};

struct CaretLocation
{
  LineAlignment  line      = LineAlignment::Bottom;
  CaretAlignment alignment = CaretAlignment::LineStart;
};

struct CaretCodepoint
{
  usize line      = 0;
  usize codepoint = 0;
  bool  after     = false;
};

struct CaretGlyph
{
  usize line  = 0;
  usize glyph = 0;
  bool  after = false;
};

struct ShapeInfo;
struct Canvas;

enum class TextRegion : u32
{
  None          = 0,
  Block         = 1,
  Background    = 2,
  GlyphShadows  = 3,
  Glyphs        = 4,
  Underline     = 5,
  Strikethrough = 6,
  Highlight     = 7,
  Caret         = 8
};

struct TextRenderInfo
{
  /// @brief Text region currently being rendered
  TextRegion region = TextRegion::None;

  /// @brief set to the current line being rendered for
  Option<usize> line = none;

  /// @brief set to the current text-run being rendered for
  Option<usize> run = none;

  /// @brief set to the current run-style being rendered for
  Option<usize> run_style = none;

  /// @brief set to the current glyph being rendered for
  Option<usize> glyph = none;

  /// @brief set to the current glyph being rendered for
  Option<usize> cluster = none;

  /// @brief set to the current caret being rendered for
  Option<isize> caret = none;
};

typedef Fn<void(Canvas &, ShapeInfo const &, TextRenderInfo const &)>
  TextRenderer;

/// @brief cached/pre-computed text layout
/// @param max_width maximum width the text was laid out with
/// @param extent current extent of the text block after layout
/// @param segments each segment matches a codepoint in the source text.
/// @param glyphs laid-out glyphs for all the text. it is re-usable and
/// independent of the font style as long as the font matches.
/// @param lines lines in the text as constrained by max_width and paragraphs
/// found in the text.
///
///
///
///
/// # Caret Layout:
///
///    |      codepoint:0     |      codepoint:1      |      codepoint:2     |     codepoint:3     |
/// caret:0                caret:1                  caret:2                caret:3               caret:4
struct TextLayout
{
  f32             max_width;
  usize           num_carets;
  usize           num_codepoints;
  Vec2            extent;
  Vec<GlyphShape> glyphs;
  Vec<TextRun>    runs;
  Vec<Line>       lines;

  explicit TextLayout(AllocatorRef allocator) :
    max_width{0},
    num_carets{0},
    num_codepoints{0},
    extent{},
    glyphs{allocator},
    runs{allocator},
    lines{allocator}
  {
  }

  TextLayout(TextLayout const &)             = delete;
  TextLayout & operator=(TextLayout const &) = delete;
  TextLayout(TextLayout &&)                  = default;
  TextLayout & operator=(TextLayout &&)      = default;
  ~TextLayout()                              = default;

  void clear()
  {
    max_width      = F32_MAX;
    extent         = Vec2{0, 0};
    num_carets     = 0;
    num_codepoints = 0;
    glyphs.clear();
    runs.clear();
    lines.clear();
  }

  Option<isize> to_caret(usize codepoint, bool before) const;

  isize align_caret(CaretLocation loc) const;

  Slice get_caret_selection(Slice carets) const;

  Slice to_caret_selection(Slice codepoints) const;

  Option<CaretCodepoint> get_caret_codepoint(isize caret) const;

  Option<CaretGlyph> get_caret_glyph(isize caret) const;

  /// @brief given a position in the laid-out text return the caret the cursor points
  /// to and its location.
  /// @param pos relative position in laid-out text to hit
  Tuple<isize, CaretLocation> hit(TextBlock const &      block,
                                  TextBlockStyle const & style, Vec2 pos) const;

  static void default_renderer(Canvas & canvas, ShapeInfo const & shape,
                               TextRenderInfo const & info);

  /// @brief Render Text using pre-computed layout
  /// @param info only info.center, info.transform, info.tiling, and info.sampler are used
  /// @param block Text Block to be rendered
  /// @param layout Layout of text block to be rendered
  /// @param style styling of the text block, contains styling for the runs and alignment of the block
  /// @param highlights caret highlights to draw. Overlapping highlights should be
  /// merged as the performance cost increases with increasing number of highlights
  /// @param carets carets to draw
  /// @param clip clip rect for culling draw commands of the text block
  /// @param renderer the renderer to use for rendering the text's regions
  void render(Canvas & canvas, ShapeInfo const & info, TextBlock const & block,
              TextBlockStyle const & style, Span<Slice const> highlights,
              Span<isize const> carets, CRect const & clip,
              TextRenderer renderer = default_renderer) const;
};

}    // namespace ash
