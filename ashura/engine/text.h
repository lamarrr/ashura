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

/// @param font font to use to render the text
/// @param shadow_scale relative. multiplied by font_height
/// @param shadow_offset px. offset from center of glyph
/// @param letter_spacing px. additional letter spacing, can be negative
/// @param word_spacing px. additional word spacing, can be negative
/// @param line_height relative. multiplied by font_height
/// @param use_kerning use provided font kerning
/// @param use_ligatures use standard and contextual font ligature substitution
struct TextStyle
{
  Font font                    = nullptr;
  f32  font_height             = 20;
  f32  outline_thickness       = 0;
  f32  underline_thickness     = 0;
  f32  strikethrough_thickness = 0;
  f32  letter_spacing          = 0;
  f32  word_spacing            = 0;
  f32  line_height             = 1.2f;
  f32  shadow_scale            = 0;
  Vec2 shadow_offset           = Vec2{0, 0};
  Vec4 foreground_color[4]     = {};
  Vec4 outline_color[4]        = {};
  Vec4 shadow_color[4]         = {};
  Vec4 background_color[4]     = {};
  Vec4 underline_color[4]      = {};
  Vec4 strikethrough_color[4]  = {};
  bool use_kerning             = true;
  bool use_ligatures           = true;
};

/// A text run is a sequence of characters sharing a single property.
/// i.e. foreground color, font etc.
/// @param size byte size coverage of this run. i.e. for the first
/// run with size 20 all text within [0, 20] bytes range
/// of the text will be styled using this run
/// @param style run style to use
struct TextRun
{
  u32 size  = 0;
  u32 style = 0;
};

/// @param text utf-32-encoded text
/// @param runs each style element applies to each run range.
/// @param default_style default run styling
/// @param align text alignment
/// @param direction base text direction
/// @param language base language to use for selecting opentype features to
/// used on the text, uses default if not set
struct TextBlock
{
  Span<u32 const>       text          = {};
  Span<TextRun const>   runs          = {};
  Span<TextStyle const> styles        = {};
  TextStyle             default_style = {};
  f32                   x_align       = -1;
  TextDirection         direction     = TextDirection::LeftToRight;
  Span<char const>      language      = {};
};

/// RunSegment is a part of a text run split by groups of spacing characters
/// word contained in a run. The spacing characters translate to break
/// opportunities.
/// @param has_spacing if it has trailing spacing characters (tabs and spaces)
/// where we can break the text, this corresponds to the
/// unicode Break-After (BA)
/// @param text utf-32 text of segment
/// @param direction direction of text
/// @param style resolved run text styling
/// @param first_shaping first glyph shaping
/// @param num_shapings number of glyph shapings
/// @param width sum of advances, letter spacing & word spacing
struct TextRunSegment
{
  bool            has_spacing   = false;
  Span<u32 const> text          = {};
  TextDirection   direction     = TextDirection::LeftToRight;
  u32             style         = 0;
  u32             first_shaping = 0;
  u32             num_shapings  = 0;
  f32             width         = 0;
};

/// @param width width of the line
/// @param ascent maximum ascent of all the runs on the line
/// @param descent maximum descent of all the runs on the line
/// @param line_height maximum line height of all the runs on the line
/// @param base_direction base direction of the line
/// @param first_segment begin index of line's run segments
/// @param num_segments number of run segments
struct LineMetrics
{
  f32           width          = 0;
  f32           ascent         = 0;
  f32           descent        = 0;
  f32           line_height    = 0;
  TextDirection base_direction = TextDirection::LeftToRight;
  u32           first_segment  = 0;
  u32           num_segments   = 0;
};

/// @param advance context-dependent horizontal-layout advance
/// @param offset context-dependent text shaping offset from normal font glyph
/// position
struct GlyphShaping
{
  u32  glyph   = 0;
  u32  cluster = 0;
  f32  advance = 0;
  Vec2 offset  = {};
};

/// @brief cached/pre-computed text layout
/// @param span extent of the whole text block
/// @param text_scale_factor scale of the text block relative to the size of the
/// pre-rendered text
struct TextLayout
{
  Span<LineMetrics const>    lines             = {};
  Span<TextRunSegment const> run_segments      = {};
  Span<GlyphShaping const>   glyph_shapings    = {};
  f32                        max_line_width    = 0;
  Vec2                       span              = {};
  f32                        text_scale_factor = 0;
};

void layout_text(TextBlock const &block, f32 text_scale_factor,
                 f32 max_line_width, TextLayout &layout);

}        // namespace ash