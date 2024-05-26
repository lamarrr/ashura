#pragma once

#include "ashura/engine/font.h"
#include "ashura/std/types.h"
#include "ashura/std/unicode.h"

namespace ash
{

enum class TextDirection : u8
{
  LeftToRight = 0,
  RightToLeft = 1
};

enum class TextAlign : u8
{
  Start  = 0,
  Center = 1,
  End    = 2
};

enum class TextOverflow : u8
{
  Wrap     = 0,
  Ellipsis = 1
};

/// @font: name to use to match the font. if font is not found or empty
/// the fallback fonts are tried.
/// @fallback_fonts: font to fallback to if {font} is not available. if none of
/// the specified fallback fonts are found the first font in the
/// font bundle will be used
/// @shadow_scale: relative. multiplied by font_height
/// @shadow_offset: px. offset from center of glyph
/// @letter_spacing: px. additional letter spacing, can be negative
/// @word_spacing: px. additional word spacing, can be negative
/// @line_height: relative. multiplied by font_height
/// @use_kerning: use provided font kerning
/// @use_ligatures: use standard and contextual font ligature substitution
struct TextStyle
{
  Font             font                    = nullptr;
  Span<Font const> fallback_fonts          = {};
  f32              font_height             = 20;
  f32              outline_thickness       = 0;
  f32              underline_thickness     = 0;
  f32              strikethrough_thickness = 0;
  f32              letter_spacing          = 0;
  f32              word_spacing            = 0;
  f32              line_height             = 1.2f;
  f32              shadow_scale            = 0;
  Vec2             shadow_offset           = Vec2{0, 0};
  Vec4             foreground_color        = Vec4{0, 0, 0, 1};
  Vec4             outline_color           = Vec4{0, 0, 0, 1};
  Vec4             shadow_color            = Vec4{0, 0, 0, 1};
  Vec4             background_color        = Vec4{0, 0, 0, 0};
  Vec4             underline_color         = Vec4{0, 0, 0, 1};
  Vec4             strikethrough_color     = Vec4{0, 0, 0, 1};
  bool             use_kerning             = true;
  bool             use_ligatures           = true;
};

/// A text run is a sequence of characters sharing a single property.
/// i.e. foreground color, font etc.
/// @size: byte size coverage of this run. i.e. for the first
/// run with size 20 all text within [0, 20] bytes range
/// of the text will be styled using this run
/// @style: run style to use
struct TextRun
{
  usize size  = 0;
  usize style = 0;
};

/// @text: utf-8-encoded text, Span because string view
/// doesnt support non-string types
/// @runs: parts of text not styled by a run
/// will use the paragraphs run style
/// styles for the text block's contents
/// @default_style: default run styling
/// @align: text alignment
/// @direction: base text direction
/// @language: base language to use for selecting opentype features to
/// used on the text, uses default if not set
struct TextBlock
{
  Span<char const>      text          = {};
  Span<TextRun const>   runs          = {};
  Span<TextStyle const> styles        = {};
  TextStyle             default_style = {};
  TextAlign             align         = TextAlign::Start;
  TextDirection         direction     = TextDirection::LeftToRight;
  Span<char const>      language      = {};
};

/// RunSegment is a part of a text run split by groups of spacing characters
/// word contained in a run. The spacing characters translate to break
/// opportunities.
/// @has_spacing: if it has trailing spacing characters (tabs and spaces)
/// where we can break the text, this corresponds to the
/// unicode Break-After (BA)
/// @text: utf8 text of segment
/// @direction: direction of text
/// @style: resolved run text styling
/// @font: resolved font index in font bundle
/// @width: sum of advances, letter spacing & word spacing
struct TextRunSegment
{
  bool             has_spacing           = false;
  Span<char const> text                  = {};
  TextDirection    direction             = TextDirection::LeftToRight;
  usize            style                 = 0;
  usize            font                  = 0;
  usize            glyph_shapings_offset = 0;
  usize            nglyph_shapings       = 0;
  f32              width                 = 0;
};

/// @width: width of the line
/// @ascent: maximum ascent of all the runs on the line
/// @descent: maximum descent of all the runs on the line
/// @line_height: maximum line height of all the runs on the line
/// @base_direction: base direction of the line
/// @run_segments_offset: begin index of line's segments
/// @nrun_segments: number of segments
struct LineMetrics
{
  f32           width               = 0;
  f32           ascent              = 0;
  f32           descent             = 0;
  f32           line_height         = 0;
  TextDirection base_direction      = TextDirection::LeftToRight;
  usize         run_segments_offset = 0;
  usize         nrun_segments       = 0;
};

/// @advance: context-dependent horizontal-layout advance
/// @offset: context-dependent text shaping offset from normal font glyph
/// position
struct GlyphShaping
{
  u32  glyph   = 0;
  u32  cluster = 0;
  f32  advance = 0;
  Vec2 offset  = {};
};

/// @brief Cached/pre-computed text layout
struct TextLayout
{
  Span<LineMetrics const>    lines             = {};
  Span<TextRunSegment const> run_segments      = {};
  Span<GlyphShaping const>   glyph_shapings    = {};
  f32                        max_line_width    = 0;
  Vec2                       span              = {};
  f32                        text_scale_factor = 0;
};

}        // namespace ash