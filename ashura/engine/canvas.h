/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/color.h"
#include "ashura/engine/font.h"
#include "ashura/engine/passes.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/scene.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocators.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief all points are stored in the [-1, +1] range, all arguments must be
/// normalized to same range.
namespace path
{

void rect(Vec<Vec2> &vtx);

/// @brief generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param stop stop angle
void arc(Vec<Vec2> &vtx, f32 start, f32 stop, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<Vec2> &vtx, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<Vec2> &vtx, f32 degree, u32 segments);

/// @brief generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<Vec2> &vtx, Vec4 corner_radii, u32 segments);

/// @brief generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<Vec2> &vtx, Vec4 slants);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, u32 segments);

/// @brief generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                  u32 segments);

/// @brief generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                 u32 segments);

/// @brief triangulate a stroke path, given the vertices for its points
void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vtx, Vec<u32> &idx,
                        f32 thickness);

/// @brief generate indices for a triangle list
void triangles(u32 first_vertex, u32 num_vertices, Vec<u32> &idx);

/// @brief generate vertices for a quadratic bezier curve
void triangulate_convex(Vec<u32> &idx, u32 first_vertex, u32 num_vertices);

};        // namespace path

enum class ScaleMode : u8
{
  Stretch = 0,
  Tile    = 1
};

/// @brief Canvas Shape Description
/// @param center center of the shape in world-space
/// @param extent extent of the shape in world-space
/// @param transform world-space transform matrix
/// @param corner_radii corner radii of each corner if rrect
/// @param stroke lerp intensity between stroke and fill, 0 to fill, 1 to stroke
/// @param thickness thickness of the stroke
/// @param scissor in surface pixel coordinates
/// @param tint Linear Color gradient to use as tint
/// @param sampler sampler to use in rendering the shape
/// @param texture texture index in the global texture store
/// @param uv uv coordinates of the upper-left and lower-right
/// @param tiling tiling factor
/// @param edge_smoothness edge smoothness to apply if it is a rrect
struct ShapeInfo
{
  Vec2 center = {0, 0};

  Vec2 extent = {0, 0};

  Mat4 transform = Mat4::identity();

  Vec4 corner_radii = {0, 0, 0, 0};

  f32 stroke = 0.0F;

  f32 thickness = 1.0F;

  ColorGradient tint = {};

  u32 sampler = 0;

  u32 texture = 0;

  Vec2 uv[2] = {{0, 0}, {1, 1}};

  f32 tiling = 1;

  f32 edge_smoothness = 0.015F;
};

struct Canvas
{
  struct RenderContext
  {
    Canvas              &canvas;
    GpuContext          &gpu;
    PassContext         &passes;
    RenderTarget const  &rt;
    gpu::CommandEncoder &enc;
    SSBO const          &rrects;
    SSBO const          &ngons;
    SSBO const          &ngon_vertices;
    SSBO const          &ngon_indices;
  };

  enum class BatchType : u8
  {
    None  = 0,
    RRect = 1,
    Ngon  = 2
  };

  struct Batch
  {
    BatchType type = BatchType::None;
    CRect   clip{.center{F32_MAX / 2, F32_MAX / 2}, .extent{F32_MAX, F32_MAX}};
    Slice32 objects{};
  };

  typedef Dyn<Fn<void(RenderContext const &)>> PassFn;

  struct Pass
  {
    Span<char const> name = {};
    PassFn           task{};
  };

  Vec2 viewport_extent{};

  Vec2U surface_extent{};

  f32 viewport_aspect_ratio = 1;

  Mat4 world_to_view = Mat4::identity();

  CRect current_clip{.center{F32_MAX / 2, F32_MAX / 2},
                     .extent{F32_MAX, F32_MAX}};

  Vec<RRectParam> rrect_params;

  Vec<NgonParam> ngon_params;

  Vec<Vec2> ngon_vertices;

  Vec<u32> ngon_indices;

  Vec<u32> ngon_index_counts;

  Batch batch{};

  Vec<Pass> passes;

  // declared last so it would release allocated memory after all operations
  // are done executing
  ArenaPool frame_arena;

  explicit Canvas(AllocatorImpl allocator) :
      rrect_params{allocator},
      ngon_params{allocator},
      ngon_vertices{allocator},
      ngon_indices{allocator},
      ngon_index_counts{allocator},
      passes{allocator},
      frame_arena{allocator}
  {
  }

  Canvas(Canvas const &)            = delete;
  Canvas(Canvas &&)                 = default;
  Canvas &operator=(Canvas const &) = delete;
  Canvas &operator=(Canvas &&)      = default;
  ~Canvas()                         = default;

  Canvas &begin_recording(Vec2 viewport_extent, Vec2U surface_extent);

  Canvas &end_recording();

  Canvas &reset();

  Canvas &clip(CRect const &area);

  /// @brief render a circle
  Canvas &circle(ShapeInfo const &info);

  /// @brief render a rectangle
  Canvas &rect(ShapeInfo const &info);

  /// @brief render a rounded rectangle
  Canvas &rrect(ShapeInfo const &info);

  /// @brief render a beveled rectangle
  Canvas &brect(ShapeInfo const &info);

  /// @brief render a squircle (triangulation based)
  /// @param num_segments an upper bound on the number of segments to
  /// @param degree
  Canvas &squircle(ShapeInfo const &info, f32 degree, u32 segments);

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
  Canvas &nine_slice(ShapeInfo const &info, ScaleMode mode,
                     Span<Vec4 const> uvs);

  /// @brief Render text using font atlases
  /// @param info only info.center, info.transform, info.tiling, and
  /// info.sampler are used
  /// @param block Text Block to be rendered
  /// @param layout Layout of text block to be rendered
  /// @param style styling of the text block, contains styling for the runs and
  /// alignment of the block
  /// @param atlases font atlases
  /// @param clip clip rect for culling draw commands of the text block
  Canvas &text(ShapeInfo const &info, TextBlock const &block,
               TextLayout const &layout, TextBlockStyle const &style,
               CRect const &clip = {{F32_MAX / 2, F32_MAX / 2},
                                    {F32_MAX, F32_MAX}});

  /// @brief Render Non-Indexed Triangles
  Canvas &triangles(ShapeInfo const &info, Span<Vec2 const> vertices);

  /// @brief Render Indexed Triangles
  Canvas &triangles(ShapeInfo const &info, Span<Vec2 const> vertices,
                    Span<u32 const> indices);

  /// @brief triangulate and render line
  Canvas &line(ShapeInfo const &info, Span<Vec2 const> vertices);

  /// @brief perform a Canvas-space blur
  /// @param area region in the canvas to apply the blur to
  /// @param num_passes number of blur passes to execute, higher values result
  /// in blurrier results
  Canvas &blur(CRect const &area, u32 num_passes);

  /// @brief register a custom canvas pass to be executed in the render thread
  Canvas &add_pass(Pass &&pass);

  template <typename Lambda>
  Canvas &add_pass(Span<char const> name, Lambda &&task)
  {
    // relocate lambda to heap
    Dyn<Lambda *> lambda =
        dyn(frame_arena.to_allocator(), static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.inner.allocator = noop_allocator;

    return add_pass(Pass{
        .name = name, .task = transmute(std::move(lambda), fn(lambda.get()))});
  }
};

}        // namespace ash
