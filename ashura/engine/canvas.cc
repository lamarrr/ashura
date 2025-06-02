/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
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

void path::arc(Vec<Vec2> & vtx, f32 start, f32 stop, usize segments)
{
  if (segments < 2)
  {
    return;
  }

  auto const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (stop - start) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) - 0.5F;
  }
}

void path::circle(Vec<Vec2> & vtx, usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) - 0.5F;
  }
}

void path::squircle(Vec<Vec2> & vtx, f32 elasticity, usize segments)
{
  if (segments < 32)
  {
    return;
  }

  auto const n = segments >> 2;

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

void path::rrect(Vec<Vec2> & vtx, Vec4 radii, usize segments)
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

  auto const curve_segments = (segments - 8) >> 2;
  f32 const  step =
    (curve_segments == 0) ? 0.0F : ((PI * 0.5F) / curve_segments);
  auto const first = vtx.size32();

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

void path::bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, usize segments)
{
  if (segments < 3)
  {
    return;
  }

  auto const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
                          ash::bezier(cp0.y, cp1.y, cp2.y, step * i)};
  }
}

void path::cubic_bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                        usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const first = vtx.size32();

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
                       usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const beg = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
      Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
           ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

template <typename I>
void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vertices,
                        Vec<I> & indices, f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  auto const first_vtx    = vertices.size32();
  auto const first_idx    = indices.size32();
  auto const num_points   = points.size32();
  auto const num_vertices = (num_points - 1) * 4;
  auto const num_indices  = (num_points - 1) * 6 + (num_points - 2) * 6;
  vertices.extend_uninit(num_vertices).unwrap();
  indices.extend_uninit(num_indices).unwrap();

  Vec2 * vtx  = vertices.data() + first_vtx;
  I *    idx  = indices.data() + first_idx;
  I      ivtx = 0;

  for (I i = 0; i < num_points - 1; i++)
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
      I prev  = ivtx - 2;
      idx[6]  = prev;
      idx[7]  = prev + 1;
      idx[8]  = ivtx + 1;
      idx[9]  = prev;
      idx[10] = prev + 1;
      idx[11] = ivtx;
      idx += 6;
    }

    idx += 6;
    vtx += 4;
    ivtx += 4;
  }
}

void path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vertices,
                              Vec<u16> & indices, f32 thickness)
{
  ::ash::triangulate_stroke(points, vertices, indices, thickness);
}

void path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vertices,
                              Vec<u32> & indices, f32 thickness)
{
  ::ash::triangulate_stroke(points, vertices, indices, thickness);
}

template <typename I>
void triangles(I first_vertex, I num_vertices, Vec<I> & indices)
{
  CHECK(num_vertices > 3, "");
  auto const num_triangles = num_vertices / 3;
  auto const first_idx     = indices.size32();
  indices.extend_uninit(num_triangles * 3).unwrap();

  I * idx = indices.data() + first_idx;
  for (I i = 0; i < num_triangles * 3; i += 3)
  {
    idx[i]     = first_vertex + i;
    idx[i + 1] = first_vertex + i + 1;
    idx[i + 2] = first_vertex + i + 2;
  }
}

void path::triangles(u32 first_vertex, u32 num_vertices, Vec<u32> & indices)
{
  ::ash::triangles(first_vertex, num_vertices, indices);
}

void path::triangles(u16 first_vertex, u16 num_vertices, Vec<u16> & indices)
{
  ::ash::triangles(first_vertex, num_vertices, indices);
}

template <typename I>
void triangulate_convex(Vec<I> & idx, I first_vertex, I num_vertices)
{
  if (num_vertices < 3)
  {
    return;
  }

  auto const num_indices = (num_vertices - 2) * 3;
  auto const first_index = idx.size32();

  idx.extend_uninit(num_indices).unwrap();

  for (I i = 0, v = 1; i < num_indices; i += 3, v++)
  {
    idx[first_index + i]     = first_vertex;
    idx[first_index + i + 1] = first_vertex + v;
    idx[first_index + i + 2] = first_vertex + v + 1;
  }
}

void path::triangulate_convex(Vec<u32> & idx, u32 first_vertex,
                              u32 num_vertices)
{
  ::ash::triangulate_convex(idx, first_vertex, num_vertices);
}

void path::triangulate_convex(Vec<u16> & idx, u16 first_vertex,
                              u16 num_vertices)
{
  ::ash::triangulate_convex(idx, first_vertex, num_vertices);
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

  viewport            = new_viewport;
  extent              = new_extent;
  framebuffer_extent  = new_framebuffer_extent;
  framebuffer_uv_base = 1 / as_vec2(new_framebuffer_extent);

  if (extent.x == 0 | extent.y == 0)
  {
    aspect_ratio = 1;
  }
  else
  {
    aspect_ratio = extent.x / extent.y;
  }

  virtual_scale = viewport.extent.x / new_extent.x;

  // (-0.5w, +0.5w) (-0.5w, +0.5h) -> (-1, +1), (-1, +1)
  world_to_ndc = scale3d(vec3(2 / extent, 1));

  ndc_to_viewport =
    // -0.5 extent, +0.5 => 0, extent
    translate3d(vec3(0.5F * extent, 0.0F)) *
    // -1, +1 => -0.5 extent, +0.5 half_extent
    scale3d(vec3(0.5F * extent, 1.0F));

  // viewport coordinate to framebuffer coordinate
  viewport_to_fb =
    // 0, framebuffer-space extent -> viewport.offset, viewport.offset + framebuffer-space extent
    translate3d(vec3(viewport.offset, 0.0F)) *
    // 0, viewport-space extent -> 0, framebuffer-space extent
    scale3d(vec3(Vec2::splat(virtual_scale), 1.0F));

  return *this;
}

RectU Canvas::clip_to_scissor(CRect const & clip) const
{
  // clips are always unscaled
  Rect scissor_f{.offset = viewport.offset +
                           (clip.begin() + 0.5F * extent) * virtual_scale,
                 .extent = clip.extent * virtual_scale};

  scissor_f =
    Rect::range(clamp_vec(scissor_f.offset, Vec2::splat(0.0F), MAX_CLIP.extent),
                clamp_vec(scissor_f.end(), Vec2::splat(0.0F), MAX_CLIP.extent));

  return RectU::range(
    clamp_vec(as_vec2u(scissor_f.begin()), Vec2U::splat(0), framebuffer_extent),
    clamp_vec(as_vec2u(scissor_f.end()), Vec2U::splat(0), framebuffer_extent));
}

static inline void add_rrect(Canvas & c, RRectParam const & param,
                             CRect const & clip)
{
  auto const index = c.rrect_params.size32();
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

static inline void add_squircle(Canvas & c, SquircleParam const & param,
                                CRect const & clip)
{
  auto const index = c.squircle_params.size32();
  c.squircle_params.push(param).unwrap();

  if (c.batches.is_empty() ||
      c.batches.last().type != Canvas::BatchType::Squircle ||
      c.batches.last().clip != clip)
  {
    c.batches
      .push(Canvas::Batch{
        .type = Canvas::BatchType::Squircle, .run{index, 1},
           .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches.last().run.span++;
}

static inline void add_ngon(Canvas & c, NgonParam const & param,
                            CRect const & clip, u32 num_indices)
{
  auto const index = c.ngon_params.size32();
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

Canvas & Canvas::clip(CRect const & c)
{
  current_clip = c;
  return *this;
}

constexpr Mat4 object_to_world(Mat4 const & transform, CRect const & area)
{
  return transform * translate3d(vec3(area.center, 0)) *
         scale3d(vec3(area.extent, 1));
}

Canvas & Canvas::circle(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.area.extent.y;
  add_rrect(*this,
            RRectParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = {1, 1, 1, 1},
              .uv    = {info.uv[0], info.uv[1]},
              .tiling          = info.tiling,
              .aspect_ratio    = info.area.extent.x * inv_y,
              .stroke          = info.stroke,
              .thickness       = info.thickness.x * inv_y,
              .edge_smoothness = info.edge_smoothness * inv_y,
              .sampler         = info.sampler,
              .albedo          = info.texture
  },
            current_clip);

  return *this;
}

Canvas & Canvas::rect(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.area.extent.y;
  add_rrect(*this,
            RRectParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = {0, 0, 0, 0},
              .uv    = {info.uv[0], info.uv[1]},
              .tiling          = info.tiling,
              .aspect_ratio    = info.area.extent.x * inv_y,
              .stroke          = info.stroke,
              .thickness       = info.thickness.x * inv_y,
              .edge_smoothness = info.edge_smoothness * inv_y,
              .sampler         = info.sampler,
              .albedo          = info.texture
  },
            current_clip);
  return *this;
}

Canvas & Canvas::rrect(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.area.extent.y;
  f32 const max_radius =
    0.5F * min(info.area.extent.x, info.area.extent.y) * inv_y;
  Vec4 r = info.corner_radii * inv_y;

  r.x = min(r.x, max_radius);
  r.y = min(r.y, max_radius);
  r.z = min(r.z, max_radius);
  r.w = min(r.w, max_radius);

  add_rrect(*this,
            RRectParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = r,
              .uv    = {info.uv[0], info.uv[1]},
              .tiling          = info.tiling,
              .aspect_ratio    = info.area.extent.x * inv_y,
              .stroke          = info.stroke,
              .thickness       = info.thickness.x * inv_y,
              .edge_smoothness = info.edge_smoothness * inv_y,
              .sampler         = info.sampler,
              .albedo          = info.texture
  },
            current_clip);
  return *this;
}

Canvas & Canvas::brect(ShapeInfo const & info)
{
  auto const first_vertex = ngon_vertices.size32();
  auto const first_index  = ngon_indices.size32();

  path::brect(ngon_vertices, info.corner_radii);

  auto const num_vertices = ngon_vertices.size32() - first_vertex;

  path::triangulate_convex(ngon_indices, first_vertex, num_vertices);

  auto const num_indices = ngon_indices.size32() - first_index;

  add_ngon(*this,
           NgonParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           current_clip, num_indices);

  return *this;
}

Canvas & Canvas::squircle(ShapeInfo const & info)
{
  f32 const width = max(info.area.extent.x, info.area.extent.y);
  f32 const inv_y = 1 / width;

  add_squircle(
    *this,
    SquircleParam{
      .transform = object_to_world(info.transform,
                                   CRect{info.area.center, Vec2::splat(width)}
                                   ),
      .tint      = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv        = {info.uv[0], info.uv[1]},
      .degree    = info.corner_radii.x,
      .tiling    = info.tiling,
      .stroke    = info.stroke,
      .thickness = info.thickness.x * inv_y,
      .edge_smoothness = info.edge_smoothness * inv_y,
      .sampler         = info.sampler,
      .albedo          = info.texture
  },
    current_clip);

  return *this;
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return *this;
  }

  auto const first_index  = ngon_indices.size32();
  auto const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend(points).unwrap();
  path::triangles(first_vertex, points.size32(), ngon_indices);

  auto const num_indices = ngon_vertices.size32() - first_vertex;

  add_ngon(*this,
           NgonParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
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

  auto const first_index  = ngon_indices.size32();
  auto const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend(points).unwrap();
  ngon_indices.extend(idx).unwrap();

  for (u32 & v : ngon_indices.view().slice(first_index))
  {
    v += first_vertex;
  }

  add_ngon(*this,
           NgonParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
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

  auto const first_index  = ngon_indices.size32();
  auto const first_vertex = ngon_vertices.size32();
  path::triangulate_stroke(points, ngon_vertices, ngon_indices,
                           info.thickness.x / info.area.extent.y);

  auto const num_indices = ngon_indices.size32() - first_index;

  add_ngon(*this,
           NgonParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           current_clip, num_indices);

  return *this;
}

Canvas & Canvas::blur(ShapeInfo const & info)
{
  // doesn't currently handle rotation correctly :(
  auto const index = blurs.size32();

  auto const world_xfm = object_to_world(info.transform, info.area);

  auto const fb_xfm =
    viewport_to_fb * ndc_to_viewport * world_to_ndc * world_xfm;

  auto const tl = transform(fb_xfm, Vec3{-0.5, -0.5, 0.0}).xy();
  auto const tr = transform(fb_xfm, Vec3{0.5, -0.5, 0.0}).xy();
  auto const bl = transform(fb_xfm, Vec3{-0.5, 0.5, 0.0}).xy();
  auto const br = transform(fb_xfm, Vec3{0.5, 0.5, 0.0}).xy();

  auto const bounding = CRect::bounding(tl, tr, bl, br);

  /// [ ] we need to apply a uv-transform,
  Vec2 uv0 = bounding.begin() * framebuffer_uv_base;
  Vec2 uv1 = bounding.end() * framebuffer_uv_base;

  auto const to_brightness = [](Vec4 tint) {
    return vec4(Vec3::splat((tint.x + tint.y + tint.z) * (1 / 3.0F)), 1.0F);
  };

  auto const inv_y = 1 / info.area.extent.y;

  RRectParam rrect{
    .transform = world_xfm,
    .tint{to_brightness(info.tint[0]), to_brightness(info.tint[1]),
          to_brightness(info.tint[2]), to_brightness(info.tint[3])},
    .radii = info.corner_radii * inv_y,
    .uv{uv0, uv1},
    .tiling          = 1,
    .aspect_ratio    = info.area.extent.x * inv_y,
    .stroke          = info.stroke,
    .thickness       = 0 * inv_y,
    .edge_smoothness = info.edge_smoothness * inv_y,
    .sampler         = SamplerId::LinearClamped,
    .albedo          = TextureId::Base
  };

  auto const area =
    RectU::range(
      as_vec2u(clamp_vec(bounding.begin(), Vec2::splat(0), MAX_CLIP.extent)),
      as_vec2u(clamp_vec(bounding.end(), Vec2::splat(0), MAX_CLIP.extent)))
      .clamp_to_extent(framebuffer_extent);

  auto const spread_radius =
    as_vec2u(clamp_vec(info.thickness * virtual_scale, Vec2::splat(0),
                       Vec2::splat(MAX_CLIP_DISTANCE)));

  blurs.push(Blur{.rrect = rrect, .area = area, .spread_radius = spread_radius})
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

Canvas & Canvas::text(Span<TextLayer const> layers,
                      Span<ShapeInfo const> shapes, Span<TextRenderInfo const>,
                      Span<usize const>     sorted)
{
  for (auto i : sorted)
  {
    auto layer = layers[i];
    auto shape = shapes[i];

    switch (layer)
    {
      case TextLayer::Glyphs:
      case TextLayer::GlyphShadows:
        rect(shape);
        break;
      default:
        rrect(shape);
        break;
    }
  }

  return *this;
}

}    // namespace ash
