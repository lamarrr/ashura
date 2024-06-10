#pragma once

#include "ashura/engine/font.h"
#include "ashura/std/types.h"
#include "ashura/std/unicode.h"
#include "ashura/std/vec.h"

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
  f32  letter_spacing          = 0;
  f32  line_height             = 1.2f;
  f32  outline_thickness       = 0;
  f32  underline_thickness     = 0;
  f32  strikethrough_thickness = 0;
  f32  shadow_scale            = 0;
  Vec2 shadow_offset           = Vec2{0, 0};
  Vec4 foreground_color[4]     = {};
  Vec4 background_color[4]     = {};
  Vec4 outline_color[4]        = {};
  Vec4 underline_color[4]      = {};
  Vec4 strikethrough_color[4]  = {};
  Vec4 shadow_color[4]         = {};
  bool use_kerning             = true;
  bool use_ligatures           = true;
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
  Span<u32 const>       text      = {};
  Span<Segment const>   segments  = {};
  Span<TextStyle const> styles    = {};
  f32                   x_align   = -1;
  TextDirection         direction = TextDirection::LeftToRight;
  Span<char const>      language  = {};
};

/// @param width width of the line
/// @param ascent maximum ascent of all the runs on the line
/// @param descent maximum descent of all the runs on the line
/// @param line_height maximum line height of all the runs on the line
/// @param base_direction base direction of the line
struct LineMetrics
{
  f32           width          = 0;
  f32           ascent         = 0;
  f32           descent        = 0;
  f32           line_height    = 0;
  TextDirection base_direction = TextDirection::LeftToRight;
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

/// @param script script of the current codepoint, OpenType (ISO15924) Script
/// Tag. See: https://unicode.org/reports/tr24/#Relation_To_ISO15924
/// @param paragraph if the current codepoint begins a paragraph
/// @param paragraph_direction the current paragraph's direction
/// @param direction directionality of the current codepoint in the paragraph
/// @param breakable if this codepoint begins a breakable text
struct TextSegment
{
  u32           script             = 0;
  bool          paragraph : 1      = 0;
  TextDirection base_direction : 1 = TextDirection::LeftToRight;
  TextDirection direction : 1      = TextDirection::LeftToRight;
  bool          breakable : 1      = false;
};

/// @brief cached/pre-computed text layout
/// @param scale scale of the text block relative to the size of the
/// pre-rendered text
/// @param segments each segment matches a codepoint in the source text.
struct TextLayout
{
  f32               max_width = 0;
  Vec2              extent    = {};
  Vec<TextSegment>  segments  = {};
  Vec<GlyphShaping> shapings  = {};
  Vec<LineMetrics>  lines     = {};

  void clear();
};

void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout);

}        // namespace ash