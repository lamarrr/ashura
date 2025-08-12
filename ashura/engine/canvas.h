/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/encoders.h"
#include "ashura/engine/pass_bundle.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocators.h"
#include "ashura/std/color.h"
#include "ashura/std/math.h"
#include "ashura/std/rc.h"
#include "ashura/std/types.h"

namespace ash
{

namespace path
{

// Don't allow linearized segments to be off by more than 1/4th of a pixel from
// the true curve. This value should be scaled by the max basis of the
// X and Y directions.
constexpr f32 cubic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                                 f32x2 p3, f32 precision = 4)
{
  auto k = scale_factor * .75F * precision;
  auto a = (p0 - p1 * 2 + p2).abs();
  auto b = (p1 - p2 * 2 + p3).abs();
  return sqrt(k * a.max(b).length());
}

constexpr f32 quadratic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1,
                                     f32x2 p2, f32 precision = 4)
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
constexpr f32 conic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                                 f32 w, f32 precision = 4)
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

void rect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center);

/// @brief generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param turn turn angle
void arc(Vec<f32x2> & vtx, f32x2 radii, f32x2 center, f32 start, f32 turn,
         usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32 degree,
              usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 corner_radii,
           usize segments);

/// @brief generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 slants);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, usize segments);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3,
                  usize segments);

/// @brief generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Vec<f32x2> & vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3,
                 usize segments);

/// @brief triangulate a stroke path, given the vertices for its points
void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vtx,
                        Vec<u32> & idx, f32 thickness);

void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vtx,
                        Vec<u16> & idx, f32 thickness);

/// @brief generate indices for a triangle list
void triangles(u32 first_vertex, u32 num_vertices, Vec<u32> & idx);

void triangles(u16 first_vertex, u16 num_vertices, Vec<u16> & idx);

/// @brief generate vertices for a quadratic bezier curve
void triangulate_convex(Vec<u32> & idx, u32 first_vertex, u32 num_vertices);

void triangulate_convex(Vec<u16> & idx, u16 first_vertex, u16 num_vertices);

};    // namespace path

enum class TileMode : u8
{
  Stretch = 0,
  Tile    = 1
};

/// @brief a normative clip rect that will cover the entire Canvas.
constexpr f32 MAX_CLIP_DISTANCE = 0xFF'FFFF;

inline constexpr CRect MAX_CLIP{.center = f32x2::splat(0),
                                .extent = f32x2::splat(MAX_CLIP_DISTANCE)};

using shader::sdf::ShadeType;

/// @brief Canvas Shape Description
struct ShapeInfo
{
  /// @brief center of the shape in world-space
  CRect area = {};

  /// @brief object-world-space transform matrix
  f32x4x4 transform = f32x4x4::identity();

  f32x4 radii = {};

  ShadeType shade_type = ShadeType::Flood;

  /// @brief thickness of the stroke
  f32 feather = 0;

  /// @brief Linear Color gradient to use as tint
  ColorGradient tint = {};

  /// @brief sampler to use in rendering the shape
  SamplerId sampler = SamplerId::LinearBlack;

  /// @brief texture to use in rendering the shape
  TextureId texture = TextureId::White;

  /// @brief uv coordinates of the upper-left and lower-right part of the
  /// texture to sample from
  f32x2 uv[2] = {
    {0, 0},
    {1, 1}
  };

  CRect clip = MAX_CLIP;
};

struct Quad
{
  f32x4 top_left     = {};
  f32x4 top_right    = {};
  f32x4 bottom_right = {};
  f32x4 bottom_left  = {};
};

/// ┏━━━━━━━━━━━━━━━━━┑
/// ┃  0  ┃  1  ┃  2  ┃
/// ┃╸╸╸╸╸┃╸╸╸╸╸┃╸╸╸╸╸┃
/// ┃  3  ┃  4  ┃  5  ┃
/// ┃╸╸╸╸╸┃╸╸╸╸╸┃╸╸╸╸╸┃
/// ┃  6  ┃  7  ┃  8  ┃
/// ┗━━━━━━━━━━━━━━━━━┛
///
/// Stretching:
///
/// 0 2 6 8; None
/// 1 7; Horizontal
/// 3 5;	Vertical
/// 4;	Horizontal + Vertical
struct NineSlice
{
  TileMode mode             = TileMode::Stretch;
  f32x2    top_left[2]      = {};
  f32x2    top_center[2]    = {};
  f32x2    top_right[2]     = {};
  f32x2    mid_left[2]      = {};
  f32x2    mid_center[2]    = {};
  f32x2    mid_right[2]     = {};
  f32x2    bottom_left[2]   = {};
  f32x2    bottom_center[2] = {};
  f32x2    bottom_right[2]  = {};
};

struct FrameGraph;

struct PassBundle;

enum class ContourEdgeType : u8
{
  Line        = 0,
  Arc         = 1,
  Bezier      = 2,
  CubicBezier = 3
};

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

struct Canvas
{
  typedef Fn<void(FrameGraph &, PassBundle &)> PassFnRef;

  typedef Dyn<PassFnRef> PassFn;

  using Encoder = Enum<None, SdfEncoder, QuadEncoder, NgonEncoder,
                       FillStencilEncoder, BezierStencilEncoder, PassFn>;

  InplaceVec<ColorTexture, 4> color_textures_;

  InplaceVec<Option<ColorMsaaTexture>, 4> msaa_color_textures_;

  InplaceVec<DepthStencilTexture, 4> depth_stencil_textures_;

  u32 target_;

  Option<Tuple<u32, PassStencil>> stencil_;

  /// @brief the viewport of the framebuffer this canvas will be targetting
  /// this is in the Framebuffer coordinates (Physical px coordinates)
  gpu::Viewport viewport_;

  /// @brief the viewport's local extent. This will scale to the viewport's extent.
  /// This is typically the screen's virtual size (Logical px coordinates).
  /// This distinction helps support high-density displays.
  f32x2 extent_;

  /// @brief the pixel size of the backing framebuffer (Physical px coordinates)
  u32x2 framebuffer_extent_;

  f32x2 framebuffer_uv_base_;

  /// @brief aspect ratio of the viewport
  f32 aspect_ratio_;

  /// @brief the ratio of the viewport's framebuffer coordinate extent
  /// to the viewport's virtual extent
  f32 virtual_scale_;

  /// @brief the world to viewport transformation matrix for the shader (-1.0, 1.0)
  affinef32x4 world_to_ndc_;

  affinef32x4 ndc_to_viewport_;

  affinef32x4 viewport_to_fb_;

  affinef32x4 world_to_fb_;

  Encoder encoder_;

  ref<FrameGraph> frame_graph_;

  ref<PassBundle> passes_;

  // declared last so it would release allocated memory after all operations
  // are done executing
  ArenaPool frame_arena_;

  explicit Canvas(Allocator allocator, FrameGraph & frame_graph,
                  PassBundle & passes) :
    color_textures_{},
    msaa_color_textures_{},
    depth_stencil_textures_{},
    target_{0},
    stencil_{none},
    viewport_{},
    extent_{},
    framebuffer_extent_{},
    framebuffer_uv_base_{},
    aspect_ratio_{1},
    virtual_scale_{1},
    world_to_ndc_{affinef32x4::identity()},
    ndc_to_viewport_{affinef32x4::identity()},
    viewport_to_fb_{affinef32x4::identity()},
    world_to_fb_{affinef32x4::identity()},
    encoder_{none},
    frame_graph_{frame_graph},
    passes_{passes},
    frame_arena_{allocator}
  {
  }

  Canvas(Canvas const &)             = delete;
  Canvas(Canvas &&)                  = default;
  Canvas & operator=(Canvas const &) = delete;
  Canvas & operator=(Canvas &&)      = default;
  ~Canvas()                          = default;

  Canvas &
    begin_recording(FrameGraph & frame_graph, PassBundle & passes,
                    Span<ColorTexture const>             color_textures,
                    Span<Option<ColorMsaaTexture> const> msaa_color_textures,
                    Span<DepthStencilTexture const>      depth_stencil_textures,
                    gpu::Viewport const & viewport, f32x2 extent,
                    u32x2 framebuffer_extent);

  Canvas & end_recording();

  Canvas & reset();

  RectU clip_to_scissor(CRect const & clip) const;

  u32 num_targets() const;

  Canvas & clear_target(gpu::Color color);

  Canvas & set_target(u32 target);

  u32 target() const;

  u32 num_stencils() const;

  u32 num_stencil_bits() const;

  Canvas & clear_stencil(u32 stencil_value);

  Canvas & set_stencil(Option<Tuple<u32, PassStencil>> stencil);

  Option<Tuple<u32, PassStencil>> stencil() const;

  void flush_encoder_();

  template <typename Shape, typename Material>
  Canvas & push_sdf_(SdfEncoder::Item<Shape, Material> const & item)
  {
    // [ ] refactor; this is a mess, should be a function that can be called on the types or an alternative function if the active item is not matched
    encoder_.match([&](None) { encoder_ = SdfEncoder(frame_arena_, item); },
                   [&](SdfEncoder & enc) {
                     if (!enc.push(item))
                     {
                       flush_encoder_();
                       encoder_ = SdfEncoder(frame_arena_, item);
                     }
                   },
                   [&](QuadEncoder &) {
                     flush_encoder_();
                     encoder_ = SdfEncoder(frame_arena_, item);
                   },
                   [&](NgonEncoder &) {
                     flush_encoder_();
                     encoder_ = SdfEncoder(frame_arena_, item);
                   },
                   [&](FillStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = SdfEncoder(frame_arena_, item);
                   },
                   [&](BezierStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = SdfEncoder(frame_arena_, item);
                   },
                   [&](PassFn &) {
                     flush_encoder_();
                     encoder_ = SdfEncoder(frame_arena_, item);
                   });

    return *this;
  }

  template <typename Material>
  Canvas & push_ngon_(NgonEncoder::Item<Material> const & item)
  {
    encoder_.match([&](None) { encoder_ = NgonEncoder(frame_arena_, item); },
                   [&](SdfEncoder &) {
                     flush_encoder_();
                     encoder_ = NgonEncoder(frame_arena_, item);
                   },
                   [&](QuadEncoder &) {
                     flush_encoder_();
                     encoder_ = NgonEncoder(frame_arena_, item);
                   },
                   [&](NgonEncoder & enc) {
                     if (!enc.push(item))
                     {
                       flush_encoder_();
                       encoder_ = NgonEncoder(frame_arena_, item);
                     }
                   },
                   [&](FillStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = NgonEncoder(frame_arena_, item);
                   },
                   [&](BezierStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = NgonEncoder(frame_arena_, item);
                   },
                   [&](PassFn &) {
                     flush_encoder_();
                     encoder_ = NgonEncoder(frame_arena_, item);
                   });

    return *this;
  }

  template <typename Material>
  Canvas & push_quad_(QuadEncoder::Item<Material> const & item)
  {
    encoder_.match([&](None) { encoder_ = QuadEncoder(frame_arena_, item); },
                   [&](SdfEncoder &) {
                     flush_encoder_();
                     encoder_ = QuadEncoder(frame_arena_, item);
                   },
                   [&](QuadEncoder & enc) {
                     if (!enc.push(item))
                     {
                       flush_encoder_();
                       encoder_ = QuadEncoder(frame_arena_, item);
                     }
                   },
                   [&](NgonEncoder &) {
                     flush_encoder_();
                     encoder_ = QuadEncoder(frame_arena_, item);
                   },
                   [&](FillStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = QuadEncoder(frame_arena_, item);
                   },
                   [&](BezierStencilEncoder &) {
                     flush_encoder_();
                     encoder_ = QuadEncoder(frame_arena_, item);
                   },
                   [&](PassFn &) {
                     flush_encoder_();
                     encoder_ = QuadEncoder(frame_arena_, item);
                   });
  }

  Canvas & sdf_shape_(ShapeInfo const & info, shader::sdf::ShapeType shape);

  Canvas & ngon_shape_(ShapeInfo const & info, Span<f32x2 const> vertices,
                       Span<u32 const> indices);

  /// @brief register a custom canvas pass to be executed in the render thread
  template <Callable<FrameGraph &, PassBundle &> Lambda>
  Canvas & pass(Lambda task)
  {
    flush_encoder_();
    // relocate lambda to heap
    Dyn<Lambda *> lambda =
      dyn(frame_arena_, static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.allocator_ = noop_allocator;

    auto f = PassFnRef(lambda.get());

    encoder_ = transmute(std::move(lambda), f);

    return *this;
  }

  /// @brief render a circle
  Canvas & circle(ShapeInfo const & info);

  /// @brief render a rectangle
  Canvas & rect(ShapeInfo const & info);

  /// @brief render a rounded rectangle
  Canvas & rrect(ShapeInfo const & info);

  /// @brief render a squircle (triangulation based)
  /// @param num_segments an upper bound on the number of segments to
  /// @param elasticity elasticity of the squircle [0, 1]
  Canvas & squircle(ShapeInfo const & info);

  /// @brief draw a nine-sliced image
  Canvas & nine_slice(ShapeInfo const & info, NineSlice const & slice);

  /// @brief Render Non-Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<f32x2 const> vertices);

  /// @brief Render Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<f32x2 const> vertices,
                     Span<u32 const> indices);

  /// @brief triangulate and render line
  //  [ ] joints & caps
  // [ ] path-command-style arguments
  // [ ] +tesselation arguments
  Canvas & line(ShapeInfo const & info, Span<f32x2 const> vertices);

  /// @brief perform a Canvas-space blur
  /// @param area region in the canvas to apply the blur to
  Canvas & blur(ShapeInfo const & info);

  template <typename Material>
  Canvas & quad(ShaderVariantId shader, Quad const & quad,
                Material const & material);

  Canvas & quad(Quad const & quad, shader::quad::FlatMaterial const & material);

  // [ ] blending for noise material
  Canvas & quad(Quad const &                        quad,
                shader::quad::NoiseMaterial const & material);

  // [ ] Rendering Lines
  // [ ] Handling Self-Intersection; Fill Rules; STC
  // [ ] Masks
  // [ ] filling or lines; line caps; line joints;
  // [ ] dashed-line single pass shader?: will need uv-transforms
  // [ ] bezier line renderer + line renderer : cap + opacity + blending
  // [ ] batch multiple contour stencil passes so we can perform them in a single write, different write masks. rect-allocation?
  // [ ] render to layer
  // [ ] with_mask()?
  // [ ] draw linec
  // [ ] linear encoding variant
  // [ ] how will depth/stencil in framegraph work with canvas?
  Canvas & contour_stencil_(u32 stencil, u32 write_mask, bool tesselate,
                            Span<f32x2 const>           control_points,
                            Span<ContourEdgeType const> edge_types,
                            Span<u16 const> subdivision_counts, bool invert,
                            FillRule fill_rule, f32x4x4 const & transform,
                            CRect const & clip);

  Canvas & contour_line_(Span<f32x2 const>           control_points,
                         Span<ContourEdgeType const> edge_types,
                         Span<u16 const>             subdivision_counts);

  Canvas & text(Span<TextLayer const> layers, Span<ShapeInfo const> shapes,
                Span<TextRenderInfo const> infos, Span<usize const> sorted);

  TextRenderer text_renderer()
  {
    return TextRenderer{
      this,
      [](Canvas * p, Span<TextLayer const> layers, Span<ShapeInfo const> shapes,
         Span<TextRenderInfo const> infos,
         Span<usize const> sorted) { p->text(layers, shapes, infos, sorted); }};
  }
};

}    // namespace ash
