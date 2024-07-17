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
  Vec4          border_radii    = {0, 0, 0, 0};
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
  static void arc(Vec<Vec2> &vtx, u32 segments, f32 start, f32 stop);
  static void circle(Vec<Vec2> &vtx, u32 segments);
  static void squircle(Vec<Vec2> &vtx, u32 segments);
  static void rrect(Vec<Vec2> &vtx, u32 segments, Vec4 border_radii);
  static void brect(Vec<Vec2> &vtx, Vec4 slants);
  static void bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                     Vec2 cp2);
  static void cubic_bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                           Vec2 cp2, Vec2 cp3);
  static void catmull_rom(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                          Vec2 cp2, Vec2 cp3);
  static void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vtx,
                                 Vec<u32> &idx, f32 thickness);
  static void triangles(Span<Vec2 const> points, Vec<u32> &idx);
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

  constexpr Mat4 mvp(Mat4 const &transform, Vec2 center, Vec2 extent) const
  {
    return
        // translate the object to its screen position, using (0, 0) as top
        translate3d(to_vec3((center / (0.5f * viewport_extent)) - 1, 0)) *
        // scale the object in the -1 to + 1 space
        scale3d(to_vec3(2 / viewport_extent, 1)) *
        // perform object-space transformation
        transform * scale3d(to_vec3(extent / 2, 1));
  }

  void init();

  void uninit();

  void begin(Vec2 viewport_extent);

  void clear();

  void clip(CRect const &area);

  void circle(ShapeDesc const &desc);

  void rect(ShapeDesc const &desc);

  void rrect(ShapeDesc const &desc);

  void nine_slice(ShapeDesc const &desc, ScaleMode mode, Span<Vec2 const> uvs);

  void text(ShapeDesc const &desc, TextBlock const &block,
            TextLayout const &layout, TextBlockStyle const &style,
            Span<FontAtlasResource const *const> atlases);

  void triangles(ShapeDesc const &desc, Span<Vec2 const> vertices);

  void triangles(ShapeDesc const &desc, Span<Vec2 const> vertices,
                 Span<u32 const> indices);

  void line(ShapeDesc const &desc, Span<Vec2 const> vertices);

  void blur(CRect const &area, u32 num_passes);

  void custom(CustomCanvasPass pass);
};

}        // namespace ash
