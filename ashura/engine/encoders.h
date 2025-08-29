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
  Ngon          = 2,
  FillStencil   = 3,
  BezierStencil = 4,
  Pbr           = 5,
  Other         = 6
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

  constexpr virtual bool push(void const * item) = 0;

  constexpr virtual ~ICanvasEncoder() = default;
};

template <Callable<GpuFramePlan> Lambda>
struct CustomCanvasEncoder final : ICanvasEncoder
{
  template <typename... Args>
  CustomCanvasEncoder(Args &&... args) :
    ICanvasEncoder{CanvasEncoderType::Other},
    lambda_{static_cast<Args &&>(args)...}
  {
  }

  virtual void operator()(GpuFramePlan plan) override
  {
    lambda_(plan);
  }

  virtual bool push(void const *) override
  {
    return false;
  }

  virtual ~CustomCanvasEncoder() override = default;

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
    f32x4x4                 transform;
    Span<u8 const>          material;
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

  Vec<f32x4x4> transforms_;

  Vec<u8> materials_;

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
    transforms_{allocator},
    materials_{allocator},
    variant_{item.variant}
  {
    push_(item.shape, item.transform, item.material);
  }

  SdfEncoder(SdfEncoder const &)             = delete;
  SdfEncoder(SdfEncoder &&)                  = default;
  SdfEncoder & operator=(SdfEncoder const &) = delete;
  SdfEncoder & operator=(SdfEncoder &&)      = default;
  virtual ~SdfEncoder() override             = default;

  void push_(Span<u8 const> shape, f32x4x4 const & transform,
             Span<u8 const> material)
  {
    shapes_.extend(shape).unwrap();
    transforms_.push(transform).unwrap();
    materials_.extend(material).unwrap();
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

    push_(item.shape, item.transform, item.material);

    return true;
  }

  virtual bool push(void const * object) override
  {
    return push(*static_cast<Item const *>(object));
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
    f32x4x4                 quad;
    f32x4x4                 transform;
    Span<u8 const>          material;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> quads_;

  Vec<f32x4x4> transforms_;

  Vec<u8> materials_;

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
    transforms_{allocator},
    materials_{allocator},
    variant_{item.variant}
  {
    push_(item.quad, item.transform, item.material);
  }

  QuadEncoder(QuadEncoder const &)             = delete;
  QuadEncoder(QuadEncoder &&)                  = default;
  QuadEncoder & operator=(QuadEncoder const &) = delete;
  QuadEncoder & operator=(QuadEncoder &&)      = default;
  virtual ~QuadEncoder() override              = default;

  void push_(f32x4x4 const & quad, f32x4x4 const & transform,
             Span<u8 const> material)
  {
    quads_.push(quad).unwrap();
    transforms_.push(transform).unwrap();
    materials_.extend(material).unwrap();
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

    quads_.push(item.quad).unwrap();
    transforms_.push(item.transform).unwrap();
    materials_.extend(Span{&item.material, 1}.as_u8()).unwrap();

    return true;
  }

  virtual bool push(void const * object) override
  {
    return push(*static_cast<Item const *>(object));
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct NgonEncoder final : ICanvasEncoder
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
    f32x4x4                 transform;
    Span<f32x2 const>       vertices;
    Span<u32 const>         indices;
    Span<u8 const>          material;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_counts_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<u8> materials_;

  PipelineVariantId variant_;

  explicit NgonEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Ngon},
    depth_stencil_{item.depth_stencil},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    index_counts_{allocator},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    materials_{allocator},
    variant_{item.variant}
  {
    push_(item.transform, item.vertices, item.indices, item.material);
  }

  NgonEncoder(NgonEncoder const &)             = delete;
  NgonEncoder(NgonEncoder &&)                  = default;
  NgonEncoder & operator=(NgonEncoder const &) = delete;
  NgonEncoder & operator=(NgonEncoder &&)      = default;
  virtual ~NgonEncoder() override              = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Span<u8 const> material)
  {
    index_counts_.push(size32(indices)).unwrap();
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    materials_.extend(material).unwrap();
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

    push_(item.transform, item.vertices, item.indices, item.material);

    return true;
  }

  virtual bool push(void const * object) override
  {
    return push(*static_cast<Item const *>(object));
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct FillStencilEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32               depth_stencil;
    u32               write_mask;
    RectU             scissor;
    gpu::Viewport     viewport;
    FillRule          fill_rule;
    bool              invert;
    f32x4x4           world_to_ndc;
    f32x4x4           transform;
    Span<f32x2 const> vertices;
    Span<u32 const>   indices;
    Span<u32 const>   index_counts;
  };

  u32 depth_stencil_;

  u32 write_mask_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<u32> index_counts_;

  explicit FillStencilEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::FillStencil},
    depth_stencil_{item.depth_stencil},
    write_mask_{item.write_mask},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    index_counts_{allocator}
  {
    push_(item.transform, item.vertices, item.indices, item.index_counts);
  }

  FillStencilEncoder(FillStencilEncoder const &)             = delete;
  FillStencilEncoder(FillStencilEncoder &&)                  = default;
  FillStencilEncoder & operator=(FillStencilEncoder const &) = delete;
  FillStencilEncoder & operator=(FillStencilEncoder &&)      = default;
  virtual ~FillStencilEncoder() override                     = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Span<u32 const> index_counts)
  {
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    index_counts_.extend(index_counts).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{depth_stencil_, write_mask_, scissor_, viewport_, fill_rule_,
            invert_, world_to_ndc_},
      Tuple{depth_stencil_, item.write_mask, item.scissor, item.viewport,
            item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.transform, item.vertices, item.indices, item.index_counts);

    return true;
  }

  virtual bool push(void const * object) override
  {
    return push(*static_cast<Item const *>(object));
  }

  virtual void operator()(GpuFramePlan plan) override;
};

struct BezierStencilEncoder final : ICanvasEncoder
{
  struct Item
  {
    u32                               depth_stencil;
    u32                               write_mask;
    RectU                             scissor;
    gpu::Viewport                     viewport;
    FillRule                          fill_rule;
    bool                              invert;
    f32x4x4                           world_to_ndc;
    f32x4x4                           transform;
    Span<f32x2 const>                 vertices;
    Span<u32 const>                   indices;
    Span<shader::BezierRegions const> regions;
    Span<u32 const>                   region_index_counts;
  };

  u32 depth_stencil_;

  u32 write_mask_;

  RectU scissor_;

  gpu::Viewport viewport_;

  FillRule fill_rule_;

  bool invert_;

  f32x4x4 world_to_ndc_;

  Vec<f32x4x4> transforms_;

  Vec<f32x2> vertices_;

  Vec<u32> indices_;

  Vec<shader::BezierRegions> regions_;

  Vec<u32> region_index_counts_;

  explicit BezierStencilEncoder(Allocator allocator, Item const & item) :
    ICanvasEncoder{CanvasEncoderType::BezierStencil},
    depth_stencil_{item.depth_stencil},
    write_mask_{item.write_mask},
    scissor_{item.scissor},
    viewport_{item.viewport},
    fill_rule_{item.fill_rule},
    invert_{item.invert},
    world_to_ndc_{item.world_to_ndc},
    transforms_{allocator},
    vertices_{allocator},
    indices_{allocator},
    regions_{allocator},
    region_index_counts_{allocator}
  {
    push_(item.transform, item.vertices, item.indices, item.regions,
          item.region_index_counts);
  }

  BezierStencilEncoder(BezierStencilEncoder const &)             = delete;
  BezierStencilEncoder(BezierStencilEncoder &&)                  = default;
  BezierStencilEncoder & operator=(BezierStencilEncoder const &) = delete;
  BezierStencilEncoder & operator=(BezierStencilEncoder &&)      = default;
  virtual ~BezierStencilEncoder() override                       = default;

  void push_(f32x4x4 const & transform, Span<f32x2 const> vertices,
             Span<u32 const> indices, Span<shader::BezierRegions const> regions,
             Span<u32 const> region_index_counts)
  {
    transforms_.push(transform).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices).unwrap();
    regions_.extend(regions).unwrap();
    region_index_counts_.extend(region_index_counts).unwrap();
  }

  bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(
      Tuple{depth_stencil_, write_mask_, scissor_, viewport_, fill_rule_,
            invert_, world_to_ndc_},
      Tuple{depth_stencil_, item.write_mask, item.scissor, item.viewport,
            item.fill_rule, item.invert, item.world_to_ndc});

    if (!mergeable)
    {
      return false;
    }

    push_(item.transform, item.vertices, item.indices, item.regions,
          item.region_index_counts);

    return true;
  }

  virtual bool push(void const * object) override
  {
    return push(*static_cast<Item const *>(object));
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
    Span<u8 const>          world;
    Span<u8 const>          material;
    Span<u8 const>          lights;
    PipelineVariantId       variant;
  };

  u32 color_;

  Option<u32> depth_stencil_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::PolygonMode polygon_mode_;

  TextureSet texture_set_;

  GpuBufferSpan vertices_;    // = shader::pbr::Vertex

  GpuBufferSpan indices_;    // = shader::pbr::Index

  u32 num_indices_;

  Vec<u8> world_;    // = shader::pbr::World

  Vec<u8> material_;    // = shader::pbr::BaseMaterial,

  Vec<u8> lights_;    // = shader::pbr::PunctualLight

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
    world_{allocator},
    material_{allocator},
    lights_{allocator},
    variant_{item.variant}
  {
    world_.extend(Span{&item.world, 1}.as_u8()).unwrap();
    material_.extend(Span{&item.material, 1}.as_u8()).unwrap();
    lights_.extend(item.lights.as_u8()).unwrap();
  }

  PbrEncoder(PbrEncoder const &)             = delete;
  PbrEncoder(PbrEncoder &&)                  = default;
  PbrEncoder & operator=(PbrEncoder const &) = delete;
  PbrEncoder & operator=(PbrEncoder &&)      = default;
  virtual ~PbrEncoder() override             = default;

  virtual bool push(void const *) override
  {
    return false;
  }

  virtual void operator()(GpuFramePlan plan) override;
};

}    // namespace ash
