/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/engine/systems.h"
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

Canvas & Canvas::begin_recording(
  FrameGraph & frame_graph, PassBundle & passes,
  Span<ColorTexture const>             color_textures,
  Span<Option<ColorMsaaTexture> const> msaa_color_textures,
  Span<DepthStencilTexture const>      depth_stencil_textures,
  gpu::Viewport const & viewport, Vec2 extent, Vec2U framebuffer_extent)
{
  reset();

  CHECK(color_textures.size() >= 3, "");
  CHECK(msaa_color_textures.size() >= 3, "");
  CHECK(depth_stencil_textures.size() >= 2, "");

  frame_graph_         = frame_graph;
  passes_              = passes;
  viewport_            = viewport;
  extent_              = extent;
  framebuffer_extent_  = framebuffer_extent;
  framebuffer_uv_base_ = 1 / as_vec2(framebuffer_extent);

  if (extent_.x == 0 | extent_.y == 0)
  {
    aspect_ratio_ = 1;
  }
  else
  {
    aspect_ratio_ = extent_.x / extent_.y;
  }

  virtual_scale_ = viewport_.extent.x / extent.x;

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

  world_to_fb_ = viewport_to_fb_ * ndc_to_viewport_ * world_to_ndc_;

  target_  = 0;
  stencil_ = none;

  return *this;
}

Canvas & Canvas::end_recording()
{
  return *this;
}

Canvas & Canvas::reset()
{
  color_textures_.reset();
  msaa_color_textures_.reset();
  depth_stencil_textures_.reset();
  target_              = 0;
  stencil_             = none;
  viewport_            = {};
  extent_              = {};
  framebuffer_extent_  = {};
  framebuffer_uv_base_ = {};
  aspect_ratio_        = 1;
  virtual_scale_       = 1;
  world_to_ndc_        = Affine4::IDENTITY;
  ndc_to_viewport_     = Affine4::IDENTITY;
  viewport_to_fb_      = Affine4::IDENTITY;
  world_to_fb_         = Affine4::IDENTITY;
  encoder_             = none;
  frame_arena_.reclaim();
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

u32 Canvas::num_targets() const
{
  return size32(color_textures_);
}

Canvas & Canvas::clear_target()
{
  // [ ] impl
  return *this;
}

Canvas & Canvas::set_target(u32 target)
{
  CHECK(target < num_targets(), "");
  target_ = target;
  return *this;
}

u32 Canvas::target() const
{
  return target_;
}

u32 Canvas::num_stencils() const
{
  return size32(depth_stencil_textures_);
}

Canvas & Canvas::clear_stencil()
{
  // [ ] impl
  return *this;
}

Canvas & Canvas::set_stencil(Option<Tuple<u32, PassStencil>> stencil)
{
  stencil.match([&](auto s) { CHECK(s.v0 < num_stencils(), ""); });
  stencil_ = stencil;
  return *this;
}

Option<Tuple<u32, PassStencil>> Canvas::stencil() const
{
  return stencil_;
}

constexpr Mat4 object_to_world(Mat4 const & transform, CRect const & area)
{
  return transform * translate3d(vec3(area.center, 0)) *
         scale3d(vec3(area.extent, 1));
}

Canvas & Canvas::circle(ShapeInfo const & info_)
{
  auto info          = info_;
  info.area.extent.x = max(info.area.extent.x, info.area.extent.y);
  info.radii         = Vec4::splat(info.area.extent.x * 0.5F);
  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

Canvas & Canvas::rect(ShapeInfo const & info_)
{
  auto info  = info_;
  info.radii = Vec4::splat(0);
  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

Canvas & Canvas::sdf_shape_(ShapeInfo const &      info,
                            shader::sdf::ShapeType shape_tyoe)
{
  auto const framebuffer = Framebuffer{
    .color      = color_textures_[target_],
    .color_msaa = msaa_color_textures_[target_],
    .depth_stencil =
      stencil_.map([&](auto s) { return depth_stencil_textures_[s.v0]; })
        .unwrap_or()};

  auto const stencil     = stencil_.map([](auto s) { return s.v1; });
  auto const bbox_extent = info.area.extent + info.feather * 2;
  auto const bbox        = CRect{info.area.center, bbox_extent};

  // [ ] switching to noise shader
  auto const shape = shader::sdf::Shape{.radii            = info.radii,
                                        .half_bbox_extent = bbox_extent * 0.5F,
                                        .half_extent = info.area.extent * 0.5F,
                                        .feather     = info.feather,
                                        .shade_type  = info.shade_type,
                                        .type        = shape_tyoe};

  auto const material = shader::sdf::FlatMaterial{
    .tint       = shader::quad::FlatMaterial{.top             = info.tint.top_,
                                             .bottom          = info.tint.bottom_,
                                             .gradient_rotor  = info.tint.rotor_,
                                             .uv0             = info.uv[0],
                                             .uv1             = info.uv[1],
                                             .gradient_center = info.tint.center_,
                                             .sampler         = info.sampler,
                                             .texture         = info.texture},
    .sampler_id = SamplerId::LinearBlack,
    .map_id     = TextureId::Base
  };

  auto const item =
    SdfEncoder::Item<shader::sdf::Shape, shader::sdf::FlatMaterial>{
      .framebuffer    = framebuffer,
      .stencil        = stencil,
      .scissor        = clip_to_scissor(info.clip),
      .viewport       = viewport_,
      .samplers       = sys->gpu.samplers_,
      .textures       = sys->gpu.textures_,
      .world_to_ndc   = world_to_ndc_,
      .shape          = shape,
      .transform      = object_to_world(info.transform, bbox),
      .material       = material,
      .shader_variant = ShaderVariantId::Base};

  push_sdf_(item);

  return *this;
}

Canvas & Canvas::ngon_shape_(ShapeInfo const & info, Span<f32x2 const> vertices,
                             Span<u32 const> indices)
{
  auto const framebuffer = Framebuffer{
    .color      = color_textures_[target_],
    .color_msaa = msaa_color_textures_[target_],
    .depth_stencil =
      stencil_.map([&](auto s) { return depth_stencil_textures_[s.v0]; })
        .unwrap_or()};

  auto const stencil = stencil_.map([](auto s) { return s.v1; });

  auto const material =
    shader::ngon::FlatMaterial{.top             = info.tint.top_,
                               .bottom          = info.tint.bottom_,
                               .gradient_rotor  = info.tint.rotor_,
                               .uv0             = info.uv[0],
                               .uv1             = info.uv[1],
                               .gradient_center = info.tint.center_,
                               .sampler         = info.sampler,
                               .texture         = info.texture};

  NgonEncoder::Item<shader::ngon::FlatMaterial> item{
    .framebuffer    = framebuffer,
    .stencil        = stencil,
    .scissor        = clip_to_scissor(info.clip),
    .viewport       = viewport_,
    .samplers       = sys->gpu.samplers_,
    .textures       = sys->gpu.textures_,
    .world_to_ndc   = world_to_ndc_,
    .transform      = object_to_world(info.transform, info.area),
    .vertices       = vertices,
    .indices        = indices,
    .material       = material,
    .shader_variant = ShaderVariantId::Base};

  return push_ngon_(item);
}

Canvas & Canvas::rrect(ShapeInfo const & info_)
{
  auto       info       = info_;
  auto const max_radius = 0.5F * min(info.area.extent.x, info.area.extent.y);
  info.radii.x          = min(info.radii.x, max_radius);
  info.radii.y          = min(info.radii.y, max_radius);
  info.radii.z          = min(info.radii.z, max_radius);
  info.radii.w          = min(info.radii.w, max_radius);

  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

Canvas & Canvas::squircle(ShapeInfo const & info_)
{
  auto       info       = info_;
  auto const max_radius = 0.5F * min(info.area.extent.x, info.area.extent.y);
  info.radii.x          = min(info.radii.x, max_radius);

  return sdf_shape_(info, shader::sdf::ShapeType::Squircle);
}

Canvas & Canvas::bevel_rect(ShapeInfo const & info)
{
  Vec<f32x2> vertices{frame_arena_};
  Vec<u32>   indices{frame_arena_};

  path::brect(vertices, info.area.extent, info.area.center, info.radii);
  path::triangulate_convex(indices, 0, vertices.size());
  return ngon_shape_(info, vertices, indices);
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

  Vec<u32> indices{frame_arena_};
  path::triangles(0, points.size(), indices);

  return triangles(info, points, indices);
}

Canvas & Canvas::triangles(ShapeInfo const & info, Span<Vec2 const> points,
                           Span<u32 const> indices)
{
  if (points.size() < 3)
  {
    return *this;
  }

  return ngon_shape_(info, points, indices);
}

Canvas & Canvas::line(ShapeInfo const & info, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return *this;
  }

  Vec<f32x2> vertices{frame_arena_};
  Vec<u32>   indices{frame_arena_};

  path::triangulate_stroke(points, vertices, indices, info.feather * 2);
  return ngon_shape_(info, vertices, indices);
}

Canvas & Canvas::blur(ShapeInfo const & info_)
{
  auto info            = info_;
  auto max_radius      = 0.5F * min(info.area.extent.x, info.area.extent.y);
  info.radii.x         = min(info.radii.x, max_radius);
  info.radii.y         = min(info.radii.y, max_radius);
  info.radii.z         = min(info.radii.z, max_radius);
  info.radii.w         = min(info.radii.w, max_radius);
  auto const world_xfm = object_to_world(info.transform, info.area);
  auto const fb_xfm    = world_to_fb_ * world_xfm;
  auto const tl        = transform(fb_xfm, Vec3{-0.5, -0.5, 0.0}).xy();
  auto const tr        = transform(fb_xfm, Vec3{0.5, -0.5, 0.0}).xy();
  auto const bl        = transform(fb_xfm, Vec3{-0.5, 0.5, 0.0}).xy();
  auto const br        = transform(fb_xfm, Vec3{0.5, 0.5, 0.0}).xy();
  auto const bounding  = CRect::bounding(tl, tr, bl, br);

  auto const area =
    RectU::range(
      as_vec2u(clamp_vec(bounding.begin(), Vec2::splat(0), MAX_CLIP.extent)),
      as_vec2u(clamp_vec(bounding.end(), Vec2::splat(0), MAX_CLIP.extent)))
      .clamp_to_extent(framebuffer_extent_);

  auto spread_radius = as_vec2u(
    Vec2::splat(clamp(info.feather * virtual_scale_, 0.0F, MAX_CLIP_DISTANCE)));

  if (!area.is_visible() || !spread_radius.is_visible())
  {
    return *this;
  }

  static constexpr u32 MAX_SPREAD_RADIUS = 16;
  static constexpr u32 MAX_PASSES        = 16;

  spread_radius =
    clamp_vec(spread_radius, Vec2U::splat(1U), Vec2U::splat(MAX_SPREAD_RADIUS));

  auto const major_spread_radius = max(spread_radius.x, spread_radius.y);
  auto const padding      = Vec2U::splat(max(major_spread_radius + 8, 16U));
  auto const padded_begin = sat_sub(area.begin(), padding);
  auto const padded_end   = sat_add(area.end(), padding);
  auto const padded_area =
    RectU::range(padded_begin, padded_end).clamp_to_extent(framebuffer_extent_);
  auto const num_passes = clamp(major_spread_radius, 1U, MAX_PASSES);

  if (!padded_area.is_visible() || num_passes == 0)
  {
    return *this;
  }

  auto const main_fb      = color_textures_[0];
  auto const scratch_fb0  = color_textures_[1];
  auto const scratch_fb1  = color_textures_[2];
  auto const pass_stencil = stencil_.map([](auto s) { return s.v1; });
  auto const stencil_tex =
    stencil_.map([&](auto s) { return depth_stencil_textures_[s.v0]; })
      .unwrap_or();

  pass([info, main_fb, scratch_fb0, scratch_fb1, padded_area, num_passes,
        spread_radius, area, viewport = this->viewport_,
        scissor      = clip_to_scissor(info.clip),
        world_to_ndc = this->world_to_ndc_, pass_stencil,
        stencil_tex](FrameGraph & fg, PassBundle & passes) {
    fg.add_pass(
      "Blur.TextureCopy"_str, [main_fb, scratch_fb0, scratch_fb1, padded_area](
                                FrameGraph &, gpu::CommandEncoder & enc) {
        // copy to scratch texture 0 & 1. This is to prevent texture-spilling when ping-ponging between the two textures
        enc.blit_image(
          main_fb.image, scratch_fb0.image,
          span({
            gpu::ImageBlit{.src_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .src_area = as_boxu(padded_area),
                           .dst_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .dst_area = as_boxu(padded_area)}
        }),
          gpu::Filter::Linear);

        // NOTE: we can avoid a second copy operation if the padded area is same as the blur area,
        // which will happen if we are blurring the entire texture.
        enc.blit_image(
          main_fb.image, scratch_fb1.image,
          span({
            gpu::ImageBlit{.src_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .src_area = as_boxu(padded_area),
                           .dst_layers{.aspects   = gpu::ImageAspects::Color,
                                       .mip_level = 0,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1},
                           .dst_area = as_boxu(padded_area)}
        }),
          gpu::Filter::Linear);
      });

    ColorTexture const textures[2] = {scratch_fb0, scratch_fb1};

    usize src = 0;
    usize dst = 1;

    // downsample pass
    for (usize i = 1; i <= num_passes; i++)
    {
      src                = (src + 1) & 1;
      dst                = (src + 1) & 1;
      auto const src_tex = textures[src];
      auto const dst_tex = textures[dst];
      auto const spread =
        clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), spread_radius);
      auto const base = 1 / as_vec2(src_tex.extent().xy());
      auto const blur = shader::blur::Blur{.uv0 = as_vec2(area.begin()) * base,
                                           .uv1 = as_vec2(area.end()) * base,
                                           .radius  = as_vec2(spread) * base,
                                           .sampler = SamplerId::LinearClamped,
                                           .tex     = src_tex.texture_id};

      auto const blur_id = fg.push_ssbo(span({blur}));

      fg.add_pass("Blur.Downsample"_str,
                  [blur_id, &passes, src_tex, dst_tex, padded_area,
                   viewport](FrameGraph & fg, gpu::CommandEncoder & enc) {
                    auto blur = fg.get(blur_id);

                    passes.blur->encode(
                      enc, BlurPassParams{
                             .framebuffer = Framebuffer{.color      = dst_tex,
                                                        .color_msaa = {},
                                                        .depth_stencil = {}},
                             .stencil     = none,
                             .scissor     = padded_area,
                             .viewport    = viewport,
                             .samplers    = sys->gpu.samplers_,
                             .textures    = src_tex.texture,
                             .blurs       = blur,
                             .instances   = {0, 1},
                             .upsample    = false
                    });
                  });
    }

    // upsample pass
    for (usize i = num_passes; i != 0; i--)
    {
      src                = (src + 1) & 1;
      dst                = (src + 1) & 1;
      auto const src_tex = textures[src];
      auto const dst_tex = textures[dst];
      auto const spread =
        clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), spread_radius);
      auto const base = 1 / as_vec2(src_tex.extent().xy());
      auto const blur = shader::blur::Blur{.uv0 = as_vec2(area.begin()) * base,
                                           .uv1 = as_vec2(area.end()) * base,
                                           .radius  = as_vec2(spread) * base,
                                           .sampler = SamplerId::LinearClamped,
                                           .tex     = src_tex.texture_id};

      auto const blur_id = fg.push_ssbo(span({blur}));

      fg.add_pass("Blur.Upsample"_str,
                  [blur_id, &passes, src_tex, dst_tex, padded_area,
                   viewport](FrameGraph & fg, gpu::CommandEncoder & enc) {
                    auto blur = fg.get(blur_id);

                    passes.blur->encode(
                      enc, BlurPassParams{
                             .framebuffer = Framebuffer{.color      = dst_tex,
                                                        .color_msaa = {},
                                                        .depth_stencil = {}},
                             .stencil     = none,
                             .scissor     = padded_area,
                             .viewport    = viewport,
                             .samplers    = sys->gpu.samplers_,
                             .textures    = src_tex.texture,
                             .blurs       = blur,
                             .instances   = {0, 1},
                             .upsample    = true
                    });
                  });
    }

    // the last output was to scratch 1
    CHECK(dst == 1, "");

    // final pass: draw from scratch 1 to main, use stencil if any
    // [ ] how will layering work with this?; tint
    // [ ] we can apply tint in another pass

    {
      auto const src_tex  = textures[dst];
      auto const base     = 1 / as_vec2(src_tex.extent().xy());
      auto const material = shader::sdf::FlatMaterial{
        .tint       = shader::quad::FlatMaterial{.top            = info.tint.top_,
                                                 .bottom         = info.tint.bottom_,
                                                 .gradient_rotor = info.tint.rotor_,
                                                 .uv0 = as_vec2(area.begin()) * base,
                                                 .uv1 = as_vec2(area.end()) * base,
                                                 .gradient_center = info.tint.center_,
                                                 .sampler = SamplerId::LinearClamped,
                                                 .texture = src_tex.texture_id},
        .sampler_id = SamplerId::LinearClamped,
        .map_id     = TextureId::Base
      };

      auto const shape =
        shader::sdf::Shape{.radii            = info.radii,
                           .half_bbox_extent = info.area.extent * 0.5F,
                           .half_extent      = info.area.extent * 0.5F,
                           .feather          = 0,
                           .shade_type       = ShadeType::Flood,
                           .type             = shader::sdf::ShapeType::RRect};

      // [ ] msaa? is it handled in other encoders? no!
      auto const item =
        SdfEncoder::Item<shader::sdf::Shape, shader::sdf::FlatMaterial>{
          .framebuffer    = {.color         = main_fb,
                             .color_msaa    = {},
                             .depth_stencil = stencil_tex},
          .stencil        = pass_stencil,
          .scissor        = scissor,
          .viewport       = viewport,
          .samplers       = sys->gpu.samplers_,
          .textures       = src_tex.texture,
          .world_to_ndc   = world_to_ndc,
          .shape          = shape,
          .transform      = info.transform,
          .material       = material,
          .shader_variant = ShaderVariantId::Base
      };

      // [ ] change allocator
      auto const encoder = SdfEncoder{default_allocator, item};

      encoder.pass(fg, passes);
    }
  });

  return *this;
}

Canvas & Canvas::contour_stencil_(u32 stencil, u32 write_mask,
                                  Contour2D const & contour)
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

}    // namespace ash
