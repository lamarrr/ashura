#include "ashura/engine/canvas.h"
#include "ashura/engine/font_impl.h"
#include "ashura/std/math.h"

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

  vtx[beg + i++] = Vec2{1, 1 - radii.z};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[beg + i++] = (1 - radii.z) + radii.z * rotor(s * step);
  }

  vtx[beg + i++] = Vec2{1 - radii.z, 1};

  vtx[beg + i++] = Vec2{-1 + radii.w, 1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[beg + i++] =
        Vec2{-1 + radii.w, 1 - radii.w} + radii.w * rotor(PI / 2 + s * step);
  }

  vtx[beg + i++] = Vec2{-1, 1 - radii.w};

  vtx[beg + i++] = Vec2{-1, -1 + radii.x};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[beg + i++] = (-1 + radii.x) + radii.x * rotor(PI + s * step);
  }

  vtx[beg + i++] = Vec2{-1 + radii.x, -1};

  vtx[beg + i++] = Vec2{1 - radii.y, -1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[beg + i++] = Vec2{1 - radii.y, (-1 + radii.y)} +
                     radii.y * rotor(PI * 3.0f / 2.0f + s * step);
  }

  vtx[beg + i++] = Vec2{1, -1 + radii.y};
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
  ngon_index_counts.uninit();
  ngon_params.uninit();
  rrect_params.uninit();
  blur_params.uninit();
  custom_params.uninit();
  pass_runs.uninit();
}

void Canvas::begin(CanvasSurface const &isurface)
{
  surface = isurface;
}

void Canvas::clear()
{
  vertices.clear();
  indices.clear();
  ngon_index_counts.clear();
  ngon_params.clear();
  rrect_params.clear();
  blur_params.clear();
  custom_params.clear();
  pass_runs.clear();
}

static void add_run(Canvas &canvas, CanvasPassType type, gfx::Rect scissor)
{
  bool new_run = true;

  if (!canvas.pass_runs.is_empty())
  {
    CanvasPassRun const &run = canvas.pass_runs[canvas.pass_runs.size() - 1];
    new_run = run.type != type || run.scissor.offset != scissor.offset ||
              run.scissor.extent != scissor.extent;
  }

  if (!new_run)
  {
    canvas.pass_runs[canvas.pass_runs.size() - 1].end++;
    return;
  }

  u32 current = 0;
  switch (type)
  {
    case CanvasPassType::Blur:
      current = (u32) canvas.blur_params.size();
      break;
    case CanvasPassType::Custom:
      current = (u32) canvas.custom_params.size();
      break;
    case CanvasPassType::Ngon:
      current = (u32) canvas.ngon_params.size();
      break;
    case CanvasPassType::RRect:
      current = (u32) canvas.rrect_params.size();
      break;

    default:
      break;
  }

  CHECK(canvas.pass_runs.push(
      CanvasPassRun{.type = type, .end = current, .scissor = scissor}));
}

void Canvas::circle(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {1, 1, 1, 1},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

void Canvas::rect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {0, 0, 0, 0},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

void Canvas::rrect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.center, desc.extent, desc.transform),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = desc.border_radii / desc.extent.y,
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness,
      .edge_smoothness = desc.edge_smoothness,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

constexpr bool is_transparent(Vec4 const (&colors)[4])
{
  return colors[0].w == 0 && colors[1].w == 0 && colors[2].w == 0 &&
         colors[3].w == 0;
}

void Canvas::text(ShapeDesc const &desc, TextBlock const &block,
                  TextLayout const &layout, TextBlockStyle const &style)
{
  CHECK(style.runs.size() == block.runs.size());
  CHECK(style.runs.size() == block.fonts.size());
  f32 line_y = 0;
  for (Line const &l : layout.lines)
  {
    LineMetrics const &m = l.metrics;
    line_y += m.line_height;
    f32 const padding  = max(m.line_height - (m.ascent + m.descent), 0.0f);
    f32 const baseline = line_y - padding / 2;
    f32 const spacing  = min(layout.extent.x, style.align_width) - m.width;
    f32 const aligned_spacing =
        (m.base_direction == TextDirection::LeftToRight) ?
            space_align(spacing, style.alignment) :
            space_align(spacing, -style.alignment);
    f32 cursor = aligned_spacing;
    for (u32 r = 0; r < l.num_runs;)
    {
      u32 const      first       = r++;
      TextRun const &first_run   = layout.runs[l.first_run + first];
      f32            dir_advance = 0;

      while (r < l.num_runs &&
             layout.runs[l.first_run + r].direction == first_run.direction)
      {
        TextRun const &run = layout.runs[l.first_run + r];
        dir_advance += pt_to_px(run.metrics.advance, run.font_height);
        r++;
      }

      f32 advance =
          (first_run.direction == TextDirection::LeftToRight) ? 0 : dir_advance;
      for (u32 ri = first; ri < r; ri++)
      {
        TextRun const   &run        = layout.runs[l.first_run + ri];
        FontStyle const &font_style = block.fonts[run.style];
        TextStyle const &run_style  = style.runs[run.style];
        FontImpl const  *font       = (FontImpl const *) font_style.font;

        if (!is_transparent(run_style.background_color))
        {
          Vec3 center{cursor + advance / 2, line_y - run.line_height / 2, 0};
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      m.line_height};
          rect(ShapeDesc{.center    = desc.center,
                         .extent    = extent,
                         .tint      = {run_style.background_color[0],
                                       run_style.background_color[1],
                                       run_style.background_color[2],
                                       run_style.background_color[3]},
                         .transform = translate3d(center) * desc.transform *
                                      translate3d(-center),
                         .scissor = desc.scissor});
        }

        for (u32 layer = 0; layer < 2; layer++)
        {
          f32 g_cursor = 0;
          for (u32 g = 0; g < run.num_glyphs; g++)
          {
            GlyphShape const &sh = layout.glyphs[run.first_glyph + g];
            Glyph const      &gl = font->glyphs[sh.glyph];
            Vec2 extent          = pt_to_px(gl.metrics.extent, run.font_height);
            Vec3 center          = to_vec3(
                Vec2{cursor + advance + g_cursor +
                         pt_to_px(gl.metrics.bearing.x, run.font_height),
                     baseline -
                         pt_to_px(gl.metrics.bearing.y, run.font_height)} +
                    pt_to_px(sh.offset, run.font_height) + extent / 2,
                0);

            if (layer == 0 && run_style.shadow_scale != 0)
            {
              Vec3 shadow_center = center + to_vec3(run_style.shadow_offset, 0);
              Vec2 shadow_extent = extent * run_style.shadow_scale;
              rect(ShapeDesc{
                  .center = desc.center,
                  .extent = shadow_extent,
                  .tint = {run_style.shadow_color[0], run_style.shadow_color[1],
                           run_style.shadow_color[2],
                           run_style.shadow_color[3]},
                  .texture   = font->textures[gl.layer],
                  .uv        = {gl.uv[0], gl.uv[1]},
                  .transform = translate3d(shadow_center) * desc.transform *
                               translate3d(-shadow_center),
                  .scissor = desc.scissor});
            }

            if (layer == 1 && !is_transparent(run_style.foreground_color))
            {
              rect(ShapeDesc{.center    = desc.center,
                             .extent    = extent,
                             .tint      = {run_style.foreground_color[0],
                                           run_style.foreground_color[1],
                                           run_style.foreground_color[2],
                                           run_style.foreground_color[3]},
                             .texture   = font->textures[gl.layer],
                             .uv        = {gl.uv[0], gl.uv[1]},
                             .transform = translate3d(center) * desc.transform *
                                          translate3d(-center),
                             .scissor = desc.scissor});
            }

            g_cursor += pt_to_px(sh.advance.x, run.font_height);
          }
        }

        if (run_style.strikethrough_thickness != 0)
        {
          Vec3 center{cursor + advance / 2, baseline - run.font_height / 2, 0};
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      run_style.strikethrough_thickness};
          rect(ShapeDesc{.center    = desc.center,
                         .extent    = extent,
                         .tint      = {run_style.strikethrough_color[0],
                                       run_style.strikethrough_color[1],
                                       run_style.strikethrough_color[2],
                                       run_style.strikethrough_color[3]},
                         .transform = translate3d(center) * desc.transform *
                                      translate3d(-center),
                         .scissor = desc.scissor});
        }

        if (run_style.underline_thickness != 0)
        {
          Vec3 center{cursor + advance / 2, baseline, 0};
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      run_style.underline_thickness};
          rect(ShapeDesc{.center    = desc.center,
                         .extent    = extent,
                         .tint      = {run_style.underline_color[0],
                                       run_style.underline_color[1],
                                       run_style.underline_color[2],
                                       run_style.underline_color[3]},
                         .transform = translate3d(center) * desc.transform *
                                      translate3d(-center),
                         .scissor = desc.scissor});
        }

        advance += (first_run.direction == TextDirection::LeftToRight) ?
                       run.metrics.advance :
                       -run.metrics.advance;
      }

      cursor += dir_advance;
    }
  }
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
      .tiling       = desc.tiling,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));
  u32 const num_indices = (u32) (vertices.size() - first_vertex);

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon, desc.scissor);
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
      .tiling       = desc.tiling,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));
  u32 const num_indices = (u32) (vertices.size() - first_vertex);

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon, desc.scissor);
}

void Canvas::blur(ShapeDesc const &desc, u32 radius)
{
  CHECK(blur_params.push(radius));
}

void Canvas::custom(ShapeDesc const &desc, CustomCanvasPassInfo const &pass)
{
  CHECK(custom_params.push(pass));
  CHECK(pass_runs.push(CanvasPassRun{.type    = CanvasPassType::Custom,
                                     .end     = (u32) custom_params.size(),
                                     .scissor = desc.scissor}));
}

}        // namespace ash
