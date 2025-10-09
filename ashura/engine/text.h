/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/std/color.h"
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
  ColorGradient color        = {};
  f32x4         corner_radii = f32x4::splat(0);
  f32           stroke       = 0;
  f32           thickness    = 1;
  void *        user_data    = nullptr;
};

struct CaretStyle
{
  ColorGradient color        = {};
  f32           thickness    = 1;
  f32x4         corner_radii = f32x4::splat(0.25F);
  void *        user_data    = nullptr;

  constexpr bool is_none() const
  {
    return thickness == 0;
  }
};

struct TextCursor
{
  isize span_ = 0;
  isize base_ = 0;

  constexpr TextCursor & select(Slice s)
  {
    base_ = s.offset;
    span_ = s.span;
    return *this;
  }

  constexpr isize caret() const
  {
    return base_ + span_;
  }

  constexpr isize left_caret() const
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

  constexpr isize right_caret() const
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

  constexpr TextCursor & unselect()
  {
    auto c = caret();
    base_  = c;
    span_  = 0;
    return *this;
  }

  constexpr TextCursor & unselect_left()
  {
    auto c = left_caret();
    base_  = c;
    span_  = 0;
    return *this;
  }

  constexpr TextCursor & unselect_right()
  {
    auto c = right_caret();
    base_  = c;
    span_  = c;
    return *this;
  }

  constexpr bool has_selection() const
  {
    return span_ != 0;
  }

  constexpr TextCursor & normalize(isize num_carets)
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

  constexpr Slice selection() const
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

  constexpr TextCursor & span_by(isize distance)
  {
    span_ = distance;
    return *this;
  }

  constexpr isize span() const
  {
    return span_;
  }

  constexpr TextCursor & span_to(isize selection_pos)
  {
    span_ = (selection_pos - base_);
    return *this;
  }

  constexpr TextCursor & extend_selection(isize extension)
  {
    span_ += extension;
    return *this;
  }

  constexpr TextCursor & move_to(isize pos)
  {
    base_ = pos;
    return *this;
  }

  constexpr TextCursor & translate(isize dist)
  {
    base_ += dist;
    return *this;
  }
};

struct FontStyle
{
  /// @brief font to use to render the text
  FontId font = FontId::None;

  f32 height = 20;

  /// @brief relative. multiplied by font_height
  f32 line_height = 1.2F;

  /// @brief px. additional word spacing, can be negative
  f32 word_spacing = 0;

  void * user_data = nullptr;
};

struct TextStyle
{
  f32 underline_thickness     = 0;
  f32 underline_offset        = 2.0F;
  f32 strikethrough_thickness = 0;

  /// @brief relative. multiplied by font_height
  f32 shadow_scale = 0;

  /// @brief offset from center of glyph
  f32x2 shadow_offset = f32x2{0, 0};

  ColorGradient foreground    = {};
  ColorGradient background    = {};
  ColorGradient underline     = {};
  ColorGradient strikethrough = {};
  ColorGradient shadow        = {};
  f32x4         corner_radii  = f32x4::splat(0.5F);
  void *        user_data     = nullptr;

  constexpr bool has_shadow() const
  {
    return shadow_scale != 0;
  }

  constexpr bool has_color() const
  {
    return !foreground.is_transparent();
  }
};

/// @brief A block of text to be laid-out, consists of multiple runs of text
/// spanning multiple paragraphs.
struct TextBlock
{
  /// @brief utf-32-encoded text
  Str32 text = {};

  /// @brief run-end-encoded offset of each text run
  Span<usize const> run_indices = {};

  /// @brief font style of each text run
  Span<FontStyle const> fonts = {};

  f32 font_scale = 1;

  /// @brief base text direction
  TextDirection direction = TextDirection::LeftToRight;

  /// @brief base language to use for selecting opentype features to
  /// use on the text, uses system default if not set
  Str language = {};

  bool wrap = true;

  /// @brief use provided font kerning
  bool use_kerning = true;

  /// @brief use standard and contextual font ligature substitution
  bool use_ligatures = true;
};

struct TextBlockStyle
{
  /// @param styles styles for each run in the source text, for each
  /// `TextBlock::runs`
  Span<TextStyle const> runs = {};

  /// @brief alignment of the text to its base direction. -1 = line
  /// start, +1 = line end, 0 = center of line, the direction depends on the
  /// directionality of the line.
  f32 alignment = 0;

  /// @brief width to align the text block to when rendering.
  f32 align_width = 0;

  TextHighlightStyle highlight           = {};
  f32                min_highlight_width = 15.0F;
  CaretStyle         caret               = {};
  void *             user_data           = nullptr;
};

/// @param cluster codepoint cluster within the text run
/// @param advance context-dependent horizontal-layout advance
/// @param offset context-dependent text shaping offset from normal font glyph
/// position, i.e. offset from GlyphMetrics::bearing
struct GlyphShape
{
  usize glyph   = 0;
  usize cluster = 0;
  i32   advance = 0;
  i32x2 offset  = {};
};

struct TextSegment
{
  /// @brief the text/font style of the current run
  u32 style = 0;

  /// @brief script of the current codepoint
  TextScript script = TextScript::None;

  bool linebreak_begin : 1 = false;
  bool paragraph_begin : 1 = false;
  bool whitespace      : 1 = false;
  bool tab             : 1 = false;

  /// @brief if this codepoint begins a wrappable text, i.e. has spaces
  /// or tabs before it
  bool wrappable : 1 = false;

  /// @brief the current paragraph's embedding level
  u8 base_level = 0;

  /// @brief embedding level of the current codepoint in the paragraph
  u8 level = 0;

  constexpr bool is_wrap_point() const
  {
    return whitespace | tab;
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

  /// @brief Style in the list of specified text styles
  u32 style = 0;

  f32 font_height = 0;

  f32 line_height = 0;

  Slice glyphs = {};

  FontMetrics metrics = {};

  u8 base_level = 0;

  u8 level = 0;

  /// @brief If the run represents a break-opportunity as constrained by the max-width.
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

struct LineMetrics
{
  /// @brief width of the line
  f32 width = 0;

  /// @brief maximum line height of all the runs on the line (px)
  f32 height = 0;

  /// @brief maximum ascent of all the runs on the line (px)
  f32 ascent = 0;

  /// @brief maximum descent of all the runs on the line (px)
  f32 descent = 0;

  /// @brief base level of the line
  u8 level = 0;

  constexpr TextDirection direction() const
  {
    return level_to_direction(level);
  }

  constexpr f32 leading() const
  {
    return 0.5F * (height - (ascent + descent));
  }
};

struct Line
{
  /// @brief Codepoints in the line (excludes the preceding line-breaks if any).
  Slice codepoints = {};

  /// @brief Logical Carets on the current line. If the Line is an RTL line,
  /// the first caret will be visually placed on the right
  Slice carets = {};

  Slice runs = {};

  LineMetrics metrics = {};
};

struct Paragraph
{
  Slice lines = {};

  Slice runs = {};

  Slice codepoints = {};

  Slice break_codepoints = {};
};

enum class CaretXAlignment : isize
{
  Start = 0,
  End   = ISIZE_MAX
};

enum class CaretYAlignment : isize
{
  Top    = ISIZE_MIN,
  First  = 0,
  Bottom = ISIZE_MAX
};

struct CaretAlignment
{
  CaretXAlignment x = CaretXAlignment::Start;
  CaretYAlignment y = CaretYAlignment::Top;
};

struct CaretCodepoint
{
  usize line      = 0;
  usize codepoint = 0;
  bool  after     = false;
};

struct CaretPlacement
{
  usize         line  = 0;
  Option<usize> glyph = none;
  bool          after = false;
};

struct TextPlacement
{
  struct Block
  {
    /// @brief bbox relative to the text block center
    CRect bbox = {};
  };

  struct Line
  {
    CRect bbox = {};
    usize line = 0;
  };

  struct Background
  {
    CRect bbox      = {};
    usize line      = 0;
    usize column    = 0;
    usize run       = 0;
    usize run_style = 0;
  };

  struct GlyphShadow
  {
    CRect bbox = {};

    /// @brief Set to the current line being rendered for
    usize line = 0;

    /// @brief Column of the current region/item being rendered
    usize column = 0;

    /// @brief Set to the current text-run being rendered for
    usize run = 0;

    /// @brief Set to the current run-style being rendered for
    usize run_style = 0;

    /// @brief Set to the current glyph being rendered for
    usize glyph = 0;

    /// @brief Set to the current glyph being rendered for
    usize cluster = 0;
  };

  struct Glyph
  {
    CRect bbox      = {};
    usize line      = 0;
    usize column    = 0;
    usize run       = 0;
    usize run_style = 0;
    usize glyph     = 0;
    usize cluster   = 0;
  };

  struct Underline
  {
    CRect bbox      = {};
    usize line      = 0;
    usize column    = 0;
    usize run       = 0;
    usize run_style = 0;
  };

  struct Strikethrough
  {
    CRect bbox      = {};
    usize line      = 0;
    usize column    = 0;
    usize run       = 0;
    usize run_style = 0;
  };

  struct Highlight
  {
    // [ ] indices; slice32 span; so we know if it spans the entire line
    CRect bbox = {};
    usize line = 0;
  };

  struct Caret
  {
    CRect bbox   = {};
    usize line   = 0;
    usize column = 0;

    /// @brief Set to the current caret being rendered for
    usize caret = 0;
  };

  Span<Block const>         blocks         = {};
  Span<Line const>          lines          = {};
  Span<Background const>    backgrounds    = {};
  Span<GlyphShadow const>   glyph_shadows  = {};
  Span<Glyph const>         glyphs         = {};
  Span<Underline const>     underlines     = {};
  Span<Strikethrough const> strikethroughs = {};
  Span<Highlight const>     highlights     = {};
  Span<Caret const>         carets         = {};
};

struct TextRenderInfo
{
  /// @brief carets to draw
  f32x2 center = {};

  /// @brief carets to draw
  f32x4x4 transform = {};

  /// @brief clip rect for culling draw commands of the text block
  CRect clip = {};

  /// @brief Text Block to be rendered
  TextBlock block = {};

  /// @brief styling of the text block, contains styling for the runs and alignment of the block
  TextBlockStyle style = {};

  /// @brief caret highlights to draw. Overlapping highlights should be
  /// merged as the performance cost increases with increasing number of highlights
  Span<Slice const> highlights = {};

  /// @brief carets to draw
  Span<usize const> carets = {};
};

typedef Fn<void(TextRenderInfo const &, TextPlacement const &)> TextRenderer;

/// @brief Cached/pre-computed text layout
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
  bool            laid_out;
  f32             max_width;
  usize           num_carets;
  usize           num_codepoints;
  f32x2           extent;
  Vec<GlyphShape> glyphs;
  Vec<TextRun>    runs;
  Vec<Line>       lines;
  Vec<Paragraph>  paragraphs;

  explicit TextLayout(Allocator allocator) :
    laid_out{false},
    max_width{0},
    num_carets{0},
    num_codepoints{0},
    extent{},
    glyphs{allocator},
    runs{allocator},
    lines{allocator},
    paragraphs{allocator}
  {
  }

  TextLayout(TextLayout const &)             = delete;
  TextLayout & operator=(TextLayout const &) = delete;
  TextLayout(TextLayout &&)                  = default;
  TextLayout & operator=(TextLayout &&)      = default;
  ~TextLayout()                              = default;

  void clear()
  {
    laid_out       = false;
    max_width      = 0;
    num_carets     = 0;
    num_codepoints = 0;
    extent         = f32x2{0, 0};
    glyphs.clear();
    runs.clear();
    lines.clear();
    paragraphs.clear();
  }

  isize to_caret(usize codepoint, bool before) const;

  isize align_caret(CaretAlignment alignment) const;

  Slice get_caret_selection(Slice carets) const;

  Slice to_caret_selection(Slice codepoints) const;

  CaretCodepoint get_caret_codepoint(usize caret) const;

  CaretPlacement get_caret_placement(usize caret) const;

  /// @brief Given a position in the laid-out text return the caret the cursor points
  /// to and its location.
  /// @param pos relative position in laid-out text to hit
  Tuple<isize, CaretAlignment>
    hit(TextBlock const & block, TextBlockStyle const & style, f32x2 pos) const;

  /// @brief Render Text using pre-computed layout
  /// @param renderer the renderer to use for rendering the text's regions
  void render(TextRenderer renderer, TextRenderInfo const & info,
              Allocator scratch) const;
};

}    // namespace ash
