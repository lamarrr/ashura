/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_context.h"
#include "ashura/engine/passes.h"
#include "ashura/std/dyn.h"
#include "ashura/std/math.h"

namespace ash
{

struct PassContext
{
  BloomPass    *    bloom;
  BlurPass     *    blur;
  NgonPass     *    ngon;
  PBRPass      *    pbr;
  RRectPass    *    rrect;
  Vec<Dyn<Pass *>> all;

  explicit PassContext(AllocatorImpl allocator) :
      bloom{}, blur{}, ngon{}, pbr{}, rrect{}, all{allocator}
  {
    all.push(Dyn<Pass *>{&bloom, noop_allocator, noop}).unwrap();
    all.push(Dyn<Pass *>{&blur, noop_allocator, noop}).unwrap();
    all.push(Dyn<Pass *>{&ngon, noop_allocator, noop}).unwrap();
    all.push(Dyn<Pass *>{&pbr, noop_allocator, noop}).unwrap();
    all.push(Dyn<Pass *>{&rrect, noop_allocator, noop}).unwrap();
  }

  PassContext(PassContext const &)            = delete;
  PassContext(PassContext &&)                 = delete;
  PassContext &operator=(PassContext const &) = delete;
  PassContext &operator=(PassContext &&)      = delete;
  ~PassContext()                              = default;

  void acquire(GpuContext &ctx)
  {
    for (auto const &p : all)
    {
      p->acquire(ctx);
    };
  }

  void release(GpuContext &ctx)
  {
    for (auto const &p : all)
    {
      p->release(ctx);
    };
  }
};

struct RenderPipeline
{
  virtual Span<char const> id() = 0;

  virtual void acquire(GpuContext &ctx, PassContext &passes) = 0;

  virtual void release(GpuContext &ctx, PassContext &passes) = 0;

  virtual void begin_frame(GpuContext &ctx, PassContext &passes,
                           gpu::CommandEncoderImpl const &enc) = 0;

  virtual void end_frame(GpuContext &ctx, PassContext &passes,
                         gpu::CommandEncoderImpl const &enc) = 0;

  virtual ~RenderPipeline() = default;
};

struct RenderTarget
{
  gpu::RenderingInfo info{};
  gpu::Viewport      viewport{};
  gpu::Extent        extent{};
  gpu::DescriptorSet color_descriptor   = nullptr;
  gpu::DescriptorSet depth_descriptor   = nullptr;
  gpu::DescriptorSet stencil_descriptor = nullptr;
};

struct Canvas;

struct Renderer
{
  struct Resources
  {
    SSBO pbr_params{.label = "PBR Params SSBO"_span};

    SSBO pbr_light_params{.label = "Params Lights Params SSBO"_span};

    SSBO ngon_vertices{.label = "Ngon Vertices SSBO"_span};

    SSBO ngon_indices{.label = "Ngon Indices SSBO"_span};

    SSBO ngon_params{.label = "Ngon Params SSBO"_span};

    SSBO rrect_params{.label = "RRect Params SSBO"_span};
  };

  InplaceVec<Resources, gpu::MAX_FRAME_BUFFERING> resources;

  PassContext passes;

  Vec<Dyn<RenderPipeline *>> pipelines;

  explicit Renderer(AllocatorImpl allocator) :
      resources{}, passes{allocator}, pipelines{allocator}
  {
  }

  Renderer(Renderer const &)            = delete;
  Renderer(Renderer &&)                 = default;
  Renderer &operator=(Renderer const &) = delete;
  Renderer &operator=(Renderer &&)      = default;
  ~Renderer()                           = default;

  void acquire(GpuContext &ctx)
  {
    passes.acquire(ctx);

    for (Dyn<RenderPipeline *> const &p : pipelines)
    {
      p->acquire(ctx, passes);
    }

    resources.resize_defaulted(ctx.buffering).unwrap();
  }

  void release(GpuContext &ctx)
  {
    for (Resources &r : resources)
    {
      r.pbr_params.uninit(ctx);
      r.pbr_light_params.uninit(ctx);
      r.ngon_vertices.uninit(ctx);
      r.ngon_indices.uninit(ctx);
      r.ngon_params.uninit(ctx);
      r.rrect_params.uninit(ctx);
    }

    resources.reset();

    for (Dyn<RenderPipeline *> const &p : pipelines)
    {
      p->release(ctx, passes);
    }

    passes.release(ctx);
  }

  void add_pass(GpuContext &ctx, Dyn<Pass *> pass)
  {
    pass->acquire(ctx);
    passes.all.push(std::move(pass)).unwrap();
  }

  void add_pipeline(GpuContext &ctx, Dyn<RenderPipeline *> pipeline)
  {
    pipeline->acquire(ctx, passes);
    pipelines.push(std::move(pipeline)).unwrap();
  }

  void begin_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);

  void end_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);

  void render_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);
};

}        // namespace ash
