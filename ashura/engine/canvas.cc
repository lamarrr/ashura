/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/std/math.h"

namespace ash
{

void path::rect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center)
{
  Vec2 const coords[] = {
    Vec2{-0.5, -0.5}
    * extent + center, Vec2{0.5,  -0.5}
    * extent + center,
    Vec2{0.5,  0.5 }
    * extent + center, Vec2{-0.5, 0.5 }
    * extent + center
  };

  vtx.extend(coords).unwrap();
}

void path::arc(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, f32 start, f32 stop,
               usize segments)
{
  if (segments < 2)
  {
    return;
  }

  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (stop - start) / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[first + i] = (rotor(i * step) - 0.5F) * extent + center;
  }
}

void path::circle(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (2 * PI) / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[first + i] = (rotor(i * step) - 0.5F) * extent + center;
  }
}

void path::squircle(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, f32 elasticity,
                    usize segments)
{
  if (segments < 32)
  {
    return;
  }

  auto const n = segments >> 2;

  elasticity = clamp(elasticity * 0.5F, 0.0F, 0.5F);

  path::cubic_bezier(vtx, extent, center, {0, -0.5F}, {elasticity, -0.5F},
                     {0.5F, -0.5F}, {0.5F, 0}, n);
  path::cubic_bezier(vtx, extent, center, {0.5F, 0}, {0.5F, elasticity},
                     {0.5F, 0.5F}, {0, 0.5F}, n);
  path::cubic_bezier(vtx, extent, center, {0, 0.5F}, {-elasticity, 0.5F},
                     {-0.5F, 0.5F}, {-0.5F, 0}, n);
  path::cubic_bezier(vtx, extent, center, {-0.5F, 0}, {-0.5F, -elasticity},
                     {-0.5F, -0.5F}, {0, -0.5F}, n);
}

void path::rrect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec4 radii,
                 usize segments)
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
  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  usize i = 0;

  vtx[first + i++] = Vec2{0.5F, 0.5F - radii.z};

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
      ((0.5F - radii.z) + radii.z * rotor(s * step)) * extent + center;
  }

  vtx[first + i++] = Vec2{0.5F - radii.z, 0.5F} * extent + center;

  vtx[first + i++] = Vec2{-0.5F + radii.w, 0.5F} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (Vec2{-0.5F + radii.w, 0.5F - radii.w} +
                        radii.w * rotor(PI * 0.5F + s * step)) *
                         extent +
                       center;
  }

  vtx[first + i++] = Vec2{-0.5F, 0.5F - radii.w} * extent + center;

  vtx[first + i++] = Vec2{-0.5F, -0.5F + radii.x} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
      ((-0.5F + radii.x) + radii.x * rotor(PI + s * step)) * extent + center;
  }

  vtx[first + i++] = Vec2{-0.5F + radii.x, -0.5F} * extent + center;

  vtx[first + i++] = Vec2{0.5F - radii.y, -0.5F} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (Vec2{0.5F - radii.y, (-0.5F + radii.y)} +
                        radii.y * rotor(PI * 1.5F + s * step)) *
                         extent +
                       center;
  }

  vtx[first + i++] = Vec2{0.5F, -0.5F + radii.y};
}

void path::brect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec4 slant)
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
    Vec2{-0.5F + slant.x, -0.5F          }
    * extent + center,
    Vec2{0.5F - slant.y,  -0.5F          }
    * extent + center,
    Vec2{0.5F,            -0.5F + slant.y}
    * extent + center,
    Vec2{0.5F,            0.5F - slant.z }
    * extent + center,
    Vec2{0.5F - slant.z,  0.5F           }
    * extent + center,
    Vec2{-0.5F + slant.w, 0.5F           }
    * extent + center,
    Vec2{-0.5F,           0.5F - slant.w }
    * extent + center,
    Vec2{-0.5F,           -0.5F + slant.x}
    * extent + center
  };

  vtx.extend(vertices).unwrap();
}

void path::bezier(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0, Vec2 cp1,
                  Vec2 cp2, usize segments)
{
  if (segments < 3)
  {
    return;
  }

  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[first + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
                          ash::bezier(cp0.y, cp1.y, cp2.y, step * i)} *
                       extent +
                     center;
  }
}

void path::cubic_bezier(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0,
                        Vec2 cp1, Vec2 cp2, Vec2 cp3, usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[first + i] =
      Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
           ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)} *
        extent +
      center;
  }
}

void path::catmull_rom(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0,
                       Vec2 cp1, Vec2 cp2, Vec2 cp3, usize segments)
{
  if (segments < 4)
  {
    return;
  }

  auto const beg = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[beg + i] =
      Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
           ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)} *
        extent +
      center;
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

  I const first_vtx    = vertices.size();
  I const first_idx    = indices.size();
  I const num_points   = points.size();
  I const num_vertices = (num_points - 1) * 4;
  I const num_indices  = (num_points - 1) * 6 + (num_points - 2) * 6;
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
  I const num_triangles = num_vertices / 3;
  I const first_idx     = indices.size();
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

  I const num_indices = (num_vertices - 2) * 3;
  I const first_index = idx.size();

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
  rrect_params_.reset();
  ngon_.params.reset();
  ngon_.vertices.reset();
  ngon_.indices.reset();
  ngon_.index_counts.reset();
  blurs_.reset();
  passes_.reset();
  batches_.reset();
  frame_arena_.reclaim();

  return *this;
}

Canvas & Canvas::begin_recording(u32 num_layers, u32 num_masks,
                                 gpu::Viewport const & new_viewport,
                                 Vec2 new_extent, Vec2U new_framebuffer_extent)
{
  reset();

  CHECK(num_layers >= 3, "");
  CHECK(num_masks >= 2, "");

  num_layers_          = num_layers;
  num_masks_           = num_masks;
  viewport_            = new_viewport;
  extent_              = new_extent;
  framebuffer_extent_  = new_framebuffer_extent;
  framebuffer_uv_base_ = 1 / as_vec2(new_framebuffer_extent);

  if (extent_.x == 0 | extent_.y == 0)
  {
    aspect_ratio_ = 1;
  }
  else
  {
    aspect_ratio_ = extent_.x / extent_.y;
  }

  virtual_scale_ = viewport_.extent.x / new_extent.x;

  // (-0.5w, +0.5w) (-0.5w, +0.5h) -> (-1, +1), (-1, +1)
  world_to_ndc_ = scale3d(vec3(2 / extent_, 1));

  ndc_to_viewport_ =
    // -0.5 extent, +0.5 => 0, extent
    translate3d(vec3(0.5F * extent_, 0.0F)) *
    // -1, +1 => -0.5 extent, +0.5 half_extent
    scale3d(vec3(0.5F * extent_, 1.0F));

  // viewport coordinate to framebuffer coordinate
  viewport_to_fb_ =
    // 0, framebuffer-space extent -> viewport.offset, viewport.offset + framebuffer-space extent
    translate3d(vec3(viewport_.offset, 0.0F)) *
    // 0, viewport-space extent -> 0, framebuffer-space extent
    scale3d(vec3(Vec2::splat(virtual_scale_), 1.0F));

  return *this;
}

RectU Canvas::clip_to_scissor(CRect const & clip) const
{
  // clips are always unscaled
  Rect scissor_f{.offset = viewport_.offset +
                           (clip.begin() + 0.5F * extent_) * virtual_scale_,
                 .extent = clip.extent * virtual_scale_};

  scissor_f =
    Rect::range(clamp_vec(scissor_f.offset, Vec2::splat(0.0F), MAX_CLIP.extent),
                clamp_vec(scissor_f.end(), Vec2::splat(0.0F), MAX_CLIP.extent));

  return RectU::range(
    clamp_vec(as_vec2u(scissor_f.begin()), Vec2U::splat(0),
              framebuffer_extent_),
    clamp_vec(as_vec2u(scissor_f.end()), Vec2U::splat(0), framebuffer_extent_));
}

static inline void add_rrect(Canvas & c, RRectShaderParam const & param,
                             CRect const & clip)
{
  auto const index = size32(c.rrect_params_);
  c.rrect_params_.push(param).unwrap();

  if (c.batches_.is_empty() ||
      c.batches_.last().type != Canvas::BatchType::RRect ||
      c.batches_.last().clip != clip)
  {
    c.batches_
      .push(Canvas::Batch{
        .type = Canvas::BatchType::RRect, .run{index, 1},
           .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches_.last().run.span++;
}

static inline void add_squircle(Canvas & c, SquircleShaderParam const & param,
                                CRect const & clip)
{
  auto const index = size32(c.squircle_params_);
  c.squircle_params_.push(param).unwrap();

  if (c.batches_.is_empty() ||
      c.batches_.last().type != Canvas::BatchType::Squircle ||
      c.batches_.last().clip != clip)
  {
    c.batches_
      .push(Canvas::Batch{
        .type = Canvas::BatchType::Squircle, .run{index, 1},
           .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches_.last().run.span++;
}

static inline void add_ngon(Canvas & c, NgonShaderParam const & param,
                            CRect const & clip, u32 num_indices)
{
  auto const index = size32(c.ngon_.params);
  c.ngon_.index_counts.push(num_indices).unwrap();
  c.ngon_.params.push(param).unwrap();

  if (c.batches_.is_empty() ||
      c.batches_.last().type != Canvas::BatchType::Ngon ||
      c.batches_.last().clip != clip)
  {
    c.batches_
      .push(Canvas::Batch{
        .type = Canvas::BatchType::Ngon,
        .run{.offset = index, .span = 1},
        .clip = clip
    })
      .unwrap();
    return;
  }

  c.batches_.last().run.span++;
}

Canvas & Canvas::end_recording()
{
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
            RRectShaderParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = {1, 1, 1, 1},
              .uv    = {info.uv[0], info.uv[1]},
              .tiling       = info.tiling,
              .aspect_ratio = info.area.extent.x * inv_y,
              .stroke       = info.stroke,
              .thickness    = info.thickness.x * inv_y,
              .feathering   = info.feathering * inv_y,
              .sampler      = info.sampler,
              .albedo       = info.texture
  },
            info.clip);

  return *this;
}

Canvas & Canvas::rect(ShapeInfo const & info)
{
  f32 const inv_y = 1 / info.area.extent.y;
  add_rrect(*this,
            RRectShaderParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = {0, 0, 0, 0},
              .uv    = {info.uv[0], info.uv[1]},
              .tiling       = info.tiling,
              .aspect_ratio = info.area.extent.x * inv_y,
              .stroke       = info.stroke,
              .thickness    = info.thickness.x * inv_y,
              .feathering   = info.feathering * inv_y,
              .sampler      = info.sampler,
              .albedo       = info.texture
  },
            info.clip);
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

  // [ ] account for feathering and thickness
  // [ ] remove alpha in shader and use stroke instead

  add_rrect(*this,
            RRectShaderParam{
              .transform = object_to_world(info.transform, info.area),
              .tint  = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
              .radii = r,
              .uv    = {info.uv[0], info.uv[1]},
              .tiling       = info.tiling,
              .aspect_ratio = info.area.extent.x * inv_y,
              .stroke       = info.stroke,
              .thickness    = info.thickness.x * inv_y,
              .feathering   = info.feathering * inv_y,
              .sampler      = info.sampler,
              .albedo       = info.texture
  },
            info.clip);
  return *this;
}

Canvas & Canvas::brect(ShapeInfo const & info)
{
  auto const first_vertex = size32(ngon_.vertices);
  auto const first_index  = size32(ngon_.indices);

  path::brect(ngon_.vertices, Vec2::splat(1), Vec2::splat(0),
              info.corner_radii);

  auto const num_vertices = size32(ngon_.vertices) - first_vertex;

  path::triangulate_convex(ngon_.indices, first_vertex, num_vertices);

  auto const num_indices = size32(ngon_.indices) - first_index;

  add_ngon(*this,
           NgonShaderParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           info.clip, num_indices);

  return *this;
}

Canvas & Canvas::squircle(ShapeInfo const & info)
{
  f32 const width = max(info.area.extent.x, info.area.extent.y);
  f32 const inv_y = 1 / width;

  add_squircle(
    *this,
    SquircleShaderParam{
      .transform  = object_to_world(info.transform,
                                    CRect{info.area.center, Vec2::splat(width)}
                                    ),
      .tint       = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
      .uv         = {info.uv[0], info.uv[1]},
      .degree     = info.corner_radii.x,
      .tiling     = info.tiling,
      .stroke     = info.stroke,
      .thickness  = info.thickness.x * inv_y,
      .feathering = info.feathering * inv_y,
      .sampler    = info.sampler,
      .albedo     = info.texture
  },
    info.clip);

  return *this;
}

Canvas & Canvas::nine_slice(ShapeInfo const & info, NineSlice const & slice)
{
  // [ ] implement
  return *this;
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return *this;
  }

  auto const first_index  = size32(ngon_.indices);
  auto const first_vertex = size32(ngon_.vertices);

  ngon_.vertices.extend(points).unwrap();
  path::triangles(first_vertex, size32(points), ngon_.indices);

  auto const num_indices = size32(ngon_.vertices) - first_vertex;

  add_ngon(*this,
           NgonShaderParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           info.clip, num_indices);

  return *this;
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points,
                           Span<u32 const> idx)
{
  if (points.size() < 3)
  {
    return *this;
  }

  auto const first_index  = size32(ngon_.indices);
  auto const first_vertex = size32(ngon_.vertices);

  ngon_.vertices.extend(points).unwrap();
  ngon_.indices.extend(idx).unwrap();

  for (auto & v : ngon_.indices.view().slice(first_index))
  {
    v += first_vertex;
  }

  add_ngon(*this,
           NgonShaderParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           info.clip, size32(idx));

  return *this;
}

Canvas & Canvas::line(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return *this;
  }

  auto const first_index  = size32(ngon_.indices);
  auto const first_vertex = size32(ngon_.vertices);
  path::triangulate_stroke(points, ngon_.vertices, ngon_.indices,
                           info.thickness.x / info.area.extent.y);

  auto const num_indices = size32(ngon_.indices) - first_index;

  add_ngon(*this,
           NgonShaderParam{
             .transform = object_to_world(info.transform, info.area),
             .tint   = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
             .uv     = {info.uv[0], info.uv[1]},
             .tiling = info.tiling,
             .sampler      = info.sampler,
             .albedo       = info.texture,
             .first_index  = first_index,
             .first_vertex = first_vertex
  },
           info.clip, num_indices);

  return *this;
}

Canvas & Canvas::blur(ShapeInfo const & info)
{
  auto const index = size32(blurs_);

  auto const world_xfm = object_to_world(info.transform, info.area);

  auto const fb_xfm =
    viewport_to_fb_ * ndc_to_viewport_ * world_to_ndc_ * world_xfm;

  auto const tl = transform(fb_xfm, Vec3{-0.5, -0.5, 0.0}).xy();
  auto const tr = transform(fb_xfm, Vec3{0.5, -0.5, 0.0}).xy();
  auto const bl = transform(fb_xfm, Vec3{-0.5, 0.5, 0.0}).xy();
  auto const br = transform(fb_xfm, Vec3{0.5, 0.5, 0.0}).xy();

  auto const bounding = CRect::bounding(tl, tr, bl, br);

  auto const uv_scale = 1 / as_vec2(framebuffer_extent_);
  auto const uv0      = tl * uv_scale;
  auto const uv1      = br * uv_scale;

  auto const to_brightness = [](Vec4 tint) {
    return vec4(Vec3::splat((tint.x + tint.y + tint.z) * (1 / 3.0F)), 1.0F);
  };

  auto const inv_y = 1 / info.area.extent.y;

  RRectShaderParam rrect{
    .transform = world_xfm,
    .tint{to_brightness(info.tint[0]), to_brightness(info.tint[1]),
          to_brightness(info.tint[2]), to_brightness(info.tint[3])},
    .radii = info.corner_radii * inv_y,
    .uv{uv0, uv1},
    .tiling       = 1,
    .aspect_ratio = info.area.extent.x * inv_y,
    .stroke       = info.stroke,
    .thickness    = 0 * inv_y,
    .feathering   = info.feathering * inv_y,
    .sampler      = SamplerId::LinearClamped,
    .albedo       = TextureId::Base
  };

  auto const area =
    RectU::range(
      as_vec2u(clamp_vec(bounding.begin(), Vec2::splat(0), MAX_CLIP.extent)),
      as_vec2u(clamp_vec(bounding.end(), Vec2::splat(0), MAX_CLIP.extent)))
      .clamp_to_extent(framebuffer_extent_);

  auto const spread_radius =
    as_vec2u(clamp_vec(info.thickness * virtual_scale_, Vec2::splat(0),
                       Vec2::splat(MAX_CLIP_DISTANCE)));

  blurs_
    .push(Blur{.rrect = rrect, .area = area, .spread_radius = spread_radius})
    .unwrap();

  batches_
    .push(Batch{
      .type = BatchType::Blur, .run{index, 1},
         .clip = info.clip
  })
    .unwrap();

  return *this;
}

Canvas & Canvas::pass_(Pass pass)
{
  auto const index = size32(passes_);

  passes_.push(std::move(pass)).unwrap();

  batches_
    .push(Batch{
      .type = BatchType::Pass,
      .run{index, 1},
  })
    .unwrap();

  return *this;
}

u32 Canvas::num_layers() const
{
  return num_layers_;
}

Canvas & Canvas::clear_layer()
{
  // [ ] impl
  return *this;
}

Canvas & Canvas::set_layer(u32 layer)
{
  CHECK(layer < num_layers_, "");
  return *this;
}

u32 Canvas::layer() const
{
  return layer_;
}

u32 Canvas::num_masks() const
{
  return num_masks_;
}

Canvas & Canvas::clear_mask()
{
  // [ ] impl
  return *this;
}

Canvas & Canvas::set_mask(Option<u32> mask)
{
  mask.match([&](u32 i) { CHECK(i < num_masks_, ""); });
  mask_ = mask;
  return *this;
}

Option<u32> Canvas::mask() const
{
  return mask_;
}

Canvas & Canvas::contour_mask(u32 mask, u32 write_mask, Contour2DView contour)
{
  // [ ] copy contour

  // [ ] Vec2 uv0, uv1, uv2; screen-space
  // [ ] f32 in
  // [ ] f32 out

  pass("Contour"_str,
       [contour](FrameGraph & fg, PassBundle & passes, Canvas const & canvas,
                 Framebuffer const & fb, Span<ColorTexture const> colors,
                 Span<DepthStencilTexture const> depth_stencils) {
         // fg.push_ssbo(contour.points.as_u8());
         // fg.push_ssbo(contour.points.as_u8());
         // [ ] lerp  with an always-filled triangle
       });

  return *this;
}

Canvas & Canvas::contour()
{
  // [ ] optimizations: convex
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

BlurPass::Config BlurPass::config(BlurPassParams const & params)
{
  auto const spread_radius = clamp_vec(params.spread_radius, Vec2U::splat(1U),
                                       Vec2U::splat(MAX_SPREAD_RADIUS));

  auto const major_spread_radius = max(spread_radius.x, spread_radius.y);

  auto const padding = Vec2U::splat(max(major_spread_radius + 8, 16U));

  auto const padded_begin = sat_sub(params.area.begin(), padding);
  auto const padded_end   = sat_add(params.area.end(), padding);

  auto const padded_area = RectU::range(padded_begin, padded_end)
                             .clamp_to_extent(params.framebuffer.extent().xy());

  auto const num_passes = clamp(major_spread_radius, 1U, MAX_PASSES);

  return {.spread_radius       = spread_radius,
          .major_spread_radius = major_spread_radius,
          .padding             = padding,
          .padded_area         = padded_area,
          .num_passes          = num_passes};
}

Option<ColorTextureResult> BlurPass::encode(gpu::CommandEncoder &  e,
                                            BlurPassParams const & params)
{
  /*

struct BlurConfig
  {
    Vec2U spread_radius       = {};
    u32   major_spread_radius = {};
    Vec2U padding             = {};
    RectU padded_area         = {};
    u32   num_passes          = 0;
  };

  static constexpr u32 MAX_SPREAD_RADIUS = 16;
  static constexpr u32 MAX_PASSES        = 16;

struct BlurRenderParam
{
  RRectShaderParam rrect         = {};
  RectU         area          = {};
  Vec2U         spread_radius = {};
  RectU         scissor       = {};
  gpu::Viewport viewport      = {};
  Mat4          world_to_ndc  = {};
};

struct BlurRenderer
{
  static void render(FrameGraph & graph, Framebuffer const & fb,
                     Span<ColorTexture const>, Span<DepthStencilTexture const>,
                     PassBundle const & passes, BlurRenderParam const & param);
};

*/

  /*
  auto const scale            = 1 / as_vec2(src_extent);
  auto const uv_spread_radius = as_vec2(spread_radius) * scale;
  auto const uv0              = as_vec2(src_area.begin()) * scale;
  auto const uv1              = as_vec2(src_area.end()) * scale;
  */
  if (!(params.area.is_visible() && params.spread_radius.is_visible()))
  {
    return none;
  }

  auto cfg = config(params);

  if (!cfg.padded_area.is_visible() || cfg.num_passes == 0)
  {
    return none;
  }

  e.blit_image(params.framebuffer.color.image, sys->gpu.scratch_color_[0].image,
               span({
                 gpu::ImageBlit{.src_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .src_area = as_boxu(cfg.padded_area),
                                .dst_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .dst_area = as_boxu(cfg.padded_area)}
  }),
               gpu::Filter::Linear);

  // NOTE: we can avoid a second copy operation if the padded area is same as the blur area,
  // which will happen if we are blurring the entire texture.
  e.blit_image(params.framebuffer.color.image, sys->gpu.scratch_color_[1].image,
               span({
                 gpu::ImageBlit{.src_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .src_area = as_boxu(cfg.padded_area),
                                .dst_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .dst_area = as_boxu(cfg.padded_area)}
  }),
               gpu::Filter::Linear);

  ColorTexture const * const fbs[2] = {&sys->gpu.scratch_color_[0],
                                       &sys->gpu.scratch_color_[1]};

  u32 src = 0;
  u32 dst = 1;

  // downsample pass
  for (u32 i = 1; i <= cfg.num_passes; i++)
  {
    src = (src + 1) & 1;
    dst = (src + 1) & 1;
    auto const spread_radius =
      clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), cfg.spread_radius);
    sample(*this, e, spread_radius, fbs[src]->texture, fbs[src]->texture_id,
           fbs[src]->extent().xy(), params.area, fbs[dst]->view, params.area,
           false);
  }

  // upsample pass
  for (u32 i = cfg.num_passes; i != 0; i--)
  {
    src = (src + 1) & 1;
    dst = (src + 1) & 1;
    auto const spread_radius =
      clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), cfg.spread_radius);
    sample(*this, e, spread_radius, fbs[src]->texture, fbs[src]->texture_id,
           fbs[src]->extent().xy(), params.area, fbs[dst]->view, params.area,
           true);
  }

  // the last output was to scratch 1
  CHECK(dst == 1, "");

  return ColorTextureResult{.color = *fbs[dst], .rect = params.area};
}
}    // namespace ash
