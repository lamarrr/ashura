/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/engine/types.h"
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

/// @param font font to use to render the text
/// @param word_spacing px. additional word spacing, can be negative
/// @param line_height relative. multiplied by font_height
/// @param letter_spacing px
struct FontStyle
{
  Font font        = nullptr;
  f32  font_height = 20;
  f32  line_height = 1.2f;
};

/// @param shadow_scale relative. multiplied by font_height
/// @param shadow_offset px. offset from center of glyph
struct TextStyle
{
  f32           underline_thickness     = 0;
  f32           strikethrough_thickness = 0;
  f32           shadow_scale            = 0;
  Vec2          shadow_offset           = Vec2{0, 0};
  ColorGradient foreground              = {};
  ColorGradient background              = {};
  ColorGradient underline               = {};
  ColorGradient strikethrough           = {};
  ColorGradient shadow                  = {};
};

/// @param text utf-32-encoded text
/// @param runs end offset of each text run
/// @param fonts font style of each text run
/// @param align text alignment
/// @param direction base text direction
/// @param language base language to use for selecting opentype features to
/// used on the text, uses default if not set
/// @param use_kerning use provided font kerning
/// @param use_ligatures use standard and contextual font ligature substitution
struct TextBlock
{
  Span<u32 const>       text          = {};
  Span<u32 const>       runs          = {};
  Span<FontStyle const> fonts         = {};
  TextDirection         direction     = TextDirection::LeftToRight;
  Span<char const>      language      = {};
  bool                  use_kerning   = true;
  bool                  use_ligatures = true;
};

/// @param styles styles for each run in the source text
/// @param align_width width to align the text block to.
/// @param alignment alignment of the text to its base direction. -1 = line
/// start, +1 = line end, 0.5 = center of line, the direction depends on the
/// directionality of the line.
struct TextBlockStyle
{
  Span<TextStyle const> runs        = {};
  f32                   alignment   = 0;
  f32                   align_width = 0;
};

/// @param cluster unicode grapheme cluster within the text run
/// @param advance context-dependent horizontal-layout advance
/// @param offset context-dependent text shaping offset from normal font glyph
/// position, i.e. offset from Glyph::bearing
struct GlyphShape
{
  u32   glyph   = 0;
  u32   cluster = 0;
  Vec2I advance = {};
  Vec2I offset  = {};
};

/// @param style the text/font style of the current run
/// @param script script of the current codepoint
/// @param paragraph_begin if this codepoint marks the beginning of a new
/// paragraph
/// @param paragraph_end if this codepoint marks the end of a paragraph
/// @param base_level the current paragraph's embedding level
/// @param level embedding level of the current codepoint in the paragraph
/// @param breakable if this codepoint begins a breakable text, i.e. has spaces
/// or tabs before it
struct alignas(4) TextSegment
{
  u16        style               = 0;
  TextScript script              = TextScript::None;
  bool       paragraph_begin : 1 = false;
  bool       paragraph_end : 1   = false;
  bool       breakable : 1       = false;
  u8         base_level          = 0;
  u8         level               = 0;
};

struct TextRunMetrics
{
  i32 advance = 0;
  i32 ascent  = 0;
  i32 descent = 0;
};

/// @param first index of first codepoint in the source text
/// @param count number of codepoints the run spans in the source text
/// @param font font-style in the list of specified fonts
/// @param paragraph if the run is at the beginning of a paragraph
/// @param breakable if the run represents a break-opportunity as constrained by
/// the max-width.
struct TextRun
{
  u32            first_codepoint = 0;
  u32            num_codepoints  = 0;
  u16            style           = 0;
  f32            font_height     = 0;
  f32            line_height     = 0;
  u32            first_glyph     = 0;
  u32            num_glyphs      = 0;
  TextRunMetrics metrics         = {};
  u8             base_level      = 0;
  u8             level           = 0;
  bool           paragraph       = false;
  bool           breakable       = false;
};

/// @param width width of the line
/// @param height maximum line height of all the runs on the line
/// @param ascent maximum ascent of all the runs on the line
/// @param descent maximum descent of all the runs on the line
/// @param direction base direction of the line
struct LineMetrics
{
  f32 width   = 0;
  f32 height  = 0;
  f32 ascent  = 0;
  f32 descent = 0;
  u8  level   = 0;
};

/// @brief
/// @param paragraph if the new line is a new paragraph
struct Line
{
  u32         first_codepoint = 0;
  u32         num_codepoints  = 0;
  u32         first_run       = 0;
  u32         num_runs        = 0;
  LineMetrics metrics         = {};
  bool        paragraph       = false;
};

struct TextHitResult
{
  u32 cluster = 0;
  u32 line    = 0;
  u32 column  = 0;
};

/// @brief cached/pre-computed text layout
/// @param max_width maximum width the text was laid out with
/// @param extent current extent of the text block after layout
/// @param segments each segment matches a codepoint in the source text.
/// @param glyphs laid-out glyphs for all the text. it is re-usable and
/// independent of the font style as long as the font matches.
/// @param lines lines in the text as constrained by max_width and paragraphs
/// found in the text.
struct TextLayout
{
  f32              max_width = 0;
  Vec2             extent    = {};
  Vec<TextSegment> segments  = {};
  Vec<GlyphShape>  glyphs    = {};
  Vec<TextRun>     runs      = {};
  Vec<Line>        lines     = {};

  void clear()
  {
    max_width = F32_MAX;
    extent    = Vec2{0, 0};
    segments.clear();
    glyphs.clear();
    runs.clear();
    lines.clear();
  }

  void uninit()
  {
    segments.uninit();
    glyphs.uninit();
    runs.uninit();
    lines.uninit();
  }

  void reset()
  {
    clear();
    segments.reset();
    glyphs.reset();
    runs.reset();
    lines.reset();
  }
};

constexpr TextDirection level_to_direction(u8 level)
{
  return ((level & 0x1) == 0) ? TextDirection::LeftToRight :
                                TextDirection::RightToLeft;
}

void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout);

/// @brief given a position in the laid-out text return the location of the
/// grapheme the cursor points to. returns the last column if the position
/// overlaps with the row and returns the last line if no overlap was found.
/// @param pos position in laid-out text to return from.
TextHitResult hit_text(TextLayout const &layout, Vec2 pos);

}        // namespace ash