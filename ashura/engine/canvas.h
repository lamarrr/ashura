#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

enum class LineCap : u8
{
  Butt   = 0,
  Round  = 1,
  Square = 2
};

enum class CanvasPassType : u8
{
  None          = 0,
  RRect         = 1,
  Blur          = 2,
  ConvexPolygon = 3,
  Line          = 4,
  Custom        = 5
};

struct Vertex
{
  Vec4 position = {0, 0, 0, 1};
  Vec4 tint     = {1, 1, 1, 1};
  Vec2 uv       = {0, 0};
};

// todo(lamarrr): only expose linear and arcs. the rest should use interpolation
// or other estimation methods via free functions.
// arc splines and curves
//
// Arc, Biarc, Arc Spline, BiArc Splines
//
// enum class CurveType : u8
// {
//   Linear          = 0,
//   Arc             = 1,
//   QuadraticBezier = 2,
//   CubicBezier     = 3,
//   CatmullRom      = 4
// };
//
// - automatic pass begin and ending
//
//
// each primitive is rendered out of order, the order is specified by the
// z-index.
//
// TODO(lamarrr): z-index sorted by radix sort, radix sort should come in:
// [integer key, elements or indices]
//
/// @todo(lamarrr): color
/// is this representation good?
/// we don't need uvs for arcs?
///
/// TODO(lamarrr): color space transformation. along with beziers?

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

struct CanvasPassRun
{
  CanvasPassType type   = CanvasPassType::None;
  u32            offset = 0;
  u32            count  = 0;
};

///@note Will be called from the render thread
struct CustomCanvasPassInfo
{
  Fn<void(void *, Renderer &)> encoder = to_fn([](void *, Renderer &) {});
  void                        *data    = nullptr;
};

struct CanvasSurface
{
  Vec2  viewport_size = {0, 0};
  Vec2U surface_size  = {0, 0};
  f32   dpi           = 0;

  /// todo(lamarrr): make calculation use 1-x, 1 form
  constexpr f32 aspect_ratio() const;
  constexpr f32 pixel_density() const;
};

///
/// @brief An Elliptical Arc in 3d space.
///
/// begin and end only describe the vector used to determine the angle of
/// inclination of the arc and the direction. i.e. norm(begin - center) is the
/// starting direction vector, and the angle between norm(begin - center) and
/// norm(end - center) is the degree of the arc.
///
///
///
///
struct Arc
{
  Vec4 center = {0, 0, 0, 1};
  Vec4 begin  = {0, 0, 0, 1};
  Vec4 end    = {0, 0, 0, 1};
  Vec2 radii  = {0, 0};
};

struct Canvas
{
  CanvasSurface             surface       = {};
  Vec<Vertex>               vertices      = {};
  Vec<RRectParam>           rrect_params  = {};
  Vec<BlurParam>            blur_params   = {};
  Vec<CustomCanvasPassInfo> custom_params = {};
  Vec<CanvasPassRun>        pass_runs     = {};
  Vec<i32>                  z_indexes     = {};

  void begin(CanvasSurface const &surface);

  void submit(Renderer &renderer);

  /// @brief Draw a circle or ellipse
  /// @param style
  /// @param radius x and y radius of the ellipse
  void circle(CanvasStyle const &style, Vec2 radii);

  /// @brief
  /// @param style
  /// @param extent
  void rect(CanvasStyle const &style, Vec2 extent);

  void rrect(CanvasStyle const &style, Vec2 extent, Vec4 radii);

  /// @brief draw a simple text with simple layout. Layout is uncached since it
  /// is considered to be cheap for the text and doesn't require complex text
  /// layout algorithms.
  /// @param style
  /// @param text
  void simple_text(CanvasStyle const &style, TextBlock const &block);

  /// @brief draw a complex multi-paragraph text with pre-computed layout
  /// @param style
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
  void arcs(CanvasStyle const &style, Span<Arc const> arcs);

  void arc_spline(CanvasStyle const &style, Span<Vertex const> vertices);

  void blur(CanvasStyle const &style);

  void custom(CanvasStyle const &style, CustomCanvasPassInfo const &pass);
};

}        // namespace ash
