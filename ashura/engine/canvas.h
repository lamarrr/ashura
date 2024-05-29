#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct Vertex
{
  Vec4 position = {0, 0, 0, 1};
  Vec4 tint     = {1, 1, 1, 1};
  Vec2 uv       = {0, 0};
};

enum class LineCap : u8
{
  Butt   = 0,
  Round  = 1,
  Square = 2
};

// todo(lamarrr): only expose linear and arcs. the rest should use interpolation
// or other estimation methods via free functions.
// arc splines and curves
//
enum class CurveType : u8
{
  Linear          = 0,
  Arc             = 1,
  QuadraticBezier = 2,
  CubicBezier     = 3,
  CatmullRom      = 4
};

/// @brief
/// @param transform transform from from model to view space
/// @param scissor_offset, scissor_extent in surface pixel coordinates
struct CanvasStyle
{
  i32     z_index   = 0;
  Vec4    position  = {0, 0, 0, 1};
  bool    stroke    = false;
  LineCap line_cap  = LineCap::Butt;
  f32     thickness = 1.0f;
  Vec4    tint[4]   = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
  u32     texture   = 0;
  Vec2    uv[2]     = {{0, 0}, {1, 1}};
  f32     edge_smoothness = 0.0015F;
  Mat4    transform       = Mat4::identity();
  Vec2U   scissor_offset  = {0, 0};
  Vec2U   scissor_extent  = {U32_MAX, U32_MAX};
};

// to support custom param, need to store data relating to it  and the encoder
// for it to be forwarded to when rendering
enum class CanvasPassType : u8
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
  i32            z_index = 0;
  CanvasPassType type    = CanvasPassType::None;
  u32            offset  = 0;
  u32            count   = 0;
};

// TODO(lamarrr): might need screen scaling context
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
// TODO(lamarrr): each primitive is rendered out of order, the order is
// specified by the z-index.
//
// z-index sorted by radix sort, radix sort should come in: [integer key,
// elements or indices]
//
struct Canvas
{
  // screen or render target params
  // viewport and surface config
  // zoom factor: used for tessellation? or use transform?
  Vec2  viewport_size = {0, 0};
  Vec2U surface_size  = {0, 0};

  Vec<Vertex>               vertices      = {};
  Vec<RRectParam>           rrect_params  = {};
  Vec<BlurParam>            blur_params   = {};
  Vec<CustomCanvasPassInfo> custom_params = {};
  Vec<CanvasPassRun>        pass_runs     = {};

  void begin();

  void end();

  /// @brief Draw a circle or ellipse
  /// @param style
  /// @param center
  /// @param radius x and y radius of the ellipse
  void circle(CanvasStyle const &style, Vec2 radii);

  /// @brief
  /// @param style
  /// @param center
  /// @param extent
  void rect(CanvasStyle const &style, Vec2 extent);

  void rrect(CanvasStyle const &style, Vec2 extent, Vec4 radii);

  void arc(CanvasStyle const &style, f32 radius, f32 begin, f32 end);

  /// @brief draw a simple text with simple layout. Layout is uncached since it
  /// is considered to be cheap for the text and doesn't require complex text
  /// layout algorithms.
  /// @param style
  /// @param baseline
  /// @param text
  void simple_text(CanvasStyle const &style, TextBlock const &block);

  /// @brief draw a complex multi-paragraph text with pre-computed layout
  /// @param style
  /// @param center
  /// @param block
  /// @param layout
  void text(CanvasStyle const &style, TextBlock const &block,
            TextLayout const &layout);

  /// @brief the vertices need not be closed
  /// @param style
  /// @param vertices minimum of 3 vertices
  void convex_polygon(CanvasStyle const &style, Span<Vertex const> vertices);

  /// @brief
  /// @param type
  /// @param vertices vertices defining the control points of the spline
  /// @param subdivisions number of subdivisions to use for tesselation
  void curve(CanvasStyle const &style, CurveType type,
             Span<Vertex const> vertices, Span<Vec3 const> control_points,
             u32 subdivisions);

  void spline(CanvasStyle const &style, CurveType type,
              Span<Vertex const> vertices, u32 subdivisions);

  void blur(CanvasStyle const &style, Vec2U offset, Vec2U extent);

  void custom(CanvasStyle const &style, CustomCanvasPassInfo const &pass);
};

}        // namespace ash
