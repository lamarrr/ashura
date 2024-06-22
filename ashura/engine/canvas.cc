#include "ashura/engine/canvas.h"
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

  u32 const first = (u32) vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (stop - start) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) * 2 - 1;
  }
}

void Path::circle(Vec<Vec2> &vtx, u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first = (u32) vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) * 2 - 1;
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

  u32 const curve_segments = (segments - 8) / 4;
  f32 const step  = (curve_segments == 0) ? 0.0f : ((PI / 2) / curve_segments);
  u32 const first = (u32) vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  u32 i = 0;

  vtx[first + i++] = Vec2{1, 1 - radii.z};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (1 - radii.z) + radii.z * rotor(s * step);
  }

  vtx[first + i++] = Vec2{1 - radii.z, 1};

  vtx[first + i++] = Vec2{-1 + radii.w, 1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
        Vec2{-1 + radii.w, 1 - radii.w} + radii.w * rotor(PI / 2 + s * step);
  }

  vtx[first + i++] = Vec2{-1, 1 - radii.w};

  vtx[first + i++] = Vec2{-1, -1 + radii.x};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (-1 + radii.x) + radii.x * rotor(PI + s * step);
  }

  vtx[first + i++] = Vec2{-1 + radii.x, -1};

  vtx[first + i++] = Vec2{1 - radii.y, -1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = Vec2{1 - radii.y, (-1 + radii.y)} +
                       radii.y * rotor(PI * 3.0f / 2.0f + s * step);
  }

  vtx[first + i++] = Vec2{1, -1 + radii.y};
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

  u32 const first = (u32) vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
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

  u32 const first = (u32) vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] =
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

  u32 const beg = vtx.size();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = 1.0f / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
        Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
             ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void Path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vertices,
                              Vec<u32> &indices, f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  u32 const first_vtx    = (u32) vertices.size();
  u32 const first_idx    = (u32) indices.size();
  u32 const num_points   = (u32) points.size();
  u32 const num_vertices = (num_points - 1) * 4;
  u32 const num_indices  = (num_points - 1) * 6 + (num_points - 2) * 6;
  CHECK(vertices.extend_uninitialized(num_vertices));
  CHECK(indices.extend_uninitialized(num_indices));

  Vec2 *vtx  = vertices.data() + first_vtx;
  u32  *idx  = indices.data() + first_idx;
  u32   ivtx = 0;

  for (u32 i = 0; i < num_points - 1; i++)
  {
    Vec2 const p0    = points[i];
    Vec2 const p1    = points[i + 1];
    Vec2 const d     = p1 - p0;
    f32 const  alpha = std::atanf(d.y / d.x);

    // parallel angle
    Vec2 const down = (thickness / 2) * rotor(alpha + PI / 2);

    // perpendicular angle
    Vec2 const up = -down;

    vtx[0] = p0 + up;
    vtx[1] = p0 + down;
    vtx[2] = p1 + up;
    vtx[3] = p1 + down;
    idx[0] = ivtx;
    idx[1] = ivtx + 1;
    idx[2] = ivtx + 3;
    idx[3] = ivtx;
    idx[4] = ivtx + 3;
    idx[5] = ivtx + 2;

    if (i != 0)
    {
      u32 prev = ivtx - 2;
      idx[6]   = prev;
      idx[7]   = prev + 1;
      idx[8]   = ivtx + 1;
      idx[9]   = prev;
      idx[10]  = prev + 1;
      idx[11]  = ivtx;
      idx += 6;
    }

    idx += 6;
    vtx += 4;
    ivtx += 4;
  }
}

void Path::triangles(Span<Vec2 const> points, Vec<u32> &indices)
{
  if (points.size() < 3)
  {
    return;
  }
  u32 const num_points    = (u32) points.size();
  u32 const num_triangles = num_points / 3;
  u32 const first_idx     = (u32) indices.size();
  CHECK(indices.extend_uninitialized(num_triangles * 3));

  u32 *idx = indices.data() + first_idx;
  for (u32 i = 0; i < num_triangles * 3; i += 3)
  {
    idx[i]     = i;
    idx[i + 1] = i + 1;
    idx[i + 2] = i + 2;
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

static inline void add_run(Canvas &canvas, CanvasPassType type,
                           gfx::Rect scissor)
{
  scissor.offset.x = min(scissor.offset.x, canvas.surface.extent.x);
  scissor.offset.y = min(scissor.offset.y, canvas.surface.extent.y);
  scissor.extent.x =
      min(canvas.surface.extent.x - scissor.offset.x, scissor.extent.x);
  scissor.extent.y =
      min(canvas.surface.extent.y - scissor.offset.y, scissor.extent.y);

  bool new_run = true;

  if (!canvas.pass_runs.is_empty())
  {
    CanvasPassRun const &run = canvas.pass_runs[canvas.pass_runs.size() - 1];
    new_run = run.type != type || run.scissor.offset != scissor.offset ||
              run.scissor.extent != scissor.extent;
  }

  if (!new_run)
  {
    canvas.pass_runs[canvas.pass_runs.size() - 1].count++;
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

  CHECK(canvas.pass_runs.push(CanvasPassRun{
      .type = type, .first = current - 1, .count = 1, .scissor = scissor}));
}

void Canvas::circle(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {1, 1, 1, 1},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness / desc.extent.y,
      .edge_smoothness = desc.edge_smoothness,
      .sampler         = desc.sampler,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

void Canvas::rect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = {0, 0, 0, 0},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness / desc.extent.y,
      .edge_smoothness = desc.edge_smoothness,
      .sampler         = desc.sampler,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

void Canvas::rrect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = surface.mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = desc.border_radii / desc.extent.y,
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness / desc.extent.y,
      .edge_smoothness = desc.edge_smoothness,
      .sampler         = desc.sampler,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect, desc.scissor);
}

constexpr bool is_transparent(ColorGradient const &g)
{
  return g[0].w == 0 && g[1].w == 0 && g[2].w == 0 && g[3].w == 0;
}

void Canvas::text(ShapeDesc const &desc, TextBlock const &block,
                  TextLayout const &layout, TextBlockStyle const &style,
                  Span<FontAtlasResource const *const> atlases)
{
  CHECK(style.runs.size() == block.runs.size());
  CHECK(style.runs.size() == block.fonts.size());
  for (u32 i = 0; i < (u32) block.fonts.size(); i++)
  {
    CHECK(atlases[i] != nullptr);
    CHECK(block.fonts[i].font != nullptr);
    FontInfo f = get_font_info(block.fonts[i].font);
    CHECK(atlases[i]->glyphs.size() == f.glyphs.size());
  }

  f32 const  block_width = max(layout.extent.x, style.align_width);
  Vec2 const half_block_extent{block_width / 2, layout.extent.y / 2};
  f32        line_y = 0;
  for (Line const &l : layout.lines)
  {
    line_y += l.metrics.height;
    f32 const baseline = line_y - l.metrics.descent;
    f32 const spacing  = max(block_width - l.metrics.width, 0.0f);
    f32 const aligned_spacing =
        (l.metrics.base_direction == TextDirection::LeftToRight) ?
            space_align(spacing, style.alignment) :
            space_align(spacing, -style.alignment);
    f32 cursor = aligned_spacing;
    for (u32 r = 0; r < l.num_runs;)
    {
      u32 const      first     = r++;
      TextRun const &first_run = layout.runs[l.first_run + first];
      f32            dir_advance =
          pt_to_px(first_run.metrics.advance, first_run.font_height);

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
        TextRun const           &run        = layout.runs[l.first_run + ri];
        FontStyle const         &font_style = block.fonts[run.style];
        TextStyle const         &run_style  = style.runs[run.style];
        FontInfo                 font       = get_font_info(font_style.font);
        FontAtlasResource const *atlas      = atlases[run.style];

        if (run.direction == TextDirection::RightToLeft)
        {
          advance -= pt_to_px(run.metrics.advance, run.font_height);
        }

        if (!is_transparent(run_style.background))
        {
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      pt_to_px(run.metrics.ascent, run.font_height) +
                          l.metrics.height};
          Vec2 offset{cursor + advance, line_y - l.metrics.height};
          rect(ShapeDesc{.center = desc.center,
                         .extent = extent,
                         .transform =
                             desc.transform *
                             translate3d(to_vec3(offset + extent / 2, 0)) *
                             translate3d(to_vec3(-half_block_extent, 0)),
                         .tint    = run_style.background,
                         .scissor = desc.scissor});
        }

        for (u32 layer = 0; layer < 2; layer++)
        {
          f32 g_cursor = 0;
          for (u32 g = 0; g < run.num_glyphs; g++)
          {
            GlyphShape const &sh  = layout.glyphs[run.first_glyph + g];
            Glyph const      &gl  = font.glyphs[sh.glyph];
            AtlasGlyph const &agl = atlas->glyphs[sh.glyph];
            Vec2 const extent = pt_to_px(gl.metrics.extent, run.font_height);
            Vec2 const offset =
                Vec2{cursor + advance + g_cursor, baseline} +
                pt_to_px(gl.metrics.bearing, run.font_height) * Vec2{1, -1} +
                pt_to_px(sh.offset, run.font_height);

            if (layer == 0 && run_style.shadow_scale != 0 &&
                !is_transparent(run_style.shadow))
            {
              Vec2 shadow_extent = extent * run_style.shadow_scale;
              Vec2 shadow_offset = (offset + extent / 2) - shadow_extent / 2 +
                                   run_style.shadow_offset;
              rect(ShapeDesc{.center = desc.center,
                             .extent = shadow_extent,
                             .transform =
                                 desc.transform *
                                 translate3d(to_vec3(
                                     shadow_offset + shadow_extent / 2, 0)) *
                                 translate3d(to_vec3(-half_block_extent, 0)),
                             .tint            = run_style.shadow,
                             .sampler         = desc.sampler,
                             .texture         = atlas->textures[agl.layer],
                             .uv              = {agl.uv[0], agl.uv[1]},
                             .tiling          = desc.tiling,
                             .edge_smoothness = desc.edge_smoothness,
                             .scissor         = desc.scissor});
            }

            if (layer == 1 && !is_transparent(run_style.foreground))
            {
              rect(ShapeDesc{.center = desc.center,
                             .extent = extent,
                             .transform =
                                 desc.transform *
                                 translate3d(to_vec3(offset + extent / 2, 0)) *
                                 translate3d(to_vec3(-half_block_extent, 0)),
                             .tint            = run_style.foreground,
                             .sampler         = desc.sampler,
                             .texture         = atlas->textures[agl.layer],
                             .uv              = {agl.uv[0], agl.uv[1]},
                             .tiling          = desc.tiling,
                             .edge_smoothness = desc.edge_smoothness,
                             .scissor         = desc.scissor});
            }

            g_cursor += pt_to_px(sh.advance.x, run.font_height);
          }
        }

        if (run_style.strikethrough_thickness != 0)
        {
          Vec2 offset{cursor + advance, baseline - run.font_height / 2};
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      run_style.strikethrough_thickness};
          rect(ShapeDesc{.center = desc.center,
                         .extent = extent,
                         .transform =
                             desc.transform *
                             translate3d(to_vec3(offset + extent / 2, 0)) *
                             translate3d(to_vec3(-half_block_extent, 0)),
                         .tint            = run_style.strikethrough,
                         .sampler         = desc.sampler,
                         .texture         = 0,
                         .uv              = {},
                         .tiling          = desc.tiling,
                         .edge_smoothness = desc.edge_smoothness,
                         .scissor         = desc.scissor});
        }

        if (run_style.underline_thickness != 0)
        {
          Vec2 offset{cursor + advance, baseline + 2};
          Vec2 extent{pt_to_px(run.metrics.advance, run.font_height),
                      run_style.underline_thickness};
          rect(ShapeDesc{.center = desc.center,
                         .extent = extent,
                         .transform =
                             desc.transform *
                             translate3d(to_vec3(offset + extent / 2, 0)) *
                             translate3d(to_vec3(-half_block_extent, 0)),
                         .tint            = run_style.underline,
                         .sampler         = desc.sampler,
                         .texture         = 0,
                         .uv              = {},
                         .tiling          = desc.tiling,
                         .edge_smoothness = desc.edge_smoothness,
                         .scissor         = desc.scissor});
        }

        if (run.direction == TextDirection::LeftToRight)
        {
          advance += pt_to_px(run.metrics.advance, run.font_height);
        }
      }

      cursor += dir_advance;
    }
  }
}

void Canvas::triangles(ShapeDesc const &desc, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return;
  }

  u32 const first_index  = (u32) indices.size();
  u32 const first_vertex = (u32) vertices.size();

  CHECK(vertices.extend_copy(points));
  Path::triangles(points, indices);

  CHECK(ngon_params.push(NgonParam{
      .transform    = surface.mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .sampler      = desc.sampler,
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
  Path::triangulate_stroke(points, vertices, indices,
                           desc.thickness / desc.extent.y);
  CHECK(ngon_params.push(NgonParam{
      .transform    = surface.mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .sampler      = desc.sampler,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));
  u32 const num_indices = (u32) (indices.size() - first_index);

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon, desc.scissor);
}

void Canvas::blur(ShapeDesc const &desc, u32 radius)
{
  CHECK(blur_params.push(radius));
  add_run(*this, CanvasPassType::Blur, desc.scissor);
}

void Canvas::custom(ShapeDesc const &desc, CustomCanvasPassInfo const &pass)
{
  CHECK(custom_params.push(pass));
  add_run(*this, CanvasPassType::Custom, desc.scissor);
}

}        // namespace ash
