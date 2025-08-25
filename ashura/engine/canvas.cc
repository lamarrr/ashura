/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"

namespace ash
{

static constexpr u8 num_control_points(ContourEdgeType edge)
{
  switch (edge)
  {
    case ContourEdgeType::Line:
      return 2;
    case ContourEdgeType::Arc:
    case ContourEdgeType::Bezier:
      return 3;
    case ContourEdgeType::CubicBezier:
      return 4;
    default:
      return 0;
  }
}

void path::rect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center)
{
  f32x2 const coords[] = {
    f32x2{-0.5, -0.5}
    * extent + center, f32x2{0.5,  -0.5}
    * extent + center,
    f32x2{0.5,  0.5 }
    * extent + center, f32x2{-0.5, 0.5 }
    * extent + center
  };

  vtx.extend(coords).unwrap();
}

void path::arc(Vec<f32x2> & vtx, f32x2 radii, f32x2 center, f32 start, f32 turn,
               usize segments)
{
  if (segments < 2)
  {
    return;
  }

  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = turn / (segments - 1);

  for (usize i = 0; i < segments; i++)
  {
    vtx[first + i] = (rotor(start + i * step) - 0.5F) * 2 * radii + center;
  }
}

void path::circle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, usize segments)
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

void path::squircle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center,
                    f32 elasticity, usize segments)
{
  if (segments < 32)
  {
    return;
  }

  auto const n = segments >> 2;

  elasticity = clamp(elasticity * 0.5F, 0.0F, 0.5F);

  path::cubic_bezier(vtx, center + extent * f32x2{0, -0.5F},
                     center + extent * f32x2{elasticity, -0.5F},
                     center + extent * f32x2{0.5F, -0.5F},
                     center + extent * f32x2{0.5F, 0}, n);
  path::cubic_bezier(vtx, center + extent * f32x2{0.5F, 0},
                     center + extent * f32x2{0.5F, elasticity},
                     center + extent * f32x2{0.5F, 0.5F},
                     center + extent * f32x2{0, 0.5F}, n);
  path::cubic_bezier(vtx, center + extent * f32x2{0, 0.5F},
                     center + extent * f32x2{-elasticity, 0.5F},
                     center + extent * f32x2{-0.5F, 0.5F},
                     center + extent * f32x2{-0.5F, 0}, n);
  path::cubic_bezier(vtx, center + extent * f32x2{-0.5F, 0},
                     center + extent * f32x2{-0.5F, -elasticity},
                     center + extent * f32x2{-0.5F, -0.5F},
                     center + extent * f32x2{0, -0.5F}, n);
}

void path::rrect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 radii,
                 usize segments)
{
  if (segments < 8)
  {
    return;
  }

  radii = radii.max(f32x4::zero());

  // [ ] fix
  /// clipping
  /*
  radii.y()          = min(radii.y(), 1.0F - radii.x());
  f32 max_radius_z = min(1.0F - radii.x(), 0.5F - radii.y());
  radii.z()          = min(radii.z(), max_radius_z);
  f32 max_radius_w = min(max_radius_z, 0.5F - radii.z());
  radii.w()          = min(radii.w(), max_radius_w);
  */

  auto const curve_segments = (segments - 8) >> 2;
  f32 const  step =
    (curve_segments == 0) ? 0.0F : ((PI * 0.5F) / curve_segments);
  auto const first = vtx.size();

  vtx.extend_uninit(segments).unwrap();

  usize i = 0;

  vtx[first + i++] = f32x2{0.5F, 0.5F - radii.z()};

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
      ((0.5F - radii.z()) + radii.z() * rotor(s * step)) * extent + center;
  }

  vtx[first + i++] = f32x2{0.5F - radii.z(), 0.5F} * extent + center;

  vtx[first + i++] = f32x2{-0.5F + radii.w(), 0.5F} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (f32x2{-0.5F + radii.w(), 0.5F - radii.w()} +
                        radii.w() * rotor(PI * 0.5F + s * step)) *
                         extent +
                       center;
  }

  vtx[first + i++] = f32x2{-0.5F, 0.5F - radii.w()} * extent + center;

  vtx[first + i++] = f32x2{-0.5F, -0.5F + radii.x()} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
      ((-0.5F + radii.x()) + radii.x() * rotor(PI + s * step)) * extent +
      center;
  }

  vtx[first + i++] = f32x2{-0.5F + radii.x(), -0.5F} * extent + center;

  vtx[first + i++] = f32x2{0.5F - radii.y(), -0.5F} * extent + center;

  for (usize s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (f32x2{0.5F - radii.y(), (-0.5F + radii.y())} +
                        radii.y() * rotor(PI * 1.5F + s * step)) *
                         extent +
                       center;
  }

  vtx[first + i++] = f32x2{0.5F, -0.5F + radii.y()};
}

void path::bezier(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2,
                  usize segments)
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
    vtx[first + i] = f32x2{ash::bezier(cp0.x(), cp1.x(), cp2.x(), step * i),
                           ash::bezier(cp0.y(), cp1.y(), cp2.y(), step * i)};
  }
}

void path::cubic_bezier(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2,
                        f32x2 cp3, usize segments)
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
      f32x2{ash::cubic_bezier(cp0.x(), cp1.x(), cp2.x(), cp3.x(), step * i),
            ash::cubic_bezier(cp0.y(), cp1.y(), cp2.y(), cp3.y(), step * i)};
  }
}

void path::catmull_rom(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2,
                       f32x2 cp3, usize segments)
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
      f32x2{ash::cubic_bezier(cp0.x(), cp1.x(), cp2.x(), cp3.x(), step * i),
            ash::cubic_bezier(cp0.y(), cp1.y(), cp2.y(), cp3.y(), step * i)};
  }
}

template <typename I>
void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vertices,
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

  auto * vtx  = vertices.data() + first_vtx;
  I *    idx  = indices.data() + first_idx;
  I      ivtx = 0;

  for (I i = 0; i < num_points - 1; i++)
  {
    auto const p0    = points[i];
    auto const p1    = points[i + 1];
    auto const d     = p1 - p0;
    auto const alpha = atan2f(d.y(), d.x());

    // parallel angle
    auto const down = (thickness * 0.5F) * rotor(alpha + PI * 0.5F);

    // perpendicular angle
    auto const up = -down;

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

void path::triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vertices,
                              Vec<u16> & indices, f32 thickness)
{
  ::ash::triangulate_stroke(points, vertices, indices, thickness);
}

void path::triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vertices,
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

ICanvas & ICanvas::begin(GpuFramePlan plan, gpu::Viewport const & viewport,
                         f32x2 extent, u32x2 framebuffer_extent)
{
  // [ ] use CANVASSTATE
  // reset();

  CHECK(color_textures.size() >= 3, "");
  CHECK(msaa_color_textures.size() >= 3, "");
  CHECK(depth_stencil_textures.size() >= 2, "");

  frame_graph_         = frame_graph;
  passes_              = passes;
  viewport_            = viewport;
  extent_              = extent;
  framebuffer_extent_  = framebuffer_extent;
  framebuffer_uv_base_ = 1 / framebuffer_extent.to<f32>();

  if (extent_.x() == 0 | extent_.y() == 0)
  {
    aspect_ratio_ = 1;
  }
  else
  {
    aspect_ratio_ = extent_.x() / extent_.y();
  }

  virtual_scale_ = viewport_.extent.x() / extent.x();

  // (-0.5w, +0.5w) (-0.5w, +0.5h) -> (-1, +1), (-1, +1)
  world_to_ndc_ = scale3d((2 / extent_).append(1));

  ndc_to_viewport_ =
    // -0.5 extent, +0.5 => 0, extent
    translate3d((0.5F * extent_).append(0.0F)) *
    // -1, +1 => -0.5 extent, +0.5 half_extent
    scale3d((0.5F * extent_).append(1.0F));

  // viewport coordinate to framebuffer coordinate
  viewport_to_fb_ =
    // 0, framebuffer-space extent -> viewport.offset, viewport.offset + framebuffer-space extent
    translate3d(viewport_.offset.append(0.0F)) *
    // 0, viewport-space extent -> 0, framebuffer-space extent
    scale3d(f32x2::splat(virtual_scale_).append(1.0F));

  world_to_fb_ = viewport_to_fb_ * ndc_to_viewport_ * world_to_ndc_;

  target_  = 0;
  stencil_ = none;

  return *this;
}

ICanvas & ICanvas::end()
{
  return *this;
}

ICanvas & ICanvas::reset()
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
  world_to_ndc_        = affinef32x4::identity();
  ndc_to_viewport_     = affinef32x4::identity();
  viewport_to_fb_      = affinef32x4::identity();
  world_to_fb_         = affinef32x4::identity();
  encoder_             = none;
  frame_arena_.reclaim();
  return *this;
}

RectU ICanvas::clip_to_scissor(CRect const & clip) const
{
  // clips are always unscaled
  Rect scissor_f{.offset = viewport_.offset +
                           (clip.begin() + 0.5F * extent_) * virtual_scale_,
                 .extent = clip.extent * virtual_scale_};

  scissor_f =
    Rect::range(scissor_f.offset.clamp(f32x2::splat(0.0F), MAX_CLIP.extent),
                scissor_f.end().clamp(f32x2::splat(0.0F), MAX_CLIP.extent));

  return RectU::range(scissor_f.begin().to<u32>().min(framebuffer_extent_),
                      scissor_f.end().to<u32>().min(framebuffer_extent_));
}

u32 ICanvas::num_targets() const
{
  return size32(color_textures_);
}

ICanvas & ICanvas::clear_target(gpu::Color color)
{
  pass([image      = color_textures_[target_],
        msaa_image = msaa_color_textures_[target_],
        color](FrameGraph & frame_graph, PassBundle &) {
    frame_graph.add_pass(
      "Target:Clear"_str,
      [image, msaa_image, color](FrameGraph &, gpu::CommandEncoder & encoder) {
        encoder.clear_color_image(
          image.image, color,
          span({
            gpu::ImageSubresourceRange{
                                       .aspects           = gpu::ImageAspects::Color,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = gpu::REMAINING_MIP_LEVELS,
                                       .first_array_layer = 0,
                                       .num_array_layers  = gpu::REMAINING_ARRAY_LAYERS}
        }));

        msaa_image.match([&](auto & image) {
          encoder.clear_color_image(
            image.image, color,
            span({
              gpu::ImageSubresourceRange{
                                         .aspects           = gpu::ImageAspects::Color,
                                         .first_mip_level   = 0,
                                         .num_mip_levels    = gpu::REMAINING_MIP_LEVELS,
                                         .first_array_layer = 0,
                                         .num_array_layers  = gpu::REMAINING_ARRAY_LAYERS}
          }));
        });
      });
  });
  return *this;
}

ICanvas & ICanvas::set_target(u32 target)
{
  CHECK(target < num_targets(), "");
  target_ = target;
  return *this;
}

u32 ICanvas::target() const
{
  return target_;
}

u32 ICanvas::num_stencils() const
{
  return size32(depth_stencil_textures_);
}

ICanvas & ICanvas::clear_stencil(u32 stencil_value)
{
  stencil_.match([this, stencil_value](auto s) {
    pass([image = depth_stencil_textures_[s.v0],
          stencil_value](FrameGraph & frame_graph, PassBundle &) {
      frame_graph.add_pass(
        "Stencil:Clear"_str,
        [image, stencil_value](FrameGraph &, gpu::CommandEncoder & encoder) {
          encoder.clear_depth_stencil_image(
            image.image,
            gpu::DepthStencil{
              .stencil = stencil_value
          },
            span({gpu::ImageSubresourceRange{
              .aspects           = gpu::ImageAspects::Stencil,
              .first_mip_level   = 0,
              .num_mip_levels    = gpu::REMAINING_MIP_LEVELS,
              .first_array_layer = 0,
              .num_array_layers  = gpu::REMAINING_ARRAY_LAYERS}}));
        });
    });
  });

  return *this;
}

ICanvas & ICanvas::set_stencil(Option<Tuple<u32, PipelineStencil>> stencil)
{
  stencil.match([&](auto s) { CHECK(s.v0 < num_stencils(), ""); });
  stencil_ = stencil;
  return *this;
}

Option<Tuple<u32, PipelineStencil>> ICanvas::stencil() const
{
  return stencil_;
}

constexpr f32x4x4 object_to_world(f32x4x4 const & transform, CRect const & area)
{
  return transform * translate3d(area.center.append(0)) *
         scale3d(area.extent.append(1));
}

ICanvas & ICanvas::circle(ShapeInfo const & info_)
{
  auto info            = info_;
  info.area.extent.x() = max(info.area.extent.x(), info.area.extent.y());
  info.radii           = f32x4::splat(info.area.extent.x() * 0.5F);
  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

ICanvas & ICanvas::rect(ShapeInfo const & info_)
{
  auto info  = info_;
  info.radii = f32x4::splat(0);
  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

ICanvas & ICanvas::sdf_shape_(ShapeInfo const &      info,
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
      .world_to_ndc   = static_cast<f32x4x4>(world_to_ndc_),
      .shape          = shape,
      .transform      = object_to_world(info.transform, bbox),
      .material       = material,
      .shader_variant = ShaderVariantId::Base};

  push_sdf_(item);

  return *this;
}

ICanvas & ICanvas::ngon_shape_(ShapeInfo const & info,
                               Span<f32x2 const> vertices,
                               Span<u32 const>   indices)
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
    .world_to_ndc   = static_cast<f32x4x4>(world_to_ndc_),
    .transform      = object_to_world(info.transform, info.area),
    .vertices       = vertices,
    .indices        = indices,
    .material       = material,
    .shader_variant = ShaderVariantId::Base};

  return push_ngon_(item);
}

ICanvas & ICanvas::rrect(ShapeInfo const & info_)
{
  auto       info = info_;
  auto const max_radius =
    0.5F * min(info.area.extent.x(), info.area.extent.y());
  info.radii = info.radii.min(max_radius);
  return sdf_shape_(info, shader::sdf::ShapeType::RRect);
}

ICanvas & ICanvas::squircle(ShapeInfo const & info_)
{
  auto       info = info_;
  auto const max_radius =
    0.5F * min(info.area.extent.x(), info.area.extent.y());
  info.radii.x() = min(info.radii.x(), max_radius);

  return sdf_shape_(info, shader::sdf::ShapeType::Squircle);
}

ICanvas & ICanvas::nine_slice(ShapeInfo const & info, NineSlice const & slice)
{
  // [ ] implement
  return *this;
}

ICanvas & ICanvas::triangles(ShapeInfo const & info, Span<f32x2 const> points)
{
  if (points.size() < 3)
  {
    return *this;
  }

  Vec<u32> indices{frame_arena_};
  path::triangles(0, points.size(), indices);

  return triangles(info, points, indices);
}

ICanvas & ICanvas::triangles(ShapeInfo const & info, Span<f32x2 const> points,
                             Span<u32 const> indices)
{
  if (points.size() < 3)
  {
    return *this;
  }

  return ngon_shape_(info, points, indices);
}

ICanvas & ICanvas::line(ShapeInfo const & info, Span<f32x2 const> points)
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

ICanvas & ICanvas::blur(ShapeInfo const & info_)
{
  auto info            = info_;
  auto max_radius      = 0.5F * min(info.area.extent.x(), info.area.extent.y());
  info.radii           = info.radii.min(max_radius);
  auto const world_xfm = object_to_world(info.transform, info.area);
  auto const fb_xfm    = world_to_fb_ * world_xfm;
  auto const tl        = transform(fb_xfm, f32x3{-0.5, -0.5, 0.0}).xy();
  auto const tr        = transform(fb_xfm, f32x3{0.5, -0.5, 0.0}).xy();
  auto const bl        = transform(fb_xfm, f32x3{-0.5, 0.5, 0.0}).xy();
  auto const br        = transform(fb_xfm, f32x3{0.5, 0.5, 0.0}).xy();
  auto const bounding  = CRect::bounding(tl, tr, bl, br);

  auto const area =
    RectU::range(
      bounding.begin().clamp(f32x2::splat(0), MAX_CLIP.extent).to<u32>(),
      bounding.end().clamp(f32x2::splat(0), MAX_CLIP.extent).to<u32>())
      .clamp_to_extent(framebuffer_extent_);

  auto spread_radius =
    f32x2::splat(clamp(info.feather * virtual_scale_, 0.0F, MAX_CLIP_DISTANCE))
      .to<u32>();

  if (!area.is_visible() || spread_radius.any_zero())
  {
    return *this;
  }

  static constexpr u32 MAX_SPREAD_RADIUS = 16;
  static constexpr u32 MAX_PASSES        = 16;

  spread_radius =
    spread_radius.clamp(u32x2::splat(1U), u32x2::splat(MAX_SPREAD_RADIUS));

  auto const major_spread_radius = max(spread_radius.x(), spread_radius.y());
  auto const padding      = u32x2::splat(max(major_spread_radius + 8, 16U));
  auto const padded_begin = area.begin().sat_sub(padding);
  auto const padded_end   = area.end().sat_add(padding);
  auto const padded_area =
    RectU::range(padded_begin, padded_end).clamp_to_extent(framebuffer_extent_);
  auto const num_passes = clamp(major_spread_radius, 1U, MAX_PASSES);

  if (!padded_area.is_visible() || num_passes == 0)
  {
    return *this;
  }

  // [ ] blur of a specific layer?
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
        world_to_ndc = this->world_to_ndc_, pass_stencil, stencil_tex,
        frame_arena  = &frame_arena_](FrameGraph & frame_graph,
                                     PassBundle & passes) {
    frame_graph.add_pass(
      "Blur:TextureCopy"_str, [main_fb, scratch_fb0, scratch_fb1, padded_area](
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
        u32x2::splat(i).clamp(u32x2::splat(1U), spread_radius);
      auto const base = 1 / src_tex.extent().xy().to<f32>();
      auto const blur = shader::blur::Blur{.uv0 = area.begin().to<f32>() * base,
                                           .uv1 = area.end().to<f32>() * base,
                                           .radius  = spread.to<f32>() * base,
                                           .sampler = SamplerId::LinearClamped,
                                           .tex     = src_tex.texture_id};

      auto const blur_id = frame_graph.push_ssbo(span({blur}));

      frame_graph.add_pass(
        "Blur:Downsample"_str,
        [blur_id, &passes, src_tex, dst_tex, padded_area,
         viewport](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
          auto blur = frame_graph.get(blur_id);

          passes.blur->encode(enc,
                              BlurPassParams{
                                .framebuffer = Framebuffer{.color = dst_tex,
                                                           .color_msaa    = {},
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
        u32x2::splat(i).clamp(u32x2::splat(1U), spread_radius);
      auto const base = 1 / src_tex.extent().xy().to<f32>();
      auto const blur = shader::blur::Blur{.uv0 = area.begin().to<f32>() * base,
                                           .uv1 = area.end().to<f32>() * base,
                                           .radius  = spread.to<f32>() * base,
                                           .sampler = SamplerId::LinearClamped,
                                           .tex     = src_tex.texture_id};

      auto const blur_id = frame_graph.push_ssbo(span({blur}));

      frame_graph.add_pass(
        "Blur:Upsample"_str,
        [blur_id, &passes, src_tex, dst_tex, padded_area,
         viewport](FrameGraph & frame_graph, gpu::CommandEncoder & enc) {
          auto blur = frame_graph.get(blur_id);

          passes.blur->encode(enc,
                              BlurPassParams{
                                .framebuffer = Framebuffer{.color = dst_tex,
                                                           .color_msaa    = {},
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

    {
      auto const src_tex  = textures[dst];
      auto const base     = 1 / src_tex.extent().xy().to<f32>();
      auto const material = shader::sdf::FlatMaterial{
        .tint       = shader::quad::FlatMaterial{.top            = info.tint.top_,
                                                 .bottom         = info.tint.bottom_,
                                                 .gradient_rotor = info.tint.rotor_,
                                                 .uv0 = area.begin().to<f32>() * base,
                                                 .uv1 = area.end().to<f32>() * base,
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
          .world_to_ndc   = static_cast<f32x4x4>(world_to_ndc),
          .shape          = shape,
          .transform      = info.transform,
          .material       = material,
          .shader_variant = ShaderVariantId::Base
      };

      auto const encoder = SdfEncoder{*frame_arena, item};

      encoder.pass(frame_graph, passes);
    }
  });

  return *this;
}

ICanvas & ICanvas::contour_stencil_(u32 stencil, u32 write_mask, bool tesselate,
                                    Span<f32x2 const>           control_points,
                                    Span<ContourEdgeType const> edge_types,
                                    Span<u16 const> subdivision_counts,
                                    bool invert, FillRule fill_rule,
                                    f32x4x4 const & transform,
                                    CRect const &   clip)
{
  if (edge_types.is_empty() || control_points.is_empty())
  {
    return *this;
  }
  // [ ] move scratch textures into FrameGraph

  if (!tesselate)
  {
    // [ ] make triangle list; no vertices
    Vec<f32x2> vertices{frame_arena_};
    vertices.extend(control_points).unwrap();
    Vec<u32> indices{frame_arena_};

    // edge fills
    {
      u32 first_cp = 0;

      for (u32 i = 0; i < size32(edge_types); i++)
      {
        auto type    = edge_types[i];
        auto num_cps = num_control_points(type);
        auto last_cp = first_cp + (num_cps - 1);

        // [ ] fix triangle indices to fan-based?

        if (i != 0)
        {
          u32 const triangles[] = {0, first_cp, last_cp};
          indices.extend(triangles).unwrap();
        }

        first_cp = last_cp;
      }
    }

    auto num_straight_indices = size32(indices);

    // bezier fills
    {
      u32 first_cp = 0;

      for (u32 i = 0; i < size32(edge_types); i++)
      {
        auto type    = edge_types[i];
        auto num_cps = num_control_points(type);
        auto last_cp = first_cp + (num_cps - 1);

        switch (type)
        {
          case ContourEdgeType::Line:
          {
          }
          break;
          case ContourEdgeType::Arc:
          {
            CHECK(false, "Unsupported");
          }
          break;
          case ContourEdgeType::Bezier:
          {
            u32 const cps[] = {first_cp, first_cp + 1, first_cp + 2};
            indices.extend(cps).unwrap();
          }
          break;
          case ContourEdgeType::CubicBezier:
          {
            CHECK(false, "Unsupported");
          }
          break;

          default:
          {
          }
          break;
        }

        first_cp = last_cp;
      }
    }

    auto                       num_inside_bezier_indices = size32(indices);
    Vec<shader::BezierRegions> regions{frame_arena_};

    regions
      .extend(span({shader::BezierRegions::All, shader::BezierRegions::Inside}))
      .unwrap();

    Vec<u32> region_index_counts{frame_arena_};
    region_index_counts
      .extend(span({num_straight_indices, num_inside_bezier_indices}))
      .unwrap();

    auto item = BezierStencilEncoder::Item{
      .stencil             = depth_stencil_textures_[stencil],
      .write_mask          = write_mask,
      .scissor             = clip_to_scissor(clip),
      .viewport            = viewport_,
      .fill_rule           = fill_rule,
      .invert              = invert,
      .world_to_ndc        = static_cast<f32x4x4>(world_to_ndc_),
      .transform           = transform,
      .vertices            = vertices,
      .indices             = indices,
      .regions             = regions,
      .region_index_counts = region_index_counts};

    BezierStencilEncoder encoder{frame_arena_, item};

    encoder.pass(frame_graph_, passes_);
  }
  else
  {
    // [ ] can't be batched due to using all stencil bits?

    Vec<f32x2> vertices{frame_arena_};
    Vec<u32>   indices{frame_arena_};

    u32 first_cp = 0;

    CHECK(subdivision_counts.size() == edge_types.size(), "");

    for (u32 i = 0; i < size32(edge_types); i++)
    {
      auto type             = edge_types[i];
      auto num_cps          = num_control_points(type);
      auto last_cp          = first_cp + (num_cps - 1);
      auto num_subdivisions = subdivision_counts[i];

      switch (type)
      {
        case ContourEdgeType::Arc:
        {
          auto cp0    = control_points[first_cp];
          auto center = control_points[first_cp + 1];
          auto cp2    = control_points[first_cp + 2];

          auto v0    = cp0 - center;
          auto v1    = cp2 - center;
          auto radii = v0.length();

          auto start = std::atan2(v0.y(), v0.x());
          auto end   = std::atan2(v1.y(), v1.x());

          auto turn = end - start;

          // turn angle (CCW)
          if (turn < 0)
          {
            turn += 2.0F * PI;    // normalizew to [0, 2PI]
          }

          path::arc(vertices, f32x2::splat(radii), center, start, turn,
                    num_subdivisions);
        }
        break;
        case ContourEdgeType::Bezier:
        {
          path::cubic_bezier(vertices, control_points[first_cp],
                             control_points[first_cp + 1],
                             control_points[first_cp + 2],
                             control_points[first_cp + 3], num_subdivisions);
        }
        break;
        case ContourEdgeType::CubicBezier:
        {
          path::bezier(vertices, control_points[first_cp],
                       control_points[first_cp + 1],
                       control_points[first_cp + 2], num_subdivisions);
        }
        break;
        case ContourEdgeType::Line:
        {
          vertices
            .extend(
              span({control_points[first_cp], control_points[first_cp + 1]}))
            .unwrap();
        }
        break;
        default:
          break;
      }

      first_cp = last_cp;
    }

    path::triangulate_convex(indices, 0, size32(vertices));

    auto index_counts = span({size32(indices)});

    auto item = FillStencilEncoder::Item{
      .stencil      = depth_stencil_textures_[stencil],
      .write_mask   = write_mask,
      .scissor      = clip_to_scissor(clip),
      .viewport     = viewport_,
      .fill_rule    = fill_rule,
      .invert       = invert,
      .world_to_ndc = static_cast<f32x4x4>(world_to_ndc_),
      .transform    = transform,
      .vertices     = vertices,
      .indices      = indices,
      .index_counts = index_counts};

    FillStencilEncoder encoder{frame_arena_, item};

    // [ ] clear current encoder
    encoder.pass(frame_graph_, passes_);
  }

  return *this;
}

ICanvas & ICanvas::text(Span<TextLayer const> layers,
                        Span<ShapeInfo const> shapes,
                        Span<TextRenderInfo const>, Span<usize const> sorted)
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
