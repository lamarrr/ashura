/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/engine/systems.h"

namespace ash
{

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

  auto first  = to_caret(codepoints.first(), true);
  auto ifirst = first.unwrap();
  CHECK(ifirst >= 0, "");

  if (codepoints.is_empty())
  {
    return Slice{(usize) ifirst, 0};
  }

  auto last  = to_caret(codepoints.last(), false);
  auto ilast = last.unwrap();

  CHECK(ilast >= 0, "");
  CHECK(ilast >= ifirst, "");

  return Slice::from_range((usize) ifirst, (usize) (ilast + 1));
}

Option<CaretCodepoint> TextLayout::get_caret_codepoint(isize caret) const
{
  CHECK(caret >= 0, "");
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

Option<CaretGlyph> TextLayout::get_caret_glyph(isize caret) const
{
  return get_caret_codepoint(caret).map([&](CaretCodepoint c) {
    auto const & line = lines[c.line];

    auto runs =
      binary_find(this->runs.view().slice(line.runs), [&](TextRun const & run) {
        return run.codepoints.end() > c.codepoint;
      });

    CHECK(!runs.is_empty(), "");

    auto const & run = runs[0];

    // find the glyph with the nearest glyph cluster to the caret's codepoint position

    Option<GlyphMatch> match;

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

    auto matched = match.unwrap();

    return CaretGlyph{.line = c.line, .glyph = matched.glyph, .after = c.after};
  });
}

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
  f32        ln_top            = -half_block_extent.y;
  f32        last_ln_bottom    = half_block_extent.y;
  isize      ln                = 0;

  // separated vertical and horizontal hit test
  if (pos.y < ln_top)
  {
    ln = ISIZE_MIN;
  }
  else if (pos.y > last_ln_bottom)
  {
    ln = ISIZE_MAX;
  }
  else
  {
    for (auto [i, line] : enumerate(lines))
    {
      auto line_bottom = ln_top + line.metrics.height;
      if (pos.y <= line_bottom)
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

  auto const & line      = lines[ln];
  auto const   direction = line.metrics.direction();
  f32 const    alignment =
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
    auto const & font_style  = block.fonts[run.style];
    f32 const    font_height = block.font_scale * run.font_height;
    auto const   metrics     = run.metrics.resolve(font_height);
    auto const   direction   = run.direction();
    bool const   intersects =
      pos.x >= cursor && pos.x <= (cursor + metrics.advance);
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
        pos.x >= glyph_cursor && pos.x <= (glyph_cursor + advance);

      if (intersects)
      {
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

void TextLayout::default_renderer(Canvas & canvas, ShapeInfo const & shape,
                                  TextRenderInfo const & info)
{
  switch (info.region)
  {
    case TextRegion::Glyphs:
    case TextRegion::GlyphShadows:
      canvas.rect(shape);
      break;
    default:
      canvas.rrect(shape);
      break;
  }
}

constexpr Tuple<bool, bool> highlight_test(Span<Slice const> highlights,
                                           Slice             carets)
{
  bool fully_covered = false;
  bool any_covered   = false;

  for (auto highlight : highlights)
  {
    if (highlight.contains(carets))
    {
      fully_covered = true;
      any_covered   = true;
      break;
    }

    if (carets.overlaps(highlight))
    {
      any_covered = true;
    }
  }

  return {fully_covered, any_covered};
}

constexpr bool caret_test(Span<isize const> cursor_carets, Slice carets)
{
  return any_is(cursor_carets,
                [&](isize cursor) { return carets.contains(cursor); });
}

void TextLayout::render(Canvas & canvas, ShapeInfo const & info,
                        TextBlock const & block, TextBlockStyle const & style,
                        Span<Slice const> highlights, Span<isize const> carets,
                        CRect const & clip, TextRenderer renderer) const
{
  CHECK(style.runs.size() == block.runs.size(), "");
  CHECK(style.runs.size() == block.fonts.size(), "");

  auto const block_width = max(extent.x, style.align_width);
  Vec2 const block_extent{block_width, extent.y};

  char scratch[512];

  FallbackAllocator allocator{Arena::from(scratch), default_allocator};

  Vec<CaretGlyph> caret_glyphs{allocator};

  for (auto caret : carets)
  {
    caret_glyphs.push(get_caret_glyph(caret).unwrap()).unwrap();
  }

  enum Pass : u8
  {
    Block         = 0,
    Background    = 1,
    GlyphShadows  = 2,
    Glyphs        = 3,
    Underline     = 4,
    Strikethrough = 5,
    Highlight     = 6,
    Caret         = 7,
    NUM_PASSES    = 8
  };

  for (u32 pass = 0; pass < NUM_PASSES; pass++)
  {
    if (pass == Pass::Highlight && highlights.is_empty())
    {
      continue;
    }

    if (pass == Pass::Caret && carets.is_empty())
    {
      continue;
    }

    f32 ln_top = -(0.5F * block_extent.y);

    if (pass == Pass::Block)
    {
      renderer(
        canvas,
        {.center    = info.center,
         .extent    = block_extent,
         .transform = info.transform * translate3d(vec3(info.center, 0))},
        {.region = TextRegion::Block});

      continue;
    }

    for (auto [iln, ln] : enumerate(lines))
    {
      auto const ln_bottom = ln_top + ln.metrics.height;
      auto const baseline  = ln_bottom - ln.metrics.descent;
      auto const direction = ln.metrics.direction();
      // flip the alignment axis direction if it is an RTL line
      auto const alignment =
        style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
      Vec2 const ln_extent{ln.metrics.width, ln.metrics.height};
      Vec2 const ln_center{space_align(block_width, ln_extent.x, alignment),
                           ln_top + 0.5F * ln_extent.y};
      auto       cursor = ln_center.x - 0.5F * ln_extent.x;

      CRect const ln_rect{
        .center = info.center + ln_center,
        .extent{ln.metrics.width, ln.metrics.height}
      };

      if (!clip.overlaps(ln_rect))
      {
        goto next_line;
      }

      if (pass == Pass::Highlight)
      {
        auto [fully_covered, has_any] = highlight_test(highlights, ln.carets);

        if (fully_covered)
        {
          renderer(
            canvas,
            {.center    = info.center,
             .extent    = ln_rect.extent,
             .transform = info.transform * translate3d(vec3(ln_rect.center, 0)),
             .corner_radii = style.highlight.corner_radii,
             .stroke       = style.highlight.stroke,
             .thickness    = style.highlight.thickness,
             .tint         = style.highlight.color},
            {.region = TextRegion::Highlight, .line = iln});

          goto next_line;
        }

        if (!has_any)
        {
          goto next_line;
        }
      }

      if (pass == Pass::Caret)
      {
        auto has_any = caret_test(carets, ln.carets);
        if (!has_any)
        {
          goto next_line;
        }
      }

      for (auto [i, run] : enumerate(runs.view().slice(ln.runs)))
      {
        auto const   irun        = ln.runs.offset + i;
        auto const & font_style  = block.fonts[run.style];
        auto const & run_style   = style.runs[run.style];
        auto const   font        = sys->font.get(font_style.font);
        auto const & atlas       = font.gpu_atlas.v();
        auto const   font_height = block.font_scale * run.font_height;
        auto const   metrics     = run.metrics.resolve(font_height);
        auto const   run_width =
          metrics.advance +
          (run.is_spacing() ? 0 : (block.font_scale * font_style.word_spacing));
        auto const direction = run.direction();

        auto glyph_cursor = cursor;

        if (pass == Pass::Background)
        {
          if (!run_style.background.is_transparent())
          {
            Vec2 const extent{run_width, metrics.height()};
            Vec2 const center{cursor + extent.x * 0.5F,
                              baseline - metrics.ascent + extent.y * 0.5F};

            renderer(
              canvas,
              {.center       = info.center,
               .extent       = extent,
               .transform    = info.transform * translate3d(vec3(center, 0)),
               .corner_radii = run_style.corner_radii,
               .tint         = run_style.background},
              {.region    = TextRegion::Background,
               .line      = iln,
               .run       = irun,
               .run_style = run.style});
          }

          goto next_run;
        }

        if (pass == Pass::Highlight)
        {
          auto [fully_covered, has_any] =
            highlight_test(highlights, run.carets(ln.carets, ln.codepoints));

          if (fully_covered)
          {
            Vec2 const extent{run_width, metrics.height()};
            Vec2 const center = Vec2{cursor, ln_top} + 0.5F * extent;

            renderer(
              canvas,
              {.center       = info.center,
               .extent       = extent,
               .transform    = info.transform * translate3d(vec3(center, 0)),
               .corner_radii = style.highlight.corner_radii,
               .stroke       = style.highlight.stroke,
               .thickness    = style.highlight.thickness,
               .tint         = style.highlight.color},
              {.region    = TextRegion::Highlight,
               .line      = iln,
               .run       = irun,
               .run_style = run.style});

            goto next_run;
          }

          if (!has_any)
          {
            goto next_run;
          }
        }

        if (pass == Pass::Caret)
        {
          auto has_any =
            caret_test(carets, run.carets(ln.carets, ln.codepoints));
          if (!has_any)
          {
            goto next_run;
          }
        }

        if (pass == Pass::Strikethrough)
        {
          if (run_style.strikethrough_thickness != 0)
          {
            Vec2 const extent{run_width, block.font_scale *
                                           run_style.strikethrough_thickness};
            Vec2 const center =
              Vec2{cursor, baseline - metrics.ascent * 0.5F} + extent * 0.5F;

            renderer(
              canvas,
              {.center          = info.center,
               .extent          = extent,
               .transform       = info.transform * translate3d(vec3(center, 0)),
               .tint            = run_style.strikethrough,
               .sampler         = info.sampler,
               .edge_smoothness = info.edge_smoothness},
              {.region    = TextRegion::Strikethrough,
               .line      = iln,
               .run       = irun,
               .run_style = run.style});
          }

          goto next_run;
        }

        if (pass == Pass::Underline)
        {
          if (run_style.underline_thickness != 0)
          {
            Vec2 const extent{run_width,
                              block.font_scale * run_style.underline_thickness};
            Vec2 const center =
              Vec2{cursor,
                   baseline + block.font_scale * run_style.underline_offset} +
              extent * 0.5F;

            renderer(
              canvas,
              {.center          = info.center,
               .extent          = extent,
               .transform       = info.transform * translate3d(vec3(center, 0)),
               .tint            = run_style.underline,
               .sampler         = info.sampler,
               .edge_smoothness = info.edge_smoothness},
              {.region    = TextRegion::Underline,
               .line      = iln,
               .run       = irun,
               .run_style = run.style});
          }

          goto next_run;
        }

        for (auto [i, sh] : enumerate(glyphs.view().slice(run.glyphs)))
        {
          auto const           iglyph = run.glyphs.offset + i;
          GlyphMetrics const & m      = font.glyphs[sh.glyph];
          AtlasGlyph const &   agl    = atlas.glyphs[sh.glyph];
          Vec2 const           extent = au_to_px(m.extent, font_height);
          Vec2 const           center = Vec2{glyph_cursor, baseline} +
                              au_to_px(m.bearing, font_height) +
                              au_to_px(sh.offset, font_height) + 0.5F * extent;
          auto const advance = au_to_px(sh.advance, font_height);

          // before and after carets
          auto const glyph_carets =
            Slice{ln.carets.offset + (sh.cluster - ln.codepoints.offset), 2};

          if (pass == Pass::GlyphShadows && run_style.has_shadow())
          {
            Vec2 const shadow_extent = extent * run_style.shadow_scale;
            Vec2 const shadow_center =
              center + block.font_scale * run_style.shadow_offset;

            renderer(canvas,
                     {
                       .center = info.center,
                       .extent = shadow_extent,
                       .transform =
                         info.transform * translate3d(vec3(shadow_center, 0)),
                       .tint            = run_style.shadow,
                       .sampler         = info.sampler,
                       .texture         = atlas.textures[agl.layer],
                       .uv              = {agl.uv[0], agl.uv[1]},
                       .edge_smoothness = info.edge_smoothness
            },
                     {.region    = TextRegion::GlyphShadows,
                      .line      = iln,
                      .run       = irun,
                      .run_style = run.style,
                      .glyph     = iglyph,
                      .cluster   = sh.cluster});
          }

          if (pass == Pass::Glyphs && run_style.has_color())
          {
            renderer(
              canvas,
              {
                .center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = agl.has_color ? colors::WHITE : run_style.color,
                .sampler   = info.sampler,
                .texture   = atlas.textures[agl.layer],
                .uv        = {agl.uv[0], agl.uv[1]},
                .edge_smoothness = info.edge_smoothness
            },
              {.region    = TextRegion::Glyphs,
               .line      = iln,
               .run       = irun,
               .run_style = run.style,
               .glyph     = iglyph,
               .cluster   = sh.cluster});
          }

          if (pass == Pass::Caret && !style.caret.is_none())
          {
            auto const glyph_left  = center.x - 0.5 * extent.x;
            auto const glyph_right = center.x + 0.5 * extent.x;

            for (auto const [c, g] : zip(carets, caret_glyphs))
            {
              if (g.glyph == iglyph)
              {
                Vec2 extent{style.caret.thickness, ln.metrics.height};
                Vec2 center;

                if ((direction == TextDirection::LeftToRight && g.after) ||
                    (direction == TextDirection::RightToLeft && !g.after))
                {
                  center.x = glyph_right;
                }
                else
                {
                  center.x = glyph_left;
                }

                center.y = ln_top + 0.5F * ln.metrics.height;

                renderer(
                  canvas,
                  {.center    = info.center,
                   .extent    = extent,
                   .transform = info.transform * translate3d(vec3(center, 0)),
                   .corner_radii    = style.caret.corner_radii,
                   .tint            = style.caret.color,
                   .sampler         = info.sampler,
                   .edge_smoothness = info.edge_smoothness},
                  {.region    = TextRegion::Caret,
                   .line      = iln,
                   .run       = irun,
                   .run_style = run.style,
                   .glyph     = iglyph,
                   .cluster   = sh.cluster,
                   .caret     = c});
              }
            }
          }

          if (pass == Pass::Highlight)
          {
            auto any = any_is(
              highlights, [&](auto & h) { return h.contains(glyph_carets); });

            if (any)
            {
              Vec2 const extent{advance, metrics.height()};
              Vec2 const center = Vec2{glyph_cursor, ln_top} + 0.5F * extent;

              renderer(
                canvas,
                {.center       = info.center,
                 .extent       = extent,
                 .transform    = info.transform * translate3d(vec3(center, 0)),
                 .corner_radii = style.highlight.corner_radii,
                 .stroke       = style.highlight.stroke,
                 .thickness    = style.highlight.thickness,
                 .tint         = style.highlight.color},
                {.region    = TextRegion::Highlight,
                 .line      = iln,
                 .run       = irun,
                 .run_style = run.style,
                 .glyph     = iglyph,
                 .cluster   = sh.cluster});
            }
          }

          glyph_cursor += advance;
        }

      next_run:
        cursor += run_width;
      }

    next_line:
      ln_top = ln_bottom;
    }
  }
}

}    // namespace ash
