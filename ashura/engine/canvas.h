/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/color.h"
#include "ashura/engine/passes.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocators.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief all points are stored in the [-0.5, +0.5] range, all arguments must be
/// normalized to same range.
namespace path
{

void rect(Vec<Vec2> & vtx);

/// @brief generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param stop stop angle
void arc(Vec<Vec2> & vtx, f32 start, f32 stop, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<Vec2> & vtx, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<Vec2> & vtx, f32 degree, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<Vec2> & vtx, Vec4 corner_radii, u32 segments);

/// @brief generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<Vec2> & vtx, Vec4 slants);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, u32 segments);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                  u32 segments);

/// @brief generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Vec<Vec2> & vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                 u32 segments);

/// @brief triangulate a stroke path, given the vertices for its points
void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> & vtx,
                        Vec<u32> & idx, f32 thickness);

/// @brief generate indices for a triangle list
void triangles(u32 first_vertex, u32 num_vertices, Vec<u32> & idx);

/// @brief generate vertices for a quadratic bezier curve
void triangulate_convex(Vec<u32> & idx, u32 first_vertex, u32 num_vertices);

};    // namespace path

enum class TileMode : u32
{
  Stretch = 0,
  Tile    = 1
};

/// @brief Canvas Shape Description
struct ShapeInfo
{
  /// @brief center of the shape in world-space
  Vec2 center = {0, 0};

  /// @brief extent of the shape in world-space
  Vec2 extent = {0, 0};

  /// @brief object-world-space transform matrix
  Mat4 transform = Mat4::identity();

  /// @brief corner radii of each corner if rrect
  Vec4 corner_radii = {};

  /// @brief lerp intensity between stroke and fill, 0 to fill, 1 to stroke
  f32 stroke = 0.0F;

  /// @brief thickness thickness of the stroke
  f32 thickness = 1.0F;

  /// @brief Linear Color gradient to use as tint
  ColorGradient tint = {};

  /// @brief sampler to use in rendering the shape
  SamplerId sampler = SamplerId::Linear;

  /// @brief texture to use in rendering the shape
  TextureId texture = TextureId::White;

  /// @brief uv coordinates of the upper-left and lower-right part of the
  /// texture to sample from
  Vec2 uv[2] = {
    {0, 0},
    {1, 1}
  };

  /// @brief tiling factor
  f32 tiling = 1;

  /// @brief edge smoothness to apply if it is a rrect
  f32 edge_smoothness = 1;
};

/// @brief a normative clip rect that will cover the entire Canvas.
inline constexpr Rect MAX_CLIP{
  .offset{0,         0        },
  .extent{0xFF'FFFF, 0xFF'FFFF}
};

inline constexpr CRect MAX_CLIP_CENTERED = MAX_CLIP.centered();

struct FrameGraph;
struct PassContext;

struct Canvas
{
  enum class BatchType : u8
  {
    None     = 0,
    RRect    = 1,
    Squircle = 2,
    Ngon     = 3,
    Blur     = 4,
    Pass     = 5
  };

  struct Batch
  {
    BatchType type = BatchType::None;

    Slice32 run{};

    Rect clip = MAX_CLIP;
  };

  struct Blur
  {
    RectU area         = {};
    Vec2U radius       = {};
    Vec4  corner_radii = {};
    Mat4  transform    = Mat4::identity();
    f32   aspect_ratio = 1;
  };

  typedef Dyn<
    Fn<void(FrameGraph &, PassContext &, Canvas const &, Framebuffer const &)>>
    PassFn;

  struct Pass
  {
    Str label = {};

    PassFn task{};
  };

  /// @brief the viewport of the framebuffer this canvas will be targetting
  /// this is in the Framebuffer coordinates (Physical px coordinates)
  gpu::Viewport viewport{};

  /// @brief the viewport's local extent. This will scale to the viewport's extent.
  /// This is typically the screen's virtual size (Logical px coordinates).
  /// This distinction helps support high-density displays.
  Vec2 extent{};

  /// @brief the pixel size of the backing framebuffer (Physical px coordinates)
  Vec2U framebuffer_extent{};

  /// @brief aspect ratio of the viewport
  f32 aspect_ratio = 1;

  /// @brief the ratio of the viewport's framebuffer coordinate extent
  /// to the viewport's virtual extent
  f32 virtual_scale = 1;

  /// @brief the world to viewport transformation matrix
  Affine4 world_to_view = Affine4::identity();

  Rect current_clip = MAX_CLIP;

  Vec<RRectParam> rrect_params;

  Vec<SquircleParam> squircle_params;

  Vec<NgonParam> ngon_params;

  Vec<Vec2> ngon_vertices;

  Vec<u32> ngon_indices;

  Vec<u32> ngon_index_counts;

  Vec<Blur> blurs;

  Vec<Pass> passes;

  Vec<Batch> batches;

  // declared last so it would release allocated memory after all operations
  // are done executing
  ArenaPool frame_arena;

  explicit Canvas(AllocatorRef allocator) :
    rrect_params{allocator},
    ngon_params{allocator},
    ngon_vertices{allocator},
    ngon_indices{allocator},
    ngon_index_counts{allocator},
    blurs{allocator},
    passes{allocator},
    batches{allocator},
    frame_arena{allocator}
  {
  }

  Canvas(Canvas const &)             = delete;
  Canvas(Canvas &&)                  = default;
  Canvas & operator=(Canvas const &) = delete;
  Canvas & operator=(Canvas &&)      = default;
  ~Canvas()                          = default;

  Canvas & begin_recording(gpu::Viewport const & viewport, Vec2 extent,
                           Vec2U framebuffer_extent);

  Canvas & end_recording();

  Canvas & reset();

  Canvas & reset_clip();

  Canvas & clip(Rect const & area);

  RectU clip_to_scissor(Rect const & clip) const;

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

  /// @brief
  ///
  ///
  /// ┏━━━━━━━━━━━━━━━━━┑
  /// ┃  0  ┃  1  ┃  2  ┃
  /// ┃╸╸╸╸╸┃╸╸╸╸╸┃╸╸╸╸╸┃
  /// ┃  3  ┃  4  ┃  5  ┃
  /// ┃╸╸╸╸╸┃╸╸╸╸╸┃╸╸╸╸╸┃
  /// ┃  6  ┃  7  ┃  8  ┃
  /// ┗━━━━━━━━━━━━━━━━━┛
  ///
  /// Scaling:
  ///
  /// 0 2 6 8; None
  /// 1 7; Horizontal
  /// 3 5;	Vertical
  /// 4;	Horizontal + Vertical
  ///
  /// @param info
  /// @param mode
  /// @param uvs
  Canvas & nine_slice(ShapeInfo const & info, TileMode mode,
                      Span<Vec4 const> uvs);

  /// @brief Render Non-Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<Vec2 const> vertices);

  /// @brief Render Indexed Triangles
  Canvas & triangles(ShapeInfo const & info, Span<Vec2 const> vertices,
                     Span<u32 const> indices);

  /// @brief triangulate and render line
  Canvas & line(ShapeInfo const & info, Span<Vec2 const> vertices);

  /// @brief perform a Canvas-space blur
  /// @param area region in the canvas to apply the blur to
  Canvas & blur(Rect const & area, Vec2 radius,
                Vec4 corner_radii = {0, 0, 0, 0});

  /// @brief register a custom canvas pass to be executed in the render thread
  Canvas & pass(Pass pass);

  template <typename Lambda>
  Canvas & pass(Str label, Lambda task)
  {
    // relocate lambda to heap
    Dyn<Lambda *> lambda =
      dyn(frame_arena, static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.allocator_ = noop_allocator;

    auto f = fn(*lambda);

    return pass(Pass{.label = label, .task = transmute(std::move(lambda), f)});
  }
};

}    // namespace ash
