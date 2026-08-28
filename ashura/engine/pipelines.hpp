/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.hpp"
#include "ashura/engine/pipeline.hpp"
#include "ashura/gpu/gpu.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

struct BezierStencilPipelineParams
{
    struct State
    {
        FillRule       fill_rule  : 1;
        bool           invert     : 1;
        gpu::FrontFace front_face : 1;
        u32            write_mask;
        RectU          scissor;
        gpu::Viewport  viewport;

        constexpr bool operator==(State const & other) const = default;
    };

    gpu::RenderingAttachment          stencil_attachment;
    RectU                             render_area;
    shader::BezierStencilShaderParams params;
    Span<u32 const>                   index_runs;
    Span<State const>                 states;
    Span<u32 const>                   state_runs;
};

struct BezierStencilPipeline final : IPipeline
{
    gpu::GraphicsPipeline pipeline_ = nullptr;

    BezierStencilPipeline(Allocator);

    BezierStencilPipeline(BezierStencilPipeline const &)             = delete;
    BezierStencilPipeline(BezierStencilPipeline &&)                  = delete;
    BezierStencilPipeline & operator=(BezierStencilPipeline const &) = delete;
    BezierStencilPipeline & operator=(BezierStencilPipeline &&)      = delete;

    virtual ~BezierStencilPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    void encode(gpu::CommandEncoder                 encoder,
                BezierStencilPipelineParams const & params);
};

struct BloomPipelineParams
{
};

struct BloomPipeline final : IPipeline
{
    BloomPipeline(Allocator);
    BloomPipeline(BloomPipeline const &)             = delete;
    BloomPipeline(BloomPipeline &&)                  = delete;
    BloomPipeline & operator=(BloomPipeline const &) = delete;
    BloomPipeline & operator=(BloomPipeline &&)      = delete;

    virtual ~BloomPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    void encode(gpu::CommandEncoder encoder, BloomPipelineParams const & params);
};

struct BlurPipelineParams
{
    Framebuffer              framebuffer;
    Option<PipelineStencil>  stencil;
    RectU                    scissor;
    gpu::Viewport            viewport;
    gpu::DescriptorSet       samplers;
    gpu::DescriptorSet       textures;
    shader::BlurShaderParams params;
    Slice32                  instances;
    bool                     upsample;
};

struct BlurPipeline final : IPipeline
{
    gpu::GraphicsPipeline downsample_pipeline_ = nullptr;

    gpu::GraphicsPipeline upsample_pipeline_ = nullptr;

    BlurPipeline(Allocator);

    BlurPipeline(BlurPipeline const &)             = delete;
    BlurPipeline(BlurPipeline &&)                  = delete;
    BlurPipeline & operator=(BlurPipeline const &) = delete;
    BlurPipeline & operator=(BlurPipeline &&)      = delete;

    virtual ~BlurPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    void encode(gpu::CommandEncoder encoder, BlurPipelineParams const & params);
};

constexpr auto fill_rule_stencil(FillRule fill_rule, bool invert, u32 write_mask)
{
    auto even_odd_pass_op = gpu::StencilOp::Invert;
    auto even_odd_fail_op = gpu::StencilOp::Keep;

    auto non_zero_front_pass_op = gpu::StencilOp::IncrementAndWrap;
    auto non_zero_front_fail_op = gpu::StencilOp::Keep;

    auto non_zero_back_pass_op = gpu::StencilOp::DecrementAndWrap;
    auto non_zero_back_fail_op = gpu::StencilOp::Keep;

    auto front_fail_op =
      (fill_rule == FillRule::EvenOdd) ? even_odd_fail_op : non_zero_front_fail_op;

    auto front_pass_op =
      (fill_rule == FillRule::EvenOdd) ? even_odd_pass_op : non_zero_front_pass_op;

    auto back_fail_op =
      (fill_rule == FillRule::EvenOdd) ? even_odd_fail_op : non_zero_back_fail_op;

    auto back_pass_op =
      (fill_rule == FillRule::EvenOdd) ? even_odd_pass_op : non_zero_back_pass_op;

    if (invert)
    {
        swap(front_fail_op, front_pass_op);
        swap(back_fail_op, back_pass_op);
    }

    return Tuple{
      gpu::StencilState{.fail_op       = front_fail_op,
                        .pass_op       = front_pass_op,
                        .depth_fail_op = gpu::StencilOp::Keep,
                        .compare_op    = gpu::CompareOp::Never,
                        .compare_mask  = 0,
                        .write_mask    = write_mask,
                        .reference     = 0},

      gpu::StencilState{.fail_op       = back_fail_op,
                        .pass_op       = back_pass_op,
                        .depth_fail_op = gpu::StencilOp::Keep,
                        .compare_op    = gpu::CompareOp::Never,
                        .compare_mask  = 0,
                        .write_mask    = write_mask,
                        .reference     = 0}
    };
}

struct FillStencilPipelineParams
{
    struct State
    {
        FillRule       fill_rule  : 1;
        bool           invert     : 1;
        gpu::FrontFace front_face : 1;
        u32            write_mask;
        RectU          scissor;
        gpu::Viewport  viewport;

        constexpr bool operator==(State const & other) const = default;
    };

    gpu::RenderingAttachment        stencil_attachment;
    RectU                           render_area;
    shader::FillStencilShaderParams params;
    Span<u32 const>                 index_runs;
    Span<State const>               states;
    Span<u32 const>                 state_runs;
};

struct FillStencilPipeline final : IPipeline
{
    gpu::GraphicsPipeline pipeline_;

    FillStencilPipeline(Allocator);
    FillStencilPipeline(FillStencilPipeline const &)             = delete;
    FillStencilPipeline(FillStencilPipeline &&)                  = delete;
    FillStencilPipeline & operator=(FillStencilPipeline const &) = delete;
    FillStencilPipeline & operator=(FillStencilPipeline &&)      = delete;

    virtual ~FillStencilPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    void encode(gpu::CommandEncoder encoder, FillStencilPipelineParams const & params);
};

struct PBRPipelineParams
{
    Framebuffer             framebuffer;
    Option<PipelineStencil> stencil;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::PolygonMode        polygon_mode;
    gpu::DescriptorSet      samplers;
    gpu::DescriptorSet      textures;
    shader::PbrShaderParams params;
    u32                     num_indices;
    gpu::CullMode           cull_mode;
    gpu::FrontFace          front_face;
    PipelineVariantId       variant;
};

struct PBRPipeline final : IPipeline
{
    struct Pipeline
    {
        gpu::GraphicsPipeline fill  = nullptr;
        gpu::GraphicsPipeline line  = nullptr;
        gpu::GraphicsPipeline point = nullptr;
    };

    SparseVec<PipelineVariantId, Tuple<Str, Pipeline>> variants_;

    PBRPipeline(Allocator);

    PBRPipeline(PBRPipeline const &)             = delete;
    PBRPipeline(PBRPipeline &&)                  = delete;
    PBRPipeline & operator=(PBRPipeline const &) = delete;
    PBRPipeline & operator=(PBRPipeline &&)      = delete;

    virtual ~PBRPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(GpuFramePlan plan, Str label);

    void encode(gpu::CommandEncoder encoder, PBRPipelineParams const & params);
};

struct QuadPipelineParams
{
    struct State
    {
        Option<PipelineStencil> stencil;
        RectU                   scissor;
        gpu::Viewport           viewport;

        constexpr bool operator==(State const & other) const = default;
    };

    Framebuffer              framebuffer;
    gpu::DescriptorSet       samplers;
    gpu::DescriptorSet       textures;
    shader::QuadShaderParams params;
    Span<State const>        states;
    Span<u32 const>          state_runs;
    PipelineVariantId        variant;
};

struct QuadPipeline final : IPipeline
{
    SparseVec<PipelineVariantId, Tuple<Str, gpu::GraphicsPipeline>> variants_;

    QuadPipeline(Allocator);

    QuadPipeline(QuadPipeline const &)             = delete;
    QuadPipeline(QuadPipeline &&)                  = delete;
    QuadPipeline & operator=(QuadPipeline const &) = delete;
    QuadPipeline & operator=(QuadPipeline &&)      = delete;

    virtual ~QuadPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(Str label);

    void encode(gpu::CommandEncoder encoder, QuadPipelineParams const & params);
};

struct SdfPipelineParams
{
    struct State
    {
        Option<PipelineStencil> stencil;
        RectU                   scissor;
        gpu::Viewport           viewport;

        constexpr bool operator==(State const & other) const = default;
    };

    Framebuffer             framebuffer;
    gpu::DescriptorSet      samplers;
    gpu::DescriptorSet      textures;
    shader::SdfShaderParams params;
    Span<State const>       states;
    Span<u32 const>         state_runs;
    PipelineVariantId       variant;
};

struct SdfPipeline final : IPipeline
{
    static constexpr PipelineVariantId GRADIENT      = PipelineVariantId::Base;
    static constexpr PipelineVariantId NOISE         = PipelineVariantId{1};
    static constexpr PipelineVariantId MESH_GRADIENT = PipelineVariantId{2};

    SparseVec<PipelineVariantId, Tuple<Str, gpu::GraphicsPipeline>> variants_;

    SdfPipeline(Allocator);

    SdfPipeline(SdfPipeline const &)             = delete;
    SdfPipeline(SdfPipeline &&)                  = delete;
    SdfPipeline & operator=(SdfPipeline const &) = delete;
    SdfPipeline & operator=(SdfPipeline &&)      = delete;

    virtual ~SdfPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(Str label);

    void encode(gpu::CommandEncoder encoder, SdfPipelineParams const & params);
};

struct TriangleFillPipelineParams
{
    struct State
    {
        gpu::CullMode           cull_mode  : 2;
        gpu::FrontFace          front_face : 1;
        RectU                   scissor;
        gpu::Viewport           viewport;
        Option<PipelineStencil> stencil;

        constexpr bool operator==(State const & other) const = default;
    };

    Framebuffer                      framebuffer;
    gpu::DescriptorSet               samplers;
    gpu::DescriptorSet               textures;
    shader::TriangleFillShaderParams params;
    Span<u32 const>                  index_runs;
    Span<State const>                states;
    Span<u32 const>                  state_runs;
    PipelineVariantId                variant;
};

struct TriangleFillPipeline final : IPipeline
{
    SparseVec<PipelineVariantId, Tuple<Str, gpu::GraphicsPipeline>> pipelines_;

    TriangleFillPipeline(Allocator);

    TriangleFillPipeline(TriangleFillPipeline const &)             = delete;
    TriangleFillPipeline(TriangleFillPipeline &&)                  = delete;
    TriangleFillPipeline & operator=(TriangleFillPipeline const &) = delete;
    TriangleFillPipeline & operator=(TriangleFillPipeline &&)      = delete;

    virtual ~TriangleFillPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(Str label);

    void encode(gpu::CommandEncoder encoder, TriangleFillPipelineParams const & params);
};

struct VectorPathCoveragePipelineParams
{
    struct State
    {
        gpu::FrontFace front_face : 1;
        RectU          scissor;
        gpu::Viewport  viewport;

        constexpr bool operator==(State const & other) const = default;
    };

    DepthStencilImage                      stencil;
    gpu::DescriptorSet                     write_alpha_masks;
    gpu::DescriptorSet                     write_fill_ids;
    shader::VectorPathCoverageShaderParams params;
    Span<u32 const>                        index_runs;
    Span<State const>                      states;
    Span<u32 const>                        state_runs;
};

struct VectorPathFillPipelineParams
{
    using State = VectorPathCoveragePipelineParams::State;

    Framebuffer                        framebuffer;
    gpu::DescriptorSet                 samplers;
    gpu::DescriptorSet                 textures;
    gpu::DescriptorSet                 read_alpha_masks;
    gpu::DescriptorSet                 read_fill_ids;
    shader::VectorPathFillShaderParams params;
    Span<State const>                  states;
    Span<u32 const>                    state_runs;
    PipelineVariantId                  variant;
};

struct VectorPathPipeline final : IPipeline
{
    gpu::GraphicsPipeline                                           coverage_pipeline_;
    SparseVec<PipelineVariantId, Tuple<Str, gpu::GraphicsPipeline>> fill_pipelines_;

    VectorPathPipeline(Allocator);

    VectorPathPipeline(VectorPathPipeline const &)             = delete;
    VectorPathPipeline(VectorPathPipeline &&)                  = delete;
    VectorPathPipeline & operator=(VectorPathPipeline const &) = delete;
    VectorPathPipeline & operator=(VectorPathPipeline &&)      = delete;

    virtual ~VectorPathPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_fill_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                       Allocator allocator);

    void remove_fill_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_fill_variant_id(Str label);

    void encode(gpu::CommandEncoder                      encoder,
                VectorPathCoveragePipelineParams const & params);

    void encode(gpu::CommandEncoder                  encoder,
                VectorPathFillPipelineParams const & params);
};

}    // namespace ash
