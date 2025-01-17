/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/font.h"

namespace ash
{

Option<TextHitResult> hit_text(TextLayout const & layout, f32 style_align_width,
                               f32 style_alignment, Vec2 pos)
{
  u32 const num_lines = layout.lines.size32();

  if (num_lines == 0)
  {
    return none;
  }

  // TODO(lamarrr): add acceleration structure: block pos

  f32 const  block_width = max(layout.extent.x, style_align_width);
  Vec2 const block_extent{block_width, layout.extent.y};
  f32        line_y = -block_extent.y * 0.5F;
  u32        l      = 0;

  // separated vertical and horizontal clamped hit test
  for (; l < num_lines; l++)
  {
    line_y += layout.lines[l].metrics.height;
    if (line_y >= pos.y)
    {
      break;
    }
  }

  l = min(l, num_lines - 1);

  Line const &        ln        = layout.lines[l];
  TextDirection const direction = level_to_direction(ln.metrics.level);
  f32 const           alignment =
    style_alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
  f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
               ln.metrics.width * 0.5F;

  for (u32 r = 0; r < ln.num_runs; r++)
  {
    TextRun const & run = layout.runs[r];
    bool const      intersects =
      (pos.x >= cursor &&
       pos.x <= (cursor + au_to_px(run.metrics.advance, run.font_height))) ||
      (r == ln.num_runs - 1);
    if (!intersects)
    {
      continue;
    }
    f32 glyph_cursor = cursor;
    for (u32 g = 0; g < run.num_glyphs; g++)
    {
      GlyphShape const & glyph   = layout.glyphs[run.first_glyph + g];
      f32 const          advance = au_to_px(glyph.advance, run.font_height);
      bool const         intersects =
        (pos.x >= glyph_cursor && pos.x <= (glyph_cursor + advance)) ||
        (g == run.num_glyphs - 1);
      if (!intersects)
      {
        glyph_cursor += advance;
        continue;
      }
      u32 const column = (glyph.cluster > ln.first_codepoint) ?
                           (glyph.cluster - ln.first_codepoint) :
                           0;
      return TextHitResult{
        .cluster = glyph.cluster, .line = l, .column = column};
    }
    cursor += au_to_px(run.metrics.advance, run.font_height);
  }

  u32 const column = (ln.num_codepoints == 0) ? 0 : (ln.num_codepoints - 1);
  return TextHitResult{
    .cluster = ln.first_codepoint + column, .line = l, .column = column};
}

}    // namespace ash
