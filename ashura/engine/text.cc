/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/font.h"
#include "ashura/engine/font_system_impl.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"

namespace ash
{

isize TextLayout::to_caret(usize codepoint, bool before) const
{
  CHECK(laid_out, "");

  if (codepoint == 0 && before)
  {
    return 0;
  }

  if (codepoint >= num_codepoints)
  {
    return num_carets - 1;
  }

  auto l = binary_find(lines.view(), [&](Line const & l) {
    return l.codepoints.end() > codepoint;
  });

  CHECK(!l.is_empty(), "");

  auto const & line = l[0];

  if (line.codepoints.contains(codepoint))
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
  else
  {
    // line-break codepoints are not part of the line's codepoints
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
}

isize TextLayout::align_caret(CaretAlignment alignment) const
{
  CHECK(laid_out, "");

  if (alignment.y < CaretYAlignment::First)
  {
    return 0;
  }

  if (alignment.y >= CaretYAlignment{(isize) lines.size()} ||
      alignment.y >= CaretYAlignment::Bottom)
  {
    return lines.last().carets.last();
  }

  auto const & line = lines[(usize) alignment.y];

  if (alignment.x <= CaretXAlignment::Start)
  {
    return line.carets.offset;
  }

  if ((alignment.x >= CaretXAlignment::End) ||
      ((isize) alignment.x >= (isize) line.carets.span))
  {
    return line.carets.last();
  }

  return line.carets.offset + (isize) alignment.x;
}

Slice TextLayout::get_caret_selection(Slice carets) const
{
  CHECK(laid_out, "");

  carets = carets(num_carets);

  auto line0 = binary_find(lines.view(), [&](Line const & l) {
    return l.carets.end() > carets.begin();
  });

  auto line1 = binary_find(lines.view(), [&](Line const & l) {
    return l.carets.end() >= carets.end();
  });

  auto line0_begin = carets.begin() - line0[0].carets.begin();
  auto line1_end   = carets.end() - line1[0].carets.begin();

  return Slice::range(line0[0].codepoints.offset + line0_begin,
                      line1[0].codepoints.offset + line1_end);
}

Slice TextLayout::to_caret_selection(Slice codepoints) const
{
  CHECK(laid_out, "");

  codepoints = codepoints(num_codepoints);

  if (num_codepoints == 0)
  {
    return Slice{0, 0};
  }

  auto first = to_caret(codepoints.first(), true);

  if (codepoints.is_empty())
  {
    return Slice{(usize) first, 0};
  }

  auto last = to_caret(codepoints.last(), false);

  CHECK(last >= first, "");

  return Slice::range((usize) first, (usize) (last + 1));
}

CaretCodepoint TextLayout::get_caret_codepoint(usize caret) const
{
  CHECK(laid_out, "");
  CHECK(caret <= num_carets, "");

  auto l =
    binary_find(lines.view(), [&](Line & l) { return l.carets.end() > caret; });

  l = l.is_empty() ? lines.view().slice(lines.size() - 1, 1) : l;

  auto         iln       = l.as_slice_of(lines).offset;
  auto const & ln        = l[0];
  auto         column    = caret - ln.carets.offset;
  auto         codepoint = ln.codepoints.offset + column;
  auto         after     = column >= (ln.carets.span - 1);

  return CaretCodepoint{.line = iln, .codepoint = codepoint, .after = after};
}

struct GlyphMatch
{
  usize glyph   = 0;
  usize cluster = 0;

  constexpr bool better_than(usize codepoint, GlyphMatch other,
                             TextDirection direction) const
  {
    auto const dist       = abs_diff(cluster, codepoint);
    auto const other_dist = abs_diff(other.cluster, codepoint);

    // if the current match's cluster is closer to the codepoint, select it
    if (dist < other_dist)
    {
      return true;
    }
    else if (dist > other_dist)
    {
      return false;
    }
    else
      switch (direction)
      {
        // adjust to the right-most glyph in the cluster
        case TextDirection::LeftToRight:
          return glyph > other.glyph;
        // adjust to the left-most glyph in the cluster
        case TextDirection::RightToLeft:
          return glyph < other.glyph;
        default:
          ASH_UNREACHABLE;
      }
  }
};

CaretPlacement TextLayout::get_caret_placement(usize caret) const
{
  CHECK(laid_out, "");
  auto c = get_caret_codepoint(caret);

  auto const & line = lines[c.line];

  Option<GlyphMatch> match;

  for (auto const & run : runs.view().slice(line.runs))
  {
    // find the glyph with the nearest glyph cluster to the caret's codepoint position
    for (auto [i, glyph] : enumerate(glyphs.view().slice(run.glyphs)))
    {
      GlyphMatch current{.glyph   = i + run.glyphs.offset,
                         .cluster = glyph.cluster};
      match.match(
        [&](GlyphMatch & m) {
          if (current.better_than(c.codepoint, m, run.direction()))
          {
            m = current;
          }
        },
        [&]() { match = current; });
    }
  }

  return match.match(
    [&](GlyphMatch const & m) {
      return CaretPlacement{.line = c.line, .glyph = m.glyph, .after = c.after};
    },
    [&]() {
      // special-case: might not contain any codepoints (1 caret) or matching-glyphs
      return CaretPlacement{.line = c.line, .glyph = none, .after = false};
    });
}

Tuple<isize, CaretAlignment> TextLayout::hit(TextBlock const &      block,
                                             TextBlockStyle const & style,
                                             f32x2                  pos) const
{
  CHECK(laid_out, "");

  f32x2 const block_extent{max(extent.x(), style.align_width), extent.y()};
  f32x2 const half_block_extent = 0.5F * block_extent;
  f32         ln_top            = -half_block_extent.y();
  f32         last_ln_bottom    = half_block_extent.y();
  isize       ln                = 0;

  // separated vertical and horizontal hit test
  if (pos.y() < ln_top)
  {
    ln = ISIZE_MIN;
  }
  else if (pos.y() > last_ln_bottom)
  {
    ln = ISIZE_MAX;
  }
  else
  {
    for (auto [i, line] : enumerate(lines))
    {
      auto line_bottom = ln_top + line.metrics.height;
      if (pos.y() <= line_bottom)
      {
        ln = (isize) i;
        break;
      }

      ln_top = line_bottom;
    }
  }

  if (ln < 0)
  {
    return {
      0,
      CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment::First}
    };
  }

  if (ln >= (isize) lines.size())
  {
    return {
      (isize) num_carets, CaretAlignment{.x = CaretXAlignment::Start,
                                         .y = CaretYAlignment::Bottom}
    };
  }

  auto const & line      = lines[ln];
  auto const   direction = line.metrics.direction();
  f32 const    alignment =
    style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
  f32 cursor = space_align(block_extent.x(), line.metrics.width, alignment) -
               line.metrics.width * 0.5F;

  // left of line
  if (pos.x() < cursor)
  {
    switch (direction)
    {
      case TextDirection::LeftToRight:
        return {
          (isize) line.carets.first(),
          CaretAlignment{.x = CaretXAlignment::Start,
                         .y = CaretYAlignment{ln}}
        };

      case TextDirection::RightToLeft:
        return {
          (isize) line.carets.last(),
          CaretAlignment{.x = CaretXAlignment::End, .y = CaretYAlignment{ln}}
        };

      default:
        ASH_UNREACHABLE;
    }
  }

  for (auto [irun, run] : enumerate(runs.view().slice(line.runs)))
  {
    auto const & font_style  = block.fonts[run.style];
    f32 const    font_height = block.font_scale * run.font_height;
    auto const   metrics     = run.metrics.resolve(font_height);
    auto const   direction   = run.direction();
    bool const   intersects =
      pos.x() >= cursor && pos.x() <= (cursor + metrics.advance);
    f32        glyph_cursor = cursor;
    auto const run_width =
      metrics.advance +
      (run.is_spacing() ? 0 : (block.font_scale * font_style.word_spacing));

    if (!intersects)
    {
      goto next_run;
    }

    for (auto [iglyph, sh] : enumerate(glyphs.view().slice(run.glyphs)))
    {
      f32 const  advance = au_to_px(sh.advance, font_height);
      bool const intersects =
        pos.x() >= glyph_cursor && pos.x() <= (glyph_cursor + advance);

      if (intersects)
      {
        usize codepoint = 0;

        if (direction == TextDirection::LeftToRight)
        {
          if (pos.x() <= (glyph_cursor + 0.5F * advance))
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
          if (pos.x() <= (glyph_cursor + 0.5F * advance))
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
          caret, CaretAlignment{
                                .x = CaretXAlignment{caret - (isize) line.carets.offset},
                                .y = CaretYAlignment{ln}}
        };
      }

      glyph_cursor += advance;
    }

  next_run:
    cursor += run_width;
  }

  // right of line
  switch (direction)
  {
    case TextDirection::LeftToRight:
      return {
        (isize) line.carets.last(), CaretAlignment{
                                                   .x = CaretXAlignment::End,
                                                   .y = CaretYAlignment{ln},
                                                   }
      };

    case TextDirection::RightToLeft:
      return {
        (isize) line.carets.first(),
        CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment{ln}}
      };

    default:
      ASH_UNREACHABLE;
  }
}

enum class HighlightSpan : u8
{
  None    = 0,
  Partial = 1,
  Full    = 2
};

constexpr HighlightSpan highlight_test(Span<Slice const> highlights,
                                       Slice             carets)
{
  HighlightSpan s = HighlightSpan::None;

  for (auto highlight : highlights)
  {
    if (!highlight.is_empty() && highlight.contains(carets))
    {
      return HighlightSpan::Full;
    }

    if (!highlight.is_empty() && (carets.contains(highlight.first()) ||
                                  carets.contains(highlight.last())))
    {
      s = HighlightSpan::Partial;
    }
  }

  return s;
}

void TextLayout::render(TextRenderer renderer, TextRenderInfo const & info,
                        Allocator scratch) const
{
  // [ ] merge highlight rects; highlight rect type with merging, next line, close?
  CHECK(laid_out, "");
  CHECK(info.style.runs.size() == info.block.fonts.size(), "");

  auto const  block_width = max(extent.x(), info.style.align_width);
  f32x2 const block_extent{block_width, extent.y()};

  Vec<CaretPlacement> caret_placements{scratch};

  for (auto caret : info.carets)
  {
    caret_placements.push(get_caret_placement(caret)).unwrap();
  }

  Vec<TextPlacement::Block>         blocks{scratch};
  Vec<TextPlacement::Line>          lines{scratch};
  Vec<TextPlacement::Background>    backgrounds{scratch};
  Vec<TextPlacement::GlyphShadow>   glyph_shadows{scratch};
  Vec<TextPlacement::Glyph>         glyphs{scratch};
  Vec<TextPlacement::Underline>     underlines{scratch};
  Vec<TextPlacement::Strikethrough> strikethroughs{scratch};
  Vec<TextPlacement::Highlight>     highlights{scratch};
  Vec<TextPlacement::Caret>         carets{scratch};

  f32 ln_top = -(0.5F * block_extent.y());

  blocks
    .push(TextPlacement::Block{
      .bbox{.center = {}, .extent = block_extent}
  })
    .unwrap();

  for (auto [iln, ln] : enumerate(this->lines))
  {
    auto const ln_bottom = ln_top + ln.metrics.height;
    auto const baseline =
      ln_bottom - (ln.metrics.leading() + ln.metrics.descent);
    auto const direction = ln.metrics.direction();
    // flip the alignment axis direction if it is an RTL line
    auto const alignment = info.style.alignment *
                           ((direction == TextDirection::LeftToRight) ? 1 : -1);
    f32x2 const ln_extent{ln.metrics.width, ln.metrics.height};
    f32x2 const ln_center{space_align(block_width, ln_extent.x(), alignment),
                          ln_top + 0.5F * ln_extent.y()};
    auto        cursor = ln_center.x() - 0.5F * ln_extent.x();

    CRect const ln_rect{
      .center = ln_center, .extent{ln.metrics.width, ln.metrics.height}
    };

    if (!info.clip.overlaps(
          CRect{.center = info.center, .extent = ln_rect.extent}.transform(
            transform3d_to_2d(info.transform) * translate2d(ln_rect.center))))
    {
      goto next_line;
    }

    {
      if (!info.style.caret.is_none())
      {
        f32x2 center{cursor, ln_top + 0.5F * ln.metrics.height};
        f32x2 extent{info.style.caret.thickness, ln.metrics.height};

        for (auto const & p : caret_placements)
        {
          if (p.glyph.is_none() && p.line == iln)
          {
            carets
              .push(TextPlacement::Caret{
                .bbox{.center = center, .extent = extent},
                .line   = iln,
                .column = 0, // [ ] fill
                .caret  = ln.carets.first(),
            })
              .unwrap();
          }
        }
      }

      auto ln_highlight_span = highlight_test(info.highlights, ln.carets);

      if (ln_highlight_span == HighlightSpan::Full)
      {
        f32x2 extent{
          min(max(ln_rect.extent.x(),
                  info.block.font_scale * info.style.min_highlight_width),
              block_width),
          ln_rect.extent.y()};
        f32x2 center{space_align(block_width, extent.x(), alignment),
                     ln_center.y()};

        highlights
          .push(TextPlacement::Highlight{
            .bbox{.center = center, .extent = extent},
            .line = iln
        })
          .unwrap();
      }

      for (auto [i, run] : enumerate(runs.view().slice(ln.runs)))
      {
        auto const   irun        = ln.runs.offset + i;
        auto const & font_style  = info.block.fonts[run.style];
        auto const & run_style   = info.style.runs[run.style];
        auto const   font        = sys.font->get(font_style.font);
        auto const   font_height = info.block.font_scale * run.font_height;
        auto const   metrics     = run.metrics.resolve(font_height);
        auto const   run_width =
          metrics.advance + (run.is_spacing() ? 0 :
                                                (info.block.font_scale *
                                                 font_style.word_spacing));
        auto const direction = run.direction();

        auto glyph_cursor = cursor;

        if (!run_style.background.is_transparent())
        {
          f32x2 const extent{run_width, metrics.height()};
          f32x2 const center{cursor + extent.x() * 0.5F,
                             baseline - metrics.ascent + extent.y() * 0.5F};

          backgrounds
            .push(TextPlacement::Background{
              .bbox      = {.center = center, .extent = extent},
              .line      = iln,
              .column    = i,
              .run       = irun,
              .run_style = run.style
          })
            .unwrap();
        }

        HighlightSpan run_highlight_span = HighlightSpan::None;

        if (ln_highlight_span == HighlightSpan::Partial)
        {
          run_highlight_span = highlight_test(
            info.highlights, run.carets(ln.carets, ln.codepoints));

          if (run_highlight_span == HighlightSpan::Full)
          {
            f32x2 const extent{run_width, metrics.height()};
            f32x2 const center = f32x2{cursor, ln_top} + 0.5F * extent;

            highlights
              .push(TextPlacement::Highlight{
                .bbox{.center = center, .extent = extent},
                .line = iln
            })
              .unwrap();
          }
        }

        if (run_style.strikethrough_thickness != 0)
        {
          f32x2 const extent{run_width, info.block.font_scale *
                                          run_style.strikethrough_thickness};
          f32x2 const center =
            f32x2{cursor, baseline - metrics.ascent * 0.5F} + extent * 0.5F;

          strikethroughs
            .push(TextPlacement::Strikethrough{
              .bbox{.center = center, .extent = extent},
              .line      = iln,
              .column    = i,
              .run       = irun,
              .run_style = run.style
          })
            .unwrap();
        }

        if (run_style.underline_thickness != 0)
        {
          f32x2 const extent{run_width, info.block.font_scale *
                                          run_style.underline_thickness};
          f32x2 const center =
            f32x2{cursor, baseline + info.block.font_scale *
                                       run_style.underline_offset} +
            extent * 0.5F;

          underlines
            .push(TextPlacement::Underline{
              .bbox{.center = center, .extent = extent},
              .line      = iln,
              .column    = i,
              .run       = irun,
              .run_style = run.style
          })
            .unwrap();
        }

        for (auto [i, sh] : enumerate(this->glyphs.view().slice(run.glyphs)))
        {
          auto const           iglyph = run.glyphs.offset + i;
          GlyphMetrics const & m      = font.glyphs[sh.glyph];
          f32x2 const          extent = au_to_px(m.extent, font_height);
          f32x2 const          center = f32x2{glyph_cursor, baseline} +
                               au_to_px(m.bearing, font_height) +
                               au_to_px(sh.offset, font_height) + 0.5F * extent;
          auto const advance = au_to_px(sh.advance, font_height);

          // before and after carets
          auto const glyph_carets =
            Slice{ln.carets.offset + (sh.cluster - ln.codepoints.offset), 1};

          if (run_style.has_shadow())
          {
            f32x2 const shadow_extent = extent * run_style.shadow_scale;
            f32x2 const shadow_center =
              center + info.block.font_scale * run_style.shadow_offset;

            glyph_shadows
              .push(TextPlacement::GlyphShadow{
                .bbox{shadow_center, shadow_extent},
                .line      = iln,
                .column    = i,
                .run       = irun,
                .run_style = run.style,
                .glyph     = iglyph,
                .cluster   = sh.cluster
            })
              .unwrap();
          }

          if (run_style.has_color())
          {
            glyphs
              .push(TextPlacement::Glyph{
                .bbox{center, extent},
                .line      = iln,
                .column    = i,
                .run       = irun,
                .run_style = run.style,
                .glyph     = iglyph,
                .cluster   = sh.cluster
            })
              .unwrap();
          }

          if (!info.style.caret.is_none())
          {
            auto const glyph_left  = glyph_cursor;
            auto const glyph_right = glyph_cursor + advance;

            for (auto const [c, p] : zip(info.carets, caret_placements))
            {
              if (p.glyph == iglyph)
              {
                f32x2 extent{info.style.caret.thickness, ln.metrics.height};
                f32x2 center;

                if ((direction == TextDirection::LeftToRight && p.after) ||
                    (direction == TextDirection::RightToLeft && !p.after))
                {
                  center.x() = glyph_right;
                }
                else
                {
                  center.x() = glyph_left;
                }

                center.y() = ln_top + 0.5F * ln.metrics.height;

                carets
                  .push(TextPlacement::Caret{
                    .bbox{center, extent},
                    .line   = iln,
                    .column = c - ln.carets.first(),
                    .caret  = c
                })
                  .unwrap();
              }
            }
          }

          if (run_highlight_span == HighlightSpan::Partial)
          {
            auto glyph_highlight_span =
              highlight_test(info.highlights, glyph_carets);

            if (glyph_highlight_span != HighlightSpan::None)
            {
              f32x2 const extent{advance, metrics.height()};
              f32x2 const center = f32x2{glyph_cursor, ln_top} + 0.5F * extent;

              highlights
                .push(TextPlacement::Highlight{
                  .bbox{.center = center, .extent = extent},
                  .line = iln
              })
                .unwrap();
            }
          }

          glyph_cursor += advance;
        }

        cursor += run_width;
      }
    }

  next_line:
    ln_top = ln_bottom;
  }

  auto placement = TextPlacement{.blocks         = blocks,
                                 .lines          = lines,
                                 .backgrounds    = backgrounds,
                                 .glyph_shadows  = glyph_shadows,
                                 .glyphs         = glyphs,
                                 .underlines     = underlines,
                                 .strikethroughs = strikethroughs,
                                 .highlights     = highlights,
                                 .carets         = carets};

  renderer(info, placement);
}

}    // namespace ash
