/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/contour.h"
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

/// @brief all points are stored in the [-0.5, +0.5] range, all arguments must be
/// normalized to same range.
namespace path
{

void rect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center);

/// @brief generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param stop stop angle
void arc(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, f32 start, f32 stop,
         usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, f32 degree,
              usize segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec4 corner_radii,
           usize segments);

/// @brief generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec4 slants);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0, Vec2 cp1,
            Vec2 cp2, usize segments);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0, Vec2 cp1,
                  Vec2 cp2, Vec2 cp3, usize segments);

/// @brief generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Vec<Vec2> & vtx, Vec2 extent, Vec2 center, Vec2 cp0, Vec2 cp1,
                 Vec2 cp2, Vec2 cp3, usize segments);

/// @brief triangulate a stroke path, given the vertices for its points
void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vtx,
                        Vec<u32> & idx, f32 thickness);

void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vtx,
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

inline constexpr CRect MAX_CLIP{.center = Vec2::splat(0),
                                .extent = Vec2::splat(MAX_CLIP_DISTANCE)};

using shader::sdf::ShadeType;

/// @brief Canvas Shape Description
struct ShapeInfo
{
  /// @brief center of the shape in world-space
  CRect area = {};

  /// @brief object-world-space transform matrix
  Mat4 transform = Mat4::IDENTITY;

  Vec4 radii = {};

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
  Vec2 uv[2] = {
    {0, 0},
    {1, 1}
  };

  CRect clip = MAX_CLIP;
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
  Vec2     top_left[2]      = {};
  Vec2     top_center[2]    = {};
  Vec2     top_right[2]     = {};
  Vec2     mid_left[2]      = {};
  Vec2     mid_center[2]    = {};
  Vec2     mid_right[2]     = {};
  Vec2     bottom_left[2]   = {};
  Vec2     bottom_center[2] = {};
  Vec2     bottom_right[2]  = {};
};

struct FrameGraph;

struct PassBundle;

struct Canvas
{
  typedef Fn<void(CanvasPassInfo const &)> PassFnRef;

  typedef Dyn<PassFnRef> PassFn;

  // [ ] contour stencil & rendering pass?
  enum class BatchType : u8
  {
    None    = 0,
    Pass    = 1,
    Sdf     = 2,
    Ngon    = 3,
    Blur    = 4,
    Contour = 5
  };

  struct Batch
  {
    BatchType type       = BatchType::None;
    Slice32   items      = {};
    hash64    batch_hash = 0;
  };

  struct CustomData
  {
    Str    label = {};
    PassFn task{};
  };

  u32 num_targets_;

  u32 num_masks_;

  /// @brief the viewport of the framebuffer this canvas will be targetting
  /// this is in the Framebuffer coordinates (Physical px coordinates)
  gpu::Viewport viewport_;

  /// @brief the viewport's local extent. This will scale to the viewport's extent.
  /// This is typically the screen's virtual size (Logical px coordinates).
  /// This distinction helps support high-density displays.
  Vec2 extent_;

  /// @brief the pixel size of the backing framebuffer (Physical px coordinates)
  Vec2U framebuffer_extent_;

  Vec2 framebuffer_uv_base_;

  /// @brief aspect ratio of the viewport
  f32 aspect_ratio_;

  /// @brief the ratio of the viewport's framebuffer coordinate extent
  /// to the viewport's virtual extent
  f32 virtual_scale_;

  /// @brief the world to viewport transformation matrix for the shader (-1.0, 1.0)
  Affine4 world_to_ndc_;

  Affine4 ndc_to_viewport_;

  Affine4 viewport_to_fb_;

  impl::BlurCanvas blur_;

  impl::NgonCanvas<> ngon_;

  impl::SdfCanvas<> sdf_;

  impl::CompositeSdfCanvas<> composited_sdf_;

  impl::ContourCanvas<> contour_;

  impl::SdfCanvas<shader::sdf::NoiseMaterial> noise_;

  Vec<Pass> passes_;

  Vec<Batch> batches_;

  // declared last so it would release allocated memory after all operations
  // are done executing
  ArenaPool frame_arena_;

  u32 target_;

  Option<u32> stencil_;

  explicit Canvas(AllocatorRef allocator) :
    num_layers_{0},
    num_masks_{0},
    viewport_{},
    extent_{},
    framebuffer_extent_{},
    framebuffer_uv_base_{},
    aspect_ratio_{1},
    virtual_scale_{1},
    world_to_ndc_{Affine4::IDENTITY},
    ndc_to_viewport_{Affine4::IDENTITY},
    viewport_to_fb_{Affine4::IDENTITY},
    rrect_params_{allocator},
    ngon_{allocator},
    blurs_{allocator},
    passes_{allocator},
    contour_{allocator},
    batches_{allocator},
    frame_arena_{allocator},
    mask_{none},
    layer_{0}
  {
  }

  Canvas(Canvas const &)             = delete;
  Canvas(Canvas &&)                  = default;
  Canvas & operator=(Canvas const &) = delete;
  Canvas & operator=(Canvas &&)      = default;
  ~Canvas()                          = default;

  Canvas & begin_recording(u32 num_layers, u32 num_masks,
                           gpu::Viewport const & viewport, Vec2 extent,
                           Vec2U framebuffer_extent);

  Canvas & end_recording();

  Canvas & reset();

  RectU clip_to_scissor(CRect const & clip) const;

  /// @brief render a circle
  Canvas & circle(ShapeInfo const & info);

  /// @brief render a rectangle
  Canvas & rect(ShapeInfo const & info);

  /// @brief render a rounded rectangle
  Canvas & rrect(ShapeInfo const & info);

  /// @brief render a beveled rectangle
  Canvas & brect(ShapeInfo const & info);

  /// @brief render a squircle (triangulation based)
  /// @param num_segments an upper bound on the number of segments to
  /// @param elasticity elasticity of the squircle [0, 1]
  Canvas & squircle(ShapeInfo const & info);

  /// @brief draw a nine-sliced image
  Canvas & nine_slice(ShapeInfo const & info, NineSlice const & slice);

  /// @brief Render Non-Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<Vec2 const> vertices);

  /// @brief Render Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<Vec2 const> vertices,
                     Span<u32 const> indices);

  /// @brief triangulate and render line
  Canvas & line(ShapeInfo const & info, Span<Vec2 const> vertices);

  /// @brief perform a Canvas-space blur
  /// @param area region in the canvas to apply the blur to
  Canvas & blur(ShapeInfo const & info);

  Canvas & pass_(Pass pass);

  /// @brief register a custom canvas pass to be executed in the render thread
  template <typename Lambda>
  Canvas & pass(Str label, Lambda task)
  {
    // relocate lambda to heap
    Dyn<Lambda *> lambda =
      dyn(frame_arena_, static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.allocator_ = noop_allocator;

    auto f = PassFnRef(lambda.get());

    return pass_(Pass{.label = label, .task = transmute(std::move(lambda), f)});
  }

  u32 num_targets() const;

  Canvas & clear_target(u32 target);

  Canvas & set_target(u32 target);

  u32 target() const;

  u32 num_stencils() const;

  Canvas & clear_stencil();

  Canvas & set_stencil(Option<u32> stencil);

  Option<u32> stencil() const;

  // [ ] how to go from paths to masks; how to curve around bezier curves

  // [ ] dashed-line single pass shader?: will need uv-transforms
  // [ ] bezier line renderer + line renderer : cap + opacity + blending
  // [ ] fill path renderer
  // [ ] shader functions
  // [ ] convex tesselation is free and doesn't need fill rules
  // [ ] for concave; use stencil then cover
  // [ ] batch multiple contour stencil passes so we can perform them in a single write, different write masks. rect-allocation?

  // [ ] masks for blur shape
  // [ ] render to layer
  // [ ] with_mask()?
  // [ ] quad pass with custom shaders

  Canvas & contour_stencil(u32 mask, u32 write_mask, Contour2D const & contour);

  // [ ] draw line

  Canvas & contour();

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
