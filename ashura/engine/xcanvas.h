#pragma once

#include "ashura/engine/font.h"
#include "ashura/engine/text.h"
#include "ashura/gfx/gfx.h"
#include "ashura/gfx/image.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct Canvas
{
  Canvas &draw_glyph(Vec2 block_position, Vec2 baseline, f32 text_scale_factor,
                     Glyph const &glyph, GlyphShaping const &shaping,
                     TextStyle const &style, image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    Vec2 offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} *
                      style.font_height * text_scale_factor +
                  shaping.offset;
    Vec2 extent = glyph.metrics.extent * style.font_height * text_scale_factor;
    Mat3 transform = state.global_transform * translate2d(block_position) *
                     state.local_transform;

    if (!overlaps({0, 0}, viewport_extent, ash::transform(transform, offset),
                  ash::transform(transform, offset + extent)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {
        {.position = offset, .uv = glyph.uv0, .color = style.foreground_color},
        {.position = {offset.x + extent.x, offset.y},
         .uv       = {glyph.uv1.x, glyph.uv0.y},
         .color    = style.foreground_color},
        {.position = offset + extent,
         .uv       = glyph.uv1,
         .color    = style.foreground_color},
        {.position = {offset.x, offset.y + extent.y},
         .uv       = {glyph.uv0.x, glyph.uv1.y},
         .color    = style.foreground_color}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands
        .push(
            DrawCommand{.pipeline       = DEFAULT_GLYPH_PIPELINE,
                        .nvertices      = 4,
                        .nindices       = 6,
                        .first_instance = 0,
                        .ninstances     = 1,
                        .scissor_offset = state.scissor_offset,
                        .scissor_extent = state.scissor_extent,
                        .textures       = {atlas}}
                .with_push_constant(transpose(make_transform(block_position))))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_glyph_shadow(Vec2 block_position, Vec2 baseline,
                            f32 text_scale_factor, Glyph const &glyph,
                            GlyphShaping const &shaping, TextStyle const &style,
                            image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    // TODO(lamarrr): add offset to shadow scale? and let offset be from
    // midpoint??
    Vec2 offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} *
                      style.font_height * text_scale_factor +
                  shaping.offset;
    Vec2 extent = glyph.metrics.extent * style.font_height * text_scale_factor;
    Mat3 transform = state.global_transform *
                     (translate2d(block_position) * state.local_transform);

    Vec2 shadow_offset = offset + style.shadow_offset;
    Vec2 shadow_extent = extent * style.shadow_scale;

    if (!overlaps({0, 0}, viewport_extent,
                  ash::transform(transform, shadow_offset),
                  ash::transform(transform, shadow_offset + shadow_extent)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {
        {.position = shadow_offset,
         .uv       = glyph.uv0,
         .color    = style.shadow_color},
        {.position = {shadow_offset.x + shadow_extent.x, shadow_offset.y},
         .uv       = {glyph.uv1.x, glyph.uv0.y},
         .color    = style.shadow_color},
        {.position = shadow_offset + shadow_extent,
         .uv       = glyph.uv1,
         .color    = style.shadow_color},
        {.position = {shadow_offset.x, shadow_offset.y + shadow_extent.y},
         .uv       = {glyph.uv0.x, glyph.uv1.y},
         .color    = style.shadow_color}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands
        .push(
            DrawCommand{.pipeline       = DEFAULT_GLYPH_PIPELINE,
                        .nvertices      = 4,
                        .nindices       = 6,
                        .first_instance = 0,
                        .ninstances     = 1,
                        .scissor_offset = state.scissor_offset,
                        .scissor_extent = state.scissor_extent,
                        .textures       = {atlas}}
                .with_push_constant(transpose(make_transform(block_position))))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_text_segment_lines(Vec2 block_position, Vec2 baseline,
                                  f32 line_height, f32 segment_width,
                                  TextStyle const &style)
  {
    save();
    translate(block_position);

    if (style.strikethrough_color.w > 0 && style.strikethrough_thickness > 0)
    {
      Vertex2d const strikethrough_path[] = {
          {.position = baseline - Vec2{0, line_height / 2},
           .uv       = {},
           .color    = style.strikethrough_color},
          {.position = baseline - Vec2{-segment_width, line_height / 2},
           .uv       = {},
           .color    = style.strikethrough_color}};

      draw_path(strikethrough_path, Vec2{0, 0}, Vec2{0, 0},
                style.strikethrough_thickness, false);
    }

    if (style.underline_color.w > 0 && style.underline_thickness > 0)
    {
      Vertex2d const underline_path[] = {
          {.position = baseline, .uv = {}, .color = style.underline_color},
          {.position = baseline + Vec2{segment_width, 0},
           .uv       = {},
           .color    = style.underline_color}};

      draw_path(underline_path, Vec2{0, 0}, Vec2{0, 0},
                style.underline_thickness, false);
    }

    restore();

    return *this;
  }

  Canvas &draw_text_segment_background(Vec2 block_position, Vec2 line_top,
                                       Vec2 extent, TextStyle const &style)
  {
    save();
    translate(block_position);
    draw_rect_filled(line_top, extent, style.background_color);
    restore();
    return *this;
  }

  // TODO(lamarrr): text gradient, reset on each line or continue???? how does
  // css do it?
  Canvas &draw_text(TextBlock const &block, TextLayout const &layout,
                    Span<BundledFont const> font_bundle, Vec2 const position)
  {
    /// TEXT BACKGROUNDS ///
    {
      // TODO(lamarrr): merge segment text backgrounds
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32 x_cursor = x_alignment;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];
          if (segment_style.background_color.w > 0)
          {
            draw_text_segment_background(position, Vec2{x_cursor, line_top},
                                         Vec2{segment.width, line.line_height},
                                         segment_style);
          }

          x_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// GLYPH SHADOWS ///
    {
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_segment_cursor = x_alignment;
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];

          if (segment_style.shadow_color.w == 0 ||
              segment_style.shadow_scale <= 0)
          {
            continue;
          }

          FontAtlas const &atlas    = font_bundle[segment.font].atlas;
          f32              x_cursor = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(
                   segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph_shadow(
                position, Vec2{x_cursor, baseline}, layout.text_scale_factor,
                atlas.glyphs[shaping.glyph], shaping, segment_style,
                atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance +
                        layout.text_scale_factor * segment_style.letter_spacing;
          }

          x_segment_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// GLYPHS ///
    {
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_segment_cursor = x_alignment;
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];
          FontAtlas const &atlas    = font_bundle[segment.font].atlas;
          f32              x_cursor = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(
                   segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph(position, Vec2{x_cursor, baseline},
                       layout.text_scale_factor, atlas.glyphs[shaping.glyph],
                       shaping, segment_style,
                       atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance +
                        layout.text_scale_factor * segment_style.letter_spacing;
          }

          x_segment_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// UNDERLINES AND STRIKETHROUGHS ///
    {
      // TODO(lamarrr): merge segment lines and strikethroughs
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_cursor = x_alignment;
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];

          if ((segment_style.underline_color.w > 0 &&
               segment_style.underline_thickness > 0) ||
              (segment_style.strikethrough_color.w > 0 &&
               segment_style.strikethrough_thickness > 0)) [[unlikely]]
          {
            draw_text_segment_lines(position, Vec2{x_cursor, baseline},
                                    line.line_height, segment.width,
                                    segment_style);
          }

          x_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    return *this;
  }
};

}        // namespace ash
