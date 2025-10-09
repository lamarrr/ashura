/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/encoders.h"
#include "ashura/engine/pipeline.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocators.h"
#include "ashura/std/color.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

enum class CurveType : u8
{
  Line            = 0,
  Arc             = 1,
  QuadraticBezier = 2,
  CubicBezier     = 3
};

enum class LineCap : u8
{
  Square = 0,
  Round  = 1
};

namespace path
{

f32 cubic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2, f32x2 p3,
                       f32 precision = 4);

f32 quadratic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2,
                           f32 precision = 4);

f32 conic_subdivisions(f32 scale_factor, f32x2 p0, f32x2 p1, f32x2 p2, f32 w,
                       f32 precision = 4);

void rect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center);

void line(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1);

/// @brief Generate vertices for an arc
/// @param segments upper bound on the number of segments to divide the arc
/// into
/// @param start start angle
/// @param turn turn angle
void arc(Span<f32x2> vtx, f32x2 radii, f32x2 center, f32 start, f32 turn);

/// @brief Generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-2] control points
void bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2);

/// @brief Generate vertices for a quadratic bezier curve
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void cubic_bezier(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3);

/// @brief Generate a catmull rom spline
/// @param segments upper bound on the number of segments to divide the bezier
/// curve into
/// @param cp[0-3] control points
void catmull_rom(Span<f32x2> vtx, f32x2 cp0, f32x2 cp1, f32x2 cp2, f32x2 cp3);

/// @brief Generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
void circle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, u32 segments);

/// @brief Generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param degree number of degrees of the super-ellipse
void squircle(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32 degree,
              u32 segments);

/// @brief Generate vertices for a circle
/// @param segments upper bound on the number of segments to divide the circle
/// into
/// @param corner_radii border radius of each corner
void rrect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 corner_radii,
           u32 segments);

/// @brief Generate vertices of a bevel rect
/// @param vtx
/// @param slants each component represents the relative distance from the
/// corners of each bevel
void brect(Vec<f32x2> & vtx, f32x2 extent, f32x2 center, f32x4 slants);

/// @brief Triangulate a stroke path, given the vertices for its points
void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vtx,
                        Vec<u32> & idx, f32 thickness, LineCap cap);

void triangulate_stroke(Span<f32x2 const> points, Vec<f32x2> & vtx,
                        Vec<u16> & idx, f32 thickness, LineCap cap);

/// @brief Generate indices for a triangle list
void triangles(u32 first_vertex, u32 num_vertices, Vec<u32> & idx);

void triangles(u16 first_vertex, u16 num_vertices, Vec<u16> & idx);

void triangulate_convex(Vec<u32> & idx, u32 first_vertex, u32 num_vertices);

void triangulate_convex(Vec<u16> & idx, u16 first_vertex, u16 num_vertices);

void tesselate_curves(Vec<f32x2> & curves, Span<CurveType const> segment_types,
                      Span<u32 const>   subdivisions,
                      Span<f32x2 const> control_points);

};    // namespace path

/// @brief The base Canvas Shape Description

struct Shape
{
  /// @brief Object-world-space transform matrix
  f32x4x4 world_transform = f32x4x4::identity();

  /// @brief Nominal-space to uv-space transform matrix.
  /// uv-space ranges from [-0.5 to +0.5] on both axes.
  f32x4x4 uv_transform = f32x4x4::identity();

  /// @brief Center of the shape in world-space
  CRect area = {};

  /// @brief Extent of the shape's bounding-box in world-space
  f32x2 bbox_extent = {};

  /// @brief Radii of the shape
  f32x4 radii = {};

  /// @brief Shading method to use for the shape
  ShadeType shade = ShadeType::Flood;

  /// @brief Thickness of the feather to apply
  f32 feather = 0;

  /// @brief Linear Color gradient to use as tint
  ColorGradient tint = {};

  /// @brief Sampler to use in rendering the shape
  SamplerIndex sampler = SamplerIndex::LinearEdgeClampBlackFloat;

  /// @brief The set / group the textures are sourced from
  TextureSet texture_set = sampled_textures;

  /// @brief Texture to use in rendering the shape
  TextureIndex map = TextureIndex::White;

  /// @brief The sampler to use for sampling the signed distance map
  SamplerIndex sdf_sampler = SamplerIndex::LinearBorderClampBlackFloat;

  /// @brief Index of the signed distance map in the texture set
  TextureIndex sdf_map = TextureIndex::White;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

struct CornerColors
{
  /// @brief Top-left
  f32x4 tl = {};

  /// @brief Top-right
  f32x4 tr = {};

  /// @brief Bottom-left
  f32x4 bl = {};

  /// @brief Bottom-right
  f32x4 br = {};
};

struct MeshGradientInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  f32x4 radii = {};

  SdfShapeType shape = SdfShapeType::RRect;

  ShadeType shade = ShadeType::Flood;

  f32 feather = 0;

  CornerColors colors = {};

  f32x2 min = {};

  f32x2 max = {};

  f32 frequency = 5;

  f32 amplitude = 30;

  f32 time = 0.5F;

  SamplerIndex sdf_sampler = SamplerIndex::LinearBorderClampBlackFloat;

  TextureSet texture_set = sampled_textures;

  TextureIndex sdf_map = TextureIndex::White;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

struct TriangleSetInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  ColorGradient tint = {};

  SamplerIndex sampler = SamplerIndex::LinearBorderClampBlackFloat;

  gpu::CullMode cull_mode = gpu::CullMode::None;

  gpu::FrontFace front_face = gpu::FrontFace::CounterClockWise;

  TextureSet texture_set = sampled_textures;

  TextureIndex map = TextureIndex::White;

  Span<f32x2 const> vertices = {};

  Span<u32 const> indices = {};

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

enum class NineSliceScaling : u8
{
  Stretch = 0,
  Tile    = 1
};

// [ ] liquid metal shader

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
struct NineSliceInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  f32x4 radii = {};

  ColorGradient tint = {};

  NineSliceScaling scaling = NineSliceScaling::Stretch;

  // [ ] is this correct? right name, right specification?

  CRect top_left = {};

  CRect top_center = {};

  CRect top_right = {};

  CRect mid_left = {};

  CRect mid_center = {};

  CRect mid_right = {};

  CRect bottom_left = {};

  CRect bottom_center = {};

  CRect bottom_right = {};

  SamplerIndex sampler = SamplerIndex::LinearBorderClampBlackFloat;

  TextureSet texture_set = sampled_textures;

  TextureIndex map = TextureIndex::White;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

struct QuadInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  Span<u8 const> quad = {};

  PipelineVariantId variant = PipelineVariantId::Base;

  TextureSet texture_set = sampled_textures;

  TextureIndex map = TextureIndex::White;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

struct LineInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  f32 thickness = 1;

  LineCap cap = LineCap::Square;

  ColorGradient tint = {};

  Span<f32x2 const> vertices = {};

  Span<CurveType const> segment_types = {};

  Span<u32 const> subdivisions = {};

  SamplerIndex sampler = SamplerIndex::LinearBorderClampBlackFloat;

  TextureSet texture_set = sampled_textures;

  TextureIndex map = TextureIndex::White;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

struct FeatherInfo
{
  f32 width = 1;

  f32 inset = 0.5F;

  u32 tesselation = 32;

  ColorGradient tint = {};
};

struct PathInfo
{
  f32x4x4 world_transform = f32x4x4::identity();

  f32x4x4 uv_transform = f32x4x4::identity();

  CRect area = {};

  f32x2 bbox_extent = {};

  Span<f32x2 const> control_points = {};

  Span<CurveType const> segment_types = {};

  Span<u32 const> subdivisions = {};

  FillRule fill_rule = FillRule::EvenOdd;

  gpu::FrontFace front_face = gpu::FrontFace::CounterClockWise;

  ColorGradient tint = {};

  SamplerIndex sampler = SamplerIndex::LinearEdgeClampBlackFloat;

  TextureSet texture_set = sampled_textures;

  TextureIndex map = TextureIndex::White;

  Option<FeatherInfo> feather = none;

  constexpr CRect bbox() const
  {
    return CRect{area.center, bbox_extent};
  }

  constexpr f32x2 center() const
  {
    return area.center;
  }
};

typedef struct ICanvas * Canvas;

enum class CanvasState : u8
{
  Reset     = 0,
  Recording = 1,
  Recorded  = 2,
  Executed  = 3
};

/// @brief A normative clip rect that will cover the entire Canvas.
constexpr f32 MAX_CLIP_DISTANCE = 0xFF'FFFF;

inline constexpr CRect MAX_CLIP{.center = f32x2::splat(0),
                                .extent = f32x2::splat(MAX_CLIP_DISTANCE)};

struct ICanvas
{
  static constexpr u32 DEFAULT_NUM_IMAGE_SLOTS = 6;

  CanvasState state_;

  BitVec<u64> image_slots_;

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  /// @brief The viewport of the framebuffer this canvas will be targetting
  /// this is in the Framebuffer coordinates (Physical px coordinates)
  gpu::Viewport viewport_;

  /// @brief The viewport's local extent. This will scale to the viewport's extent.
  /// This is typically the screen's virtual size (Logical px coordinates).
  /// This distinction helps support high-density displays.
  f32x2 extent_;

  /// @brief The pixel size of the backing framebuffer (Physical px coordinates)
  u32x2 framebuffer_extent_;

  f32x2 framebuffer_uv_base_;

  /// @brief Aspect ratio of the viewport
  f32 aspect_ratio_;

  /// @brief The ratio of the viewport's framebuffer coordinate extent
  /// to the viewport's virtual extent
  f32 virtual_scale_;

  /// @brief The world to viewport transformation matrix for the shader (-1.0, 1.0)
  affinef32x4 world_to_ndc_;

  affinef32x4 ndc_to_viewport_;

  affinef32x4 viewport_to_fb_;

  affinef32x4 world_to_fb_;

  CRect clip_;

  SmallVec<CRect, 4, 0> clip_saves_;

  SmallVec<u32, 4, 0> color_saves_;

  SmallVec<Option<u32>, 4, 0> depth_stencil_saves_;

  SmallVec<Option<PipelineStencil>, 4, 0> stencil_op_saves_;

  Vec<Dyn<CanvasEncoder>> encoders_;

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
    clip_{MAX_CLIP},
    clip_saves_{allocator},
    color_saves_{allocator},
    depth_stencil_saves_{allocator},
    stencil_op_saves_{allocator},
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

  void set_clip(CRect const & area);

  /// @brief Register a custom canvas pass to be executed in the render thread
  template <Callable<GpuFramePlan> Lambda>
  void encode_pass_(Lambda && task)
  {
    auto enc = dyn<PassCanvasEncoder<Lambda>>(inplace, encoder_arena_,
                                              static_cast<Lambda &&>(task))
                 .unwrap();

    encoders_.push(cast<CanvasEncoder>(std::move(enc))).unwrap();
  }

  void encode_(SdfEncoder::Item const & item);

  void encode_(TriangleFillEncoder::Item const & item);

  void encode_(QuadEncoder::Item const & item);

  void render_(TextureSet const & texture_set, Shape const & shape,
               SdfShapeType type);

  void render_noise_(TextureSet const & texture_set, Shape const & shape,
                     SdfShapeType type);

  void render_(TextureSet const & texture_set, MeshGradientInfo const & info);

  void render_(TextureSet const & texture_set, TriangleSetInfo const & info,
               bool indexed);

  void render_(TextureSet const & texture_set, QuadInfo const & info);

  void render_(TextureSet const & texture_set, LineInfo const & info);

  void render_blur_(Shape const & info);

  // [ ] Handling Self-Intersection; Fill Rules; STC

  void render_paths_stencil_then_cover_(Span<PathInfo const> paths,
                                        bool                 has_overlaps);

  void render_paths_bezier_stencil_(Span<PathInfo const> paths,
                                    bool                 has_overlaps);

  void render_paths_vector_feathering_(Span<PathInfo const> paths,
                                       bool                 has_overlaps);

  void render_paths_(Span<PathInfo const> paths, bool has_overlaps);

  ICanvas & push_clip();

  ICanvas & pop_clip();

  ICanvas & push_color();

  ICanvas & pop_color();

  ICanvas & push_depth_stencil();

  ICanvas & pop_depth_stencil();

  ICanvas & push_stencil_op();

  ICanvas & pop_stencil_op();

  /// @brief Render a circle
  ICanvas & circle(Shape const & shape);

  /// @brief Render a rectangle
  ICanvas & rect(Shape const & shape);

  /// @brief Render a rounded rectangle
  ICanvas & rrect(Shape const & shape);

  /// @brief Render a squircle (triangulation based)
  ICanvas & squircle(Shape const & shape);

  /// @brief Render an SDF texture
  ICanvas & sdf_map(Shape const & shape);

  /// @brief Render a noise-grained shape
  ICanvas & noise_tint(Shape const & shape);

  /// @brief Render a mesh-gradient rrect
  ICanvas & mesh_gradient(MeshGradientInfo const & info);

  /// @brief Draw a nine-sliced image
  ICanvas & nine_slice(NineSliceInfo const & info);

  /// @brief Render indexed triangles
  ICanvas & indexed_triangles(TriangleSetInfo const & info);

  /// @brief Render un-indexed triangles
  ICanvas & unindexed_triangles(TriangleSetInfo const & info);

  /// @brief Render a segmented line
  ICanvas & line(LineInfo const & info);

  /// @brief Perform a canvas-space blur
  ICanvas & blur(Shape const & info);

  /// @brief Render a quad using a custom shader
  ICanvas & quad(QuadInfo const & info);

  /// @brief Render a vector path
  ICanvas & paths(Span<PathInfo const> info, bool has_overlaps);

  /// @brief Render a text block
  ICanvas & text(TextRenderInfo const & info, TextPlacement const & placement);

  // [ ] implement
  ICanvas & bloom();

  // [ ] implement
  ICanvas & crt();
};

}    // namespace ash
