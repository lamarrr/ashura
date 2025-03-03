/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/font.h"

namespace ash
{

Option<TextHitResult> TextLayout::hit(TextBlock const &      block,
                                      TextBlockStyle const & style,
                                      Vec2                   pos) const
{
  u32 const num_lines = lines.size32();

  if (num_lines == 0)
  {
    return none;
  }

  f32 const  block_width = max(extent.x, style.align_width);
  Vec2 const block_extent{block_width, extent.y};
  f32        line_y = -block_extent.y * 0.5F;
  u32        l      = 0;

  // separated vertical and horizontal clamped hit test
  for (; l < num_lines; l++)
  {
    line_y += lines[l].metrics.height;
    if (line_y >= pos.y)
    {
      break;
    }
  }

  l = min(l, num_lines - 1);

  Line const &        ln        = lines[l];
  TextDirection const direction = level_to_direction(ln.metrics.level);
  f32 const           alignment =
    style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
  f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
               ln.metrics.width * 0.5F;

  for (u32 r = 0; r < ln.runs.span; r++)
  {
    TextRun const & run         = runs[r];
    f32 const       font_height = block.font_scale * run.font_height;
    ResolvedTextRunMetrics const run_metrics = run.metrics.resolve(font_height);
    bool const                   intersects =
      (pos.x >= cursor && pos.x <= (cursor + run_metrics.advance)) ||
      (r == ln.runs.span - 1);
    if (!intersects)
    {
      continue;
    }
    f32 glyph_cursor = cursor;
    for (u32 g = 0; g < run.glyphs.span; g++)
    {
      GlyphShape const & sh      = glyphs[run.glyphs.offset + g];
      f32 const          advance = au_to_px(sh.advance, font_height);
      bool const         intersects =
        (pos.x >= glyph_cursor && pos.x <= (glyph_cursor + advance)) ||
        (g == run.glyphs.span - 1);
      if (!intersects)
      {
        glyph_cursor += advance;
        continue;
      }
      u32 const column = (sh.cluster > ln.codepoints.offset) ?
                           (sh.cluster - ln.codepoints.offset) :
                           0;
      return TextHitResult{.cluster = sh.cluster,
                           .line    = l,
                           .column  = column,
                           .pos     = glyph_cursor};
    }
    cursor += run_metrics.advance;
  }

  u32 const column = (ln.codepoints.span == 0) ? 0 : (ln.codepoints.span - 1);
  return TextHitResult{.cluster = (u32) (ln.codepoints.offset + column),
                       .line    = l,
                       .column  = column,
                       .pos     = cursor};
}

}    // namespace ash
