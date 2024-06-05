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

  usize const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] = rotor(i * step) * 2 - 1;
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

inline void add_line_stroke(Vec2 *vtx, u32 *idx, u32 offset, Vec2 p0, Vec2 p1,
                            f32 thickness)
{
  // line will be at a parallel angle
  f32 const alpha = dot(normalize(p0), normalize(p1)) * PI;

  /// parallel angle
  Vec2 const f = (thickness / 2) * rotor(alpha);

  // perpendicular angle
  Vec2 const g = f * Vec2{-1, 0};

  vtx[0] = p0 + f;
  vtx[1] = p0 + g;
  vtx[2] = p1 + g;
  vtx[3] = p1 + f;
  idx[0] = offset;
  idx[1] = offset + 1;
  idx[2] = offset + 2;
  idx[3] = offset;
  idx[4] = offset + 2;
  idx[5] = offset + 3;
}

void Path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vertices,
                              Vec<u32> &indices, f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  u32 const first_v = (u32) vertices.size();
  u32 const first_i = (u32) indices.size();
  CHECK(vertices.extend_uninitialized((points.size() - 1) * 4));
  CHECK(indices.extend_uninitialized((points.size() - 1) * 6));

  Vec2 *out_v = vertices.data() + first_v;
  u32  *out_i = indices.data() + first_i;
  u32   vtx   = 0;

  for (usize i = 0; i < points.size() - 1; i++)
  {
    Vec2 const p0 = points[i];
    Vec2 const p1 = points[i + 1];
    add_line_stroke(out_v, out_i, vtx, p0, p1, thickness);
    out_v += 4;
    out_i += 6;
    vtx += 4;
  }
}

void Path::triangulate_ngon(Span<Vec2 const> points, Vec<Vec2> &vertices,
                            Vec<u32> &indices)
{
  if (points.size() < 3)
  {
    return;
  }
  u32 const ntriangles = (u32) ((points.size()) - 1);
  u32 const first_v    = (u32) vertices.size();
  u32 const first_i    = (u32) indices.size();
  CHECK(vertices.extend_copy(points));
  CHECK(indices.extend_uninitialized(ntriangles * 3));

  u32 *out_i = indices.data() + first_i;
  u32  vtx   = 1;
  for (u32 i = 0; i < ntriangles; i++)
  {
    out_i[0] = 0;
    out_i[1] = vtx;
    out_i[2] = vtx + 1;
    out_i += 3;
    vtx += 1;
  }
}

void Canvas::init()
{
}

void Canvas::uninit()
{
  vertices.uninit();
  indices.uninit();
  ngon_params.uninit();
  rrect_params.uninit();
  custom_params.uninit();
  pass_runs.uninit();
}

void Canvas::begin(CanvasSurface const &isurface)
{
  surface = isurface;
  vertices.clear();
  indices.clear();
  ngon_params.clear();
  rrect_params.clear();
  custom_params.clear();
  pass_runs.clear();
}

void Canvas::submit(Renderer &renderer)
{
  // TODO(lamarrr): render
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
    CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::RRect,
                                       .first = ((u32) rrect_params.size()) - 1,
                                       .count = 1}));
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
    CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::RRect,
                                       .first = ((u32) rrect_params.size()) - 1,
                                       .count = 1}));
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
    CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::RRect,
                                       .first = ((u32) rrect_params.size()) - 1,
                                       .count = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::text_backgrounds_(ShapeDesc const &desc, TextBlock const &block,
                               TextLayout const &layout)
{
  f32       line_top    = 0;
  f32 const x_alignment = (block.x_align + 1) / 2;
  for (LineMetrics const &m : layout.lines)
  {
    f32 const x_dist = layout.span.x - m.width;
    f32 x_cursor = x_dist * ((m.base_direction == TextDirection::LeftToRight) ?
                                 x_alignment :
                                 (1 - x_alignment));

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.first_segment, m.num_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];
      Vec2             offset{x_cursor, line_top};
      Vec2             extent{s.width, m.line_height};
      Vec2 center = desc.center - layout.span / 2 + offset + extent / 2;

      rect(ShapeDesc{
          .center       = center,
          .extent       = extent,
          .border_radii = {},
          .stroke       = 0,
          .tint         = {style.background_color[0], style.background_color[1],
                           style.background_color[2], style.background_color[3]},
          .texture      = 0,
          .uv           = {},
          .transform    = desc.transform,
          .scissor_offset = desc.scissor_offset,
          .scissor_extent = desc.scissor_extent});

      x_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::text_underlines_(ShapeDesc const &desc, TextBlock const &block,
                              TextLayout const &layout)
{
  f32       line_top    = 0;
  f32 const x_alignment = (block.x_align + 1) / 2;
  for (LineMetrics const &m : layout.lines)
  {
    f32 const x_dist = layout.span.x - m.width;
    f32 x_cursor = x_dist * ((m.base_direction == TextDirection::LeftToRight) ?
                                 x_alignment :
                                 (1 - x_alignment));

    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.first_segment, m.num_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];

      Vec2 offset{x_cursor, baseline};
      Vec2 extent{s.width, style.underline_thickness};
      Vec2 center = desc.center - layout.span / 2 + offset + extent / 2;

      rect(ShapeDesc{
          .center         = center,
          .extent         = extent,
          .border_radii   = {},
          .stroke         = 0,
          .tint           = {style.underline_color[0], style.underline_color[1],
                             style.underline_color[2], style.underline_color[3]},
          .texture        = 0,
          .uv             = {},
          .transform      = desc.transform,
          .scissor_offset = desc.scissor_offset,
          .scissor_extent = desc.scissor_extent});

      x_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::text_strikethroughs_(ShapeDesc const &desc, TextBlock const &block,
                                  TextLayout const &layout)
{
  f32       line_top    = 0;
  f32 const x_alignment = (block.x_align + 1) / 2;
  for (LineMetrics const &m : layout.lines)
  {
    f32 const x_dist = layout.span.x - m.width;
    f32 x_cursor = x_dist * ((m.base_direction == TextDirection::LeftToRight) ?
                                 x_alignment :
                                 (1 - x_alignment));

    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.first_segment, m.num_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];

      Vec2 offset{x_cursor, baseline - m.line_height / 2};
      Vec2 extent{s.width, style.strikethrough_thickness};
      Vec2 center = desc.center - layout.span / 2 + offset + extent / 2;

      rect(ShapeDesc{
          .center       = center,
          .extent       = extent,
          .border_radii = {},
          .stroke       = 0,
          .tint = {style.strikethrough_color[0], style.strikethrough_color[1],
                   style.strikethrough_color[2], style.strikethrough_color[3]},
          .texture        = 0,
          .uv             = {},
          .transform      = desc.transform,
          .scissor_offset = desc.scissor_offset,
          .scissor_extent = desc.scissor_extent});

      x_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::glyph_shadows_(ShapeDesc const &desc, TextBlock const &block,
                            TextLayout const &layout)
{
  f32       line_top    = 0;
  f32 const x_alignment = (block.x_align + 1) / 2;
  for (LineMetrics const &m : layout.lines)
  {
    f32 const x_dist = layout.span.x - m.width;
    f32 x_cursor = x_dist * ((m.base_direction == TextDirection::LeftToRight) ?
                                 x_alignment :
                                 (1 - x_alignment));

    f32       x_segment_cursor = x_alignment;
    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.first_segment, m.num_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];

      Result font_info_r = font_manager->get_info(s.font);
      if (font_info_r.is_err())
      {
        continue;
      }

      FontInfo font_info = font_info_r.unwrap();
      f32      x_cursor  = x_segment_cursor;

      for (GlyphShaping const &sh :
           layout.glyph_shapings.slice(s.first_shaping, s.num_shapings))
      {
        Glyph const &g = font_info.glyphs[sh.glyph];

        Vec2 offset = Vec2{x_cursor, baseline} +
                      Vec2{g.metrics.bearing.x, -g.metrics.bearing.y} *
                          style.font_height * layout.text_scale_factor +
                      sh.offset;
        Vec2 extent =
            (g.metrics.extent * style.font_height * layout.text_scale_factor) *
            style.shadow_scale;
        Vec2 center = desc.center - layout.span / 2 + offset + extent / 2 +
                      style.shadow_offset;
        u32 texture = font_info.textures[font_info.glyphs[sh.glyph].layer];

        rect(ShapeDesc{.center       = center,
                       .extent       = extent,
                       .border_radii = {},
                       .stroke       = 0,
                       .tint    = {style.shadow_color[0], style.shadow_color[1],
                                   style.shadow_color[2], style.shadow_color[3]},
                       .texture = texture,
                       .uv      = {g.uv[0], g.uv[1]},
                       .transform      = desc.transform,
                       .scissor_offset = desc.scissor_offset,
                       .scissor_extent = desc.scissor_extent});

        x_cursor +=
            sh.advance + layout.text_scale_factor * style.letter_spacing;
      }

      x_segment_cursor += s.width;
    }

    line_top += m.line_height;
  }
}

void Canvas::glyphs_(ShapeDesc const &desc, TextBlock const &block,
                     TextLayout const &layout)
{
  f32       line_top    = 0;
  f32 const x_alignment = (block.x_align + 1) / 2;
  for (LineMetrics const &m : layout.lines)
  {
    f32 const x_dist = layout.span.x - m.width;
    f32 x_cursor = x_dist * ((m.base_direction == TextDirection::LeftToRight) ?
                                 x_alignment :
                                 (1 - x_alignment));

    f32       x_segment_cursor = x_alignment;
    f32 const line_gap = max(m.line_height - (m.ascent + m.descent), 0.0f) / 2;
    f32 const baseline = line_top + m.line_height - line_gap - m.descent;

    for (TextRunSegment const &s :
         layout.run_segments.slice(m.first_segment, m.num_segments))
    {
      TextStyle const &style = s.style >= block.styles.size() ?
                                   block.default_style :
                                   block.styles[s.style];

      Result font_info_r = font_manager->get_info(s.font);
      if (font_info_r.is_err())
      {
        continue;
      }

      FontInfo font_info = font_info_r.unwrap();
      f32      x_cursor  = x_segment_cursor;

      for (GlyphShaping const &sh :
           layout.glyph_shapings.slice(s.first_shaping, s.num_shapings))
      {
        Glyph const &g = font_info.glyphs[sh.glyph];

        Vec2 offset = Vec2{x_cursor, baseline} +
                      Vec2{g.metrics.bearing.x, -g.metrics.bearing.y} *
                          style.font_height * layout.text_scale_factor +
                      sh.offset;
        Vec2 extent =
            g.metrics.extent * style.font_height * layout.text_scale_factor;
        Vec2 center  = desc.center - layout.span / 2 + offset + extent / 2;
        u32  texture = font_info.textures[font_info.glyphs[sh.glyph].layer];

        rect(ShapeDesc{
            .center       = center,
            .extent       = extent,
            .border_radii = {},
            .stroke       = 0,
            .tint      = {style.foreground_color[0], style.foreground_color[1],
                          style.foreground_color[2], style.foreground_color[3]},
            .texture   = texture,
            .uv        = {g.uv[0], g.uv[1]},
            .transform = desc.transform,
            .scissor_offset = desc.scissor_offset,
            .scissor_extent = desc.scissor_extent});

        x_cursor +=
            sh.advance + layout.text_scale_factor * style.letter_spacing;
      }

      x_segment_cursor += s.width;
    }

    line_top += m.line_height;
  }
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

void Canvas::ngon(ShapeDesc const &desc, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return;
  }

  u32 const first_index  = (u32) indices.size();
  u32 const first_vertex = (u32) vertices.size();
  Path::triangulate_ngon(points, vertices, indices);
  CHECK(ngon_params.push(NgonParam{
      .transform    = desc.transform,
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));
  u32 const num_indices = (u32) (vertices.size() - first_vertex);

  CHECK(ngon_draw_commands.push(NgonDrawCommand{.num_indices = num_indices}));

  if (pass_runs.is_empty() ||
      pass_runs[pass_runs.size() - 1].type != CanvasPassType::Ngon)
  {
    CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::Ngon,
                                       .first = ((u32) ngon_params.size()) - 1,
                                       .count = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::line(ShapeDesc const &desc, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return;
  }

  u32 const first_index  = (u32) indices.size();
  u32 const first_vertex = (u32) vertices.size();
  Path::triangulate_stroke(points, vertices, indices, desc.thickness);
  CHECK(ngon_params.push(NgonParam{
      .transform    = desc.transform,
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));
  u32 const num_indices = (u32) (vertices.size() - first_vertex);

  CHECK(ngon_draw_commands.push(NgonDrawCommand{.num_indices = num_indices}));

  if (pass_runs.is_empty() ||
      pass_runs[pass_runs.size() - 1].type != CanvasPassType::Ngon)
  {
    CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::Ngon,
                                       .first = ((u32) ngon_params.size()) - 1,
                                       .count = 1}));
  }
  else
  {
    pass_runs[pass_runs.size() - 1].count++;
  }
}

void Canvas::custom(ShapeDesc const &desc, CustomCanvasPassInfo const &pass)
{
  CHECK(custom_params.push(pass));
  CHECK(pass_runs.push(CanvasPassRun{.type  = CanvasPassType::Custom,
                                     .first = ((u32) custom_params.size()) - 1,
                                     .count = 1}));
}

}        // namespace ash
