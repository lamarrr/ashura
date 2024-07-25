/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/ngon.h"
#include "ashura/engine/passes/rrect.h"
#include "ashura/engine/text.h"
#include "ashura/engine/types.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{
typedef struct PassContext PassContext;

enum class CanvasPassType : u8
{
  None   = 0,
  RRect  = 1,
  Blur   = 2,
  Ngon   = 3,
  Custom = 4
};

enum class ScaleMode : u8
{
  Stretch = 0,
  Tile    = 1
};

/// @brief
/// @param scissor in surface pixel coordinates
struct ShapeDesc
{
  Vec2          center          = {0, 0};
  Vec2          extent          = {0, 0};
  Mat4          transform       = Mat4::identity();
  Vec4          corner_radii    = {0, 0, 0, 0};
  f32           stroke          = 0.0f;
  f32           thickness       = 1.0f;
  ColorGradient tint            = {};
  u32           sampler         = 0;
  u32           texture         = 0;
  Vec2          uv[2]           = {{0, 0}, {1, 1}};
  f32           tiling          = 1;
  f32           edge_smoothness = 0.0015F;
};

struct CanvasPassRun
{
  CanvasPassType type  = CanvasPassType::None;
  CRect          clip  = {};
  u32            first = 0;
  u32            count = 0;
};

struct CanvasBlurParam
{
  CRect area       = {};
  u32   num_passes = 0;
};

/// @brief encoder function to encode the pass onto the renderer, will be called
/// from the render thread
typedef Fn<void(RenderContext &, PassContext &, gfx::RenderingInfo const &,
                gfx::DescriptorSet)>
    CustomCanvasPass;

/// @brief all points are stored in the [-1, +1] range, all arguments must be
/// normalized to same range.
struct Path
{
  static void rect(Vec<Vec2> &vtx);

  /// @brief generate vertices for an arc
  /// @param segments upper bound on the number of segments to divide the arc
  /// into
  /// @param start start angle
  /// @param stop stop angle
  static void arc(Vec<Vec2> &vtx, f32 start, f32 stop, u32 segments);

  /// @brief generate vertices for a circle
  /// @param segments upper bound on the number of segments to divide the circle
  /// into
  static void circle(Vec<Vec2> &vtx, u32 segments);

  /// @brief generate vertices for a circle
  /// @param segments upper bound on the number of segments to divide the circle
  /// into
  /// @param degree number of degrees of the super-ellipse
  static void squircle(Vec<Vec2> &vtx, f32 degree, u32 segments);

  /// @brief generate vertices for a circle
  /// @param segments upper bound on the number of segments to divide the circle
  /// into
  /// @param corner_radii border radius of each corner
  static void rrect(Vec<Vec2> &vtx, Vec4 corner_radii, u32 segments);

  /// @brief generate vertices of a bevel rect
  /// @param vtx
  /// @param slants each component represents the relative distance from the
  /// corners of each bevel
  static void brect(Vec<Vec2> &vtx, Vec4 slants);

  /// @brief generate vertices for a quadratic bezier curve
  /// @param segments upper bound on the number of segments to divide the bezier
  /// curve into
  /// @param cp[0-2] control points
  static void bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2,
                     u32 segments);

  /// @brief generate vertices for a quadratic bezier curve
  /// @param segments upper bound on the number of segments to divide the bezier
  /// curve into
  /// @param cp[0-3] control points
  static void cubic_bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2,
                           Vec2 cp3, u32 segments);

  /// @brief generate a catmull rom spline
  /// @param segments upper bound on the number of segments to divide the bezier
  /// curve into
  /// @param cp[0-3] control points
  static void catmull_rom(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2,
                          Vec2 cp3, u32 segments);

  /// @brief triangulate a stroke path, given the vertices for its points
  static void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vtx,
                                 Vec<u32> &idx, f32 thickness);

  /// @brief generate indices for a triangle list
  static void triangles(u32 first_vertex, u32 num_vertices, Vec<u32> &idx);

  /// @brief generate vertices for a quadratic bezier curve
  static void triangulate_convex(Vec<u32> &idx, u32 first_vertex,
                                 u32 num_vertices);
};

/// @param viewport viewport region of the surface the canvas is being drawn
/// onto.
/// @param extent extent of the surface the canvas is being drawn onto.
struct Canvas
{
  gfx::Format           format            = gfx::Format::Undefined;
  Vec2                  viewport_extent   = {};
  CRect                 current_clip      = {{0, 0}, {0, 0}};
  Vec<Vec2>             vertices          = {};
  Vec<u32>              indices           = {};
  Vec<u32>              ngon_index_counts = {};
  Vec<NgonParam>        ngon_params       = {};
  Vec<RRectParam>       rrect_params      = {};
  Vec<CanvasBlurParam>  blur_params       = {};
  Vec<CustomCanvasPass> custom_passes     = {};
  Vec<CanvasPassRun>    pass_runs         = {};

  constexpr f32 aspect_ratio() const
  {
    return (viewport_extent.y == 0) ? 0 :
                                      (viewport_extent.x / viewport_extent.y);
  }

  /// @brief generate a model-view-projection matrix for the object in the
  /// canvas space
  /// @param transform object-space transform
  /// @param center canvas-space position/center of the object
  /// @param extent extent of the object (used to scale from the [-1, +1]
  /// object-space coordinate)
  constexpr Mat4 mvp(Mat4 const &transform, Vec2 center, Vec2 extent) const
  {
    return
        // translate the object to its screen position, using (0, 0) as top
        translate3d(vec3((center / (0.5f * viewport_extent)) - 1, 0)) *
        // scale the object in the -1 to + 1 space
        scale3d(vec3(2 / viewport_extent, 1)) *
        // perform object-space transformation
        transform * scale3d(vec3(extent / 2, 1));
  }

  void init();

  void uninit();

  void begin(Vec2 viewport_extent);

  /// @brief Clear all commands and reset state
  void clear();

  void clip(CRect const &area);

  /// @brief render a circle
  void circle(ShapeDesc const &desc);

  /// @brief render a rectangle
  void rect(ShapeDesc const &desc);

  /// @brief render a rounded rectangle
  void rrect(ShapeDesc const &desc);

  /// @brief render a beveled rectangle
  void brect(ShapeDesc const &desc);

  /// @brief render a squircle (triangulation based)
  /// @param num_segments an upper bound on the number of segments to
  /// @param degree
  void squircle(ShapeDesc const &desc, f32 degree, u32 segments);

  void nine_slice(ShapeDesc const &desc, ScaleMode mode, Span<Vec2 const> uvs);

  /// @brief Render text using font atlases
  /// @param desc only desc.center, desc.transform, desc.tiling, and
  /// desc.sampler are used
  /// @param block Text Block to be rendered
  /// @param layout Layout of text block to be rendered
  /// @param style styling of the text block, contains styling for the runs and
  /// alignment of the block
  /// @param atlases font atlases
  /// @param clip clip rect for culling draw commands of the text block
  void text(ShapeDesc const &desc, TextBlock const &block,
            TextLayout const &layout, TextBlockStyle const &style,
            CRect const &clip = {{F32_MAX / 2, F32_MAX / 2},
                                 {F32_MAX, F32_MAX}});

  /// @brief Render Non-Indexed Triangles
  void triangles(ShapeDesc const &desc, Span<Vec2 const> vertices);

  /// @brief Render Indexed Triangles
  void triangles(ShapeDesc const &desc, Span<Vec2 const> vertices,
                 Span<u32 const> indices);

  /// @brief triangulate and render line
  void line(ShapeDesc const &desc, Span<Vec2 const> vertices);

  /// @brief perform a Canvas-space blur
  /// @param area region in the canvas to apply the blur to
  /// @param num_passes number of blur passes to execute, higher values result
  /// in blurrier results
  void blur(CRect const &area, u32 num_passes);

  /// @brief register a custom canvas pass to be executed in the render thread
  void custom(CustomCanvasPass pass);
};

}        // namespace ash
