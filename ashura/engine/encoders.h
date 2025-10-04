/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/pipeline.h"

#include "ashura/engine/pipelines/bezier_stencil.h"
#include "ashura/engine/pipelines/blur.h"
#include "ashura/engine/pipelines/fill_stencil.h"
#include "ashura/engine/pipelines/pbr.h"
#include "ashura/engine/pipelines/quad.h"
#include "ashura/engine/pipelines/sdf.h"
#include "ashura/engine/pipelines/triangle_fill.h"
#include "ashura/engine/pipelines/vector_path.h"
#include "ashura/std/allocator.h"
#include "ashura/std/math.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct ICanvasEncoder * CanvasEncoder;

enum class CanvasEncoderType : u32
{
  Sdf          = 0,
  Quad         = 1,
  TriangleFill = 2,
  FillPath     = 3,
  BezierPath   = 4,
  VectorPath   = 5,
  Pbr          = 6,
  Custom       = 7
};

typedef struct ICanvas * Canvas;

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

  constexpr virtual void submit(GpuFramePlan plan) = 0;
};

struct CustomCanvasEncoder : ICanvasEncoder
{
  CustomCanvasEncoder() : ICanvasEncoder{CanvasEncoderType::Custom}
  {
  }

  virtual void submit(GpuFramePlan) override
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

  virtual void submit(GpuFramePlan plan) override
  {
    lambda_(plan);
  }

  ~PassCanvasEncoder() = default;

  Lambda lambda_;
};

namespace impl
{
template <typename State>
void push_state(State const & state, Vec<State> & states, Vec<u32> & runs)
{
  if (runs.is_empty())
  {
    runs.extend(span({0U, 1U})).unwrap();
  }
  else
  {
    if (obj::byte_eq(state, states.last()))
    {
      runs.last()++;
    }
    else
    {
      states.push(state).unwrap();
      runs.push(runs.last() + 1).unwrap();
    }
  }
}

void push_index(u32 num_indices, Vec<u32> & runs)
{
  if (runs.is_empty())
  {
    runs.extend(span({0U, num_indices})).unwrap();
  }
  else
  {
    runs.push(runs.last() + num_indices).unwrap();
  }
}

}    // namespace impl

// [ ] encoders should work on batches primarily
struct SdfEncoder final : ICanvasEncoder
{
  using State = SdfPipelineParams::State;

  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          item;
    PipelineVariantId       variant;

    State state() const
    {
      return State{
        .stencil = stencil_op, .scissor = scissor, .viewport = viewport};
    }
  };

  struct Attachments
  {
    u32         color         = 0;
    Option<u32> depth_stencil = none;
  };

  u32 num_instances_;

  Attachments attachments_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<State> states_;

  Vec<u32> state_runs_;

  Vec<u8> items_;

  PipelineVariantId variant_;

  explicit SdfEncoder(Allocator allocator, Attachments const & attachments,
                      Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Sdf},
    num_instances_{0},
    attachments_{attachments},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    states_{allocator},
    state_runs_{allocator},
    items_{allocator},
    variant_{item.variant}
  {
    push_(item.state(), item.item);
  }

  SdfEncoder(SdfEncoder const &)             = delete;
  SdfEncoder(SdfEncoder &&)                  = default;
  SdfEncoder & operator=(SdfEncoder const &) = delete;
  SdfEncoder & operator=(SdfEncoder &&)      = default;
  ~SdfEncoder()                              = default;

  void push_(State const & state, Span<u8 const> item)
  {
    impl::push_state(state, states_, state_runs_);
    items_.extend(item).unwrap();
    num_instances_++;
  }

  [[nodiscard]] bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(Tuple{texture_set_, variant_},
                                  Tuple{item.texture_set, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.state(), item.item);

    return true;
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct QuadEncoder final : ICanvasEncoder
{
  using State = QuadPipelineParams::State;

  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          quad;
    PipelineVariantId       variant;

    State state() const
    {
      return State{
        .stencil = stencil_op, .scissor = scissor, .viewport = viewport};
    }
  };

  struct Attachments
  {
    u32         color         = 0;
    Option<u32> depth_stencil = none;
  };

  u32 num_instances_;

  Attachments attachments_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<State> states_;

  Vec<u32> state_runs_;

  Vec<u8> quads_;

  PipelineVariantId variant_;

  explicit QuadEncoder(Allocator allocator, Attachments const & attachments,
                       Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Quad},
    num_instances_{0},
    attachments_{attachments},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    states_{allocator},
    state_runs_{allocator},
    quads_{allocator},
    variant_{item.variant}
  {
    push_(item.state(), item.quad);
  }

  QuadEncoder(QuadEncoder const &)             = delete;
  QuadEncoder(QuadEncoder &&)                  = default;
  QuadEncoder & operator=(QuadEncoder const &) = delete;
  QuadEncoder & operator=(QuadEncoder &&)      = default;
  ~QuadEncoder()                               = default;

  void push_(State const & state, Span<u8 const> quad)
  {
    impl::push_state(state, states_, state_runs_);
    quads_.extend(quad).unwrap();
    num_instances_++;
  }

  [[nodiscard]] bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(Tuple{texture_set_, variant_},
                                  Tuple{item.texture_set, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.state(), item.quad);

    return true;
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct TriangleFillEncoder final : ICanvasEncoder
{
  using State = TriangleFillPipelineParams::State;

  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::CullMode           cull_mode;
    gpu::FrontFace          front_face;
    TextureSet              texture_set;
    f32x4x4                 world_to_ndc;
    Span<u8 const>          set;
    Span<u8 const>          vertices;
    Span<u32 const>         indices;
    PipelineVariantId       variant;

    State state() const
    {
      return State{.cull_mode  = cull_mode,
                   .front_face = front_face,
                   .scissor    = scissor,
                   .viewport   = viewport,
                   .stencil    = stencil_op};
    }
  };

  struct Attachments
  {
    u32         color         = 0;
    Option<u32> depth_stencil = none;
  };

  u32 num_instances_;

  Attachments attachments_;

  Option<PipelineStencil> stencil_op_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_runs_;

  Vec<State> states_;

  Vec<u32> state_runs_;

  Vec<u8> sets_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  PipelineVariantId variant_;

  explicit TriangleFillEncoder(Allocator           allocator,
                               Attachments const & attachments,
                               Item const &        item) :
    ICanvasEncoder{CanvasEncoderType::TriangleFill},
    num_instances_{0},
    attachments_{attachments},
    stencil_op_{item.stencil_op},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    index_runs_{allocator},
    states_{allocator},
    state_runs_{allocator},
    sets_{allocator},
    vertices_{allocator},
    indices_{allocator},
    variant_{item.variant}
  {
    push_(item.state(), item.set, item.vertices, item.indices);
  }

  TriangleFillEncoder(TriangleFillEncoder const &)             = delete;
  TriangleFillEncoder(TriangleFillEncoder &&)                  = default;
  TriangleFillEncoder & operator=(TriangleFillEncoder const &) = delete;
  TriangleFillEncoder & operator=(TriangleFillEncoder &&)      = default;
  ~TriangleFillEncoder()                                       = default;

  void push_(State const & state, Span<u8 const> set, Span<u8 const> vertices,
             Span<u32 const> indices)
  {
    impl::push_index(size32(indices), index_runs_);
    impl::push_state(state, states_, state_runs_);
    sets_.extend(set).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices.as_u8()).unwrap();
    num_instances_++;
  }

  [[nodiscard]] bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(Tuple{texture_set_, variant_},
                                  Tuple{item.texture_set, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.state(), item.set, item.vertices, item.indices);

    return true;
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct FillStencilEncoder final : ICanvasEncoder
{
  using State = FillStencilPipelineParams::State;

  struct Item
  {
    u32             write_mask;
    RectU           scissor;
    gpu::Viewport   viewport;
    FillRule        fill_rule;
    bool            invert;
    gpu::FrontFace  front_face;
    f32x4x4         world_to_ndc;
    f32x4x4         world_transform;
    Span<u8 const>  vertices;
    Span<u32 const> indices;

    State state() const
    {
      return State{.fill_rule  = fill_rule,
                   .invert     = invert,
                   .front_face = front_face,
                   .write_mask = write_mask,
                   .scissor    = scissor,
                   .viewport   = viewport};
    }
  };

  struct Attachments
  {
    u32 depth_stencil = 0;
  };

  u32 num_instances_;

  Attachments attachments_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_runs_;

  Vec<State> states_;

  Vec<u32> state_runs_;

  Vec<u8> world_transforms_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  explicit FillStencilEncoder(Allocator           allocator,
                              Attachments const & attachments,
                              Item const &        item) :
    ICanvasEncoder{CanvasEncoderType::Custom},
    num_instances_{0},
    attachments_{attachments},
    world_to_ndc_{item.world_to_ndc},
    index_runs_{allocator},
    states_{allocator},
    state_runs_{allocator},
    world_transforms_{allocator},
    vertices_{allocator},
    indices_{allocator}
  {
    push_(item.state(), item.world_transform, item.vertices, item.indices);
  }

  FillStencilEncoder(FillStencilEncoder const &)             = delete;
  FillStencilEncoder(FillStencilEncoder &&)                  = default;
  FillStencilEncoder & operator=(FillStencilEncoder const &) = delete;
  FillStencilEncoder & operator=(FillStencilEncoder &&)      = default;
  ~FillStencilEncoder()                                      = default;

  void push_(State const & state, f32x4x4 const & world_transform,
             Span<u8 const> vertices, Span<u32 const> indices)
  {
    impl::push_index(size32(indices), index_runs_);
    impl::push_state(state, states_, state_runs_);
    world_transforms_.extend(as_u8_span(world_transform)).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices.as_u8()).unwrap();
    num_instances_++;
  }

  void push(Item const & item)
  {
    push_(item.state(), item.world_transform, item.vertices, item.indices);
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct BezierStencilEncoder final : ICanvasEncoder
{
  using State = BezierStencilPipelineParams::State;

  struct Item
  {
    u32             write_mask;
    RectU           scissor;
    gpu::Viewport   viewport;
    FillRule        fill_rule;
    bool            invert;
    gpu::FrontFace  front_face;
    f32x4x4         world_to_ndc;
    f32x4x4         world_transform;
    u32             first_bezier_index;
    Span<u8 const>  vertices;
    Span<u32 const> indices;

    State state() const
    {
      return State{.fill_rule  = fill_rule,
                   .invert     = invert,
                   .front_face = front_face,
                   .write_mask = write_mask,
                   .scissor    = scissor,
                   .viewport   = viewport};
    }
  };

  struct Attachments
  {
    u32 depth_stencil = 0;
  };

  u32 num_instances_;

  Attachments attachments_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_runs_;

  Vec<State> states_;

  Vec<u32> state_runs_;

  Vec<u8> items_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  BezierStencilEncoder(Allocator allocator, Attachments const & attachments,
                       Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Custom},
    num_instances_{0},
    attachments_{attachments},
    world_to_ndc_{item.world_to_ndc},
    index_runs_{allocator},
    states_{allocator},
    state_runs_{allocator},
    items_{allocator},
    vertices_{allocator},
    indices_{allocator}
  {
    push_(item.state(), item.world_transform, item.vertices, item.indices,
          item.first_bezier_index);
  }

  BezierStencilEncoder(BezierStencilEncoder const &)             = delete;
  BezierStencilEncoder(BezierStencilEncoder &&)                  = default;
  BezierStencilEncoder & operator=(BezierStencilEncoder const &) = delete;
  BezierStencilEncoder & operator=(BezierStencilEncoder &&)      = default;
  ~BezierStencilEncoder()                                        = default;

  void push_(State const & state, f32x4x4 const & world_transform,
             Span<u8 const> vertices, Span<u32 const> indices,
             u32 first_bezier_index)
  {
    impl::push_index(size32(indices), index_runs_);
    impl::push_state(state, states_, state_runs_);

    auto index_prefix = index_runs_[size32(indices) - 2];
    auto item = shader::BezierStencilItem{.world_transform = world_transform,
                                          .first_bezier_index =
                                            index_prefix + first_bezier_index};
    items_.extend(as_u8_span(item)).unwrap();
    vertices_.extend(vertices).unwrap();
    indices_.extend(indices.as_u8()).unwrap();
    num_instances_++;
  }

  void push(Item const & item)
  {
    push_(item.state(), item.world_transform, item.vertices, item.indices,
          item.first_bezier_index);
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct FillPathEncoder final : ICanvasEncoder
{
  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    FillRule                fill_rule;
    TextureSet              texture_set;
    bool                    invert;
    gpu::FrontFace          front_face;
    f32x4x4                 world_to_ndc;
    f32x4x4                 world_transform;
    Span<u8 const>          vertices;
    Span<u32 const>         indices;
    Span<u8 const>          sdf_item;
    PipelineVariantId       sdf_variant;
  };

  struct Attachments
  {
    u32 color                 = 0;
    u32 scratch_depth_stencil = 0;
  };

  FillStencilEncoder stencil_;
  SdfEncoder         fill_;

  FillPathEncoder(Allocator allocator, Attachments const & attachment,
                  Item const & item) :
    ICanvasEncoder{
      CanvasEncoderType::Custom
  },
    stencil_{allocator,
             FillStencilEncoder::Attachments{
               .depth_stencil = attachment.scratch_depth_stencil},
             FillStencilEncoder::Item{.write_mask      = 1,
                                      .scissor         = item.scissor,
                                      .viewport        = item.viewport,
                                      .fill_rule       = item.fill_rule,
                                      .invert          = item.invert,
                                      .front_face      = item.front_face,
                                      .world_to_ndc    = item.world_to_ndc,
                                      .world_transform = item.world_transform,
                                      .vertices        = item.vertices,
                                      .indices         = item.indices}},
    fill_{allocator,
          SdfEncoder::Attachments{
            .color         = attachment.color,
            .depth_stencil = attachment.scratch_depth_stencil,
          },
          SdfEncoder::Item{.stencil_op   = item.stencil_op,
                           .scissor      = item.scissor,
                           .viewport     = item.viewport,
                           .texture_set  = item.texture_set,
                           .world_to_ndc = item.world_to_ndc,
                           .item         = item.sdf_item,
                           .variant      = item.sdf_variant}}
  {
  }

  [[nodiscard]] bool push(Item const & item);

  virtual void submit(GpuFramePlan plan) override;
};

struct BezierPathEncoder final : ICanvasEncoder
{
  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    FillRule                fill_rule;
    TextureSet              texture_set;
    bool                    invert;
    gpu::FrontFace          front_face;
    f32x4x4                 world_to_ndc;
    f32x4x4                 world_transform;
    u32                     first_bezier_index;
    Span<u8 const>          vertices;
    Span<u32 const>         indices;
    Span<u8 const>          sdf_item;
    PipelineVariantId       sdf_variant;
  };

  struct Attachments
  {
    u32 color                 = 0;
    u32 scratch_depth_stencil = 0;
  };

  BezierStencilEncoder stencil_;
  SdfEncoder           fill_;

  BezierPathEncoder(Allocator allocator, Attachments const & attachment,
                    Item const & item) :
    ICanvasEncoder{
      CanvasEncoderType::Custom
  },
    stencil_{
      allocator,
      BezierStencilEncoder::Attachments{.depth_stencil =
                                          attachment.scratch_depth_stencil},
      BezierStencilEncoder::Item{.write_mask         = 1,
                                 .scissor            = item.scissor,
                                 .viewport           = item.viewport,
                                 .fill_rule          = item.fill_rule,
                                 .invert             = item.invert,
                                 .front_face         = item.front_face,
                                 .world_to_ndc       = item.world_to_ndc,
                                 .world_transform    = item.world_transform,
                                 .first_bezier_index = item.first_bezier_index,
                                 .vertices           = item.vertices,
                                 .indices            = item.indices}},
    fill_{allocator,
          SdfEncoder::Attachments{
            .color         = attachment.color,
            .depth_stencil = attachment.scratch_depth_stencil,
          },
          SdfEncoder::Item{.stencil_op   = item.stencil_op,
                           .scissor      = item.scissor,
                           .viewport     = item.viewport,
                           .texture_set  = item.texture_set,
                           .world_to_ndc = item.world_to_ndc,
                           .item         = item.sdf_item,
                           .variant      = item.sdf_variant}}
  {
  }

  [[nodiscard]] bool push(Item const & item);

  virtual void submit(GpuFramePlan plan) override;
};

struct VectorPathEncoder final : ICanvasEncoder
{
  using State = VectorPathCoveragePipelineParams::State;

  struct Item
  {
    RectU                                  scissor;
    gpu::Viewport                          viewport;
    TextureSet                             texture_set;
    gpu::FrontFace                         front_face;
    f32x4x4                                world_to_ndc;
    f32x4x4                                world_transform;
    Span<shader::VectorPathVertex const>   vertices;
    Span<u32 const>                        indices;
    Span<shader::VectorPathFillItem const> fill_items;
    PipelineVariantId                      variant;

    State state() const
    {
      return State{
        .front_face = front_face, .scissor = scissor, .viewport = viewport};
    }
  };

  struct Attachments
  {
    u32         color                 = 0;
    Option<u32> depth_stencil         = none;
    u32         scratch_depth_stencil = 0;
    u32         scratch_alpha_mask    = 0;
    u32         scratch_fill_id       = 0;
  };

  u32 num_coverage_items_;

  u32 num_fill_items_;

  Attachments attachments_;

  TextureSet texture_set_;

  f32x4x4 world_to_ndc_;

  Vec<u32> index_runs_;

  Vec<State> coverage_states_;

  Vec<u32> coverage_state_runs_;

  Vec<State> fill_states_;

  Vec<u32> fill_state_runs_;

  Vec<u8> vertices_;

  Vec<u8> indices_;

  Vec<u8> coverage_items_;

  Vec<u8> fill_items_;

  PipelineVariantId variant_;

  explicit VectorPathEncoder(Allocator           allocator,
                             Attachments const & attachments,
                             Item const &        item) :
    ICanvasEncoder{CanvasEncoderType::VectorPath},
    num_coverage_items_{0},
    num_fill_items_{0},
    attachments_{attachments},
    texture_set_{item.texture_set},
    world_to_ndc_{item.world_to_ndc},
    index_runs_{allocator},
    coverage_states_{allocator},
    coverage_state_runs_{allocator},
    fill_states_{allocator},
    fill_state_runs_{allocator},
    vertices_{allocator},
    indices_{allocator},
    coverage_items_{allocator},
    fill_items_{allocator},
    variant_{item.variant}
  {
    push_(item.state(), item.world_transform, item.vertices, item.indices,
          item.fill_items);
  }

  VectorPathEncoder(VectorPathEncoder const &)             = delete;
  VectorPathEncoder(VectorPathEncoder &&)                  = default;
  VectorPathEncoder & operator=(VectorPathEncoder const &) = delete;
  VectorPathEncoder & operator=(VectorPathEncoder &&)      = default;
  ~VectorPathEncoder()                                     = default;

  void push_(State const & state, f32x4x4 const & world_transform,
             Span<shader::VectorPathVertex const>   vertices,
             Span<u32 const>                        indices,
             Span<shader::VectorPathFillItem const> fill_items)
  {
    impl::push_index(size32(indices), index_runs_);
    impl::push_state(state, coverage_states_, coverage_state_runs_);
    impl::push_state(state, fill_states_, fill_state_runs_);

    shader::VectorPathCoverageItem item{.world_transform = world_transform};

    vertices_.extend(vertices.as_u8()).unwrap();
    indices_.extend(indices.as_u8()).unwrap();
    coverage_items_.extend(as_u8_span(item)).unwrap();
    fill_items_.extend(fill_items.as_u8()).unwrap();
    num_coverage_items_++;
    num_fill_items_ += size32(fill_items);
  }

  [[nodiscard]] bool push(Item const & item)
  {
    auto mergeable = obj::byte_eq(Tuple{texture_set_, variant_},
                                  Tuple{item.texture_set, item.variant});

    if (!mergeable)
    {
      return false;
    }

    push_(item.state(), item.world_transform, item.vertices, item.indices,
          item.fill_items);

    return true;
  }

  virtual void submit(GpuFramePlan plan) override;
};

struct PbrEncoder final : ICanvasEncoder
{
  struct Item
  {
    Option<PipelineStencil> stencil_op;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::PolygonMode        polygon_mode;
    TextureSet              texture_set;
    GpuBufferSpan           vertices;
    GpuBufferSpan           indices;
    u32                     num_indices;
    Span<u8 const>          item;
    Span<u8 const>          lights;
    gpu::CullMode           cull_mode;
    gpu::FrontFace          front_face;
    PipelineVariantId       variant;
  };

  struct Attachments
  {
    u32         color         = 0;
    Option<u32> depth_stencil = none;
  };

  Attachments attachments_;

  Option<PipelineStencil> stencil_op_;

  RectU scissor_;

  gpu::Viewport viewport_;

  gpu::PolygonMode polygon_mode_;

  TextureSet texture_set_;

  GpuBufferSpan vertices_;

  GpuBufferSpan indices_;

  u32 num_indices_;

  Vec<u8> item_;

  Vec<u8> lights_;

  gpu::CullMode cull_mode_;

  gpu::FrontFace front_face_;

  PipelineVariantId variant_;

  explicit PbrEncoder(Allocator allocator, Attachments const & attachments,
                      Item const & item) :
    ICanvasEncoder{CanvasEncoderType::Pbr},
    attachments_{attachments},
    stencil_op_{item.stencil_op},
    scissor_{item.scissor},
    viewport_{item.viewport},
    polygon_mode_{item.polygon_mode},
    texture_set_{item.texture_set},
    vertices_{item.vertices},
    indices_{item.indices},
    num_indices_{item.num_indices},
    item_{allocator},
    lights_{allocator},
    cull_mode_{item.cull_mode},
    front_face_{item.front_face},
    variant_{item.variant}
  {
    item_.extend(item.item).unwrap();
    lights_.extend(item.lights).unwrap();
  }

  PbrEncoder(PbrEncoder const &)             = delete;
  PbrEncoder(PbrEncoder &&)                  = default;
  PbrEncoder & operator=(PbrEncoder const &) = delete;
  PbrEncoder & operator=(PbrEncoder &&)      = default;
  ~PbrEncoder()                              = default;

  virtual void submit(GpuFramePlan plan) override;
};

}    // namespace ash
