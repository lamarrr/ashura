#pragma once

#include "ashura/engine/font.h"
#include "ashura/std/types.h"
#include "ashura/std/unicode.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class TextWrap : u8
{
  Wrap     = 1,
  Ellipsis = 2
};

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
  Font font           = nullptr;
  f32  font_height    = 20;
  f32  line_height    = 1.2f;
  f32  letter_spacing = 0;
};

/// @param shadow_scale relative. multiplied by font_height
/// @param shadow_offset px. offset from center of glyph
struct TextStyle
{
  f32  underline_thickness     = 0;
  f32  strikethrough_thickness = 0;
  f32  shadow_scale            = 0;
  Vec2 shadow_offset           = Vec2{0, 0};
  Vec4 foreground_color[4]     = {};
  Vec4 background_color[4]     = {};
  Vec4 underline_color[4]      = {};
  Vec4 strikethrough_color[4]  = {};
  Vec4 shadow_color[4]         = {};
};

/// @param first first codepoint in the text
/// @param count last codepoint in the text
struct TextStyleRun
{
  u32 first = 0;
  u32 count = 0;
  u32 style = 0;
};

/// @param text utf-32-encoded text
/// @param runs length of each text run
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
  f32                   x_align       = -1;
  TextDirection         direction     = TextDirection::LeftToRight;
  TextWrap              wrap          = TextWrap::Wrap;
  Span<char const>      language      = {};
  bool                  use_kerning   = true;
  bool                  use_ligatures = true;
};

struct StyledTextBlock
{
  TextBlock                block  = {};
  Span<TextStyle const>    styles = {};
  Span<TextStyleRun const> runs   = {};
};

/// @param cluster unicode grapheme cluster within the text run
/// @param advance context-dependent horizontal-layout advance
/// @param offset context-dependent text shaping offset from normal font glyph
/// position
struct GlyphShape
{
  u32   glyph   = 0;
  u32   cluster = 0;
  Vec2I advance = {};
  Vec2I offset  = {};
};

/// @param script script of the current codepoint
/// @param paragraph if the current codepoint begins a paragraph
/// @param paragraph_direction the current paragraph's direction
/// @param direction directionality of the current codepoint in the paragraph
/// @param breakable if this codepoint begins a breakable text
struct alignas(4) TextSegment
{
  u16           font               = 0;
  TextScript    script             = TextScript::None;
  bool          paragraph : 1      = false;
  bool          breakable : 1      = false;
  TextDirection base_direction : 1 = TextDirection::LeftToRight;
  TextDirection direction : 1      = TextDirection::LeftToRight;
};

struct TextRunMetrics
{
  f32 advance     = 0;
  f32 ascent      = 0;
  f32 descent     = 0;
  f32 font_height = 0;
  f32 line_height = 0;
};

struct TextRun
{
  u32            first          = 0;
  u32            count          = 0;
  u16            font           = 0;
  u32            first_glyph    = 0;
  u32            num_glyphs     = 0;
  TextRunMetrics metrics        = {};
  TextDirection  base_direction = TextDirection::LeftToRight;
  TextDirection  direction      = TextDirection::LeftToRight;
  bool           paragraph      = false;
  bool           breakable      = false;
};

/// @param advance width of the line
/// @param ascent maximum ascent of all the runs on the line
/// @param descent maximum descent of all the runs on the line
/// @param line_height maximum line height of all the runs on the line
/// @param base_direction base direction of the line
struct LineMetrics
{
  f32           advance        = 0;
  f32           ascent         = 0;
  f32           descent        = 0;
  f32           line_height    = 0;
  TextDirection base_direction = TextDirection::LeftToRight;
};

struct Line
{
  LineMetrics metrics   = {};
  u32         first_run = 0;
  u32         num_runs  = 0;
  bool        paragraph = false;
};

/// @brief cached/pre-computed text layout
/// @param scale scale of the text block relative to the size of the
/// pre-rendered text
/// @param segments each segment matches a codepoint in the source text.
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
    extent    = Vec2{0, 0};
    max_width = F32_MAX;
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

void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout);

}        // namespace ash