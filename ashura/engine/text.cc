/// SPDX-License-Identifier: MIT
#include "ashura/engine/text.h"
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/engine/systems.h"

namespace ash
{

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
  f32        last_line_bottom  = half_block_extent.y;
  isize      ln                = 0;

  // separated vertical and horizontal hit test
  if (pos.y < line_top)
  {
    ln = ISIZE_MIN;
  }
  else if (pos.y > last_line_bottom)
  {
    ln = ISIZE_MAX;
  }
  else
  {
    for (auto [i, line] : enumerate(lines))
    {
      auto line_bottom = line_top + line.metrics.height;
      if (pos.y <= line_bottom)
      {
        ln = (isize) i;
        break;
      }

      line_top = line_bottom;
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

  Line const & line      = lines[ln];
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

constexpr Tuple<bool, bool> highlight_test(Span<Slice const> highlights,
                                           Slice             carets)
{
  bool fully_covered = false;
  bool any_covered   = false;
  for (Slice highlight : highlights)
  {
    if (carets.contains(highlight))
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

void TextLayout::default_renderer(Canvas & canvas, ShapeInfo const & shape,
                                  TextRegion region)
{
  // [ ]
}

constexpr bool caret_test(Span<isize const> cursor_carets, Slice carets)
{
  return any_is(cursor_carets,
                [&](isize cursor) { return carets.contains(cursor); });
}

// [ ] sync with font_system
void TextLayout::render(Canvas & canvas, ShapeInfo const & info,
                        TextBlock const & block, TextBlockStyle const & style,
                        Span<Slice const> highlights, Span<isize const> carets,
                        CRect const & clip, TextRenderer renderer)
{
  CHECK(style.runs.size() == block.runs.size(), "");
  CHECK(style.runs.size() == block.fonts.size(), "");

  f32 const  block_width = max(extent.x, style.align_width);
  Vec2 const block_extent{block_width, extent.y};

  char scratch[512];

  FallbackAllocator allocator{Arena::from(scratch), default_allocator};

  Vec<CaretGlyph> caret_glyphs{allocator};

  for (auto caret : carets)
  {
    get_caret_glyph(caret).match(
      [&](auto c) { caret_glyphs.push(c).unwrap(); });
  }

  enum Pass : u32
  {
    Background    = 0,
    GlyphShadows  = 1,
    Glyphs        = 2,
    Underline     = 3,
    Strikethrough = 4,
    Highlight     = 5,
    Caret         = 6
  };

  static constexpr u32 NUM_PASSES = 7;

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

    f32 line_top = -block_extent.y * 0.5F;

    for (Line const & ln : lines)
    {
      auto const line_bottom = line_top + ln.metrics.height;
      f32 const  baseline    = line_bottom - ln.metrics.descent;
      auto const direction   = ln.metrics.direction();
      // flip the alignment axis direction if it is an RTL line
      f32 const  alignment =
        style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
      f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
                   ln.metrics.width * 0.5F;

      // [ ] incorrect
      CRect const ln_rect{
        .center = info.center + Vec2{0,           line_bottom      },
        .extent{block_width, ln.metrics.height}
      };

      if (!overlaps(clip, ln_rect))
      {
        goto next_line;
      }

      if (pass == Pass::Highlight)
      {
        auto [fully_covered, has_any] = highlight_test(highlights, ln.carets);

        if (fully_covered)
        {
          canvas.rrect(
            {.center    = info.center,
             .extent    = ln_rect.extent,
             .transform = info.transform * translate3d(vec3(ln_rect.center, 0)),
             .corner_radii = style.highlight.corner_radii,
             .stroke       = style.highlight.stroke,
             .thickness    = style.highlight.thickness,
             .tint         = style.highlight.color});
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

      for (TextRun const & run : runs.view().slice(ln.runs))
      {
        auto const & font_style  = block.fonts[run.style];
        auto const & run_style   = style.runs[run.style];
        auto const   font        = sys->font.get(font_style.font);
        auto const & atlas       = font.gpu_atlas.value();
        f32 const    font_height = block.font_scale * run.font_height;
        auto const   run_metrics = run.metrics.resolve(font_height);
        f32 const    run_width   = run_metrics.advance;
        auto const   direction   = run.direction();

        f32 glyph_cursor = cursor;

        if (pass == Pass::Background)
        {
          if (!run_style.background.is_transparent())
          {
            Vec2 const extent{run_width, run_metrics.height()};
            Vec2 const center{cursor + extent.x * 0.5F,
                              baseline - run_metrics.ascent + extent.y * 0.5F};

            canvas.rrect(
              {.center       = info.center,
               .extent       = extent,
               .transform    = info.transform * translate3d(vec3(center, 0)),
               .corner_radii = run_style.corner_radii,
               .tint         = run_style.background});
          }

          goto next_run;
        }

        if (pass == Pass::Highlight)
        {
          auto [fully_covered, has_any] =
            highlight_test(highlights, run.carets(ln.carets, ln.codepoints));

          if (fully_covered)
          {
            // [ ] verify
            // [ ] needs spacing between highlights
            Vec2 const extent{run_width, run_metrics.height()};
            Vec2 const center{cursor + extent.x * 0.5F,
                              baseline - run_metrics.ascent + extent.y * 0.5F};

            canvas.rrect(
              {.center       = info.center,
               .extent       = extent,
               .transform    = info.transform * translate3d(vec3(center, 0)),
               .corner_radii = style.highlight.corner_radii,
               .stroke       = style.highlight.stroke,
               .thickness    = style.highlight.thickness,
               .tint         = style.highlight.color});

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
              Vec2{cursor, baseline - run_metrics.ascent * 0.5F} +
              extent * 0.5F;
            canvas.rect(
              {.center          = info.center,
               .extent          = extent,
               .transform       = info.transform * translate3d(vec3(center, 0)),
               .tint            = run_style.strikethrough,
               .sampler         = info.sampler,
               .texture         = TextureId::White,
               .uv              = {},
               .tiling          = 1,
               .edge_smoothness = info.edge_smoothness});
          }

          goto next_run;
        }

        if (pass == Pass::Underline)
        {
          if (run_style.underline_thickness != 0)
          {
            Vec2 const extent{run_width,
                              block.font_scale * run_style.underline_thickness};
            Vec2 const center = Vec2{cursor, baseline + 2} + extent * 0.5F;
            canvas.rect(
              {.center          = info.center,
               .extent          = extent,
               .transform       = info.transform * translate3d(vec3(center, 0)),
               .tint            = run_style.underline,
               .sampler         = info.sampler,
               .texture         = TextureId::White,
               .uv              = {},
               .tiling          = 1,
               .edge_smoothness = info.edge_smoothness});
          }

          goto next_run;
        }

        for (auto [i, sh] : enumerate(glyphs.view().slice(run.glyphs)))
        {
          auto const           iglyph = i + run.glyphs.offset;
          GlyphMetrics const & m      = font.glyphs[sh.glyph];
          AtlasGlyph const &   agl    = atlas.glyphs[sh.glyph];
          Vec2 const           extent = au_to_px(m.extent, font_height);
          Vec2 const           center = Vec2{glyph_cursor, baseline} +
                              au_to_px(m.bearing, font_height) +
                              au_to_px(sh.offset, font_height) + 0.5F * extent;
          f32 const advance = au_to_px(sh.advance, font_height);

          // before and after carets
          auto const glyph_carets =
            Slice{ln.carets.offset + (sh.cluster - ln.codepoints.offset), 2};

          if (pass == Pass::GlyphShadows && run_style.has_shadow())
          {
            Vec2 const shadow_extent = extent * run_style.shadow_scale;
            Vec2 const shadow_center =
              center + block.font_scale * run_style.shadow_offset;
            canvas.rect({
              .center    = info.center,
              .extent    = shadow_extent,
              .transform = info.transform * translate3d(vec3(shadow_center, 0)),
              .tint      = run_style.shadow,
              .sampler   = info.sampler,
              .texture   = atlas.textures[agl.layer],
              .uv        = {agl.uv[0], agl.uv[1]},
              .tiling    = 1,
              .edge_smoothness = info.edge_smoothness
            });
          }

          if (pass == Pass::Glyphs && run_style.has_color())
          {
            canvas.rect({
              .center    = info.center,
              .extent    = extent,
              .transform = info.transform * translate3d(vec3(center, 0)),
              .tint      = agl.has_color ? colors::WHITE : run_style.color,
              .sampler   = info.sampler,
              .texture   = atlas.textures[agl.layer],
              .uv        = {agl.uv[0], agl.uv[1]},
              .tiling    = 1,
              .edge_smoothness = info.edge_smoothness
            });
          }

          if (pass == Pass::Caret && !style.caret.is_none())
          {
            for (CaretGlyph const & c : caret_glyphs)
            {
              if (c.glyph == iglyph)
              {
                // [ ] c.after
                //  [ ] draw caret rect
                canvas.rrect(
                  {.center    = info.center,
                   .extent    = extent,
                   .transform = info.transform * translate3d(vec3(center, 0)),
                   .corner_radii = style.caret.corner_radii,
                   .tint         = style.caret.color});
              }
            }
          }

          if (pass == Pass::Highlight)
          {
            auto any = any_is(
              highlights, [&](auto & h) { return h.contains(glyph_carets); });

            if (any)
            {
              // [ ] draw highlight rect
            }
          }

          glyph_cursor += advance;
        }

      next_run:
        cursor += run_width;
      }

    next_line:
      line_top += ln.metrics.height;
    }
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

}    // namespace ash
