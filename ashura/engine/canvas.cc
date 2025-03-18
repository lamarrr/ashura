/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"

namespace ash
{

void path::rect(Vec<Vec2> & vtx)
{
  static constexpr Vec2 coords[] = {
    {-0.5, -0.5},
    {0.5,  -0.5},
    {0.5,  0.5 },
    {-0.5, 0.5 }
  };

  vtx.extend(coords).unwrap();
}

void path::arc(Vec<Vec2> & vtx, f32 start, f32 stop, u32 segments)
{
  if (segments < 2)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (stop - start) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) - 0.5F;
  }
}

void path::circle(Vec<Vec2> & vtx, u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) - 0.5F;
  }
}

void path::squircle(Vec<Vec2> & vtx, f32 elasticity, u32 segments)
{
  if (segments < 32)
  {
    return;
  }

  u32 const n = segments >> 2;

  elasticity = clamp(elasticity * 0.5F, 0.0F, 0.5F);

  path::cubic_bezier(vtx, {0, -0.5F}, {elasticity, -0.5F}, {0.5F, -0.5F},
                     {0.5F, 0}, n);
  path::cubic_bezier(vtx, {0.5F, 0}, {0.5F, elasticity}, {0.5F, 0.5F},
                     {0, 0.5F}, n);
  path::cubic_bezier(vtx, {0, 0.5F}, {-elasticity, 0.5F}, {-0.5F, 0.5F},
                     {-0.5F, 0}, n);
  path::cubic_bezier(vtx, {-0.5F, 0}, {-0.5F, -elasticity}, {-0.5F, -0.5F},
                     {0, -0.5F}, n);
}

void path::rrect(Vec<Vec2> & vtx, Vec4 radii, u32 segments)
{
  if (segments < 8)
  {
    return;
  }

  radii.x = clamp(radii.x, 0.0F, 1.0F);
  radii.y = clamp(radii.y, 0.0F, 1.0F);
  radii.z = clamp(radii.z, 0.0F, 1.0F);
  radii.w = clamp(radii.w, 0.0F, 1.0F);

  /// clipping
  radii.y          = min(radii.y, 1.0F - radii.x);
  f32 max_radius_z = min(1.0F - radii.x, 0.5F - radii.y);
  radii.z          = min(radii.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 0.5F - radii.z);
  radii.w          = min(radii.w, max_radius_w);

  u32 const curve_segments = (segments - 8) >> 2;
  f32 const step =
    (curve_segments == 0) ? 0.0F : ((PI * 0.5F) / curve_segments);
  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  u32 i = 0;

  vtx[first + i++] = Vec2{0.5F, 0.5F - radii.z};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (0.5F - radii.z) + radii.z * rotor(s * step);
  }

  vtx[first + i++] = Vec2{0.5F - radii.z, 0.5F};

  vtx[first + i++] = Vec2{-0.5F + radii.w, 0.5F};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = Vec2{-0.5F + radii.w, 0.5F - radii.w} +
                       radii.w * rotor(PI * 0.5F + s * step);
  }

  vtx[first + i++] = Vec2{-0.5F, 0.5F - radii.w};

  vtx[first + i++] = Vec2{-0.5F, -0.5F + radii.x};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (-0.5F + radii.x) + radii.x * rotor(PI + s * step);
  }

  vtx[first + i++] = Vec2{-0.5F + radii.x, -0.5F};

  vtx[first + i++] = Vec2{0.5F - radii.y, -0.5F};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = Vec2{0.5F - radii.y, (-0.5F + radii.y)} +
                       radii.y * rotor(PI * 1.5F + s * step);
  }

  vtx[first + i++] = Vec2{0.5F, -0.5F + radii.y};
}

void path::brect(Vec<Vec2> & vtx, Vec4 slant)
{
  slant.x = clamp(slant.x, 0.0F, 1.0F);
  slant.y = clamp(slant.y, 0.0F, 1.0F);
  slant.z = clamp(slant.z, 0.0F, 1.0F);
  slant.w = clamp(slant.w, 0.0F, 1.0F);

  slant.y          = min(slant.y, 1 - slant.x);
  f32 max_radius_z = min(1 - slant.x, 1 - slant.y);
  slant.z          = min(slant.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 1 - slant.z);
  slant.w          = min(slant.w, max_radius_w);

  Vec2 const vertices[] = {
    {-0.5F + slant.x, -0.5F          },
    {0.5F - slant.y,  -0.5F          },
    {0.5F,            -0.5F + slant.y},
    {0.5F,            0.5F - slant.z },
    {0.5F - slant.z,  0.5F           },
    {-0.5F + slant.w, 0.5F           },
    {-0.5F,           0.5F - slant.w },
    {-0.5F,           -0.5F + slant.x}
  };

  vtx.extend(vertices).unwrap();
}

void path::bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, u32 segments)
{
  if (segments < 3)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
                          ash::bezier(cp0.y, cp1.y, cp2.y, step * i)};
  }
}

void path::cubic_bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                        u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] =
      Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
           ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void path::catmull_rom(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                       u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const beg = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
      Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
           ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vertices,
                              Vec<u32> & indices, f32 thickness)
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
  vertices.extend_uninit(num_vertices).unwrap();
  indices.extend_uninit(num_indices).unwrap();

  Vec2 * vtx  = vertices.data() + first_vtx;
  u32 *  idx  = indices.data() + first_idx;
  u32    ivtx = 0;

  for (u32 i = 0; i < num_points - 1; i++)
  {
    Vec2 const p0    = points[i];
    Vec2 const p1    = points[i + 1];
    Vec2 const d     = p1 - p0;
    f32 const  alpha = atanf(d.y / d.x);

    // parallel angle
    Vec2 const down = (thickness * 0.5F) * rotor(alpha + PI * 0.5F);

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

void path::triangles(u32 first_vertex, u32 num_vertices, Vec<u32> & indices)
{
  CHECK(num_vertices > 3, "");
  u32 const num_triangles = num_vertices / 3;
  u32 const first_idx     = indices.size32();
  indices.extend_uninit(num_triangles * 3).unwrap();

  u32 * idx = indices.data() + first_idx;
  for (u32 i = 0; i < num_triangles * 3; i += 3)
  {
    idx[i]     = first_vertex + i;
    idx[i + 1] = first_vertex + i + 1;
    idx[i + 2] = first_vertex + i + 2;
  }
}

void path::triangulate_convex(Vec<u32> & idx, u32 first_vertex,
                              u32 num_vertices)
{
  if (num_vertices < 3)
  {
    return;
  }

  u32 const num_indices = (num_vertices - 2) * 3;
  u32 const first_index = idx.size32();

  idx.extend_uninit(num_indices).unwrap();

  for (u32 i = 0, v = 1; i < num_indices; i += 3, v++)
  {
    idx[first_index + i]     = first_vertex;
    idx[first_index + i + 1] = first_vertex + v;
    idx[first_index + i + 2] = first_vertex + v + 1;
  }
}

Canvas & Canvas::reset()
{
  rrect_params.reset();
  ngon_params.reset();
  ngon_vertices.reset();
  ngon_indices.reset();
  ngon_index_counts.reset();
  blurs.reset();
  passes.reset();
  batches.reset();
  frame_arena.reclaim();

  return *this;
}

Canvas & Canvas::reset_clip()
{
  current_clip = MAX_CLIP;
  return *this;
}

Canvas & Canvas::begin_recording(gpu::Viewport const & new_viewport,
                                 Vec2 new_extent, Vec2U new_framebuffer_extent)
{
  reset();

  viewport           = new_viewport;
  extent             = new_extent;
  framebuffer_extent = new_framebuffer_extent;

  if (extent.x == 0 | extent.y == 0)
  {
    aspect_ratio = 1;
  }
  else
  {
    aspect_ratio = extent.x / extent.y;
  }

  virtual_scale = viewport.extent.x / new_extent.x;

  world_to_view = translate3d(Vec3{-1, -1, 0}) * scale3d(vec3(2 / extent, 1));

  return *this;
}

RectU Canvas::clip_to_scissor(Rect const & clip) const
{
  // clips are always unscaled
  Rect scissor_f{.offset = viewport.offset + clip.offset * virtual_scale,
                 .extent = clip.extent * virtual_scale};

  scissor_f.offset.x = clamp(scissor_f.offset.x, 0.0F, MAX_CLIP.extent.x);
  scissor_f.offset.y = clamp(scissor_f.offset.y, 0.0F, MAX_CLIP.extent.y);
  scissor_f.extent.x = clamp(scissor_f.extent.x, 0.0F, MAX_CLIP.extent.x);
  scissor_f.extent.y = clamp(scissor_f.extent.y, 0.0F, MAX_CLIP.extent.y);

  RectU scissor{.offset = as_vec2u(scissor_f.offset),
                .extent = as_vec2u(scissor_f.extent)};

  scissor.offset.x = min(scissor.offset.x, framebuffer_extent.x);
  scissor.offset.y = min(scissor.offset.x, framebuffer_extent.y);
  scissor.extent.x =
    min(framebuffer_extent.x - scissor.offset.x, scissor.extent.x);
  scissor.extent.y =
    min(framebuffer_extent.y - scissor.offset.y, scissor.extent.y);

  return scissor;
}

static inline void add_rrect(Canvas & c, RRectParam const & param,
                             Rect const & clip)
{
  u32 const index = c.rrect_params.size32();
  c.rrect_params.push(param).unwrap();

  if (c.batches.is_empty() ||
      c.batches.last().type != Canvas::BatchType::RRect ||
      c.batches.last().clip != clip)
  {
    c.batches
      .push(Canvas::Batch{
        .type = Canvas::BatchType::RRect, .run{index, 1},
           .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches.last().run.span++;
}

static inline void add_ngon(Canvas & c, NgonParam const & param,
                            Rect const & clip, u32 num_indices)
{
  u32 const index = c.ngon_params.size32();
  c.ngon_index_counts.push(num_indices).unwrap();
  c.ngon_params.push(param).unwrap();

  if (c.batches.is_empty() ||
      c.batches.last().type != Canvas::BatchType::Ngon ||
      c.batches.last().clip != clip)
  {
    c.batches
      .push(Canvas::Batch{
        .type = Canvas::BatchType::Ngon,
        .run{.offset = index, .span = 1},
        .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches.last().run.span++;
}

Canvas & Canvas::end_recording()
{
  return *this;
}

Canvas & Canvas::clip(Rect const & c)
{
  current_clip = c;
  return *this;
}

constexpr Mat4 object_to_world(Mat4 const & transform, Vec2 center, Vec2 extent)
{
  return transform * translate3d(vec3(center, 0)) * scale3d(vec3(extent, 1));
}

Canvas & Canvas::circle(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.extent.y;
  add_rrect(
    *this,
    RRectParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .radii        = {1, 1, 1, 1},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .aspect_ratio = info.extent.x * inv_y,
      .stroke       = info.stroke,
      .thickness    = info.thickness * inv_y,
      .edge_smoothness = info.edge_smoothness * inv_y,
      .sampler         = info.sampler,
      .albedo          = info.texture
  },
    current_clip);

  return *this;
}

Canvas & Canvas::rect(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.extent.y;
  add_rrect(
    *this,
    RRectParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .radii        = {0, 0, 0, 0},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .aspect_ratio = info.extent.x * inv_y,
      .stroke       = info.stroke,
      .thickness    = info.thickness * inv_y,
      .edge_smoothness = info.edge_smoothness * inv_y,
      .sampler         = info.sampler,
      .albedo          = info.texture
  },
    current_clip);
  return *this;
}

Canvas & Canvas::rrect(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.extent.y;
  add_rrect(
    *this,
    RRectParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .radii        = info.corner_radii * inv_y,
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .aspect_ratio = info.extent.x * inv_y,
      .stroke       = info.stroke,
      .thickness    = info.thickness * inv_y,
      .edge_smoothness = info.edge_smoothness * inv_y,
      .sampler         = info.sampler,
      .albedo          = info.texture
  },
    current_clip);
  return *this;
}

Canvas & Canvas::brect(ShapeInfo const & info)
{
  u32 const first_vertex = ngon_vertices.size32();
  u32 const first_index  = ngon_indices.size32();

  path::brect(ngon_vertices, info.corner_radii);

  u32 const num_vertices = ngon_vertices.size32() - first_vertex;

  path::triangulate_convex(ngon_indices, first_vertex, num_vertices);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(
    *this,
    NgonParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .sampler      = info.sampler,
      .albedo       = info.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex
  },
    current_clip, num_indices);

  return *this;
}

Canvas & Canvas::squircle(ShapeInfo const & info, f32 elasticity, u32 segments)
{
  u32 const first_vertex = ngon_vertices.size32();
  u32 const first_index  = ngon_indices.size32();

  path::squircle(ngon_vertices, elasticity, segments);

  u32 const num_vertices = ngon_vertices.size32() - first_vertex;

  path::triangulate_convex(ngon_indices, first_vertex, num_vertices);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(
    *this,
    NgonParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .sampler      = info.sampler,
      .albedo       = info.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex
  },
    current_clip, num_indices);

  return *this;
}

Canvas & Canvas::text(ShapeInfo const & info, TextBlock const & block,
                      TextLayout const & layout, TextBlockStyle const & style,
                      CRect const & clip)
{
  CHECK(style.runs.size() == block.runs.size(), "");
  CHECK(style.runs.size() == block.fonts.size(), "");

  f32 const  block_width = max(layout.extent.x, style.align_width);
  Vec2 const block_extent{block_width, layout.extent.y};

  enum Pass : u32
  {
    BACKGROUND    = 0,
    HIGHLIGHT     = 1,
    GLYPH_SHADOWS = 2,
    GLYPHS        = 3,
    UNDERLINE     = 4,
    STRIKETHROUGH = 5
  };

  static constexpr u32 NUM_PASSES = 6;

  for (u32 pass = 0; pass < NUM_PASSES; pass++)
  {
    if (pass == Pass::HIGHLIGHT && style.highlight.slice.is_empty())
    {
      continue;
    }

    f32 line_y = -block_extent.y * 0.5F;

    for (Line const & ln : layout.lines)
    {
      CRect const ln_rect{
        .center = info.center + line_y,
        .extent{block_width, ln.metrics.height}
      };

      line_y += ln.metrics.height;

      if (!overlaps(clip, ln_rect))
      {
        continue;
      }

      f32 const           baseline  = line_y - ln.metrics.descent;
      TextDirection const direction = level_to_direction(ln.metrics.level);
      // flip the alignment axis direction if it is an RTL line
      f32 const           alignment =
        style.alignment * ((direction == TextDirection::LeftToRight) ? 1 : -1);
      f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
                   ln.metrics.width * 0.5F;

      f32 highlight_min_x = F32_MAX;
      f32 highlight_max_x = F32_MIN;

      for (TextRun const & run : layout.runs.view().slice(ln.runs))
      {
        FontStyle const &    font_style  = block.fonts[run.style];
        TextStyle const &    run_style   = style.runs[run.style];
        FontInfo const       font        = sys->font.get(font_style.font);
        GpuFontAtlas const & atlas       = font.gpu_atlas.value();
        f32 const            font_height = block.font_scale * run.font_height;
        ResolvedTextRunMetrics const run_metrics =
          run.metrics.resolve(font_height);
        f32 const run_width = run_metrics.advance;

        if (pass == Pass::BACKGROUND && !run_style.background.is_transparent())
        {
          Vec2 const extent{run_width, run_metrics.height()};
          Vec2 const center{cursor + extent.x * 0.5F,
                            baseline - run_metrics.ascent + extent.y * 0.5F};

          rrect({.center       = info.center,
                 .extent       = extent,
                 .transform    = info.transform * translate3d(vec3(center, 0)),
                 .corner_radii = run_style.corner_radii,
                 .tint         = run_style.background});
        }

        f32 glyph_cursor = cursor;

        for (GlyphShape const & sh : layout.glyphs.view().slice(run.glyphs))
        {
          GlyphMetrics const & m      = font.glyphs[sh.glyph];
          AtlasGlyph const &   agl    = atlas.glyphs[sh.glyph];
          Vec2 const           extent = au_to_px(m.extent, font_height);
          Vec2 const           center = Vec2{glyph_cursor, baseline} +
                              au_to_px(m.bearing, font_height) +
                              au_to_px(sh.offset, font_height) + extent * 0.5F;

          if (pass == Pass::HIGHLIGHT &&
              style.highlight.slice.contains(sh.cluster))
          {
            f32 const begin = center.x - 0.5 * extent.x;
            f32 const end   = center.x + 0.5 * extent.x;
            highlight_min_x = min(begin, highlight_min_x);
            highlight_max_x = max(end, highlight_max_x);
          }

          if (pass == Pass::GLYPH_SHADOWS && run_style.shadow_scale != 0 &&
              !run_style.shadow.is_transparent())
          {
            Vec2 const shadow_extent = extent * run_style.shadow_scale;
            Vec2 const shadow_center =
              center + block.font_scale * run_style.shadow_offset;
            rect({
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

          if (pass == Pass::GLYPHS && !run_style.color.is_transparent())
          {
            rect({
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

          glyph_cursor += au_to_px(sh.advance, font_height);
        }

        if (pass == Pass::STRIKETHROUGH &&
            run_style.strikethrough_thickness != 0)
        {
          Vec2 const extent{run_width, block.font_scale *
                                         run_style.strikethrough_thickness};
          Vec2 const center =
            Vec2{cursor, baseline - run_metrics.ascent * 0.5F} + extent * 0.5F;
          rect({.center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = run_style.strikethrough,
                .sampler   = info.sampler,
                .texture   = TextureId::White,
                .uv        = {},
                .tiling    = 1,
                .edge_smoothness = info.edge_smoothness});
        }

        if (pass == Pass::UNDERLINE && run_style.underline_thickness != 0)
        {
          Vec2 const extent{run_width,
                            block.font_scale * run_style.underline_thickness};
          Vec2 const center = Vec2{cursor, baseline + 2} + extent * 0.5F;
          rect({.center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = run_style.underline,
                .sampler   = info.sampler,
                .texture   = TextureId::White,
                .uv        = {},
                .tiling    = 1,
                .edge_smoothness = info.edge_smoothness});
        }

        cursor += run_width;
      }

      if (pass == Pass::HIGHLIGHT &&
          style.highlight.slice.overlaps(ln.codepoints) &&
          highlight_min_x != F32_MAX && highlight_max_x != F32_MIN)
      {
        Vec2 const extent{highlight_max_x - highlight_min_x, ln.metrics.height};
        Vec2 const center{highlight_min_x + 0.5F * extent.x,
                          line_y - 0.5F * extent.y};
        rrect({.center       = info.center,
               .extent       = extent,
               .transform    = info.transform * translate3d(vec3(center, 0)),
               .corner_radii = style.highlight.style.corner_radii,
               .tint         = style.highlight.style.color});
      }
    }
  }

  return *this;
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend(points).unwrap();
  path::triangles(first_vertex, points.size32(), ngon_indices);

  u32 const num_indices = ngon_vertices.size32() - first_vertex;

  add_ngon(
    *this,
    NgonParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .sampler      = info.sampler,
      .albedo       = info.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex
  },
    current_clip, num_indices);

  return *this;
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points,
                           Span<u32 const> idx)
{
  if (points.size() < 3)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend(points).unwrap();
  ngon_indices.extend(idx).unwrap();

  for (u32 & v : ngon_indices.view().slice(first_index))
  {
    v += first_vertex;
  }

  add_ngon(
    *this,
    NgonParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .sampler      = info.sampler,
      .albedo       = info.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex
  },
    current_clip, idx.size32());

  return *this;
}

Canvas & Canvas::line(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();
  path::triangulate_stroke(points, ngon_vertices, ngon_indices,
                           info.thickness / info.extent.y);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(
    *this,
    NgonParam{
      .transform    = object_to_world(info.transform, info.center, info.extent),
      .tint         = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv           = {info.uv[0], info.uv[1]},
      .tiling       = info.tiling,
      .sampler      = info.sampler,
      .albedo       = info.texture,
      .first_index  = first_index,
      .first_vertex = first_vertex
  },
    current_clip, num_indices);

  return *this;
}

Canvas & Canvas::blur(Rect const & raw_area, Vec2 radius, Vec4 corner_radii)
{
  u32 const index = blurs.size32();

  auto const area  = raw_area.clamp(extent);
  f32 const  inv_y = 1 / area.extent.y;

  RectU const fb_area = RectU{as_vec2u(area.offset * virtual_scale),
                              as_vec2u(area.extent * virtual_scale)}
                          .clamp(framebuffer_extent);

  blurs
    .push(Blur{.area         = fb_area,
               .radius       = as_vec2u(radius * virtual_scale),
               .corner_radii = corner_radii * inv_y,
               .transform =
                 object_to_world(Mat4::identity(), area.center(), area.extent),
               .aspect_ratio = area.extent.x * inv_y})
    .unwrap();

  batches
    .push(Batch{
      .type = BatchType::Blur, .run{index, 1},
         .clip = current_clip
  })
    .unwrap();

  return *this;
}

Canvas & Canvas::pass(Pass pass)
{
  u32 const index = passes.size32();

  passes.push(std::move(pass)).unwrap();

  batches
    .push(Batch{
      .type = BatchType::Pass,
      .run{index, 1},
  })
    .unwrap();

  return *this;
}

}    // namespace ash
