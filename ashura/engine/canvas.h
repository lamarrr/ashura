#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

enum class CanvasPassType : u8
{
  None   = 0,
  RRect  = 1,
  Blur   = 2,
  Ngon   = 3,
  Custom = 4
};

struct NgonDrawCommand
{
  u32 num_indices = 0;
};

/// @brief
/// @param scissor_offset, scissor_extent in surface pixel coordinates
struct ShapeDesc
{
  Vec2  center       = {0, 0};
  Vec2  extent       = {0, 0};
  Vec4  border_radii = {0, 0, 0, 0};
  f32   stroke       = 0.0f;
  f32   thickness    = 1.0f;
  Vec4  tint[4]      = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
  u32   texture      = 0;
  Vec2  uv[2]        = {{0, 0}, {1, 1}};
  f32   edge_smoothness = 0.0015F;
  Mat4  transform       = Mat4::identity();
  Vec2U scissor_offset  = {0, 0};
  Vec2U scissor_extent  = {U32_MAX, U32_MAX};
};

struct CanvasPassRun
{
  CanvasPassType type  = CanvasPassType::None;
  u32            first = 0;
  u32            count = 0;
};

///@note Will be called from the render thread
struct CustomCanvasPassInfo
{
  Fn<void(void *, Renderer &)> encoder = to_fn([](void *, Renderer &) {});
  void                        *data    = nullptr;
};

struct CanvasSurface
{
  Vec2  viewport = {0, 0};
  Vec2U extent   = {0, 0};

  constexpr f32 aspect_ratio() const
  {
    return (viewport.y == 0) ? 0 : (viewport.x / viewport.y);
  }

  constexpr Vec2 pixel_density() const
  {
    return Vec2{extent.x / viewport.x, extent.y / viewport.y};
  }

  constexpr Mat4 mvp(Vec2 center, Vec2 object_extent, Mat4 transform) const
  {
    return affine_scale3d(Vec3{1 / viewport.x, 1 / viewport.y, 0}) *
           affine_translate3d(Vec3{center.x, center.y, 0}) * transform *
           affine_scale3d(Vec3{object_extent.x / 2, object_extent.y / 2, 1});
  }
};

/// @brief all points are stored in the [-1, +1] range, all arguments must be
/// normalized to same range.
struct Path
{
  static void rect(Vec<Vec2> &vtx);
  static void arc(Vec<Vec2> &vtx, u32 segments, f32 start, f32 stop);
  static void circle(Vec<Vec2> &vtx, u32 segments);
  static void rrect(Vec<Vec2> &vtx, u32 segments, Vec4 border_radii);
  static void brect(Vec<Vec2> &vtx, Vec4 slants);
  static void bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                     Vec2 cp2);
  static void cubic_bezier(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                           Vec2 cp2, Vec2 cp3);
  static void catmull_rom(Vec<Vec2> &vtx, u32 segments, Vec2 cp0, Vec2 cp1,
                          Vec2 cp2, Vec2 cp3);

  /// @param points needs to be normalized to [-1, +1] range
  static void triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vtx,
                                 Vec<u32> &idx, f32 thickness);

  /// @param points needs to be normalized to [-1, +1] range
  static void triangulate_ngon(Span<Vec2 const> points, Vec<Vec2> &vtx,
                               Vec<u32> &idx);
};

struct Canvas
{
  CanvasSurface             surface            = {};
  Vec<Vec2>                 vertices           = {};
  Vec<u32>                  indices            = {};
  Vec<NgonDrawCommand>      ngon_draw_commands = {};
  Vec<NgonParam>            ngon_params        = {};
  Vec<RRectParam>           rrect_params       = {};
  Vec<CustomCanvasPassInfo> custom_params      = {};
  Vec<CanvasPassRun>        pass_runs          = {};

  void init();

  void uninit();

  void begin(CanvasSurface const &surface);

  /// @brief offload to gpu, set up passes, render. called on render thread.
  void submit(Renderer &renderer);

  void circle(ShapeDesc const &desc);

  void rect(ShapeDesc const &desc);

  void rrect(ShapeDesc const &desc);

  void text_backgrounds_(ShapeDesc const &desc, TextBlock const &block,
                         TextLayout const &layout);

  void text_underlines_(ShapeDesc const &desc, TextBlock const &block,
                        TextLayout const &layout);

  void text_strikethroughs_(ShapeDesc const &desc, TextBlock const &block,
                            TextLayout const &layout);

  void glyph_shadows_(ShapeDesc const &desc, TextBlock const &block,
                      TextLayout const &layout);

  void glyphs_(ShapeDesc const &desc, TextBlock const &block,
               TextLayout const &layout);

  /// @brief draw a complex and  layout text with pre-computed layout
  void text(ShapeDesc const &desc, TextBlock const &block,
            TextLayout const &layout);

  ///@brief convex irregular polygon
  void ngon(ShapeDesc const &desc, Span<Vec2 const> vertices);

  void line(ShapeDesc const &desc, Span<Vec2 const> vertices);

  void blur(ShapeDesc const &desc);

  void custom(ShapeDesc const &desc, CustomCanvasPassInfo const &pass);
};

}        // namespace ash
