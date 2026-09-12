/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.hpp"
#include "ashura/engine/font.hpp"
#include "ashura/engine/font_system.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/text.hpp"
#include "ashura/std/trace.hpp"

namespace ash
{

Slice advance_paragraph(Str32 str)
{
    auto str_size = str.size();
    auto i        = 0uz;

    while (i < str_size)
    {
        if (str[i] == '\r' && ((i + 1) < str_size) && str[i + 1] == '\n')
        {
            return Slice{i, 2};
        }
        else if (str[i] == '\n' || str[i] == '\r')
        {
            return Slice{i, 1};
        }

        i++;
    }

    return Slice{i, 0};
}

Slice advance_paragraph(Str8 str)
{
    auto str_size = str.size();
    auto i        = 0uz;

    while (i < str_size)
    {
        if (str[i] == '\r' && ((i + 1) < str_size) && str[i + 1] == '\n')
        {
            return Slice{i, 2};
        }
        else if (str[i] == '\n' || str[i] == '\r')
        {
            return Slice{i, 1};
        }

        i++;
    }

    return Slice{i, 0};
}

Str32 cull_paragraphs(Str32 str, Slice paragraphs)
{
    auto iparagraph         = 0uz;
    auto str_size           = str.size();
    auto str_iter           = 0uz;
    auto str_paragraph_iter = 0uz;

    while (str_iter < str_size && iparagraph < paragraphs.offset)
    {
        auto delims        = advance_paragraph(str.slice(str_iter));
        str_paragraph_iter = str_iter + delims.offset;
        str_iter += delims.end();
        iparagraph++;
    }

    auto first_paragraph_begin = str_paragraph_iter;

    while (str_iter < str_size && iparagraph < paragraphs.end())
    {
        auto delims        = advance_paragraph(str.slice(str_iter));
        str_paragraph_iter = str_iter + delims.offset;
        str_iter += delims.end();
        iparagraph++;
    }

    auto last_paragraph_end = str_paragraph_iter;

    return str.slice(Slice::offsets(first_paragraph_begin, last_paragraph_end));
}

Str8 cull_paragraphs(Str8 str, Slice paragraphs)
{
    auto iparagraph         = 0uz;
    auto str_size           = str.size();
    auto str_iter           = 0uz;
    auto str_paragraph_iter = 0uz;

    while (str_iter < str_size && iparagraph < paragraphs.offset)
    {
        auto delims        = advance_paragraph(str.slice(str_iter));
        str_paragraph_iter = str_iter + delims.offset;
        str_iter += delims.end();
        iparagraph++;
    }

    auto first_paragraph_begin = str_paragraph_iter;

    while (str_iter < str_size && iparagraph < paragraphs.end())
    {
        auto delims        = advance_paragraph(str.slice(str_iter));
        str_paragraph_iter = str_iter + delims.offset;
        str_iter += delims.end();
        iparagraph++;
    }

    auto last_paragraph_end = str_paragraph_iter;

    return str.slice(Slice::offsets(first_paragraph_begin, last_paragraph_end));
}

void TextLayout::clear()
{
    laid_out       = false;
    max_width      = 0;
    num_carets     = 0;
    num_codepoints = 0;
    extent         = f32x2::zero();
    glyphs.clear();
    runs.clear();
    lines.clear();
    paragraphs.clear();
}

isize TextLayout::to_caret(usize codepoint, bool before) const
{
    ASH_CHECK(laid_out, "");

    if (codepoint == 0 && before)
    {
        return 0;
    }

    if (codepoint >= num_codepoints)
    {
        return num_carets - 1;
    }

    auto l = binary_find(lines.view(),
                         [&](Line & l) { return l.codepoints.end() > codepoint; });

    if (l.is_empty())
    {
        return num_carets - 1;
    }

    auto & line = l[0];

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
            ASH_CHECK(l.data() > lines.data(), "");
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
    ASH_CHECK(laid_out, "");

    if (alignment.y < CaretYAlignment::First)
    {
        return 0;
    }

    if (alignment.y >= CaretYAlignment{(isize) lines.size()} ||
        alignment.y >= CaretYAlignment::Bottom)
    {
        return lines.last().carets.last();
    }

    auto & line = lines[(usize) alignment.y];

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
    ASH_CHECK(laid_out, "");

    carets = carets(num_carets);

    auto line0 = binary_find(lines.view(),
                             [&](Line & l) { return l.carets.end() > carets.begin(); });

    auto line1 = binary_find(lines.view(),
                             [&](Line & l) { return l.carets.end() >= carets.end(); });

    auto line0_begin = carets.begin() - line0[0].carets.begin();
    auto line1_end   = carets.end() - line1[0].carets.begin();

    return Slice::offsets(line0[0].codepoints.offset + line0_begin,
                          line1[0].codepoints.offset + line1_end);
}

Slice TextLayout::to_caret_selection(Slice codepoints) const
{
    ASH_CHECK(laid_out, "");

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

    ASH_CHECK(last >= first, "");

    return Slice::offsets((usize) first, (usize) (last + 1));
}

CaretCodepoint TextLayout::get_caret_codepoint(usize caret) const
{
    ASH_CHECK(laid_out, "");
    ASH_CHECK(caret <= num_carets, "");

    auto l =
      binary_find(lines.view(), [&](Line & l) { return l.carets.end() > caret; });

    l = l.is_empty() ? lines.view().slice(lines.size() - 1, 1) : l;

    auto   iline  = l.as_slice_of(lines).offset;
    auto & line   = l[0];
    auto   column = caret - line.carets.offset;
    auto   after  = column == line.codepoints.span;

    if (line.codepoints.is_empty())
    {
        return CaretCodepoint{
          .line = iline, .codepoint = line.codepoints.offset, .after = false};
    }

    auto codepoint = line.codepoints.offset + column - (after ? 1 : 0);

    return CaretCodepoint{.line = iline, .codepoint = codepoint, .after = after};
}

struct GlyphMatch
{
    usize glyph   = 0;
    usize cluster = 0;

    constexpr bool better_than(usize codepoint, GlyphMatch other,
                               TextDirection direction) const
    {
        auto dist       = abs_diff(cluster, codepoint);
        auto other_dist = abs_diff(other.cluster, codepoint);

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
    ASH_CHECK(laid_out, "");
    auto c = get_caret_codepoint(caret);

    auto & line = lines[c.line];

    Option<GlyphMatch> match;

    for (auto & run : runs.view().slice(line.runs))
    {
        // find the glyph with the nearest glyph cluster to the caret's codepoint
        // position
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
      [&](GlyphMatch & m) {
          return CaretPlacement{.line = c.line, .glyph = m.glyph, .after = c.after};
      },
      [&]() {
          // special-case: might not contain any codepoints (1 caret) or
          // matching-glyphs
          return CaretPlacement{.line = c.line, .glyph = none, .after = false};
      });
}

Tuple<isize, CaretAlignment> TextLayout::hit(TextBlock const &      block,
                                             TextBlockStyle const & style,
                                             f32x2                  pos) const
{
    ASH_CHECK(laid_out, "");

    f32x2 block_extent{extent.x(), extent.y()};
    auto  half_block_extent = 0.5F * block_extent;
    auto  ln_top            = -half_block_extent.y();
    auto  last_ln_bottom    = half_block_extent.y();
    auto  iline             = 0z;

    // separated vertical and horizontal hit test
    if (pos.y() < ln_top)
    {
        iline = ISIZE_MIN;
    }
    else if (pos.y() > last_ln_bottom)
    {
        iline = ISIZE_MAX;
    }
    else
    {
        for (auto [i, line] : enumerate(lines))
        {
            auto line_bottom = ln_top + line.metrics.height;
            if (pos.y() <= line_bottom)
            {
                iline = (isize) i;
                break;
            }

            ln_top = line_bottom;
        }
    }

    if (iline < 0)
    {
        return {
          0, CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment::First}
        };
    }

    if (iline >= (isize) lines.size())
    {
        return {
          (isize) num_carets,
          CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment::Bottom}
        };
    }

    auto & line      = lines[iline];
    auto   direction = line.metrics.direction();
    auto   alignment =
      style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
    auto cursor = space_align(block_extent.x(), line.metrics.width, alignment) -
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
                                 .y = CaretYAlignment{iline}}
                };

            case TextDirection::RightToLeft:
                return {
                  (isize) line.carets.last(),
                  CaretAlignment{.x = CaretXAlignment::End,
                                 .y = CaretYAlignment{iline}}
                };

            default:
                ASH_UNREACHABLE;
        }
    }

    for (auto [irun, run] : enumerate(runs.view().slice(line.runs)))
    {
        auto & font_style  = block.fonts[run.style];
        auto   font_height = block.font_scale * run.font_height;
        auto   metrics     = run.metrics.resolve(font_height);
        auto   direction   = run.direction();
        bool   intersects  = pos.x() >= cursor && pos.x() <= (cursor + metrics.advance);
        auto   glyph_cursor = cursor;
        auto   run_width =
          metrics.advance +
          (run.is_spacing() ? 0 : (block.font_scale * font_style.word_spacing));

        if (!intersects)
        {
            goto next_run;
        }

        for (auto [iglyph, sh] : enumerate(glyphs.view().slice(run.glyphs)))
        {
            f32  advance = au_to_px(sh.advance, font_height);
            bool intersects =
              pos.x() >= glyph_cursor && pos.x() <= (glyph_cursor + advance);

            if (intersects)
            {
                auto codepoint = 0uz;

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

                auto caret = static_cast<isize>(line.carets.offset +
                                                (codepoint - line.codepoints.offset));

                return {
                  caret, CaretAlignment{
                                        .x = CaretXAlignment{caret - (isize) line.carets.offset},
                                        .y = CaretYAlignment{iline}}
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
              (isize) line.carets.last(),
              CaretAlignment{.x = CaretXAlignment::End, .y = CaretYAlignment{iline}}
            };

        case TextDirection::RightToLeft:
            return {
              (isize) line.carets.first(),
              CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment{iline}}
            };

        default:
            ASH_UNREACHABLE;
    }
}

enum class HitSpan : u8
{
    Partial = 0,
    Full    = 1
};

constexpr Option<HitSpan> highlight_test(Slice highlight, Slice carets)
{
    if (!highlight.is_empty() && highlight.contains(carets))
    {
        return HitSpan::Full;
    }

    if (!highlight.is_empty() &&
        (carets.contains(highlight.first()) || carets.contains(highlight.last())))
    {
        return HitSpan::Partial;
    }

    return none;
}

struct CaretTestResult
{
    usize index = 0;
};

constexpr Option<CaretTestResult> caret_test(Span<usize const> caret_indices,
                                             Slice             carets)
{
    for (auto [i, caret_index] : enumerate(caret_indices))
    {
        if (carets.contains(caret_index))
        {
            return CaretTestResult{i};
        }
    }

    return none;
}

Vec<TextPlacement::Highlight> TextLayout::place_highlights_(TextRenderInfo const & info,
                                                            Allocator allocator) const
{
    auto  block_width = extent.x();
    f32x2 block_extent{block_width, extent.y()};
    auto  ln_top     = -(0.5F * block_extent.y());
    auto  highlights = Vec<TextPlacement::Highlight>{allocator};
    auto  center     = f32x2::zero();

    for (auto [ihighlight, highlight, highlight_style] :
         zip(range(info.highlights.size()), info.highlights, info.highlight_styles))
    {
        for (auto [iline, line] : enumerate(this->lines))
        {
            auto  ln_bottom = ln_top + line.metrics.height;
            auto  direction = line.metrics.direction();
            // flip the alignment axis direction if it is an RTL line
            auto  alignment = info.style.alignment *
                              ((direction == TextDirection::LeftToRight) ? 1 : -1);
            f32x2 ln_extent{line.metrics.width, line.metrics.height};
            f32x2 ln_center{space_align(block_width, ln_extent.x(), alignment),
                            ln_top + 0.5F * ln_extent.y()};
            auto  cursor  = ln_center.x() - 0.5F * ln_extent.x();
            auto  ln_rect = CRect{
              .center = ln_center, .extent{line.metrics.width, line.metrics.height}
            };
            Option<f32x2> highlight_x;
            auto          ln_highlight_span = highlight_test(highlight, line.carets);

            if (!info.clip.overlaps(
                  CRect{.center = center + ln_rect.center, .extent = ln_rect.extent}))
            {
                goto next_line;
            }

            if (ln_highlight_span == HitSpan::Full)
            {
                f32x2 extent{
                  min(max(ln_rect.extent.x(),
                          info.block.font_scale * info.style.min_highlight_width),
                      block_width),
                  line.metrics.height};
                f32x2 center{space_align(block_width, extent.x(), alignment),
                             ln_center.y()};
                auto  x0    = center.x() - 0.5F * extent.x();
                auto  x1    = center.x() + 0.5F * extent.x();
                highlight_x = highlight_x.match(
                  [&](f32x2 x) { return f32x2{min(x.x(), x0), max(x.y(), x1)}; },
                  [&] { return f32x2{x0, x1}; });

                goto next_line;
            }

            for (auto [i, run] : enumerate(runs.view().slice(line.runs)))
            {
                auto & font_style  = info.block.fonts[run.style];
                auto   font_height = info.block.font_scale * run.font_height;
                auto   metrics     = run.metrics.resolve(font_height);
                auto run_width = metrics.advance +
                                 (run.is_spacing() ?
                                    0 :
                                    (info.block.font_scale * font_style.word_spacing));

                auto glyph_cursor = cursor;

                Option<HitSpan> run_highlight_span = none;

                if (ln_highlight_span == HitSpan::Partial)
                {
                    run_highlight_span = highlight_test(
                      highlight, run.carets(line.carets, line.codepoints));

                    if (run_highlight_span == HitSpan::Full)
                    {
                        f32x2 extent{run_width, line.metrics.height};
                        auto  center = f32x2{cursor, ln_top} + 0.5F * extent;
                        auto  x0     = center.x() - 0.5F * extent.x();
                        auto  x1     = center.x() + 0.5F * extent.x();
                        highlight_x  = highlight_x.match(
                          [&](f32x2 x) {
                              return f32x2{min(x.x(), x0), max(x.y(), x1)};
                          },
                          [&]() { return f32x2{x0, x1}; });

                        goto next_run;
                    }
                }

                for (auto [i, sh] : enumerate(this->glyphs.view().slice(run.glyphs)))
                {
                    auto advance = au_to_px(sh.advance, font_height);

                    // before and after carets
                    auto glyph_carets = Slice{
                      line.carets.offset + (sh.cluster - line.codepoints.offset), 1};

                    if (run_highlight_span == HitSpan::Partial)
                    {
                        auto glyph_highlight_span =
                          highlight_test(highlight, glyph_carets);

                        if (glyph_highlight_span.is_some())
                        {
                            f32x2 extent{advance, line.metrics.height};
                            f32x2 center = f32x2{glyph_cursor, ln_top} + 0.5F * extent;
                            auto  x0     = center.x() - 0.5F * extent.x();
                            auto  x1     = center.x() + 0.5F * extent.x();
                            highlight_x  = highlight_x.match(
                              [&](f32x2 x) {
                                  return f32x2{min(x.x(), x0), max(x.y(), x1)};
                              },
                              [&]() { return f32x2{x0, x1}; });
                        }
                    }

                    glyph_cursor += advance;
                }

            next_run:
                cursor += run_width;
            }

        next_line:
        {
            highlight_x.match(
              [&](f32x2 x) {
                  highlights
                    .push(TextPlacement::Highlight{
                      .id = ihighlight,
                      .bbox{.center = {x.x() + 0.5F * (x.y() - x.x()), ln_center.y()},
                            .extent = f32x2{x.y() - x.x(), line.metrics.height}},
                      .line = iline
                  })
                    .unwrap();
              },
              [] {});
            ln_top = ln_bottom;
        }
        }
    }

    return highlights;
}

TextPlacement TextLayout::place(TextRenderInfo const & info, Allocator allocator) const
{
    ASH_TRACE_SCOPE;

    ASH_CHECK(laid_out, "");
    ASH_CHECK(info.runs.size() == info.block.fonts.size(), "");
    ASH_CHECK(info.highlight_styles.size() == info.highlights.size(), "");
    ASH_CHECK(info.caret_styles.size() == info.carets.size(), "");

    auto  block_width = extent.x();
    f32x2 block_extent{block_width, extent.y()};

    Vec<CaretPlacement> caret_placements{allocator};

    for (auto caret : info.carets)
    {
        caret_placements.push(get_caret_placement(caret)).unwrap();
    }

    auto blocks         = Vec<TextPlacement::Block>{allocator};
    auto lines          = Vec<TextPlacement::Line>{allocator};
    auto backgrounds    = Vec<TextPlacement::Background>{allocator};
    auto glyph_shadows  = Vec<TextPlacement::GlyphShadow>{allocator};
    auto glyphs         = Vec<TextPlacement::Glyph>{allocator};
    auto underlines     = Vec<TextPlacement::Underline>{allocator};
    auto strikethroughs = Vec<TextPlacement::Strikethrough>{allocator};
    auto carets         = Vec<TextPlacement::Caret>{allocator};

    auto ln_top = -(0.5F * block_extent.y());
    auto center = f32x2::zero();

    blocks
      .push(TextPlacement::Block{
        .bbox{.center = {}, .extent = block_extent}
    })
      .unwrap();

    for (auto [iline, line] : enumerate(this->lines))
    {
        auto ln_bottom = ln_top + line.metrics.height;
        auto baseline  = ln_bottom - (line.metrics.leading() + line.metrics.descent);
        auto direction = line.metrics.direction();
        // flip the alignment axis direction if it is an RTL line
        auto alignment =
          info.style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
        f32x2 ln_extent{line.metrics.width, line.metrics.height};
        f32x2 ln_center{space_align(block_width, ln_extent.x(), alignment),
                        ln_top + 0.5F * ln_extent.y()};
        auto  cursor = ln_center.x() - 0.5F * ln_extent.x();

        CRect ln_rect{
          .center = ln_center, .extent{line.metrics.width, line.metrics.height}
        };

        if (!info.clip.overlaps(
              CRect{.center = center + ln_rect.center, .extent = ln_rect.extent}))
        {
            goto next_line;
        }

        {
            // if there's a caret on this line and it has no glyph (e.g empty line),
            // render it here
            auto ln_caret_test = caret_test(info.carets, line.carets);

            if (ln_caret_test.is_some() &&
                caret_placements[ln_caret_test->index].glyph.is_none())
            {
                auto & p = caret_placements[ln_caret_test->index];

                if (p.glyph.is_none() && p.line == iline)
                {
                    f32x2 center{cursor, ln_top + 0.5F * line.metrics.height};
                    f32x2 extent{info.caret_styles[ln_caret_test->index].thickness,
                                 line.metrics.height};

                    carets
                      .push(TextPlacement::Caret{
                        .id = ln_caret_test->index,
                        .bbox{.center = center, .extent = extent},
                        .line   = iline,
                        .column = 0,
                        .caret  = line.carets.first()
                    })
                      .unwrap();
                }
            }

            for (auto [i, run] : enumerate(runs.view().slice(line.runs)))
            {
                auto   irun        = line.runs.offset + i;
                auto & font_style  = info.block.fonts[run.style];
                auto & run_style   = info.runs[run.style];
                auto   font        = sys.font->get(font_style.font);
                auto   font_height = info.block.font_scale * run.font_height;
                auto   metrics     = run.metrics.resolve(font_height);
                auto run_width = metrics.advance +
                                 (run.is_spacing() ?
                                    0 :
                                    (info.block.font_scale * font_style.word_spacing));
                auto direction = run.direction();

                auto glyph_cursor = cursor;

                if (!run_style.background.is_transparent())
                {
                    f32x2 extent{run_width, line.metrics.height};
                    f32x2 center{cursor + extent.x() * 0.5F,
                                 baseline - metrics.ascent + extent.y() * 0.5F};

                    backgrounds
                      .push(TextPlacement::Background{
                        .bbox      = {.center = center, .extent = extent},
                        .line      = iline,
                        .column    = i,
                        .run       = irun,
                        .run_style = run.style
                    })
                      .unwrap();
                }

                Option<CaretTestResult> run_caret_test = none;

                if (ln_caret_test.is_some())
                {
                    run_caret_test =
                      caret_test(info.carets, run.carets(line.carets, line.codepoints));
                }

                if (run_style.strikethrough_thickness != 0)
                {
                    f32x2 extent{run_width, info.block.font_scale *
                                              run_style.strikethrough_thickness};
                    auto  center =
                      f32x2{cursor, baseline - metrics.ascent * 0.5F} + extent * 0.5F;

                    strikethroughs
                      .push(TextPlacement::Strikethrough{
                        .bbox{.center = center, .extent = extent},
                        .line      = iline,
                        .column    = i,
                        .run       = irun,
                        .run_style = run.style
                    })
                      .unwrap();
                }

                if (run_style.underline_thickness != 0)
                {
                    f32x2 extent{run_width,
                                 info.block.font_scale * run_style.underline_thickness};
                    auto  center =
                      f32x2{cursor, baseline + info.block.font_scale *
                                                 run_style.underline_offset} +
                      extent * 0.5F;

                    underlines
                      .push(TextPlacement::Underline{
                        .bbox{.center = center, .extent = extent},
                        .line      = iline,
                        .column    = i,
                        .run       = irun,
                        .run_style = run.style
                    })
                      .unwrap();
                }

                for (auto [i, sh] : enumerate(this->glyphs.view().slice(run.glyphs)))
                {
                    auto   iglyph  = run.glyphs.offset + i;
                    auto & m       = font.glyphs[sh.glyph];
                    auto   extent  = au_to_px(m.extent, font_height);
                    auto   center  = f32x2{glyph_cursor, baseline} +
                                     au_to_px(m.bearing, font_height) +
                                     au_to_px(sh.offset, font_height) + 0.5F * extent;
                    auto   advance = au_to_px(sh.advance, font_height);

                    // before and after carets
                    auto glyph_carets = Slice{
                      line.carets.offset + (sh.cluster - line.codepoints.offset), 1};

                    if (run_style.has_shadow())
                    {
                        auto shadow_extent = extent * run_style.shadow_scale;
                        auto shadow_center =
                          center + info.block.font_scale * run_style.shadow_offset;

                        glyph_shadows
                          .push(TextPlacement::GlyphShadow{
                            .bbox{shadow_center, shadow_extent},
                            .line      = iline,
                            .column    = i,
                            .run       = irun,
                            .run_style = run.style,
                            .glyph     = sh.glyph,
                            .cluster   = sh.cluster
                        })
                          .unwrap();
                    }

                    if (run_style.has_color())
                    {
                        glyphs
                          .push(TextPlacement::Glyph{
                            .bbox{center, extent},
                            .line      = iline,
                            .column    = i,
                            .run       = irun,
                            .run_style = run.style,
                            .glyph     = sh.glyph,
                            .cluster   = sh.cluster
                        })
                          .unwrap();
                    }

                    if (run_caret_test.is_some())
                    {
                        if (auto glyph_caret_test =
                              caret_test(info.carets, glyph_carets);
                            glyph_caret_test.is_some())
                        {
                            auto & p = caret_placements[glyph_caret_test->index];

                            if (p.glyph == iglyph)
                            {
                                auto & s = info.caret_styles[glyph_caret_test->index];
                                auto   c = info.carets[glyph_caret_test->index];

                                auto glyph_left  = glyph_cursor;
                                auto glyph_right = glyph_cursor + advance;

                                f32x2 extent{s.thickness, line.metrics.height};
                                f32x2 center;

                                if ((direction == TextDirection::LeftToRight &&
                                     p.after) ||
                                    (direction == TextDirection::RightToLeft &&
                                     !p.after))
                                {
                                    center.x() = glyph_right;
                                }
                                else
                                {
                                    center.x() = glyph_left;
                                }

                                center.y() = ln_top + 0.5F * line.metrics.height;

                                carets
                                  .push(TextPlacement::Caret{
                                    .id = glyph_caret_test->index,
                                    .bbox{center, extent},
                                    .line   = iline,
                                    .column = c - line.carets.first(),
                                    .caret  = c
                                })
                                  .unwrap();
                            }
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

    auto highlights = place_highlights_(info, allocator);

    auto placement = TextPlacement{.blocks         = std::move(blocks),
                                   .lines          = std::move(lines),
                                   .backgrounds    = std::move(backgrounds),
                                   .glyph_shadows  = std::move(glyph_shadows),
                                   .glyphs         = std::move(glyphs),
                                   .underlines     = std::move(underlines),
                                   .strikethroughs = std::move(strikethroughs),
                                   .highlights     = std::move(highlights),
                                   .carets         = std::move(carets)};

    return placement;
}

TextRunsStyle TextRunsStyle::all(TextStyle const & style, FontStyle const & font)
{
    SmallVec<usize, 2, 0>     run_indices{noop_allocator};
    SmallVec<TextStyle, 1, 0> styles{noop_allocator};
    SmallVec<FontStyle, 1, 0> fonts{noop_allocator};

    run_indices.append(span({0uz, USIZE_MAX})).unwrap();
    styles.push(style).unwrap();
    fonts.push(font).unwrap();

    return TextRunsStyle{std::move(run_indices), std::move(styles), std::move(fonts)};
}

TextRunsStyle TextRunsStyle::make_sized(Allocator             allocator,
                                        Span<usize const>     run_sizes,
                                        Span<TextStyle const> styles,
                                        Span<FontStyle const> fonts)
{
    ASH_CHECK(!run_sizes.is_empty(), "run_sizes cannot be empty");
    ASH_CHECK(run_sizes.size() == styles.size(),
              "run_sizes and styles must have the same size");
    ASH_CHECK(run_sizes.size() == fonts.size(),
              "run_sizes and fonts must have the same size");

    auto run_indices =
      SmallVec<usize, 2, 0>::make(run_sizes.size() + 1, allocator).unwrap();
    auto styles_vec = small_vec::copy<1, 0>(allocator, styles).unwrap();
    auto fonts_vec  = small_vec::copy<1, 0>(allocator, fonts).unwrap();

    run_indices.push(0uz).unwrap();
    usize run_start = 0;

    for (auto run_size : run_sizes)
    {
        auto run_end = sat_add(run_start, run_size);
        run_indices.push(run_end).unwrap();
        run_start = run_end;
    }

    if (run_indices.last() != USIZE_MAX)
    {
        run_indices.last() = USIZE_MAX;
    }

    return TextRunsStyle{std::move(run_indices), std::move(styles_vec),
                         std::move(fonts_vec)};
}

TextRunsStyle TextRunsStyle::copy(Allocator allocator) const
{
    return TextRunsStyle::make_indexed(allocator, run_indices_.view(), styles_.view(),
                                       fonts_.view());
}

TextRunsStyle TextRunsStyle::make_indexed(Allocator             allocator,
                                          Span<usize const>     run_indices,
                                          Span<TextStyle const> styles,
                                          Span<FontStyle const> fonts)
{
    ASH_CHECK(!run_indices.is_empty(), "run_indices cannot be empty");
    ASH_CHECK(run_indices.size() >= 2, "run_indices must have at least two entries");
    ASH_CHECK(run_indices.first() == 0, "first entry of run_indices must be 0");
    ASH_CHECK(run_indices.size() - 1 == styles.size(),
              "run_indices and styles must have compatible sizes");
    ASH_CHECK(run_indices.size() - 1 == fonts.size(),
              "run_indices and fonts must have compatible sizes");

    auto run_indices_vec = small_vec::copy<2, 0>(allocator, run_indices).unwrap();

    if (run_indices_vec.last() != USIZE_MAX)
    {
        run_indices_vec.last() = USIZE_MAX;
    }

    auto styles_vec = small_vec::copy<1, 0>(allocator, styles).unwrap();
    auto fonts_vec  = small_vec::copy<1, 0>(allocator, fonts).unwrap();

    return TextRunsStyle{std::move(run_indices_vec), std::move(styles_vec),
                         std::move(fonts_vec)};
}

RenderText & RenderText::runs_style(TextRunsStyle style)
{
    runs_style_ = std::move(style);
    hash_       = HASH_DIRTY;
    return *this;
}

TextRunsStyle const & RenderText::runs_style() const
{
    return runs_style_;
}

RenderText & RenderText::flush_text()
{
    hash_ = HASH_DIRTY;
    return *this;
}

RenderText & RenderText::wrap(bool wrap)
{
    if (wrap_ != wrap)
    {
        flush_text();
    }

    wrap_ = wrap;
    return *this;
}

RenderText & RenderText::use_kerning(bool use_kerning)
{
    if (use_kerning_ != use_kerning)
    {
        flush_text();
    }

    use_kerning_ = use_kerning;
    return *this;
}

RenderText & RenderText::use_ligatures(bool use_ligatures)
{
    if (use_ligatures_ != use_ligatures)
    {
        flush_text();
    }
    use_ligatures_ = use_ligatures;
    return *this;
}

RenderText & RenderText::font_scale(f32 scale)
{
    if (font_scale_ != scale)
    {
        flush_text();
    }
    font_scale_ = scale;
    return *this;
}

RenderText & RenderText::direction(TextDirection direction)
{
    if (direction_ != direction)
    {
        flush_text();
    }
    direction_ = direction;
    return *this;
}

RenderText & RenderText::language(Str language)
{
    if (!span_bit_eq(language_, language))
    {
        flush_text();
    }
    language_ = language;
    return *this;
}

RenderText & RenderText::alignment(f32 alignment)
{
    if (alignment_ != alignment)
    {
        flush_text();
    }
    alignment_ = alignment;
    return *this;
}

Str32 RenderText::str() const
{
    return str_.get();
}

RenderText & RenderText::str(Rc<Str32> utf32, TextStyle const & style,
                             FontStyle const & font)
{
    str(std::move(utf32));
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str(Rc<Str32> utf32)
{
    str_ = std::move(utf32);
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str32 utf32, Allocator allocator)
{
    auto vec    = vec::copy(allocator, utf32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator, std::move(vec)).unwrap();
    auto view   = rc_vec->view().as_const();
    return str(transmute(std::move(rc_vec), view));
}

RenderText & RenderText::str_copy(Str32 utf32, TextStyle const & style,
                                  FontStyle const & font, Allocator allocator)
{
    str_copy(utf32, allocator);
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8, TextStyle const & style,
                                  FontStyle const & font, Allocator allocator)
{
    str_copy(utf8, allocator);
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8, Allocator allocator)
{
    Vec<c32> text32{allocator};
    utf8_decode(utf8, text32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator, std::move(text32)).unwrap();
    auto view   = rc_vec->view().as_const();
    str_        = transmute(std::move(rc_vec), view);
    flush_text();
    return *this;
}

RenderText & RenderText::user_data(void * data)
{
    user_data_ = data;
    return *this;
}

RenderText & RenderText::width(f32 max_width, f32 align_width)
{
    if (max_width_ != max_width || align_width_ != align_width)
    {
        hash_ = HASH_DIRTY;
    }

    max_width_   = max_width;
    align_width_ = align_width;

    return *this;
}

usize RenderText::size() const
{
    return str_.get().size();
}

TextBlock RenderText::block() const
{
    return TextBlock{.str           = str_.get(),
                     .run_indices   = runs_style_.run_indices_,
                     .fonts         = runs_style_.fonts_,
                     .font_scale    = font_scale_,
                     .direction     = direction_,
                     .language      = language_,
                     .wrap          = wrap_,
                     .use_kerning   = use_kerning_,
                     .use_ligatures = use_ligatures_};
}

TextBlockStyle RenderText::block_style() const
{
    return TextBlockStyle{.alignment = alignment_, .user_data = user_data_};
}

TextLayout const & RenderText::layout() const
{
    return layout_;
}

void RenderText::perform_layout()
{
    if (hash_ != HASH_CLEAN)
    {
        layout_ = sys.font->layout_text(block(), max_width_, align_width_);
    }

    hash_ = HASH_CLEAN;
}

Tuple<TextRenderInfo, TextPlacement>
  RenderText::place(CRect const & clip, Span<Slice const> highlights,
                    Span<TextHighlightStyle const> highlight_styles,
                    Span<usize const> carets, Span<CaretStyle const> caret_styles,
                    Allocator allocator) const
{
    ASH_TRACE_SCOPE;
    TextRenderInfo info{.clip             = clip,
                        .block            = block(),
                        .style            = block_style(),
                        .runs             = runs_style_.styles_,
                        .highlights       = highlights,
                        .highlight_styles = highlight_styles,
                        .carets           = carets,
                        .caret_styles     = caret_styles};

    return {info, layout_.place(info, allocator)};
}

Tuple<isize, CaretAlignment> RenderText::hit(f32x2 pos) const
{
    return layout_.hit(block(), block_style(), pos);
}

static constexpr bool is_symbol(Span<c32 const> symbols, c32 c)
{
    return !find(symbols, c).is_empty();
}

template <typename Fn>
static constexpr Option<usize> seek(Str32 text, usize pos, bool left, Fn && pred)
{
    if (pos >= text.size())
    {
        return none;
    }

    c32 const * iter    = text.pbegin() + pos;
    c32 const * end     = left ? (text.pbegin() - 1) : text.pend();
    isize       advance = left ? -1 : 1;

    while (iter != end && !pred(*iter))
    {
        iter += advance;
    }

    if (iter == end)
    {
        return none;
    }

    return iter - text.pbegin();
}

static constexpr Option<usize> seek_sym(Str32 str, usize pos, bool left,
                                        Span<c32 const> symbols)
{
    return seek(str, pos, left, [&](c32 c) { return is_symbol(symbols, c); });
}

template <typename Fn>
static constexpr Slice span_boundary(Str32 str, usize pos, Fn && pred)
{
    if (pos >= str.size())
    {
        return Slice{pos, 0};
    }

    if (pred(str[pos]))
    {
        auto neg   = [&](auto cp) { return !pred(cp); };
        auto begin = seek(str, pos, true, neg).unwrap_or(USIZE_MAX) + 1;
        auto end   = seek(str, pos, false, neg).unwrap_or(str.size());
        return Slice::offsets(begin, end);
    }
    else
    {
        auto begin = seek(str, pos, true, pred).unwrap_or(USIZE_MAX) + 1;
        auto end   = seek(str, pos, false, pred).unwrap_or(str.size());
        return Slice::offsets(begin, end);
    }
}

static constexpr Slice span_sym_boundary(Str32 text, usize pos, Span<c32 const> symbols)
{
    return span_boundary(text, pos, [&](c32 c) { return is_symbol(symbols, c); });
}

static inline Option<isize> translate_caret(TextLayout const & layout, isize caret,
                                            CaretXAlignment alignment,
                                            isize           line_displacement)
{
    if (layout.lines.is_empty())
    {
        return none;
    }

    auto loc = layout.get_caret_codepoint(caret);

    auto line = clamp((isize) loc.line + line_displacement, (isize) 0,
                      (isize) layout.lines.size());

    return layout.align_caret(
      CaretAlignment{.x = alignment, .y = static_cast<CaretYAlignment>(line)});
}

Str32 TextModel::str() const
{
    return text_.str();
}

TextLayout const & TextModel::layout() const
{
    return text_.layout();
}

RenderText const & TextModel::get_render_text() const
{
    return text_;
}

RenderText & TextModel::get_render_text()
{
    return text_;
}

void TextModel::text(RenderText text)
{
    text_ = std::move(text);
}

void TextModel::str(Rc<Str32> str)
{
    text_.str(std::move(str));
}

void TextModel::str(Str32 s)
{
    auto str32  = vec::copy(allocator_, s).unwrap();
    auto rc_str = rc<StrVec32>(allocator_, std::move(str32)).unwrap();
    auto view   = rc_str->view().as_const();
    str(transmute(std::move(rc_str), view));
}

void TextModel::str(Str8 s)
{
    auto str32 = Vec<c32>::make(s.size(), allocator_).unwrap();
    utf8_decode(s, str32).unwrap();
    auto rc_str = rc<StrVec32>(allocator_, std::move(str32)).unwrap();
    auto view   = rc_str->view().as_const();
    str(transmute(std::move(rc_str), view));
}

void TextModel::runs_style(TextRunsStyle runs_style)
{
    text_.runs_style(std::move(runs_style));
}

StrVec32 TextModel::copy(Allocator allocator)
{
    auto & layout = this->layout();
    auto   cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = layout.get_caret_codepoint(cursor.caret());
        cursor.select(layout.lines[cp.line].carets);
    }

    cursor.normalize(layout.num_carets);
    auto slice = layout.get_caret_selection(cursor.selection());

    return vec::copy(allocator, str().slice(slice)).unwrap();
}

void TextModel::add_cursor(TextCursor value)
{
    cursor_ = value.normalize(layout().num_carets);
}

TextCursor & TextModel::touch_cursor()
{
    if (cursor_.is_none())
    {
        cursor_ = TextCursor{};
    }

    return *cursor_;
}

void TextModel::set_cursor_style(TextHighlightStyle highlight_style,
                                 CaretStyle         caret_style)
{
    highlight_style_ = highlight_style;
    caret_style_     = caret_style;
}

void TextModel::remove_cursor()
{
    cursor_ = none;
}

void TextModel::update_cursor(TextCursor value)
{
    cursor_ = value.normalize(layout().num_carets);
}

void TextModel::unselect()
{
    if (cursor_.is_some())
    {
        cursor_->unselect();
    }
}

void TextModel::left()
{
    auto & cursor = touch_cursor();
    if (cursor.has_selection())
    {
        cursor.unselect_left().normalize(layout().num_carets);
    }
    else
    {
        cursor.translate(-1).normalize(layout().num_carets);
    }
}

void TextModel::right()
{
    auto & cursor = touch_cursor();
    if (cursor.has_selection())
    {
        cursor.unselect_right().normalize(layout().num_carets);
    }
    else
    {
        cursor.translate(1).normalize(layout().num_carets);
    }
}

void TextModel::word_start(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(
      l.to_caret(seek_sym(str(), cp.codepoint + (cp.after ? 1 : 0), true, word_symbols)
                   .unwrap_or(),
                 true));
}

void TextModel::word_end(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(
      l.to_caret(seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
}

void TextModel::line_start()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(l.lines[cp.line].carets.first());
}

void TextModel::line_end()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(l.lines[cp.line].carets.last());
}

void TextModel::up()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    if (cursor.has_selection())
    {
        cursor.unselect_left();
    }
    cursor.move_to(translate_caret(l, cursor.caret(), caret_alignment_, -1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::down()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    if (cursor.has_selection())
    {
        cursor.unselect_right();
    }
    cursor.move_to(translate_caret(l, cursor.caret(), caret_alignment_, 1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::page_up(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(
      translate_caret(l, cursor.caret(), caret_alignment_, -(isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::page_down(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(
      translate_caret(l, cursor.caret(), caret_alignment_, (isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_left()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.extend_selection(-1).normalize(l.num_carets);
}

void TextModel::select_right()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.extend_selection(1).normalize(l.num_carets);
}

void TextModel::select_up()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(translate_caret(l, cursor.caret(), caret_alignment_, -1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::select_down()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(translate_caret(l, cursor.caret(), caret_alignment_, 1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::select_to_word_start(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(
      l.to_caret(seek_sym(str(), cp.codepoint, true, word_symbols).unwrap_or(), true));
}

void TextModel::select_to_word_end(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(
      l.to_caret(seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
}

void TextModel::select_to_line_start()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(l.lines[cp.line].carets.first());
}

void TextModel::select_to_line_end()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(l.lines[cp.line].carets.last());
}

void TextModel::select_page_up(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(
      translate_caret(l, cursor.caret(), caret_alignment_, -(isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_page_down(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(
      translate_caret(l, cursor.caret(), caret_alignment_, (isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_codepoint()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_by(1).normalize(l.num_carets);
}

void TextModel::select_word(Span<c32 const> word_symbols)
{
    auto & l         = layout();
    auto & cursor    = touch_cursor();
    auto   selection = span_sym_boundary(
      str(), l.get_caret_codepoint(cursor.caret()).codepoint, word_symbols);
    cursor.select(l.get_caret_selection(selection));
}

void TextModel::select_line()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.select(l.lines[cp.line].carets);
}

void TextModel::select_all()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.select(Slice{0, l.num_carets});
}

void TextModel::hit(f32x2 pos)
{
    auto & l          = layout();
    auto & cursor     = touch_cursor();
    auto [caret, loc] = text_.hit(pos);
    caret_alignment_  = loc.x;
    cursor.unselect().move_to(l.align_caret(loc));
}

void TextModel::hit_select(f32x2 pos)
{
    auto & l          = layout();
    auto & cursor     = touch_cursor();
    auto [caret, loc] = text_.hit(pos);
    caret_alignment_  = loc.x;
    cursor.span_to(l.align_caret(loc));
}

void TextModel::erase_at(Slice selection)
{
    auto str  = text_.str();
    selection = selection(str.size());
    auto res  = StrVec32::make(str.size() - selection.span, allocator_).unwrap();
    res.append(str.slice(0, selection.begin())).unwrap();
    res.append(str.slice(selection.end())).unwrap();
    auto rc_resv = rc<StrVec32>(allocator_, std::move(res)).unwrap();
    auto view    = rc_resv->view().as_const();
    text_.str(transmute(std::move(rc_resv), view));
}

void TextModel::insert_at(usize pos, Str32 in)
{
    auto str = text_.str();
    auto res = StrVec32::make(str.size() + in.size(), allocator_).unwrap();
    res.append(str.slice(0, pos)).unwrap();
    res.append(in).unwrap();
    res.append(str.slice(pos)).unwrap();
    auto rc_resv = rc<StrVec32>(allocator_, std::move(res)).unwrap();
    auto view    = rc_resv->view().as_const();
    text_.str(transmute(std::move(rc_resv), view));
}

void TextModel::backspace()
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();
        if (!cursor.has_selection())
        {
            if (cursor.caret() == 0)
            {
                return;
            }
            cursor.translate(-1).span_by(1).normalize(l.num_carets);
        }

        erase_at(l.get_caret_selection(cursor.selection()));
    }

    cursor.unselect_left().normalize(layout().num_carets);
}

void TextModel::del()
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();

        if (!cursor.has_selection())
        {
            if (l.num_carets == 0 || cursor.caret() == ((isize) l.num_carets - 1))
            {
                return;
            }
            cursor.span_by(1).normalize(l.num_carets);
        }

        erase_at(l.get_caret_selection(cursor.selection()));
    }

    cursor.unselect_left().normalize(layout().num_carets);
}

void TextModel::insert(Str32 input)
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();
        if (auto selection = cursor.selection(); !selection.is_empty())
        {
            cursor.unselect_left();
            erase_at(l.get_caret_selection(selection));
        }
    }

    {
        auto & l = layout();
        cursor.normalize(l.num_carets);
        auto cp        = l.get_caret_codepoint(cursor.caret());
        auto codepoint = cp.codepoint + (cp.after ? 1 : 0);
        insert_at(codepoint, input);
        perform_layout();
        auto caret = l.to_caret(codepoint + input.size(), true);
        cursor.move_to(caret).normalize(l.num_carets);
    }
}

void TextModel::insert(Str8 input)
{
    ThreadScratchScope scratch;
    Vec<c32>           utf32{scratch};
    utf8_decode(input, utf32).unwrap();
    insert(utf32);
}

void TextModel::new_line()
{
    return insert(U"\n"_s);
}

void TextModel::tab()
{
    return insert(U"\t"_s);
}

StrVec32 TextModel::copy_cut(Allocator allocator)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = l.get_caret_codepoint(cursor.caret());
        cursor.select(l.lines[cp.line].carets);
    }

    cursor.normalize(l.num_carets);
    auto slice = l.get_caret_selection(cursor.selection());
    auto copy  = vec::copy(allocator, str().slice(slice)).unwrap();

    backspace();

    return copy;
}

void TextModel::cut()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = l.get_caret_codepoint(cursor.caret());
        cursor.select(l.lines[cp.line].carets);
    }

    backspace();
}

void TextModel::width(f32 max_width, f32 align_width)
{
    text_.width(max_width, align_width);
}

void TextModel::perform_layout()
{
    text_.perform_layout();
    cursor_.match(
      [&](TextCursor & cursor) { cursor.normalize(text_.layout().num_carets); }, [] {});
}

}    // namespace ash
