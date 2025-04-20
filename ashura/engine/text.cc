/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/font.h"

namespace ash
{

Tuple<isize, CaretLocation> TextLayout::hit(TextBlock const &      block,
                                            TextBlockStyle const & style,
                                            Vec2                   pos) const
{
  if (lines.is_empty())
  {
    return {0, CaretLocation{}};
  }

  Vec2 const block_extent{max(extent.x, style.align_width), extent.y};
  Vec2 const half_block_extent = 0.5F * block_extent;
  f32        line_top          = -half_block_extent.y;
  isize      ln                = 0;

  // separated vertical and horizontal hit test
  if (pos.y < -half_block_extent.y)
  {
    ln = ISIZE_MIN;
  }
  else if (pos.y > half_block_extent.y)
  {
    ln = ISIZE_MAX;
  }
  else
  {
    for (auto [i, line] : enumerate(lines))
    {
      line_top += line.metrics.height;
      if (pos.y <= line_y)
      {
        ln = (isize) i;
        break;
      }
    }
  }

  if (ln < 0)
  {
    return {
      0, CaretLocation{.line      = LineAlignment::Top,
                       .alignment = CaretAlignment::LineStart}
    };
  }

  if (ln >= (isize) lines.size())
  {
    return {
      (isize) num_carets,
      CaretLocation{.line      = LineAlignment::Bottom,
                    .alignment = CaretAlignment::LineStart}
    };
  }

  Line const &        line      = lines[ln];
  TextDirection const direction = level_to_direction(line.metrics.level);
  f32 const           alignment =
    style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
  f32 cursor = space_align(block_extent.x, line.metrics.width, alignment) -
               line.metrics.width * 0.5F;

  // left of line
  if (pos.x < cursor)
  {
    switch (direction)
    {
      case TextDirection::LeftToRight:
        return {
          (isize) line.carets.first(),
          CaretLocation{.line      = LineAlignment{ln},
                        .alignment = CaretAlignment::LineStart}
        };

      case TextDirection::RightToLeft:
        return {
          (isize) line.carets.last(),
          CaretLocation{.line      = LineAlignment{ln},
                        .alignment = CaretAlignment::LineEnd}
        };

      default:
        ASH_UNREACHABLE;
    }
  }

  for (auto [irun, run] : enumerate(runs.view().slice(line.runs)))
  {
    f32 const  font_height = block.font_scale * run.font_height;
    auto const run_metrics = run.metrics.resolve(font_height);
    auto const direction   = run.direction();

    bool const intersects =
      pos.x >= cursor && pos.x <= (cursor + run_metrics.advance);

    if (!intersects)
    {
      cursor += run_metrics.advance;
      continue;
    }

    f32 glyph_cursor = cursor;

    for (auto [iglyph, sh] : enumerate(glyphs.view().slice(run.glyphs)))
    {
      f32 const advance = au_to_px(sh.advance, font_height);

      bool const intersects =
        pos.x >= glyph_cursor && pos.x <= (glyph_cursor + advance);

      if (!intersects)
      {
        glyph_cursor += advance;
        continue;
      }

      CHECK(line.codepoints.contains(sh.cluster), "");

      usize codepoint = 0;

      if (direction == TextDirection::LeftToRight)
      {
        if (pos.x <= (glyph_cursor + 0.5F * advance))
        {
          codepoint = sh.cluster;
        }
        else
        {
          codepoint = sh.cluster + 1;
        }
      }
      else
      {
        if (pos.x <= (glyph_cursor + 0.5F * advance))
        {
          codepoint = sh.cluster + 1;
        }
        else
        {
          codepoint = sh.cluster;
        }
      }

      isize caret = line.carets.offset + (codepoint - line.codepoints.offset);

      return {
        caret, CaretLocation{.line      = LineAlignment{ln},
                             .alignment = CaretAlignment{
                               caret - (isize) line.carets.offset}}
      };
    }
  }

  // right of line
  switch (direction)
  {
    case TextDirection::LeftToRight:
      return {
        (isize) line.carets.last(),
        CaretLocation{.line      = LineAlignment{ln},
                      .alignment = CaretAlignment::LineEnd}
      };

    case TextDirection::RightToLeft:
      return {
        (isize) line.carets.first(),
        CaretLocation{.line      = LineAlignment{ln},
                      .alignment = CaretAlignment::LineStart}
      };

    default:
      ASH_UNREACHABLE;
  }
}

// [ ] IT DOESN'T CROSS LINES SMALLER THAN THE ALIGNED LINE; select up
Option<CaretCodepoint> TextLayout::get_caret_codepoint(isize caret) const
{
  CHECK(caret > 0, "");
  auto caret_u = (usize) caret;
  auto l       = binary_find(lines.view(),
                             [&](Line & l) { return l.carets.end() > caret_u; });

  if (l.is_empty())
  {
    return none;
  }

  auto         iline = l.as_slice_of(lines).offset;
  auto const & line  = l[0];
  auto codepoint     = line.codepoints.offset + (caret_u - line.carets.offset);
  auto after         = line.carets.last() >= codepoint;

  return CaretCodepoint{.line = iline, .codepoint = codepoint, .after = after};
}

Option<CaretGlyph> TextLayout::get_caret_glyph(isize caret) const
{
  // [ ] impl
  return get_caret_codepoint(caret).map([&](CaretCodepoint c) {
    auto & line = lines[c.line];

    /*
// find the glyphs with the nearest grapheme cluster to the caret position.
              // [ ] RTL- alignment

              auto const glyph = run.glyphs.offset + i;
              auto const base_codepoint =
                (usize) (icaret <= 0 ? 0 : (icaret - 1));

              auto alignment = (direction == TextDirection::LeftToRight) ?
                                 ALIGNMENT_RIGHT :
                                 ALIGNMENT_LEFT;

              if (icaret == 0)
              {
                alignment *= -1;
              }

              Vec2 const caret_pos{(glyph_cursor + 0.5F * advance) +
                                     space_align(advance, 0.0F, alignment),
                                   line_y};

              CaretHitResult const result{.glyph   = glyph,
                                          .cluster = sh.cluster,
                                          .pos     = caret_pos,
                                          .height  = ln.metrics.height};

              caret_hit.match(
                [&](auto & h) {
                  // multiple glyphs might merge to the same grapheme cluster, we need to select the closest glyph based on text direction.
                  switch (cmp(abs_diff(base_codepoint, sh.cluster),
                              abs_diff(base_codepoint, h.cluster)))
                  {
                    case Ordering::Less:
                    {
                      caret_hit = result;
                    }
                    break;

                    case Ordering::Equal:
                    {
                      // based on the run direction, select the glyph within that cluster
                      switch (direction)
                      {
                        case TextDirection::LeftToRight:
                        {
                          if (glyph > h.glyph)
                          {
                            caret_hit = result;
                          }
                        }
                        break;
                        case TextDirection::RightToLeft:
                        {
                          if (glyph < h.glyph)
                          {
                            caret_hit = result;
                          }
                        }
                        break;
                      }
                    }
                    break;

                    default:
                      break;
                  }*/
    return CaretGlyph{};
  });
}

/// caret0      caret1      caret2      caret3      caret4
///    |   0       |   1       |     2     |   3       |
Option<isize> TextLayout::to_caret(usize codepoint, bool before) const
{
  auto l = binary_find(lines.view(), [&](Line const & l) {
    return l.break_codepoints.end() > codepoint ||
           l.codepoints.end() > codepoint;
  });

  if (l.is_empty())
  {
    return none;
  }

  auto const & line = l[0];

  CHECK(line.break_codepoints.contains(codepoint) ||
          line.codepoints.contains(codepoint),
        "");

  if (line.break_codepoints.contains(codepoint))
  {
    if (before)
    {
      CHECK(l.data() > lines.data(), "");
      // adjust to the caret of the previous line
      return (l.data() - 1)[0].carets.last();
    }
    else
    {
      // adjust to the first caret of the current line
      return line.carets.offset;
    }
  }
  else
  {
    auto left_caret = line.carets.offset + (codepoint - line.codepoints.offset);
    if (before)
    {
      return left_caret;
    }
    else
    {
      return left_caret + 1;
    }
  }
}

isize TextLayout::align_caret(CaretLocation loc) const
{
  if (lines.is_empty() || loc.line < LineAlignment::First)
  {
    return 0;
  }

  if (loc.line >= LineAlignment{(isize) lines.size()} ||
      loc.line >= LineAlignment::Bottom)
  {
    return lines.last().carets.last();
  }

  auto const & line = lines[(usize) loc.line];

  if (loc.alignment <= CaretAlignment::LineStart)
  {
    return line.carets.offset;
  }

  if (loc.alignment >= CaretAlignment::LineEnd ||
      (isize) loc.alignment >= (isize) line.carets.span)
  {
    return line.carets.last();
  }

  return line.carets.offset + (isize) loc.alignment;
}

Slice TextLayout::get_caret_selection(Slice carets) const
{
  carets = carets(num_carets);

  if (lines.is_empty() || num_carets == 0)
  {
    return Slice{0, 0};
  }

  auto line0 = binary_find(lines.view(), [&](Line const & l) {
    return l.carets.end() > carets.begin();
  });

  auto line1 = binary_find(lines.view(), [&](Line const & l) {
    return l.carets.end() >= carets.end();
  });

  auto line0_begin = carets.begin() - line0[0].carets.begin();
  auto line1_end   = carets.end() - line1[0].carets.begin();

  return Slice::from_range(line0[0].codepoints.offset + line0_begin,
                           line1[0].codepoints.offset + line1_end);
}

Slice TextLayout::to_caret_selection(Slice codepoints) const
{
  codepoints = codepoints(num_codepoints);

  if (lines.is_empty() || num_codepoints == 0)
  {
    return Slice{0, 0};
  }

  // [ ] re-review
  auto first  = to_caret(codepoints.first(), true);
  auto ifirst = first.unwrap();
  CHECK(ifirst > 0, "");

  if (codepoints.is_empty())
  {
    return Slice{(usize) ifirst, 0};
  }

  auto last  = to_caret(codepoints.last(), false);
  auto ilast = last.unwrap();

  CHECK(ilast > 0, "");
  CHECK(ilast >= ifirst, "");

  return Slice::from_range((usize) ifirst, (usize) (ilast + 1));
}

}    // namespace ash
