/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/std/math.h"

namespace ash
{

void Path::rect(Vec<Vec2> &vtx)
{
  CHECK(vtx.extend_copy(span<Vec2>({{-1, -1}, {1, -1}, {1, 1}, {-1, 1}})));
}

void Path::arc(Vec<Vec2> &vtx, u32 segments, f32 start, f32 stop)
{
  if (segments < 2)
  {
    return;
  }

  u32 const first = vtx.size32();

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

  u32 const first = vtx.size32();

  CHECK(vtx.extend_uninitialized(segments));

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) * 2 - 1;
  }
}

static inline f32 squircle(f32 x, f32 deg)
{
  deg = clamp(deg, 2.0F, 20.0F);
  return std::pow(1 - std::pow(std::abs(x), deg), 1 / deg);
}

void Path::squircle(Vec<Vec2> &vtx, u32 segments, f32 degree)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first      = vtx.size32();
  u32 const num_halves = segments >> 1;
  f32 const step       = 2.0F / (num_halves - 1);

  CHECK(vtx.extend_uninitialized(num_halves << 1));

  for (u32 i = 0; i < num_halves; i++)
  {
    f32 const x    = -1.0F + step * i;
    f32 const y    = ::ash::squircle(x, degree);
    vtx[first + i] = Vec2{x, y};
  }

  for (u32 i = 0; i < num_halves; i++)
  {
    f32 const x                 = 1.0F - step * i;
    f32 const y                 = ::ash::squircle(x, degree);
    vtx[first + num_halves + i] = Vec2{x, -y};
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
  u32 const first = vtx.size32();

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

  CHECK(vtx.extend_copy(span(vertices)));
}

void Path::bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1, Vec2 cp2)
{
  if (segments < 3)
  {
    return;
  }

  u32 const first = vtx.size32();

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

  u32 const first = vtx.size32();

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

  u32 const first_vtx    = vertices.size32();
  u32 const first_idx    = indices.size32();
  u32 const num_points   = points.size32();
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
    f32 const  alpha = atanf(d.y / d.x);

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

void Path::triangles(u32 first_vertex, u32 num_vertices, Vec<u32> &indices)
{
  CHECK(num_vertices > 3);
  u32 const num_triangles = num_vertices / 3;
  u32 const first_idx     = indices.size32();
  CHECK(indices.extend_uninitialized(num_triangles * 3));

  u32 *idx = indices.data() + first_idx;
  for (u32 i = 0; i < num_triangles * 3; i += 3)
  {
    idx[i]     = i;
    idx[i + 1] = i + 1;
    idx[i + 2] = i + 2;
  }
}

void Path::triangulate_convex(Vec<u32> &idx, u32 first_vertex, u32 num_vertices)
{
  if (num_vertices < 3)
  {
    return;
  }

  u32 const num_indices = (num_vertices - 2) * 3;
  u32 const first_index = idx.size32();

  CHECK(idx.extend_uninitialized(num_indices));

  for (u32 i = 0, v = 1; i < num_indices; i += 3, v++)
  {
    idx[first_index + i]     = first_vertex;
    idx[first_index + i + 1] = first_vertex + v;
    idx[first_index + i + 2] = first_vertex + v + 1;
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
  custom_passes.uninit();
  pass_runs.uninit();
}

void Canvas::begin(Vec2 p_viewport_extent)
{
  viewport_extent = p_viewport_extent;
  current_clip    = {{0, 0}, p_viewport_extent};
}

void Canvas::clear()
{
  vertices.clear();
  indices.clear();
  ngon_index_counts.clear();
  ngon_params.clear();
  rrect_params.clear();
  blur_params.clear();
  custom_passes.clear();
  pass_runs.clear();
}

void Canvas::clip(CRect const &c)
{
  current_clip = c;
}

static inline void add_run(Canvas &canvas, CanvasPassType type)
{
  if (!canvas.pass_runs.is_empty() &&
      canvas.pass_runs[canvas.pass_runs.size() - 1].type == type &&
      canvas.pass_runs[canvas.pass_runs.size() - 1].clip == canvas.current_clip)
  {
    canvas.pass_runs[canvas.pass_runs.size() - 1].count++;
    return;
  }

  u32 num = 0;
  switch (type)
  {
    case CanvasPassType::Blur:
      num = canvas.blur_params.size32();
      break;
    case CanvasPassType::Custom:
      num = canvas.custom_passes.size32();
      break;
    case CanvasPassType::Ngon:
      num = canvas.ngon_params.size32();
      break;
    case CanvasPassType::RRect:
      num = canvas.rrect_params.size32();
      break;
    default:
      UNREACHABLE();
      break;
  }

  CHECK(canvas.pass_runs.push(CanvasPassRun{.type  = type,
                                            .clip  = canvas.current_clip,
                                            .first = num - 1,
                                            .count = 1}));
}

void Canvas::circle(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
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

  add_run(*this, CanvasPassType::RRect);
}

void Canvas::rect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
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

  add_run(*this, CanvasPassType::RRect);
}

void Canvas::rrect(ShapeDesc const &desc)
{
  CHECK(rrect_params.push(RRectParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .radii        = desc.corner_radii / desc.extent.y,
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .aspect_ratio = desc.extent.x / desc.extent.y,
      .stroke       = desc.stroke,
      .thickness    = desc.thickness / desc.extent.y,
      .edge_smoothness = desc.edge_smoothness,
      .sampler         = desc.sampler,
      .albedo          = desc.texture}));

  add_run(*this, CanvasPassType::RRect);
}

void Canvas::brect(ShapeDesc const &desc)
{
  u32 const first_vertex = vertices.size32();
  u32 const first_index  = indices.size32();

  Path::brect(vertices, desc.corner_radii);

  u32 const num_vertices = vertices.size32() - first_vertex;

  Path::triangulate_convex(indices, first_vertex, num_vertices);

  u32 const num_indices = indices.size32() - first_index;

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon);
}

void Canvas::squircle(ShapeDesc const &desc, u32 segments, f32 degree)
{
  u32 const first_vertex = vertices.size32();
  u32 const first_index  = indices.size32();

  Path::squircle(vertices, segments, degree);

  u32 const num_vertices = vertices.size32() - first_vertex;

  Path::triangulate_convex(indices, first_vertex, num_vertices);

  u32 const num_indices = indices.size32() - first_index;

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon);
}

void Canvas::text(ShapeDesc const &desc, TextBlock const &block,
                  TextLayout const &layout, TextBlockStyle const &style,
                  Span<FontAtlasResource const *const> atlases)
{
  CHECK(style.runs.size() == block.runs.size());
  CHECK(style.runs.size() == block.fonts.size());
  for (u32 i = 0; i < block.fonts.size32(); i++)
  {
    CHECK(atlases[i] != nullptr);
    CHECK(block.fonts[i].font != nullptr);
    FontInfo f = get_font_info(block.fonts[i].font);
    CHECK(atlases[i]->glyphs.size() == f.glyphs.size());
  }

  f32 const  block_width = max(layout.extent.x, style.align_width);
  Vec2 const half_block_extent{block_width / 2, layout.extent.y / 2};

  constexpr u8 PASS_BACKGROUND    = 0;
  constexpr u8 PASS_GLYPH_SHADOWS = 1;
  constexpr u8 PASS_GLYPHS        = 2;
  constexpr u8 PASS_UNDERLINE     = 3;
  constexpr u8 PASS_STRIKETHROUGH = 4;
  constexpr u8 NUM_PASSES         = 5;

  for (u8 pass = 0; pass < NUM_PASSES; pass++)
  {
    f32 line_y = 0;
    for (Line const &ln : layout.lines)
    {
      line_y += ln.metrics.height;
      f32 const           baseline  = line_y - ln.metrics.descent;
      TextDirection const direction = level_to_direction(ln.metrics.level);
      // flip the alignment axis direction if it is an RTL line
      f32 const alignment =
          style.alignment *
          ((direction == TextDirection::LeftToRight) ? 1 : -1);
      f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
                   ln.metrics.width * 0.5F;
      for (TextRun const &run :
           span(layout.runs).slice(ln.first_run, ln.num_runs))
      {
        FontStyle const         &font_style = block.fonts[run.style];
        TextStyle const         &run_style  = style.runs[run.style];
        FontInfo const           font       = get_font_info(font_style.font);
        FontAtlasResource const *atlas      = atlases[run.style];
        f32 const run_width = au_to_px(run.metrics.advance, run.font_height);

        if (pass == PASS_BACKGROUND && !is_transparent(run_style.background))
        {
          Vec2 extent{run_width, au_to_px(run.metrics.ascent, run.font_height) +
                                     ln.metrics.height};
          Vec2 offset{cursor, line_y - ln.metrics.height};
          rect(
              ShapeDesc{.center    = desc.center,
                        .extent    = extent,
                        .transform = desc.transform *
                                     translate3d(vec3(offset + extent / 2, 0)) *
                                     translate3d(vec3(-half_block_extent, 0)),
                        .tint = run_style.background});
        }

        f32 glyph_cursor = cursor;
        for (u32 g = 0; g < run.num_glyphs; g++)
        {
          GlyphShape const &sh  = layout.glyphs[run.first_glyph + g];
          Glyph const      &gl  = font.glyphs[sh.glyph];
          AtlasGlyph const &agl = atlas->glyphs[sh.glyph];
          Vec2 const extent     = au_to_px(gl.metrics.extent, run.font_height);
          Vec2 const offset =
              Vec2{glyph_cursor, baseline} +
              au_to_px(gl.metrics.bearing, run.font_height) * Vec2{1, -1} +
              au_to_px(sh.offset, run.font_height);

          if (pass == PASS_GLYPH_SHADOWS && run_style.shadow_scale != 0 &&
              !is_transparent(run_style.shadow))
          {
            Vec2 shadow_extent = extent * run_style.shadow_scale;
            Vec2 shadow_offset = (offset + extent / 2) - shadow_extent / 2 +
                                 run_style.shadow_offset;
            rect(ShapeDesc{
                .center = desc.center,
                .extent = shadow_extent,
                .transform =
                    desc.transform *
                    translate3d(vec3(shadow_offset + shadow_extent / 2, 0)) *
                    translate3d(vec3(-half_block_extent, 0)),
                .tint            = run_style.shadow,
                .sampler         = desc.sampler,
                .texture         = atlas->textures[agl.layer],
                .uv              = {agl.uv[0], agl.uv[1]},
                .tiling          = desc.tiling,
                .edge_smoothness = desc.edge_smoothness});
          }

          if (pass == PASS_GLYPHS && !is_transparent(run_style.foreground))
          {
            rect(ShapeDesc{.center = desc.center,
                           .extent = extent,
                           .transform =
                               desc.transform *
                               translate3d(vec3(offset + extent / 2, 0)) *
                               translate3d(vec3(-half_block_extent, 0)),
                           .tint            = run_style.foreground,
                           .sampler         = desc.sampler,
                           .texture         = atlas->textures[agl.layer],
                           .uv              = {agl.uv[0], agl.uv[1]},
                           .tiling          = desc.tiling,
                           .edge_smoothness = desc.edge_smoothness});
          }

          glyph_cursor += au_to_px(sh.advance.x, run.font_height);
        }

        if (pass == PASS_STRIKETHROUGH &&
            run_style.strikethrough_thickness != 0)
        {
          Vec2 offset{cursor, baseline - run.font_height / 2};
          Vec2 extent{run_width, run_style.strikethrough_thickness};
          rect(
              ShapeDesc{.center    = desc.center,
                        .extent    = extent,
                        .transform = desc.transform *
                                     translate3d(vec3(offset + extent / 2, 0)) *
                                     translate3d(vec3(-half_block_extent, 0)),
                        .tint            = run_style.strikethrough,
                        .sampler         = desc.sampler,
                        .texture         = 0,
                        .uv              = {},
                        .tiling          = desc.tiling,
                        .edge_smoothness = desc.edge_smoothness});
        }

        if (pass == PASS_UNDERLINE && run_style.underline_thickness != 0)
        {
          Vec2 offset{cursor, baseline + 2};
          Vec2 extent{run_width, run_style.underline_thickness};
          rect(
              ShapeDesc{.center    = desc.center,
                        .extent    = extent,
                        .transform = desc.transform *
                                     translate3d(vec3(offset + extent / 2, 0)) *
                                     translate3d(vec3(-half_block_extent, 0)),
                        .tint            = run_style.underline,
                        .sampler         = desc.sampler,
                        .texture         = 0,
                        .uv              = {},
                        .tiling          = desc.tiling,
                        .edge_smoothness = desc.edge_smoothness});
        }
        cursor += run_width;
      }
    }
  }
}

void Canvas::triangles(ShapeDesc const &desc, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return;
  }

  u32 const first_index  = indices.size32();
  u32 const first_vertex = vertices.size32();

  CHECK(vertices.extend_copy(points));
  Path::triangles(first_vertex, points.size32(), indices);

  CHECK(ngon_params.push(NgonParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .sampler      = desc.sampler,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));

  u32 const num_indices = vertices.size32() - first_vertex;

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon);
}

void Canvas::triangles(ShapeDesc const &desc, Span<Vec2 const> points,
                       Span<u32 const> idx)
{
  if (points.size() < 3)
  {
    return;
  }

  u32 const first_index  = indices.size32();
  u32 const first_vertex = vertices.size32();

  CHECK(vertices.extend_copy(points));
  CHECK(indices.extend_copy(idx));

  for (u32 &v : span(indices).slice(first_index))
  {
    v += first_vertex;
  }

  CHECK(ngon_params.push(NgonParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .sampler      = desc.sampler,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));

  CHECK(ngon_index_counts.push(idx.size32()));

  add_run(*this, CanvasPassType::Ngon);
}

void Canvas::line(ShapeDesc const &desc, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return;
  }

  u32 const first_index  = indices.size32();
  u32 const first_vertex = vertices.size32();
  Path::triangulate_stroke(points, vertices, indices,
                           desc.thickness / desc.extent.y);
  CHECK(ngon_params.push(NgonParam{
      .transform    = mvp(desc.transform, desc.center, desc.extent),
      .tint         = {desc.tint[0], desc.tint[1], desc.tint[2], desc.tint[3]},
      .uv           = {desc.uv[0], desc.uv[1]},
      .tiling       = desc.tiling,
      .sampler      = desc.sampler,
      .albedo       = desc.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex}));

  u32 const num_indices = indices.size32() - first_index;

  CHECK(ngon_index_counts.push(num_indices));

  add_run(*this, CanvasPassType::Ngon);
}

void Canvas::blur(CRect const &area, u32 num_passes)
{
  CHECK(num_passes > 0);
  CHECK(blur_params.push(
      CanvasBlurParam{.area = area, .num_passes = num_passes}));
  add_run(*this, CanvasPassType::Blur);
}

void Canvas::custom(CustomCanvasPass pass)
{
  CHECK(custom_passes.push(pass));
  add_run(*this, CanvasPassType::Custom);
}

}        // namespace ash
