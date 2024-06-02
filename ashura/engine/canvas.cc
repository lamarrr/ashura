#include "ashura/engine/canvas.h"
#include "ashura/engine/renderer.h"

namespace ash
{

void Path::rect(Vec<Vec2> &vtx)
{
  CHECK(vtx.extend_copy(to_span<Vec2>({{-1, -1}, {1, -1}, {1, 1}, {-1, 1}})));
}

void Path::arc(Vec<Vec2> &vtx, u32 segments, f32 start, f32 stop)
{
  if (segments < 2)
  {
    return;
  }

  usize const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (stop - start) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] = rotor(i * step) * 2 - 1;
  }
}

void Path::circle(Vec<Vec2> &vtx, u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  usize const begin = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[begin + i] = rotor(i * step) * 2 - 1;
  }
}

void Path::rrect(Vec<Vec2> &vtx, u32 segments, Vec4 radii)
{
  if (segments < 8)
  {
    return;
  }

  radii   = radii * 2;
  radii.x = min(radii.x, 2.0f);
  radii.y = min(radii.y, 2.0f);
  radii.z = min(radii.z, 2.0f);
  radii.w = min(radii.w, 2.0f);

  /// clipping
  radii.y          = min(radii.y, 2.0f - radii.x);
  f32 max_radius_z = min(2.0f - radii.x, 1.0f - radii.y);
  radii.z          = min(radii.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 1.0f - radii.z);
  radii.w          = min(radii.w, max_radius_w);

  u32 const   curve_segments = (segments - 8) / 4;
  f32 const   step = (curve_segments == 0) ? 0.0f : ((PI / 2) / curve_segments);
  usize const beg  = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  u32 i = 0;

  vtx[beg + i] = Vec2{1, 1 - radii.z};

  for (u32 s = 0; s < curve_segments; s++, i++)
  {
    vtx[beg + i] = (1 - radii.z) + radii.z * rotor(s * step);
  }

  vtx[beg + i] = Vec2{1 - radii.z, 1};
  i++;

  vtx[beg + i] = Vec2{-1 + radii.w, 1};
  i++;

  for (u32 s = 0; s < curve_segments; s++, i++)
  {
    vtx[beg + i] =
        Vec2{-1 + radii.w, 1 - radii.w} + radii.w * rotor(PI / 2 + s * step);
  }

  vtx[beg + i] = Vec2{-1, 1 - radii.w};
  i++;

  vtx[beg + i] = Vec2{-1, -1 + radii.x};
  i++;

  for (u32 s = 0; s < curve_segments; s++, i++)
  {
    vtx[beg + i] = (-1 + radii.x) + radii.x * rotor(PI + s * step);
  }

  vtx[beg + i] = Vec2{-1 + radii.x, -1};
  i++;

  vtx[beg + i] = Vec2{1 - radii.y, -1};
  i++;

  for (u32 s = 0; s < curve_segments; s++, i++)
  {
    vtx[beg + i] = Vec2{1 - radii.y, (-1 + radii.y)} +
                   radii.y * rotor(PI * 3.0f / 2.0f + s * step);
  }

  vtx[beg + i] = Vec2{1, -1 + radii.y};
}

void Path::brect(Vec<Vec2> &vtx, Vec4 slant)
{
  slant   = slant * 2.0f;
  slant.x = min(slant.x, 2.0f);
  slant.y = min(slant.y, 2.0f);
  slant.z = min(slant.z, 2.0f);
  slant.w = min(slant.w, 2.0f);

  slant.y          = min(slant.y, 2.0f - slant.x);
  f32 max_radius_z = min(2.0f - slant.x, 2.0f - slant.y);
  slant.z          = min(slant.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 2.0f - slant.z);
  slant.w          = min(slant.w, max_radius_w);

  Vec2 const vertices[] = {{-1 + slant.x, -1}, {1 - slant.y, -1},
                           {1, -1 + slant.y},  {1, 1 - slant.z},
                           {1 - slant.z, 1},   {-1 + slant.w, 1},
                           {-1, 1 - slant.w},  {-1, -1 + slant.x}};

  CHECK(vtx.extend_copy(to_span(vertices)));
}

void Path::bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1, Vec2 cp2)
{
  if (segments < 3)
  {
    return;
  }

  usize const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
                        ash::bezier(cp0.y, cp1.y, cp2.y, step * i)};
  }
}

void Path::cubic_bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                        Vec2 cp2, Vec2 cp3)
{
  if (segments < 4)
  {
    return;
  }

  usize const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
        Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
             ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void Path::catmull_rom(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                       Vec2 cp2, Vec2 cp3)
{
  if (segments < 4)
  {
    return;
  }

  usize const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
        Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
             ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

/// generates a triangle fan
inline void add_line_stroke(Vec2 *vtx, Vec2 p0, Vec2 p1, f32 thickness)
{
  // line will be at a parallel angle
  f32 const alpha = dot(normalize(p0), normalize(p1)) * PI;

  /// parallel angle
  Vec2 const f = (thickness / 2) * rotor(alpha);

  // perpendicular angle
  Vec2 const g = f * Vec2{-1, 0};

  vtx[0] = p0 + f;
  vtx[1] = p0 + g;
  vtx[2] = p1 + f;
  vtx[3] = p1 + g;
}

void Path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vtx,
                              f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  usize const beg = points.size();

  CHECK(vtx.extend_uninitialized((points.size() - 1) * 4));

  for (usize i = 0; i < points.size() - 1; i++)
  {
    Vec2 const p0 = points[i];
    Vec2 const p1 = points[i + 1];
    add_line_stroke(&vtx[beg + i * 4], p0, p1, thickness);
  }
}

void Canvas::begin(CanvasSurface const &isurface)
{
  // Canvas params for transform from px to -1 +1 relative viewport space
  // for rrect, transform needs to transform from -1 +1 to px space
  surface = isurface;
  vertices.clear();
  blur_params.clear();
  ngon_params.clear();
  rrect_params.clear();
  custom_params.clear();
  pass_runs.clear();
}

void Canvas::submit(Renderer &renderer)
{
  // offload to gpu, set up passes, render
}

void Canvas::circle(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {1, 1, 1, 1},
      .uv           = {desc.uv[0], desc.uv[1]},
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  if (pass_runs.is_empty() ||
      pass_runs[pass_runs.size() - 1].type != CanvasPassType::RRect)
  {
    CHECK(pass_runs.push(CanvasPassRun{.type   = CanvasPassType::RRect,
                                       .offset = (u32) rrect_params.size(),
                                       .count  = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::rect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {0, 0, 0, 0},
      .uv           = {desc.uv[0], desc.uv[1]},
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  if (pass_runs.is_empty() ||
      pass_runs[pass_runs.size() - 1].type != CanvasPassType::RRect)
  {
    CHECK(pass_runs.push(CanvasPassRun{.type   = CanvasPassType::RRect,
                                       .offset = (u32) rrect_params.size(),
                                       .count  = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::rrect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = desc.border_radii / desc.extent.y,
      .uv           = {desc.uv[0], desc.uv[1]},
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  if (pass_runs.is_empty() ||
      pass_runs[pass_runs.size() - 1].type != CanvasPassType::RRect)
  {
    CHECK(pass_runs.push(CanvasPassRun{.type   = CanvasPassType::RRect,
                                       .offset = (u32) rrect_params.size(),
                                       .count  = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::text_backgrounds_(ShapeDesc const &desc, TextBlock const &block,
                               TextLayout const &layout)
{
  /*
     save();
    translate(block_position);
    draw_rect_filled(line_top, extent, style.background_color);
    restore();
    return *this;
  */
  // TODO(lamarrr): merge segment text backgrounds
  f32 line_top = 0;
  for (LineMetrics const &m : layout.lines)
  {
    f32 x_alignment = 0;

    switch (block.align)
    {
      case TextAlign::Start:
      {
        if (m.base_direction == TextDirection::RightToLeft)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      case TextAlign::Center:
      {
        x_alignment = (layout.span.x - m.width) / 2;
      }
      break;

      case TextAlign::End:
      {
        if (m.base_direction == TextDirection::LeftToRight)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      default:
        break;
    }

    f32 x_cursor = x_alignment;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.run_segments_offset, m.num_run_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];
      if (style.background_color.w > 0)
      {
        draw_text_segment_background(position, Vec2{x_cursor, line_top},
                                     Vec2{s.width, m.line_height}, style);
      }

      x_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::text_underlines_(ShapeDesc const &desc, TextBlock const &block,
                              TextLayout const &layout)
{
  /*
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
  */

  // TODO(lamarrr): merge segment lines and strikethroughs
  f32 line_top = 0;
  for (LineMetrics const &m : layout.lines)
  {
    f32 x_alignment = 0;

    switch (block.align)
    {
      case TextAlign::Start:
      {
        if (m.base_direction == TextDirection::RightToLeft)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      case TextAlign::Center:
      {
        x_alignment = (layout.span.x - m.width) / 2;
      }
      break;

      case TextAlign::End:
      {
        if (m.base_direction == TextDirection::LeftToRight)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      default:
        break;
    }

    f32       x_cursor = x_alignment;
    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.run_segments_offset, m.num_run_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];

      if ((style.underline_color.w > 0 && style.underline_thickness > 0) ||
          (style.strikethrough_color.w > 0 &&
           style.strikethrough_thickness > 0)) [[unlikely]]
      {
        draw_text_segment_lines(position, Vec2{x_cursor, baseline},
                                m.line_height, s.width, style);
      }

      x_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::text_strikethroughs_(ShapeDesc const &desc, TextBlock const &block,
                                  TextLayout const &layout)
{
}

void Canvas::glyph_shadows_(ShapeDesc const &desc, TextBlock const &block,
                            TextLayout const &layout)
{
  /*
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


  */
  f32 line_top = 0;
  for (LineMetrics const &m : layout.lines)
  {
    f32 x_alignment = 0;

    switch (block.align)
    {
      case TextAlign::Start:
      {
        if (m.base_direction == TextDirection::RightToLeft)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      case TextAlign::Center:
      {
        x_alignment = (layout.span.x - m.width) / 2;
      }
      break;

      case TextAlign::End:
      {
        if (m.base_direction == TextDirection::LeftToRight)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      default:
        break;
    }

    f32       x_segment_cursor = x_alignment;
    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &segment :
         layout.run_segments.slice(m.run_segments_offset, m.num_run_segments))
    {
      TextStyle const &style = segment.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[segment.style];

      if (style.shadow_color.w == 0 || style.shadow_scale <= 0)
      {
        continue;
      }

      FontAtlas const &atlas    = font_bundle[segment.font].atlas;
      f32              x_cursor = x_segment_cursor;

      for (GlyphShaping const &shaping : layout.glyph_shapings.slice(
               segment.glyph_shapings_offset, segment.num_glyph_shapings))
      {
        draw_glyph_shadow(position, Vec2{x_cursor, baseline},
                          layout.text_scale_factor, atlas.glyphs[shaping.glyph],
                          shaping, style,
                          atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
        x_cursor +=
            shaping.advance + layout.text_scale_factor * style.letter_spacing;
      }

      x_segment_cursor += segment.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::glyphs_(ShapeDesc const &desc, TextBlock const &block,
                     TextLayout const &layout)
{
  f32 line_top = 0;
  for (LineMetrics const &m : layout.lines)
  {
    f32 x_alignment = 0;

    switch (block.align)
    {
      case TextAlign::Start:
      {
        if (m.base_direction == TextDirection::RightToLeft)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      case TextAlign::Center:
      {
        x_alignment = (layout.span.x - m.width) / 2;
      }
      break;

      case TextAlign::End:
      {
        if (m.base_direction == TextDirection::LeftToRight)
        {
          x_alignment = layout.span.x - m.width;
        }
      }
      break;

      default:
        break;
    }

    f32       x_segment_cursor = x_alignment;
    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &segment :
         layout.run_segments.slice(m.run_segments_offset, m.num_run_segments))
    {
      TextStyle const &style    = segment.style >= block.styles.size() ?
                                      block.default_style :
                                      block.styles[segment.style];
      FontAtlas const &atlas    = font_bundle[segment.font].atlas;
      f32              x_cursor = x_segment_cursor;

      for (GlyphShaping const &sh : layout.glyph_shapings.slice(
               segment.glyph_shapings_offset, segment.num_glyph_shapings))
      {
        draw_glyph(position, Vec2{x_cursor, baseline}, layout.text_scale_factor,
                   atlas.glyphs[sh.glyph], sh, style,
                   atlas.bins[atlas.glyphs[sh.glyph].bin].texture);
        x_cursor +=
            sh.advance + layout.text_scale_factor * style.letter_spacing;
      }

      x_segment_cursor += segment.width;
    }

    line_top += m.line_height;
  }

  /*
  draw_glyph
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
  */
}

void Canvas::simple_text(ShapeDesc const &desc, TextBlock const &block)
{
}

void Canvas::text(ShapeDesc const &desc, TextBlock const &block,
                  TextLayout const &layout)
{
  text_backgrounds_(desc, block, layout);
  text_underlines_(desc, block, layout);
  text_strikethroughs_(desc, block, layout);
  glyph_shadows_(desc, block, layout);
  glyphs_(desc, block, layout);
}

void Canvas::ngon(ShapeDesc const &desc, Span<Vec2 const> vertices)
{
}

void Canvas::line(ShapeDesc const &desc, Span<Vec2 const> vertices)
{
}

void Canvas::blur(ShapeDesc const &desc)
{
}

void Canvas::custom(ShapeDesc const &desc, CustomCanvasPassInfo const &pass)
{
}

}        // namespace ash
