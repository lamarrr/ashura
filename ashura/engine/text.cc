/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/font.h"
#include "ashura/engine/font_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"

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
    extent         = f32x2{0, 0};
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

    ASH_CHECK(!l.is_empty(), "");

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

    auto   iln       = l.as_slice_of(lines).offset;
    auto & ln        = l[0];
    auto   column    = caret - ln.carets.offset;
    auto   codepoint = ln.codepoints.offset + column;
    auto   after     = column >= (ln.carets.span - 1);

    return CaretCodepoint{.line = iln, .codepoint = codepoint, .after = after};
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

    f32x2 block_extent{max(extent.x(), align_width), extent.y()};
    f32x2 half_block_extent = 0.5F * block_extent;
    f32   ln_top            = -half_block_extent.y();
    f32   last_ln_bottom    = half_block_extent.y();
    auto  ln                = 0z;

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
          0, CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment::First}
        };
    }

    if (ln >= (isize) lines.size())
    {
        return {
          (isize) num_carets,
          CaretAlignment{.x = CaretXAlignment::Start, .y = CaretYAlignment::Bottom}
        };
    }

    auto & line      = lines[ln];
    auto   direction = line.metrics.direction();
    f32    alignment =
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
        auto & font_style  = block.fonts[run.style];
        f32    font_height = block.font_scale * run.font_height;
        auto   metrics     = run.metrics.resolve(font_height);
        auto   direction   = run.direction();
        bool   intersects  = pos.x() >= cursor && pos.x() <= (cursor + metrics.advance);
        f32    glyph_cursor = cursor;
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
              (isize) line.carets.last(),
              CaretAlignment{.x = CaretXAlignment::End, .y = CaretYAlignment{ln}}
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

enum class HitSpan : u8
{
    Partial = 0,
    Full    = 1
};

struct HighlightTestResult
{
    HitSpan span  = HitSpan::Partial;
    usize   index = 0;
};

constexpr Option<HighlightTestResult> highlight_test(Span<Slice const> highlights,
                                                     Slice             carets)
{
    Option<HighlightTestResult> span;

    for (auto [i, highlight] : enumerate(highlights))
    {
        if (!highlight.is_empty() && highlight.contains(carets))
        {
            return HighlightTestResult{HitSpan::Full, i};
        }

        if (!highlight.is_empty() &&
            (carets.contains(highlight.first()) || carets.contains(highlight.last())))
        {
            span = HighlightTestResult{HitSpan::Partial, i};
        }
    }

    return span;
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

TextPlacement TextLayout::place(TextRenderInfo const & info, Allocator allocator) const
{
    // [ ] merge highlight rects; highlight rect type with merging, next line,
    // close?
    ASH_CHECK(laid_out, "");
    ASH_CHECK(info.runs.size() == info.block.fonts.size(), "");
    ASH_CHECK(info.highlight_styles.size() == info.highlights.size(), "");
    ASH_CHECK(info.caret_styles.size() == info.carets.size(), "");

    auto  block_width = max(extent.x(), align_width);
    f32x2 block_extent{block_width, extent.y()};

    Vec<CaretPlacement> caret_placements{allocator};

    for (auto caret : info.carets)
    {
        caret_placements.push(get_caret_placement(caret)).unwrap();
    }

    using P = TextPlacement;

    auto blocks         = Vec<P::Block>{allocator};
    auto lines          = Vec<P::Line>{allocator};
    auto backgrounds    = Vec<P::Background>{allocator};
    auto glyph_shadows  = Vec<P::GlyphShadow>{allocator};
    auto glyphs         = Vec<P::Glyph>{allocator};
    auto underlines     = Vec<P::Underline>{allocator};
    auto strikethroughs = Vec<P::Strikethrough>{allocator};
    auto highlights     = Vec<P::Highlight>{allocator};
    auto carets         = Vec<P::Caret>{allocator};

    f32 ln_top = -(0.5F * block_extent.y());

    blocks
      .push(P::Block{
        .bbox{.center = {}, .extent = block_extent}
    })
      .unwrap();

    for (auto [iln, ln] : enumerate(this->lines))
    {
        auto ln_bottom = ln_top + ln.metrics.height;
        auto baseline  = ln_bottom - (ln.metrics.leading() + ln.metrics.descent);
        auto direction = ln.metrics.direction();
        // flip the alignment axis direction if it is an RTL line
        auto alignment =
          info.style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
        f32x2 ln_extent{ln.metrics.width, ln.metrics.height};
        f32x2 ln_center{space_align(block_width, ln_extent.x(), alignment),
                        ln_top + 0.5F * ln_extent.y()};
        auto  cursor = ln_center.x() - 0.5F * ln_extent.x();

        CRect ln_rect{
          .center = ln_center, .extent{ln.metrics.width, ln.metrics.height}
        };

        if (!info.clip.overlaps(
              CRect{.center = info.center, .extent = ln_rect.extent}.transform(
                transform3d_to_2d(info.transform) * translate2d(ln_rect.center))))
        {
            goto next_line;
        }

        {
            // if there's a caret on this line and it has no glyph (e.g empty line),
            // render it here

            auto ln_highlight_span = highlight_test(info.highlights, ln.carets);

            if (ln_highlight_span.is_some() && ln_highlight_span->span == HitSpan::Full)
            {
                f32x2 extent{
                  min(max(ln_rect.extent.x(),
                          info.block.font_scale * info.style.min_highlight_width),
                      block_width),
                  ln_rect.extent.y()};
                f32x2 center{space_align(block_width, extent.x(), alignment),
                             ln_center.y()};

                highlights
                  .push(P::Highlight{
                    .id = ln_highlight_span->index,
                    .bbox{.center = center, .extent = extent},
                    .line = iln
                })
                  .unwrap();
            }

            auto ln_caret_test = caret_test(info.carets, ln.carets);

            if (ln_caret_test.is_some() &&
                caret_placements[ln_caret_test->index].glyph.is_none())
            {
                auto & p = caret_placements[ln_caret_test->index];

                if (p.glyph.is_none() && p.line == iln)
                {
                    f32x2 center{cursor, ln_top + 0.5F * ln.metrics.height};
                    f32x2 extent{info.caret_styles[ln_caret_test->index].thickness,
                                 ln.metrics.height};

                    carets
                      .push(P::Caret{
                        .id = ln_caret_test->index,
                        .bbox{.center = center, .extent = extent},
                        .line   = iln,
                        .column = 0,
                        .caret  = ln.carets.first()
                    })
                      .unwrap();
                }
            }

            for (auto [i, run] : enumerate(runs.view().slice(ln.runs)))
            {
                auto   irun        = ln.runs.offset + i;
                auto & font_style  = info.block.fonts[run.style];
                auto & run_style   = info.runs[run.style];
                auto   font        = sys.font->get(font_style.font);
                auto   font_height = info.block.font_scale * run.font_height;
                auto   metrics     = run.metrics.resolve(font_height);
                auto   run_width   = metrics.advance +
                                 (run.is_spacing() ?
                                    0 :
                                    (info.block.font_scale * font_style.word_spacing));
                auto direction = run.direction();

                auto glyph_cursor = cursor;

                if (!run_style.background.is_transparent())
                {
                    f32x2 extent{run_width, metrics.height()};
                    f32x2 center{cursor + extent.x() * 0.5F,
                                 baseline - metrics.ascent + extent.y() * 0.5F};

                    backgrounds
                      .push(P::Background{
                        .bbox      = {.center = center, .extent = extent},
                        .line      = iln,
                        .column    = i,
                        .run       = irun,
                        .run_style = run.style
                    })
                      .unwrap();
                }

                Option<HighlightTestResult> run_highlight_span = none;

                if (ln_highlight_span.is_some() &&
                    ln_highlight_span->span == HitSpan::Partial)
                {
                    run_highlight_span = highlight_test(
                      info.highlights, run.carets(ln.carets, ln.codepoints));

                    if (run_highlight_span.is_some() &&
                        run_highlight_span->span == HitSpan::Full)
                    {
                        f32x2 extent{run_width, metrics.height()};
                        f32x2 center = f32x2{cursor, ln_top} + 0.5F * extent;

                        highlights
                          .push(P::Highlight{
                            .id = run_highlight_span->index,
                            .bbox{.center = center, .extent = extent},
                            .line = iln
                        })
                          .unwrap();
                    }
                }

                Option<CaretTestResult> run_caret_test = none;

                if (ln_caret_test.is_some())
                {
                    run_caret_test =
                      caret_test(info.carets, run.carets(ln.carets, ln.codepoints));
                }

                if (run_style.strikethrough_thickness != 0)
                {
                    f32x2 extent{run_width, info.block.font_scale *
                                              run_style.strikethrough_thickness};
                    f32x2 center =
                      f32x2{cursor, baseline - metrics.ascent * 0.5F} + extent * 0.5F;

                    strikethroughs
                      .push(P::Strikethrough{
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
                    f32x2 extent{run_width,
                                 info.block.font_scale * run_style.underline_thickness};
                    f32x2 center =
                      f32x2{cursor, baseline + info.block.font_scale *
                                                 run_style.underline_offset} +
                      extent * 0.5F;

                    underlines
                      .push(P::Underline{
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
                    auto                 iglyph = run.glyphs.offset + i;
                    GlyphMetrics const & m      = font.glyphs[sh.glyph];
                    f32x2                extent = au_to_px(m.extent, font_height);
                    f32x2                center = f32x2{glyph_cursor, baseline} +
                                   au_to_px(m.bearing, font_height) +
                                   au_to_px(sh.offset, font_height) + 0.5F * extent;
                    auto advance = au_to_px(sh.advance, font_height);

                    // before and after carets
                    auto glyph_carets =
                      Slice{ln.carets.offset + (sh.cluster - ln.codepoints.offset), 1};

                    if (run_style.has_shadow())
                    {
                        f32x2 shadow_extent = extent * run_style.shadow_scale;
                        f32x2 shadow_center =
                          center + info.block.font_scale * run_style.shadow_offset;

                        glyph_shadows
                          .push(P::GlyphShadow{
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
                          .push(P::Glyph{
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

                                f32x2 extent{s.thickness, ln.metrics.height};
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

                                center.y() = ln_top + 0.5F * ln.metrics.height;

                                carets
                                  .push(P::Caret{
                                    .id = glyph_caret_test->index,
                                    .bbox{center, extent},
                                    .line   = iln,
                                    .column = c - ln.carets.first(),
                                    .caret  = c
                                })
                                  .unwrap();
                            }
                        }
                    }

                    if (run_highlight_span.is_some() &&
                        run_highlight_span->span == HitSpan::Partial)
                    {
                        auto glyph_highlight_span =
                          highlight_test(info.highlights, glyph_carets);

                        if (glyph_highlight_span.is_some())
                        {
                            f32x2 extent{advance, metrics.height()};
                            f32x2 center = f32x2{glyph_cursor, ln_top} + 0.5F * extent;

                            highlights
                              .push(P::Highlight{
                                .id = glyph_highlight_span->index,
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

}    // namespace ash
