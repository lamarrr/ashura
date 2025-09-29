/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/pipeline_system.h"
#include "ashura/engine/pipelines/blur.h"
#include "ashura/engine/pipelines/sdf.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"

namespace ash
{

static constexpr u32 num_control_points(ContourEdgeType edge)
{
  switch (edge)
  {
    case ContourEdgeType::Line:
      return 2;
    case ContourEdgeType::Arc:
    case ContourEdgeType::QuadraticBezier:
      return 3;
    case ContourEdgeType::CubicBezier:
      return 4;
    default:
      ASH_UNREACHABLE;
  }
}

static constexpr u32 num_quadratic_beziers(ContourEdgeType edge)
{
  switch (edge)
  {
    case ContourEdgeType::Line:
      return 0;
    case ContourEdgeType::QuadraticBezier:
      return 1;
    default:
      ASH_UNREACHABLE;
  }
}

// Don't allow linearized segments to be off by more than 1/4th of a pixel from
// the true curve. This value should be scaled by the max basis of the
// X and Y directions.
f32 path::cubic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                             f32x2 p3, f32 precision)
{
  auto k = scale_factor * .75F * precision;
  auto a = (p0 - p1 * 2 + p2).abs();
  auto b = (p1 - p2 * 2 + p3).abs();
  return sqrt(k * a.max(b).length());
}

f32 path::quadratic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                                 f32 precision)
{
  f32 k = scale_factor * .25F * precision;
  return sqrt(k * (p0 - p1 * 2 + p2).length());
}

// Returns Wang's formula specialized for a conic curve.
//
// This is not actually due to Wang, but is an analogue from:
//   (Theorem 3, corollary 1):
//   J. Zheng, T. Sederberg. "Estimating Tessellation Parameter Intervals for
//   Rational Curves and Surfaces." ACM Transactions on Graphics 19(1). 2000.
f32 path::conic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                             f32 w, f32 precision)
{
  // Compute center of bounding box in projected space
  auto C = 0.5F * p0.min(p1).min(p2) + p0.max(p1).max(p2);

  // Translate by -C. This improves translation-invariance of the formula,
  // see Sec. 3.3 of cited paper
  p0 -= C;
  p1 -= C;
  p2 -= C;

  // Compute max length
  auto max_len = sqrt(max(p0.dot(p0), max(p1.dot(p1), p2.dot(p2))));

  // Compute forward differences
  auto dp = -2 * w * p1 + p0 + p2;
  auto dw = abs(-2 * w + 2);

  // Compute numerator and denominator for parametric step size of
  // linearization. Here, the epsilon referenced from the cited paper
  // is 1/precision.
  auto k          = scale_factor * precision;
  auto rp_minus_1 = max(0.0F, max_len * k - 1);
  auto numer      = sqrt(dp.dot(dp)) * k + rp_minus_1 * dw;
  auto denom      = 4 * min(w, 1.0F);

  // Number of segments = sqrt(numer / denom).
  // This assumes parametric interval of curve being linearized is
  //   [t0,t1] = [0, 1].
  // If not, the number of segments is (tmax - tmin) / sqrt(denom / numer).
  return sqrt(numer / denom);
}

void path::rect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center)
{
  extent               = extent.max(0);
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

void path::line(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1)
{
  auto const segments = size32(vtx);

  if (segments > 1)
  {
    f32 const step = 1 / (segments - 1);

    for (u32 i = 0; i < segments; i++)
    {
      vtx[i] = lerp(cp0, cp1, f32x2::splat(step * i));
    }
  }
  if (segments == 2)
  {
    vtx[0] = cp0;
    vtx[1] = cp1;
  }
  else if (segments == 1)
  {
    vtx[0] = cp1;
  }
  else if (segments == 0)
  {
  }
}

void path::arc(Span<f32x2> vtx, f32x2 radii, f32x2 center, f32 start, f32 turn)
{
  radii               = radii.max(0);
  auto const segments = size32(vtx);

  if (segments > 1)
  {
    f32 const step = turn / (segments - 1);

    for (u32 i = 0; i < segments; i++)
    {
      vtx[i] = (rotor(start + i * step) - 0.5F) * 2 * radii + center;
    }
  }
  else if (segments == 1)
  {
    vtx[0] = (rotor(start + turn) - 0.5F) * 2 * radii + center;
  }
  else if (segments == 0)
  {
  }
}

void path::bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2)
{
  auto const segments = size32(vtx);

  if (segments > 1)
  {
    f32 const step = 1.0F / (segments - 1);

    for (u32 i = 0; i < segments; i++)
    {
      vtx[i] = f32x2{ash::bezier(cp0.x(), cp1.x(), cp2.x(), step * i),
                     ash::bezier(cp0.y(), cp1.y(), cp2.y(), step * i)};
    }
  }
  else if (segments == 1)
  {
    vtx[0] = cp2;
  }
  else if (segments == 0)
  {
  }
}

void path::cubic_bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2,
                        f32x2 cp3)
{
  auto const segments = size32(vtx);

  if (segments > 1)
  {
    f32 const step = 1.0F / (segments - 1);

    for (u32 i = 0; i < segments; i++)
    {
      vtx[i] =
        f32x2{ash::cubic_bezier(cp0.x(), cp1.x(), cp2.x(), cp3.x(), step * i),
              ash::cubic_bezier(cp0.y(), cp1.y(), cp2.y(), cp3.y(), step * i)};
    }
  }
  else if (segments == 1)
  {
    vtx[0] = cp3;
  }
  else if (segments == 0)
  {
  }
}

void path::catmull_rom(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2,
                       f32x2 cp3)
{
  auto const segments = size32(vtx);

  if (segments > 1)
  {
    f32 const step = 1.0F / (segments - 1);

    for (u32 i = 0; i < segments; i++)
    {
      vtx[i] =
        f32x2{ash::cubic_bezier(cp0.x(), cp1.x(), cp2.x(), cp3.x(), step * i),
              ash::cubic_bezier(cp0.y(), cp1.y(), cp2.y(), cp3.y(), step * i)};
    }
  }
  else if (segments == 1)
  {
    vtx[0] = cp3;
  }
  else if (segments == 0)
  {
  }
}

void path::circle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  extent           = extent.max(0);
  auto const first = size32(vtx);

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = (rotor(i * step) - 0.5F) * extent + center;
  }
}

void path::squircle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center,
                    f32 elasticity, u32 segments)
{
  auto const n = segments << 2;

  elasticity = clamp(elasticity * 0.5F, 0.0F, 0.5F);

  extent = extent.max(0);

  auto offset = size32(vtx);
  vtx.extend_uninit(n).unwrap();

  path::cubic_bezier(
    vtx.view().slice(offset, segments), center + extent * f32x2{0, -0.5F},
    center + extent * f32x2{elasticity, -0.5F},
    center + extent * f32x2{0.5F, -0.5F}, center + extent * f32x2{0.5F, 0});

  offset += segments;

  path::cubic_bezier(
    vtx.view().slice(offset, segments), center + extent * f32x2{0.5F, 0},
    center + extent * f32x2{0.5F, elasticity},
    center + extent * f32x2{0.5F, 0.5F}, center + extent * f32x2{0, 0.5F});

  offset += segments;

  path::cubic_bezier(
    vtx.view().slice(offset, segments), center + extent * f32x2{0, 0.5F},
    center + extent * f32x2{-elasticity, 0.5F},
    center + extent * f32x2{-0.5F, 0.5F}, center + extent * f32x2{-0.5F, 0});

  offset += segments;

  path::cubic_bezier(
    vtx.view().slice(offset, segments), center + extent * f32x2{-0.5F, 0},
    center + extent * f32x2{-0.5F, -elasticity},
    center + extent * f32x2{-0.5F, -0.5F}, center + extent * f32x2{0, -0.5F});
}

void path::rrect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 radii,
                 u32 segments)
{
  auto const n = (segments << 2) + 8;

  extent = extent.max(0);

  auto max_radius = extent.min();
  radii           = radii.clamp(0.0F, max_radius);
  auto y_max      = max_radius - radii.x();
  radii.y()       = min(radii.y(), y_max);
  auto z_max      = min(y_max, max_radius - radii.y());
  radii.z()       = min(radii.z(), z_max);
  auto w_max      = min(z_max, max_radius - radii.z());
  radii.w()       = min(radii.w(), w_max);

  f32 const  step  = (segments == 0) ? 0.0F : ((PI * 0.5F) / segments);
  auto const first = size32(vtx);

  vtx.extend_uninit(n).unwrap();

  u32 i = 0;

  vtx[first + i++] = center + extent * 0.5F - f32x2{0, radii.z()};

  for (u32 s = 0; s < segments; s++)
  {
    vtx[first + i++] = center + extent * 0.5F - f32x2{0, radii.z()} +
                       radii.z() * rotor(s * step);
  }

  vtx[first + i++] = center + extent * 0.5F - f32x2{radii.z(), 0};

  vtx[first + i++] = center + extent * f32x2{-0.5F, 0.5F} + f32x2{radii.w(), 0};

  for (u32 s = 0; s < segments; s++)
  {
    vtx[first + i++] = center + extent * f32x2{-0.5F, 0.5F} +
                       f32x2{radii.w(), 0} +
                       radii.w() * rotor(PI * 0.5F + s * step);
  }

  vtx[first + i++] = center + extent * f32x2{-0.5F, 0.5F} - f32x2{0, radii.w()};

  vtx[first + i++] =
    center + extent * f32x2{-0.5F, -0.5F} + f32x2{0, radii.x()};

  for (u32 s = 0; s < segments; s++)
  {
    vtx[first + i++] = center + extent * f32x2{-0.5F, -0.5F} +
                       f32x2{0, radii.x()} + radii.x() * rotor(PI + s * step);
  }

  vtx[first + i++] =
    center + extent * f32x2{-0.5F, -0.5F} + f32x2{radii.x(), 0};

  vtx[first + i++] = center + extent * f32x2{0.5F, -0.5F} - f32x2{radii.y(), 0};

  for (u32 s = 0; s < segments; s++)
  {
    vtx[first + i++] = center + extent * f32x2{0.5F, -0.5F} -
                       f32x2{radii.y(), 0} +
                       radii.y() * rotor(PI * 1.5F + s * step);
  }

  vtx[first + i++] = center + extent * f32x2{0.5F, -0.5F} + f32x2{0, radii.y()};
}

template <typename I>
void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vertices,
                        Vec<I> & indices, f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  I const first_vtx    = static_cast<I>(vertices.size());
  I const first_idx    = static_cast<I>(indices.size());
  I const num_points   = static_cast<I>(points.size());
  I const num_vertices = (num_points - 1) * 4;
  I const num_indices  = (num_points - 1) * 6 + (num_points - 2) * 6;
  thickness            = max(thickness, 0.0F);

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
  I const first_idx     = static_cast<I>(indices.size());
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
  I const first_index = static_cast<I>(idx.size());

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

u32 ICanvas::num_image_slots() const
{
  return size32(image_slots_);
}

u32 ICanvas::color() const
{
  return color_;
}

u32 ICanvas::num_stencil_bits() const
{
  return 8;
}

Option<u32> ICanvas::depth_stencil() const
{
  return depth_stencil_;
}

Option<PipelineStencil> ICanvas::stencil_op() const
{
  return stencil_op_;
}

gpu::Viewport ICanvas::viewport() const
{
  return viewport_;
}

f32x2 ICanvas::extent() const
{
  return extent_;
}

u32x2 ICanvas::framebuffer_extent() const
{
  return framebuffer_extent_;
}

f32x2 ICanvas::framebuffer_uv_base() const
{
  return framebuffer_uv_base_;
}

f32 ICanvas::aspect_ratio() const
{
  return aspect_ratio_;
}

f32 ICanvas::virtual_scale() const
{
  return virtual_scale_;
}

affinef32x4 ICanvas::world_to_ndc() const
{
  return world_to_ndc_;
}

affinef32x4 ICanvas::ndc_to_viewport() const
{
  return ndc_to_viewport_;
}

affinef32x4 ICanvas::viewport_to_fb() const
{
  return viewport_to_fb_;
}

affinef32x4 ICanvas::world_to_fb() const
{
  return world_to_fb_;
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

TextRenderer ICanvas::default_text_renderer()
{
  return TextRenderer{
    this, [](Canvas c, TextBlockRenderInfo const & text) { c->text(text); }};
}

void ICanvas::begin(gpu::Viewport const & viewport, f32x2 extent,
                    u32x2 framebuffer_extent)
{
  static_assert(DEFAULT_NUM_IMAGE_SLOTS >= 3, "");
  CHECK(state_ == CanvasState::Reset, "");

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

  // reserve the first image slot for the color target
  image_slots_.view().set_bit(0);

  state_ = CanvasState::Recording;
}

void ICanvas::end()
{
  CHECK(state_ == CanvasState::Recording, "");

  state_ = CanvasState::Recorded;
}

void ICanvas::reset()
{
  CHECK(state_ == CanvasState::Executed || state_ == CanvasState::Reset, "");
  color_         = 0;
  depth_stencil_ = none;
  image_slots_.clear();
  image_slots_.resize(DEFAULT_NUM_IMAGE_SLOTS).unwrap();
  stencil_op_          = none;
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
  encoders_.shrink_clear().unwrap();
  encoder_arena_.reclaim();
  tmp_arena_.reclaim();

  state_ = CanvasState::Reset;
}

void ICanvas::execute(GpuFramePlan plan)
{
  CHECK(state_ == CanvasState::Recorded, "");

  plan->reserve_scratch_images(num_image_slots());

  for (auto & enc : encoders_)
  {
    enc->operator()(plan);
  }

  state_ = CanvasState::Executed;
}

void ICanvas::reserve_images(u32 num_images)
{
  CHECK(state_ == CanvasState::Reset, "");

  image_slots_.resize(max(num_images, size32(image_slots_))).unwrap();
}

u32 ICanvas::allocate_image()
{
  auto index = image_slots_.view().find_clear_bit();
  CHECK(index < num_image_slots(), "no more image slots available");
  image_slots_.view().set_bit(index);
  return index;
}

void ICanvas::deallocate_image(u32 index)
{
  CHECK(index < num_image_slots(), "");
  image_slots_.view().clear_bit(index);
}

void ICanvas::clear_color(u32 image, gpu::Color value)
{
  encode_pass_([image, value](GpuFramePlan plan) {
    plan->add_pass([image, value](GpuFrame frame, gpu::CommandEncoder enc) {
      auto images = frame->get_scratch_images();
      auto color  = images[image].color.image;

      enc->clear_color_image(
        color, value,
        span({
          gpu::ImageSubresourceRange{.aspects      = gpu::ImageAspects::Color,
                                     .mip_levels   = Slice32::all(),
                                     .array_layers = Slice32::all()}
      }));
    });
  });
}

void ICanvas::set_color(u32 color)
{
  CHECK(color < num_image_slots(), "");
  color_ = color;
}

void ICanvas::clear_depth_stencil(u32 image, gpu::DepthStencil value)
{
  encode_pass_([image, value](GpuFramePlan plan) {
    plan->add_pass([image, value](GpuFrame frame, gpu::CommandEncoder enc) {
      auto images = frame->get_scratch_images();
      auto ds     = images[image].depth_stencil.image;

      enc->clear_depth_stencil_image(
        ds, value,
        span({
          gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Depth |
                                                gpu::ImageAspects::Stencil,
                                     .mip_levels   = Slice32::all(),
                                     .array_layers = Slice32::all()}
      }));
    });
  });
}

void ICanvas::set_depth_stencil(Option<u32> depth_stencil)
{
  depth_stencil.match([&](auto d) { CHECK(d < num_image_slots(), ""); });
  depth_stencil_ = depth_stencil;
}

void ICanvas::set_stencil_op(Option<PipelineStencil> stencil_op)
{
  stencil_op_ = stencil_op;
}

void ICanvas::set_clip(CRect const & area)
{
  clip_ = area;
}

constexpr f32x4x4 object_to_world(f32x4x4 const & transform, CRect const & area)
{
  return transform * translate3d(area.center.append(0)) *
         scale3d(area.extent.append(1));
}

#define ENCODER_NEW(EncType)                                             \
  auto enc =                                                             \
    dyn<EncType##Encoder>(inplace, encoder_arena_, encoder_arena_, item) \
      .unwrap();                                                         \
  encoders_.push(cast<CanvasEncoder>(std::move(enc))).unwrap();

#define ENCODER_PUSH(EncType)                                       \
  if (encoders_.is_empty() ||                                       \
      encoders_.last()->type() != CanvasEncoderType::EncType ||     \
      !((EncType##Encoder *) (encoders_.last().get()))->push(item)) \
  {                                                                 \
    ENCODER_NEW(EncType)                                            \
  }

void ICanvas::encode_(SdfEncoder::Item const & item)
{
  ENCODER_PUSH(Sdf);
}

void ICanvas::encode_(TriangleFillEncoder::Item const & item)
{
  ENCODER_PUSH(TriangleFill);
}

void ICanvas::encode_(QuadEncoder::Item const & item)
{
  ENCODER_PUSH(Quad);
}

void ICanvas::encode_(FillStencilEncoder::Item const & item)
{
  ENCODER_PUSH(FillStencil);
}

void ICanvas::encode_(BezierStencilEncoder::Item const & item)
{
  ENCODER_PUSH(BezierStencil);
}

void ICanvas::encode_(PbrEncoder::Item const & item)
{
  ENCODER_NEW(Pbr);
}

void ICanvas::render_(TextureSet const & texture_set, Shape const & shape,
                      SdfShapeType type)
{
  shader::FlatSdfItem item{
    .world_transform  = object_to_world(shape.world_transform, shape.bbox()),
    .uv_transform     = shape.uv_transform,
    .radii            = shape.radii,
    .half_bbox_extent = 0.5F * shape.bbox_extent,
    .half_extent      = 0.5F * shape.area.extent,
    .feather          = shape.feather,
    .shade_type       = shape.shade,
    .type             = type,
    .material =
      shader::sdf::FlatMaterial{.top             = shape.tint.top(),
                                .bottom          = shape.tint.bottom(),
                                .gradient_rotor  = shape.tint.rotor(),
                                .gradient_center = shape.tint.center(),
                                .sampler         = shape.sampler,
                                .texture         = shape.map,
                                .sdf_sampler     = shape.sdf_sampler,
                                .sdf_map         = shape.sdf_map}
  };

  return encode_(SdfEncoder::Item{.color         = color_,
                                  .depth_stencil = depth_stencil_,
                                  .stencil_op    = stencil_op_,
                                  .scissor       = clip_to_scissor(clip_),
                                  .viewport      = viewport_,
                                  .texture_set   = texture_set,
                                  .world_to_ndc  = world_to_ndc_.to_mat(),
                                  .shape         = as_u8_span(item),
                                  .variant       = SdfPipeline::FLAT});
}

void ICanvas::render_noise_(TextureSet const & texture_set, Shape const & shape,
                            SdfShapeType type)
{
  shader::NoiseSdfItem item{
    .world_transform  = object_to_world(shape.world_transform, shape.bbox()),
    .uv_transform     = shape.uv_transform,
    .radii            = shape.radii,
    .half_bbox_extent = 0.5F * shape.bbox_extent,
    .half_extent      = 0.5F * shape.area.extent,
    .feather          = shape.feather,
    .shade_type       = shape.shade,
    .type             = type,
    .material         = shader::sdf::NoiseMaterial{.intensity   = shape.tint.top_,
                                                   .sdf_sampler = shape.sdf_sampler,
                                                   .sdf_map     = shape.sdf_map}
  };

  return encode_(SdfEncoder::Item{.color         = color_,
                                  .depth_stencil = depth_stencil_,
                                  .stencil_op    = stencil_op_,
                                  .scissor       = clip_to_scissor(clip_),
                                  .viewport      = viewport_,
                                  .texture_set   = texture_set,
                                  .world_to_ndc  = world_to_ndc_.to_mat(),
                                  .shape         = as_u8_span(item),
                                  .variant       = SdfPipeline::NOISE});
}

void ICanvas::render_(TextureSet const &       texture_set,
                      MeshGradientInfo const & shape)
{
  shader::MeshGradientSdfItem item{
    .world_transform  = object_to_world(shape.world_transform, shape.bbox()),
    .uv_transform     = shape.uv_transform,
    .radii            = shape.radii,
    .half_bbox_extent = 0.5F * shape.bbox_extent,
    .half_extent      = 0.5F * shape.area.extent,
    .feather          = shape.feather,
    .shade_type       = shape.shade,
    .type             = shape.shape,
    .material         = shader::sdf::MeshGradientMaterial{
                                                          .colors       = {shape.colors.tl, shape.colors.tr, shape.colors.br,
                               shape.colors.bl},
                                                          .min          = shape.min,
                                                          .max          = shape.max,
                                                          .aspect_ratio = shape.bbox_extent.x() / shape.bbox_extent.y(),
                                                          .frequency    = shape.frequency,
                                                          .amplitude    = shape.amplitude,
                                                          .time         = shape.time,
                                                          .sdf_sampler  = shape.sdf_sampler,
                                                          .sdf_map      = shape.sdf_map}
  };

  return encode_(SdfEncoder::Item{.color         = color_,
                                  .depth_stencil = depth_stencil_,
                                  .stencil_op    = stencil_op_,
                                  .scissor       = clip_to_scissor(clip_),
                                  .viewport      = viewport_,
                                  .texture_set   = texture_set,
                                  .world_to_ndc  = world_to_ndc_.to_mat(),
                                  .shape         = as_u8_span(item),
                                  .variant       = SdfPipeline::MESH_GRADIENT});
}

void ICanvas::render_(TextureSet const &      texture_set,
                      TriangleSetInfo const & shape, bool indexed)
{
  if (shape.vertices.is_empty() || (!indexed && shape.vertices.size() < 3) ||
      (indexed && shape.indices.size() < 3))
  {
    return;
  }

  CHECK(!(indexed && shape.indices.is_empty()), "");

  Span<u32 const> indices;

  Vec<u32> indices_tmp{tmp_arena_};

  if (indexed)
  {
    path::triangles(0, size32(shape.vertices), indices_tmp);
    indices = indices_tmp;
  }
  else
  {
    indices = shape.indices;
  }

  shader::FlatTriangleSetItem item{
    .world_transform = shape.world_transform,
    .uv_transform    = shape.uv_transform,
    .material = shader::triangle_fill::TextureMaterial{.sampler = shape.sampler,
                                                       .texture = shape.map}
  };

  return encode_(
    TriangleFillEncoder::Item{.color         = color_,
                              .depth_stencil = depth_stencil_,
                              .stencil_op    = stencil_op_,
                              .scissor       = clip_to_scissor(clip_),
                              .viewport      = viewport_,
                              .cull_mode     = shape.cull_mode,
                              .texture_set   = texture_set,
                              .world_to_ndc  = world_to_ndc_.to_mat(),
                              .set           = as_u8_span(item),
                              .vertices      = shape.vertices.as_u8(),
                              .indices       = indices.as_u8(),
                              .variant       = PipelineVariantId::Base});
}

void ICanvas::render_(TextureSet const & texture_set, QuadInfo const & shape)
{
  return encode_(QuadEncoder::Item{.color         = color_,
                                   .depth_stencil = depth_stencil_,
                                   .stencil_op    = stencil_op_,
                                   .scissor       = clip_to_scissor(clip_),
                                   .viewport      = viewport_,
                                   .texture_set   = texture_set,
                                   .world_to_ndc  = world_to_ndc_.to_mat(),
                                   .quad          = shape.quad.as_u8(),
                                   .variant       = shape.variant});
}

void ICanvas::render_blur_(Shape const & shape_)
{
  auto info       = shape_;
  auto max_radius = 0.5F * info.area.extent.min();
  info.radii      = info.radii.min(max_radius);
  auto world_xfm  = object_to_world(info.world_transform, info.area);
  auto fb_xfm     = world_to_fb_ * world_xfm;
  auto tl         = transform(fb_xfm, f32x3{-0.5, -0.5, 0.0}).xy();
  auto tr         = transform(fb_xfm, f32x3{0.5, -0.5, 0.0}).xy();
  auto bl         = transform(fb_xfm, f32x3{-0.5, 0.5, 0.0}).xy();
  auto br         = transform(fb_xfm, f32x3{0.5, 0.5, 0.0}).xy();
  auto bounding   = CRect::bounding(tl, tr, bl, br);

  auto area =
    RectU::range(
      bounding.begin().clamp(f32x2::splat(0), MAX_CLIP.extent).to<u32>(),
      bounding.end().clamp(f32x2::splat(0), MAX_CLIP.extent).to<u32>())
      .clamp_to_extent(framebuffer_extent_);

  auto spread_radius =
    f32x2::splat(clamp(info.feather * virtual_scale_, 0.0F, MAX_CLIP_DISTANCE))
      .to<u32>();

  if (!area.is_visible() || spread_radius.any_zero())
  {
    return;
  }

  static constexpr u32 MAX_SPREAD_RADIUS = 16;
  static constexpr u32 MAX_PASSES        = 16;

  spread_radius =
    spread_radius.clamp(u32x2::splat(1U), u32x2::splat(MAX_SPREAD_RADIUS));

  auto major_spread_radius = spread_radius.max();
  auto padding             = u32x2::splat(max(major_spread_radius + 8, 16U));
  auto padded_begin        = area.begin().sat_sub(padding);
  auto padded_end          = area.end().sat_add(padding);
  auto padded_area =
    RectU::range(padded_begin, padded_end).clamp_to_extent(framebuffer_extent_);
  auto num_passes = clamp(major_spread_radius, 1U, MAX_PASSES);

  if (!padded_area.is_visible() || num_passes == 0)
  {
    return;
  }

  auto scratch_0 = allocate_image();
  auto scratch_1 = allocate_image();

  encode_pass_([info, color = color_, depth_stencil = depth_stencil_, scratch_0,
                scratch_1, stencil_op = stencil_op_, padded_area, num_passes,
                spread_radius, area, viewport = this->viewport_,
                scissor            = clip_to_scissor(clip_),
                world_to_ndc       = this->world_to_ndc_,
                framebuffer_extent = this->framebuffer_extent_,
                &tmp_arena         = this->tmp_arena_](GpuFramePlan plan) {
    plan->add_pass([&](GpuFrame frame, gpu::CommandEncoder enc) {
      auto color_image     = frame->get_scratch_image(color).color;
      auto scratch_image_0 = frame->get_scratch_image(scratch_0).color;
      auto scratch_image_1 = frame->get_scratch_image(scratch_1).color;

      // copy to scratch texture 0 & 1. This is to prevent texture-spilling when ping-ponging between the two textures
      enc->blit_image(
        color_image.image, scratch_image_0.image,
        span({
          gpu::ImageBlit{.src_layers{.aspects      = gpu::ImageAspects::Color,
                                     .mip_level    = 0,
                                     .array_layers = Slice32::all()},
                         .src_area = as_boxu(padded_area),
                         .dst_layers{.aspects      = gpu::ImageAspects::Color,
                                     .mip_level    = 0,
                                     .array_layers = Slice32::all()},
                         .dst_area = as_boxu(padded_area)}
      }),
        gpu::Filter::Linear);

      // NOTE: we can avoid a second copy operation if the padded area is same as the blur area,
      // which will happen if we are blurring the entire texture.
      enc->blit_image(
        color_image.image, scratch_image_1.image,
        span({
          gpu::ImageBlit{.src_layers{.aspects      = gpu::ImageAspects::Color,
                                     .mip_level    = 0,
                                     .array_layers = Slice32::all()},
                         .src_area = as_boxu(padded_area),
                         .dst_layers{.aspects      = gpu::ImageAspects::Color,
                                     .mip_level    = 0,
                                     .array_layers = Slice32::all()},
                         .dst_area = as_boxu(padded_area)}
      }),
        gpu::Filter::Linear);
    });

    u32 images[2] = {scratch_0, scratch_1};

    usize src = 0;
    usize dst = 1;

    // downsample pass
    for (usize i = 1; i <= num_passes; i++)
    {
      src            = (src + 1) & 1;
      dst            = (src + 1) & 1;
      auto src_image = images[src];
      auto dst_image = images[dst];
      auto spread    = u32x2::splat(i).clamp(u32x2::splat(1U), spread_radius);
      auto base      = 1 / framebuffer_extent.to<f32>();
      auto blur =
        shader::BlurItem{.uv0     = area.begin().to<f32>() * base,
                         .uv1     = area.end().to<f32>() * base,
                         .radius  = spread.to<f32>() * base,
                         .sampler = SamplerIndex::LinearEdgeClampBlackFloat,
                         .tex     = ColorImage::sampled_texture_index};

      auto blur_id = plan->push_gpu(span({blur}));

      plan->add_pass([blur_id, src = src_image, dst = dst_image, padded_area,
                      viewport](GpuFrame frame, gpu::CommandEncoder enc) {
        auto blur      = frame->get(blur_id);
        auto src_image = frame->get_scratch_image(src);
        auto dst_image = frame->get_scratch_image(dst);

        sys.pipeline->blur().encode(
          enc, BlurPipelineParams{
                 .framebuffer = Framebuffer{.color         = dst_image.color,
                                            .color_msaa    = {},
                                            .depth_stencil = {}},
                 .stencil     = none,
                 .scissor     = padded_area,
                 .viewport    = viewport,
                 .samplers    = sys.gpu->samplers(),
                 .textures    = src_image.color.sampled_texture,
                 .blurs       = blur,
                 .instances   = {0, 1},
                 .upsample    = false
        });
      });
    }

    // upsample pass
    for (usize i = num_passes; i != 0; i--)
    {
      src            = (src + 1) & 1;
      dst            = (src + 1) & 1;
      auto src_image = images[src];
      auto dst_image = images[dst];
      auto spread    = u32x2::splat(i).clamp(u32x2::splat(1U), spread_radius);
      auto base      = 1 / framebuffer_extent.to<f32>();
      auto blur =
        shader::BlurItem{.uv0     = area.begin().to<f32>() * base,
                         .uv1     = area.end().to<f32>() * base,
                         .radius  = spread.to<f32>() * base,
                         .sampler = SamplerIndex::LinearEdgeClampBlackFloat,
                         .tex     = ColorImage::sampled_texture_index};

      auto blur_id = plan->push_gpu(span({blur}));

      plan->add_pass([blur_id, src = src_image, dst = dst_image, padded_area,
                      viewport](GpuFrame frame, gpu::CommandEncoder enc) {
        auto blur      = frame->get(blur_id);
        auto src_image = frame->get_scratch_image(src);
        auto dst_image = frame->get_scratch_image(dst);

        sys.pipeline->blur().encode(
          enc, BlurPipelineParams{
                 .framebuffer = Framebuffer{.color         = dst_image.color,
                                            .color_msaa    = {},
                                            .depth_stencil = {}},
                 .stencil     = none,
                 .scissor     = padded_area,
                 .viewport    = viewport,
                 .samplers    = sys.gpu->samplers(),
                 .textures    = src_image.color.sampled_texture,
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
      auto src_image = images[dst];
      auto base      = 1 / framebuffer_extent.to<f32>();

      auto sdf_shape =
        shader::sdf::Shape{.radii            = info.radii,
                           .half_bbox_extent = 0.5F * info.area.extent,
                           .half_extent      = 0.5F * info.area.extent,
                           .feather          = 0,
                           .shade_type       = ShadeType::Flood,
                           .type             = shader::sdf::ShapeType::RRect};

      auto sdf_material = shader::sdf::FlatMaterial{
        .tint =
          shader::quad::FlatMaterial{
                                     .top             = info.tint.top(),
                                     .bottom          = info.tint.bottom(),
                                     .gradient_rotor  = info.tint.rotor(),
                                     .uv0             = area.begin().to<f32>() * base,
                                     .uv1             = area.end().to<f32>() * base,
                                     .gradient_center = info.tint.center(),
                                     .sampler         = SamplerIndex::LinearEdgeClampBlackFloat,
                                     .texture         = ColorImage::sampled_texture_index},
        .sampler = SamplerIndex::LinearEdgeClampWhiteFloat,
        .map     = TextureIndex::White
      };

      auto item = SdfEncoder::Item{
        .color         = src_image,
        .depth_stencil = depth_stencil,
        .stencil_op    = stencil_op,
        .scissor       = scissor,
        .viewport      = viewport,
        .texture_set   = ScratchTexture{.image = src_image,
                                        .type  = ScratchTexureType::SampledColor},
        .world_to_ndc  = world_to_ndc.to_mat(),
        .shape         = as_u8_span(sdf_shape),
        .transform     = info.transform,
        .material      = as_u8_span(sdf_material),
        .variant       = PipelineVariantId::Base
      };

      auto enc = SdfEncoder{tmp_arena, item};

      enc(plan);
    }
  });

  deallocate_image(scratch0);
  deallocate_image(scratch1);
}

void ICanvas::render_stencil_(u32 depth_stencil, u32 write_mask,
                              ContourInfo const & shape, bool invert)
{
  if (shape.contour_types.is_empty() || shape.control_points.is_empty() ||
      shape.segments.is_empty())
  {
    return;
  }

  switch (shape.method)
  {
    case ContourMethod::StencilThenCover:
    {
      // tesselate the bezier curves into line segments and fill the resulting polygon using stencil-then-cover

      auto const num_vertices = reduce(shape.segments, 0U, add);

      Vec<f32x2> vertices{tmp_arena_};
      vertices.extend_uninit(num_vertices).unwrap();

      u32 first_cp = 0;

      CHECK(shape.segments.size() == shape.contour_types.size(), "");

      for (auto [type, num_subdivisions] :
           zip(shape.contour_types, shape.segments))
      {
        switch (type)
        {
          case ContourEdgeType::Arc:
          {
            auto cp0    = shape.control_points[first_cp];
            auto center = shape.control_points[first_cp + 1];
            auto cp2    = shape.control_points[first_cp + 2];

            auto v0    = cp0 - center;
            auto v1    = cp2 - center;
            auto radii = v0.length();

            auto start = std::atan2(v0.y(), v0.x());
            auto end   = std::atan2(v1.y(), v1.x());

            auto turn = end - start;

            // turn angle (CCW)
            if (turn < 0)
            {
              turn += 2.0F * PI;    // normalize to [0, 2PI]
            }

            path::arc(vertices.view().slice(first_cp, num_subdivisions),
                      f32x2::splat(radii), center, start, turn);

            first_cp += 2;
          }
          break;
          case ContourEdgeType::QuadraticBezier:
          {
            path::bezier(vertices.view().slice(first_cp, num_subdivisions),
                         shape.control_points[first_cp],
                         shape.control_points[first_cp + 1],
                         shape.control_points[first_cp + 2]);

            first_cp += 3;
          }
          break;
          case ContourEdgeType::CubicBezier:
          {
            path::cubic_bezier(
              vertices.view().slice(first_cp, num_subdivisions),
              shape.control_points[first_cp],
              shape.control_points[first_cp + 1],
              shape.control_points[first_cp + 2],
              shape.control_points[first_cp + 3]);

            first_cp += 4;
          }
          break;
          case ContourEdgeType::Line:
          {
            path::line(vertices.view().slice(first_cp, num_subdivisions),
                       shape.control_points[first_cp],
                       shape.control_points[first_cp + 1]);

            first_cp += 2;
          }
          break;
          default:
            ASH_UNREACHABLE;
        }
      }

      Vec<u32> indices{tmp_arena_};
      path::triangulate_convex(indices, 0, size32(vertices));

      encode_(FillStencilEncoder::Item{.depth_stencil = depth_stencil,
                                       .write_mask    = write_mask,
                                       .scissor       = clip_to_scissor(clip_),
                                       .viewport      = viewport_,
                                       .fill_rule     = shape.fill_rule,
                                       .invert        = invert,
                                       .world_to_ndc  = world_to_ndc_.to_mat(),
                                       .world_transform = shape.world_transform,
                                       .vertices = shape.control_points.as_u8(),
                                       .indices  = indices.view().as_u8()});
    }
    break;
    case ContourMethod::BezierStencil:
    {
      Vec<u32> indices{tmp_arena_};

      // this is simlar to stencil-then-cover above but it performs bezier tesselation using a screen-space fragment shader
      //
      // we first render the straight edges using a fan triangulation of the control points
      //
      // we then render the bezier edges using a separate triangle for each bezier edge
      //
      for (auto type : shape.contour_types)
      {
        CHECK(type == ContourEdgeType::Line ||
                type == ContourEdgeType::QuadraticBezier,
              "Only line and quadratic bezier edges are supported using "
              "BezierStencil contour rendering");
      }

      // edge fills
      {
        u32 current_cp = 0;

        for (auto [i, type] : enumerate<u32>(shape.contour_types))
        {
          auto num_cps = num_control_points(type);

          if (i != 0)
          {
            indices
              .extend(span({0U, current_cp + 0, current_cp + (num_cps - 1)}))
              .unwrap();
          }

          current_cp += num_cps;
        }
      }

      auto n_fan_indices = size32(indices);

      // bezier fills
      {
        u32 current_cp = 0;

        for (auto type : shape.contour_types)
        {
          switch (type)
          {
            case ContourEdgeType::Line:
            {
              current_cp += 1;
            }
            break;
            case ContourEdgeType::QuadraticBezier:
            {
              indices
                .extend(span({current_cp + 0, current_cp + 1, current_cp + 2}))
                .unwrap();
            }
            break;
            default:
              ASH_UNREACHABLE;
          }
        }
      }

      auto n_quadratic_bezier_indices = size32(indices) - n_fan_indices;

      shader::BezierRegions const regions[] = {shader::BezierRegions::All,
                                               shader::BezierRegions::Inside};

      u32 const region_index_counts[] = {n_fan_indices,
                                         n_quadratic_bezier_indices};

      encode_(
        BezierStencilEncoder::Item{.depth_stencil   = depth_stencil,
                                   .write_mask      = write_mask,
                                   .scissor         = clip_to_scissor(clip_),
                                   .viewport        = viewport_,
                                   .fill_rule       = shape.fill_rule,
                                   .invert          = invert,
                                   .world_to_ndc    = world_to_ndc_.to_mat(),
                                   .world_transform = shape.world_transform,
                                   .vertices = shape.control_points.as_u8(),
                                   .indices  = indices.view().as_u8(),
                                   .regions  = span(regions).as_u8(),
                                   .region_index_counts = region_index_counts});
    }
    break;
    default:
      break;
  }
}

ICanvas & ICanvas::push_clip()
{
  clip_saves_.push(clip_).unwrap();
  return *this;
}

ICanvas & ICanvas::pop_clip()
{
  if (!clip_saves_.is_empty())
  {
    clip_ = clip_saves_.last();
    clip_saves_.pop(1);
  }
  return *this;
}

ICanvas & ICanvas::push_color()
{
  color_saves_.push(color_).unwrap();
  return *this;
}

ICanvas & ICanvas::pop_color()
{
  if (!color_saves_.is_empty())
  {
    color_ = color_saves_.last();
    color_saves_.pop(1);
  }
  return *this;
}

ICanvas & ICanvas::push_depth_stencil()
{
  depth_stencil_saves_.push(depth_stencil_).unwrap();
  return *this;
}

ICanvas & ICanvas::pop_depth_stencil()
{
  if (!depth_stencil_saves_.is_empty())
  {
    depth_stencil_ = depth_stencil_saves_.last();
    depth_stencil_saves_.pop(1);
  }
  return *this;
}

ICanvas & ICanvas::push_stencil_op()
{
  stencil_op_saves_.push(stencil_op_).unwrap();
  return *this;
}

ICanvas & ICanvas::pop_stencil_op()
{
  if (!stencil_op_saves_.is_empty())
  {
    stencil_op_ = stencil_op_saves_.last();
    stencil_op_saves_.pop(1);
  }
  return *this;
}

ICanvas & ICanvas::circle(Shape const & shape)
{
  render_(sampled_textures, shape, SdfShapeType::RRect);
  return *this;
}

ICanvas & ICanvas::rect(Shape const & shape)
{
  render_(sampled_textures, shape, shader::sdf::ShapeType::RRect);
  return *this;
}

ICanvas & ICanvas::rrect(Shape const & shape)
{
  render_(sampled_textures, shape, shader::sdf::ShapeType::RRect);
  return *this;
}

ICanvas & ICanvas::squircle(Shape const & shape)
{
  render_(sampled_textures, shape, shader::sdf::ShapeType::Squircle);
  return *this;
}

ICanvas & ICanvas::sdf_map(Shape const & shape)
{
  render_(sampled_textures, shape, shader::sdf::ShapeType::SDFMap);
  return *this;
}

ICanvas & ICanvas::noise(Shape const & shape)
{
  render_noise_(sampled_textures, shape, shader::sdf::ShapeType::RRect);
  return *this;
}

ICanvas & ICanvas::mesh_gradient(MeshGradientInfo const & info)
{
  render_(sampled_textures, info);
  return *this;
}

void ICanvas::nine_slice(ShapeInfo const & info, NineSlice const & slice)
{
  // [ ] implement
}

ICanvas & ICanvas::indexed_triangles(TriangleSetInfo const & info)
{
  render_(sampled_textures, info, true);
  return *this;
}

ICanvas & ICanvas::unindexed_triangles(TriangleSetInfo const & info)
{
  render_(sampled_textures, info, false);
  return *this;
}

ICanvas & ICanvas::line(CurveLineInfo const & info)
{
  if (points.size() < 2)
  {
    return;
  }

  // [ ] color stride

  Vec<f32x2> vertices{tmp_arena_};
  Vec<u32>   indices{tmp_arena_};

  path::triangulate_stroke(points, vertices, indices, info.feather * 2);
  return triangle_fill_(info, vertices, indices, sampled_textures);
}

ICanvas & ICanvas::blur(Shape const & shape)
{
  render_blur_(shape);
  return *this;
}

ICanvas & ICanvas::quad(QuadInfo const & quad)
{
  render_(sampled_textures, quad);
  return *this;
}

ICanvas & ICanvas::text(TextBlockRenderInfo const & text)
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
