/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/pipeline.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/std/allocator.h"
#include "ashura/std/math.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct ICanvasEncoder * CanvasEncoder;

enum class CanvasEncoderType : u32
{
  Sdf           = 0,
  Quad          = 1,
  TriangleFill  = 2,
  FillStencil   = 3,
  BezierStencil = 4,
  Pbr           = 5,
  Custom        = 6,
  Other         = 7
};

struct ICanvasEncoder
{
  CanvasEncoderType type_;

  constexpr ICanvasEncoder(CanvasEncoderType type) : type_{type}
  {
  }

  constexpr CanvasEncoderType type() const
  {
    return type_;
  }

  constexpr virtual void operator()(GpuFramePlan plan) = 0;
};

struct CustomCanvasEncoder : ICanvasEncoder
{
  CustomCanvasEncoder() : ICanvasEncoder{CanvasEncoderType::Custom}
  {
  }

  virtual void operator()(GpuFramePlan) override
  {
  }

  ~CustomCanvasEncoder() = default;
};

template <Callable<GpuFramePlan> Lambda>
struct PassCanvasEncoder final : CustomCanvasEncoder
{
  template <typename... Args>
  PassCanvasEncoder(Args &&... args) :
    CustomCanvasEncoder{},
    lambda_{static_cast<Args &&>(args)...}
  {
  }

  virtual void operator()(GpuFramePlan plan) override
  {
    lambda_(plan);
  }

  ~PassCanvasEncoder() = default;

  Lambda lambda_;
};

struct SdfEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32                     color;
    Option<u32>             depth_stencil;
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          shape;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u8> shapes_;

  PipelineVariantId variant_;

  explicit SdfEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Sdf},
    depth_stencil_{item.depth_stencil},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    shapes_{allocator},
    variant_{item.variant}
  {
    push_(item.shape);
  }

  SdfEncoder(SdfEncoder const &)             = delete;
  SdfEncoder(SdfEncoder &&)                  = default;
  SdfEncoder & operator=(SdfEncoder const &) = delete;
  SdfEncoder & operator=(SdfEncoder &&)      = default;
  ~SdfEncoder()                              = default;

  void push_(Span<u8 const> shape)
  {
    shapes_.extend(shape).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{color_, depth_stencil_, stencil_op_, scissor_, viewport_,
            texture_set_, world_to_ndc_, variant_},
      Tuple{item.color, item.depth_stencil, item.stencil_op, item.scissor,
            item.viewport, item.texture_set, item.world_to_ndc, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.shape);

    return true;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct QuadEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32                     color;
    Option<u32>             depth_stencil;
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          quad;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u8> quads_;

  PipelineVariantId variant_;

  explicit QuadEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Quad},
    depth_stencil_{item.depth_stencil},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    quads_{allocator},
    variant_{item.variant}
  {
    push_(item.quad);
  }

  QuadEncoder(QuadEncoder const &)             = delete;
  QuadEncoder(QuadEncoder &&)                  = default;
  QuadEncoder & operator=(QuadEncoder const &) = delete;
  QuadEncoder & operator=(QuadEncoder &&)      = default;
  ~QuadEncoder()                               = default;

  void push_(Span<u8 const> quad)
  {
    quads_.extend(quad).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{color_, depth_stencil_, stencil_op_, scissor_, viewport_,
            texture_set_, world_to_ndc_, variant_},
      Tuple{item.color, item.depth_stencil, item.stencil_op, item.scissor,
            item.viewport, item.texture_set, item.world_to_ndc, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.quad);

    return true;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct TriangleFillEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32                     color;
    Option<u32>             depth_stencil;
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::CullMode           cull_mode;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          set;
    Span<u8 const>          vertices;
    Span<u8 const>          indices;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::CullMode cull_mode_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_counts_;

  Vec<u8> sets_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  PipelineVariantId variant_;

  explicit TriangleFillEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::TriangleFill},
    depth_stencil_{item.depth_stencil},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    cull_mode_{item.cull_mode},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    index_counts_{allocator},
    sets_{allocator},
    vertices_{allocator},
    indices_{allocator},
    variant_{item.variant}
  {
    push_(item.set, item.vertices, item.indices);
  }

  TriangleFillEncoder(TriangleFillEncoder const &)             = delete;
  TriangleFillEncoder(TriangleFillEncoder &&)                  = default;
  TriangleFillEncoder & operator=(TriangleFillEncoder const &) = delete;
  TriangleFillEncoder & operator=(TriangleFillEncoder &&)      = default;
  ~TriangleFillEncoder()                                       = default;

  void push_(Span<u8 const> set, Span<u8 const> vertices,
             Span<u8 const> indices)
  {
    index_counts_.push(size32(indices)).unwrap();
    sets_.extend(set).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{color_, depth_stencil_, stencil_op_, scissor_, viewport_,
            cull_mode_, texture_set_, world_to_ndc_, variant_},
      Tuple{item.color, item.depth_stencil, item.stencil_op, item.scissor,
            item.viewport, cull_mode_, item.texture_set, item.world_to_ndc,
            item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.set, item.vertices, item.indices);

    return true;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct FillStencilEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32            depth_stencil;
    u32            write_mask;
    RectU          scissor;
    gpu::Viewport  viewport;
    FillRule       fill_rule;
    bool           invert;
    f32x4x4        world_to_ndc;
    f32x4x4        world_transform;
    Span<u8 const> vertices;
    Span<u8 const> indices;
  };

  u32 depth_stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<u8> world_transforms_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  Vec<u32> index_counts_;

  Vec<u32> write_masks_;

  explicit FillStencilEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::FillStencil},
    depth_stencil_{item.depth_stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    world_transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    index_counts_{allocator},
    write_masks_{allocator}
  {
    push_(item.world_transform, item.vertices, item.indices, item.write_mask);
  }

  FillStencilEncoder(FillStencilEncoder const &)             = delete;
  FillStencilEncoder(FillStencilEncoder &&)                  = default;
  FillStencilEncoder & operator=(FillStencilEncoder const &) = delete;
  FillStencilEncoder & operator=(FillStencilEncoder &&)      = default;
  ~FillStencilEncoder()                                      = default;

  void push_(f32x4x4 const & world_transform, Span<u8 const> vertices,
             Span<u8 const> indices, u32 write_mask)
  {
    world_transforms_.extend(as_u8_span(world_transform)).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    index_counts_.push(size32(indices)).unwrap();
    write_masks_.push(write_mask).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable =
      obj::byte_eq(Tuple{depth_stencil_, scissor_, viewport_, fill_rule_,
                         invert_, world_to_ndc_},
                   Tuple{depth_stencil_, item.scissor, item.viewport,
                         item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.world_transform, item.vertices, item.indices, item.write_mask);

    return true;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct BezierStencilEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32             depth_stencil;
    u32             write_mask;
    RectU           scissor;
    gpu::Viewport   viewport;
    FillRule        fill_rule;
    bool            invert;
    f32x4x4         world_to_ndc;
    f32x4x4         world_transform;
    Span<u8 const>  vertices;
    Span<u8 const>  indices;
    Span<u8 const>  regions;
    Span<u32 const> region_index_counts;
  };

  u32 depth_stencil_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<u8> world_transforms_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  Vec<u8> regions_;

  Vec<u32> region_index_counts_;

  Vec<u32> write_masks_;

  explicit BezierStencilEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::BezierStencil},
    depth_stencil_{item.depth_stencil},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    world_transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    regions_{allocator},
    region_index_counts_{allocator},
    write_masks_{allocator}
  {
    push_(item.world_transform, item.vertices, item.indices, item.regions,
          item.region_index_counts, item.write_mask);
  }

  BezierStencilEncoder(BezierStencilEncoder const &)             = delete;
  BezierStencilEncoder(BezierStencilEncoder &&)                  = default;
  BezierStencilEncoder & operator=(BezierStencilEncoder const &) = delete;
  BezierStencilEncoder & operator=(BezierStencilEncoder &&)      = default;
  ~BezierStencilEncoder()                                        = default;

  void push_(f32x4x4 const & world_transform, Span<u8 const> vertices,
             Span<u8 const> indices, Span<u8 const> regions,
             Span<u32 const> region_index_counts, u32 write_mask)
  {
    world_transforms_.extend(as_u8_span(world_transform)).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    regions_.extend(regions).unwrap();
    region_index_counts_.extend(region_index_counts).unwrap();
    write_masks_.push(write_mask).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable =
      obj::byte_eq(Tuple{depth_stencil_, scissor_, viewport_, fill_rule_,
                         invert_, world_to_ndc_},
                   Tuple{depth_stencil_, item.scissor, item.viewport,
                         item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.world_transform, item.vertices, item.indices, item.regions,
          item.region_index_counts, item.write_mask);

    return true;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct PbrEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32                     color;
    Option<u32>             depth_stencil;
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::PolygonMode        polygon_mode;
    TextureSet              texture_set;
    GpuBufferSpan           vertices;
    GpuBufferSpan           indices;
    u32                     num_indices;
    Span<u8 const>          material;
    Span<u8 const>          lights;
    gpu::CullMode           cull_mode;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::PolygonMode polygon_mode_;

  TextureSet texture_set_;

  GpuBufferSpan vertices_;

  GpuBufferSpan indices_;

  u32 num_indices_;

  Vec<u8> material_;

  Vec<u8> lights_;

  gpu::CullMode cull_mode_;

  PipelineVariantId variant_;

  explicit PbrEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Pbr},
    color_{item.color},
    depth_stencil_{item.depth_stencil},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    polygon_mode_{item.polygon_mode},
    texture_set_{item.texture_set},
    vertices_{item.vertices},
    indices_{item.indices},
    num_indices_{item.num_indices},
    material_{allocator},
    lights_{allocator},
    cull_mode_{item.cull_mode},
    variant_{item.variant}
  {
    material_.extend(item.material).unwrap();
    lights_.extend(item.lights).unwrap();
  }

  PbrEncoder(PbrEncoder const &)             = delete;
  PbrEncoder(PbrEncoder &&)                  = default;
  PbrEncoder & operator=(PbrEncoder const &) = delete;
  PbrEncoder & operator=(PbrEncoder &&)      = default;
  ~PbrEncoder()                              = default;

  virtual void operator()(GpuFramePlan plan) override;
};

}    // namespace ash
