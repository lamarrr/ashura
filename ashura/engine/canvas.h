#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/text.h"
#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct Vertex2d
{
  Vec2 position = {0, 0};
  Vec2 uv       = {0, 0};
  Vec4 tint     = {1, 1, 1, 1};
};

struct Vertex
{
  Vec4 position = {0, 0, 0, 1};
  Vec2 uv       = {0, 0};
  Vec4 tint     = {1, 1, 1, 1};
};

struct PathStyle
{
  bool  stroke      = false;
  f32   thickness   = 1.0f;
  Vec4  tint[4]     = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
  u32   texture     = 0;
  Font  font        = nullptr;
  u32   font_height = 16;
  Vec2  uv[2]       = {{0, 0}, {1, 1}};
  f32   edge_smoothness = 0.0015F;
  Mat4  transform       = Mat4::identity();
  Vec2U scissor_offset  = {0, 0};
  Vec2U scissor_extent  = {U32_MAX, U32_MAX};
};

// to support custom param, need to store data relating to it  and the encoder
// for it to be forwarded to when rendering
enum class CanvasPass : u8
{
  None          = 0,
  RRect         = 1,
  Blur          = 2,
  ConvexPolygon = 3,
  Line          = 4,
  Custom        = 5
};

struct CanvasPassRun
{
  CanvasPass pass   = CanvasPass::None;
  u32        offset = 0;
  u32        count  = 0;
};

enum class CurveType : u8
{
  Line            = 0,
  QuadraticBezier = 1,
  CubicBezier     = 2,
  CatmullRom      = 3
};

// TODO(lamarrr): might need screen context
// viewport extent should be known ahead of time
//
// screen frame buffer
//
struct CustomCanvasPassInfo
{
  Fn<void(void *, Renderer &)> encoder = to_fn([](void *, Renderer &) {});
  void                        *data    = nullptr;
};

//
// # REQUIREMENTS
// - Automatic batching and pass reordering
// - offscreen rendering?
// - Forwarding to the renderer
// - re-using existing shaders and renderers when possible
// - intermixing of 2d and 3d renderers---------- not possible
// - automatic pass begin and ending
// - allow using of custom renderers and shaders
// - render encoders need to be stable and part of the tree? or maybe give a 3d
// context and 2d context
// - offscreen gui rendering
//
//
//
// use the passes in the renderer to achieve desired effect
//
// todo(lamarrr): custom shaders?
//
// drawing layer image to this canvas? i.e. offscreen rendered sourced from
// another pass. maybe images should support that use case.
//
//
// todo(lamarrr): convex polygons have the same repeated set of indices, we can
// use a base offset in the shader and give the shader the number of points it
// needs to triangulate instead of an index buffer.
//
//
// Nice to have: batching of non-overlapping rects, maybe with hints provided by
// the owner? z-indexes!!!
//
//
//
//
//
//
struct Canvas
{
  // screen or render target params
  // viewport and surface config
  Vec2  viewport_size = {0, 0};
  Vec2U surface_size  = {0, 0};

  Vec<Vertex2d>             vertices      = {};
  Vec<RRectParam>           rrect_params  = {};
  Vec<BlurParam>            blur_params   = {};
  Vec<CustomCanvasPassInfo> custom_params = {};
  Vec<CanvasPassRun>        pass_runs     = {};

  void begin()
  {
    // Canvas params for transform from px to -1 +1 relative viewport space
    // for rrect, transform needs to transform from -1 +1 to px space
    // rrect_params.clear();
    // blur_params.clear();
  }

  void end()
  {
    // offload to gpu, set up passes, render
  }

  void circle(PathStyle const &style, Vec2 center, Vec2 radius);

  void rect(PathStyle const &style, Vec2 center, Vec2 extent);

  void rrect(PathStyle const &style, Vec2 center, Vec2 extent, Vec4 radii);

  void arc(PathStyle const &style, Vec2 center, f32 radius, f32 angle_begin,
           f32 angle_end);

  /// @brief draw a simple text with simple layout. Layout is uncached since it
  /// is considered to be cheap for the text and doesn't require complex text
  /// layout algorithms.
  /// @param style
  /// @param baseline
  /// @param text
  void simple_text(PathStyle const &style, Vec2 baseline,
                   Span<char const> text);

  /// @brief draw a complex multi-paragraph text with pre-computed layout
  /// @param style
  /// @param center
  /// @param block
  /// @param layout
  void text(PathStyle const &style, Vec2 center, TextBlock const &block,
            TextLayout const &layout);

  /// @brief the vertices need not be closed
  /// @param style
  /// @param vertices minimum of 3 vertices
  void convex_polygon(PathStyle const &style, Span<Vertex const> vertices);

  void curve(CurveType type, Span<Vertex const> vertices);

  void blur(Vec2U offset, Vec2U extent);

  template <typename T>
  void custom(CustomCanvasPassInfo const &pass, T const &param)
  {
    // alloc memory
  }

  void custom(CustomCanvasPassInfo const &pass);
};

}        // namespace ash
