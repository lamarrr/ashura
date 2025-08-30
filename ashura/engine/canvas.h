/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/encoders.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocators.h"
#include "ashura/std/color.h"
#include "ashura/std/math.h"
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

void line(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1);

/// @brief generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param turn turn angle
void arc(Span<f32x2> vtx, f32x2 radii, f32x2 center, f32 start, f32 turn);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3);

/// @brief generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32 degree,
              u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 corner_radii,
           u32 segments);

/// @brief generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 slants);

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

  /// @brief extent of the shape's bounding-box in world-space
  f32x2 bbox_extent = {};

  /// @brief object-world-space transform matrix
  f32x4x4 transform = f32x4x4::identity();

  f32x4 radii = {};

  ShadeType shade_type = ShadeType::Flood;

  /// @brief thickness of the stroke
  f32 feather = 0;

  /// @brief Linear Color gradient to use as tint
  ColorGradient tint = {};

  /// @brief sampler to use in rendering the shape
  SamplerIndex sampler = SamplerIndex::LinearEdgeClampBlackFloat;

  /// @brief texture to use in rendering the shape
  TextureIndex texture = TextureIndex::White;

  /// @brief uv coordinates of the upper-left and lower-right part of the
  /// texture to sample from
  f32x2 uv[2] = {
    {0, 0},
    {1, 1}
  };

  CRect clip = MAX_CLIP;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }
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

enum class ContourEdgeType : u8
{
  Line            = 0,
  Arc             = 1,
  QuadraticBezier = 2,
  CubicBezier     = 3
};

typedef struct ICanvas * Canvas;

enum class CanvasState : u8
{
  Reset     = 0,
  Recording = 1,
  Recorded  = 2,
  Executed  = 3
};

// [ ] rename
enum class ContourRaster : u8
{
  StencilThenCover = 0,
  BezierStencil    = 1
};

struct ICanvas
{
  static constexpr u32 DEFAULT_NUM_IMAGE_SLOTS = 4;

  CanvasState state_;

  BitVec<u64> image_slots_;

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

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

  Vec<Dyn<CanvasEncoder>> encoders_;

  // [ ]  on render requested, allocate textures and record render commands; The offscreen ones will need an off-screen command and encoder
  //                             // . will still need texture allocation scope for passes like blur pass
  //                             // custom passes. use the GpuFramePlan
  //
  // we need to be able to know which layers to use for rendering and to select or switch between them
  //
  // texture allocation for each layer or canvas. Should canvas be per-layer?

  ArenaPool encoder_arena_;

  ArenaPool tmp_arena_;

  explicit ICanvas(Allocator allocator) :
    state_{CanvasState::Reset},
    image_slots_{allocator},
    color_{0},
    depth_stencil_{none},
    stencil_op_{none},
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
    encoders_{allocator},
    encoder_arena_{allocator, ArenaPoolCfg{.min_arena_size = 1_MB}},
    tmp_arena_{allocator, ArenaPoolCfg{.min_arena_size = 1_MB}}
  {
  }

  ICanvas(ICanvas const &)             = delete;
  ICanvas(ICanvas &&)                  = delete;
  ICanvas & operator=(ICanvas const &) = delete;
  ICanvas & operator=(ICanvas &&)      = delete;
  ~ICanvas()                           = default;

  u32 num_image_slots() const;

  u32 color() const;

  u32 num_stencil_bits() const;

  Option<u32> depth_stencil() const;

  Option<PipelineStencil> stencil_op() const;

  gpu::Viewport viewport() const;

  f32x2 extent() const;

  u32x2 framebuffer_extent() const;

  f32x2 framebuffer_uv_base() const;

  f32 aspect_ratio() const;

  f32 virtual_scale() const;

  affinef32x4 world_to_ndc() const;

  affinef32x4 ndc_to_viewport() const;

  affinef32x4 viewport_to_fb() const;

  affinef32x4 world_to_fb() const;

  RectU clip_to_scissor(CRect const & clip) const;

  TextRenderer default_text_renderer();

  void begin(gpu::Viewport const & viewport, f32x2 extent,
             u32x2 framebuffer_extent);

  void end();

  void reset();

  void execute(GpuFramePlan plan);

  void reserve_images(u32 num_images);

  u32 allocate_image();

  void deallocate_image(u32 image);

  void clear_color(u32 image, gpu::Color value);

  void set_color(u32 image);

  void clear_depth_stencil(u32 image, gpu::DepthStencil value);

  void set_depth_stencil(Option<u32> image);

  void set_stencil_op(Option<PipelineStencil> op);

  void encode_(SdfEncoder::Item const & item);

  void encode_(TriangleFillEncoder::Item const & item);

  void encode_(QuadEncoder::Item const & item);

  void encode_(FillStencilEncoder::Item const & item);

  void encode_(BezierStencilEncoder::Item const & item);

  void encode_(PbrEncoder::Item const & item);

  /// @brief register a custom canvas pass to be executed in the render thread
  template <Callable<GpuFramePlan> Lambda>
  void encode_pass_(Lambda && task)
  {
    auto enc = dyn<CustomCanvasEncoder<Lambda>>(inplace, encoder_arena_,
                                                static_cast<Lambda &&>(task))
                 .unwrap();

    encoders_.push(cast<CanvasEncoder>(std::move(enc))).unwrap();
  }

  void sdf_(ShapeInfo const & info, shader::sdf::ShapeType shape,
            TextureSet texture_set);

  // [ ] implement
  // [ ] switching to noise shader
  void sdf_noise_(ShapeInfo const & info, shader::sdf::ShapeType shape,
                  TextureSet texture_set);

  void triangle_fill_(ShapeInfo const & info, Span<f32x2 const> vertices,
                      Span<u32 const> indices, Span<f32x4 const> colors,
                      TextureSet texture_set);

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
  void contour_stencil_(u32 stencil, u32 write_mask, ContourRaster raster,
                        Span<f32x2 const>           control_points,
                        Span<ContourEdgeType const> edge_types,
                        Span<u16 const> segments, bool invert,
                        FillRule fill_rule, f32x4x4 const & transform,
                        CRect const & clip);

  void contour_line_(Span<f32x2 const>           control_points,
                     Span<ContourEdgeType const> edge_types,
                     Span<u16 const>             segments);

  void blur_(u32 color, Option<u32> depth_stencil, ShapeInfo const & info);

  /// @brief render a circle
  void circle(ShapeInfo const & info);

  /// @brief render a rectangle
  void rect(ShapeInfo const & info);

  /// @brief render a rounded rectangle
  void rrect(ShapeInfo const & info);

  /// @brief render a squircle (triangulation based)
  /// @param num_segments an upper bound on the number of segments to
  /// @param elasticity elasticity of the squircle [0, 1]
  void squircle(ShapeInfo const & info);

  /// @brief draw a nine-sliced image
  void nine_slice(ShapeInfo const & info, NineSlice const & slice);

  /// @brief Render Non-Indexed Triangles
  void triangles(ShapeInfo const & info, Span<f32x2 const> vertices,
                 Span<f32x4 const> colors);

  /// @brief Render Indexed Triangles
  void triangles(ShapeInfo const & info, Span<f32x2 const> vertices,
                 Span<u32 const> indices, Span<f32x4 const> colors);

  /// @brief triangulate and render line
  // [ ] joints & caps
  // [ ] path-command-style arguments
  // [ ] +tesselation arguments
  void line(ShapeInfo const & info, Span<f32x2 const> vertices,
            Span<f32x4 const> colors);

  /// @brief perform a canvas-space blur
  /// @param area region in the canvas to apply the blur to
  void blur(ShapeInfo const & info);

  void quad(Quad const & quad, shader::quad::FlatMaterial const & material);

  // [ ] blending for noise material
  void quad(Quad const & quad, shader::quad::NoiseMaterial const & material);

  void text(Span<TextLayer const> layers, Span<ShapeInfo const> shapes,
            Span<TextRenderInfo const> infos, Span<usize const> sorted);
};

}    // namespace ash
