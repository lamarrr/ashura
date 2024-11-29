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
  BloomPass       *bloom;
  BlurPass        *blur;
  NgonPass        *ngon;
  PBRPass         *pbr;
  RRectPass       *rrect;
  Vec<Dyn<Pass *>> all;

  explicit PassContext(BloomPass &bloom, BlurPass &blur, NgonPass &ngon,
                       PBRPass &pbr, RRectPass &rrect, Vec<Dyn<Pass *>> all) :
      bloom{&bloom},
      blur{&blur},
      ngon{&ngon},
      pbr{&pbr},
      rrect{&rrect},
      all{std::move(all)}
  {
  }

  static PassContext create(AllocatorImpl allocator)
  {
    Dyn bloom = dyn(allocator, BloomPass{}).unwrap();
    Dyn blur  = dyn(allocator, BlurPass{}).unwrap();
    Dyn ngon  = dyn(allocator, NgonPass{}).unwrap();
    Dyn pbr   = dyn(allocator, PBRPass{}).unwrap();
    Dyn rrect = dyn(allocator, RRectPass{}).unwrap();

    BloomPass *pbloom = bloom.get();
    BlurPass  *pblur  = blur.get();
    NgonPass  *pngon  = ngon.get();
    PBRPass   *ppbr   = pbr.get();
    RRectPass *prrect = rrect.get();

    Vec<Dyn<Pass *>> all{allocator};

    all.push(transmute(std::move(bloom), static_cast<Pass *>(pbloom))).unwrap();
    all.push(transmute(std::move(blur), static_cast<Pass *>(pblur))).unwrap();
    all.push(transmute(std::move(ngon), static_cast<Pass *>(pngon))).unwrap();
    all.push(transmute(std::move(pbr), static_cast<Pass *>(ppbr))).unwrap();
    all.push(transmute(std::move(rrect), static_cast<Pass *>(prrect))).unwrap();

    return PassContext{*pbloom, *pblur, *pngon, *ppbr, *prrect, std::move(all)};
  }

  PassContext(PassContext const &)            = delete;
  PassContext(PassContext &&)                 = default;
  PassContext &operator=(PassContext const &) = delete;
  PassContext &operator=(PassContext &&)      = default;
  ~PassContext()                              = default;

  void acquire(GpuContext &ctx, AssetMap &assets)
  {
    for (auto const &p : all)
    {
      p->acquire(ctx, assets);
    };
  }

  void release(GpuContext &ctx, AssetMap &assets)
  {
    for (auto const &p : all)
    {
      p->release(ctx, assets);
    };
  }
};

struct RenderPipeline
{
  virtual Span<char const> id() = 0;

  virtual void acquire(GpuContext &ctx, PassContext &passes,
                       AssetMap &assets) = 0;

  virtual void release(GpuContext &ctx, PassContext &passes,
                       AssetMap &assets) = 0;

  virtual void begin_frame(GpuContext &ctx, PassContext &passes,
                           gpu::CommandEncoder &enc) = 0;

  virtual void end_frame(GpuContext &ctx, PassContext &passes,
                         gpu::CommandEncoder &enc) = 0;

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
    SSBO pbr_params{.label = "PBR Params SSBO"_str};

    SSBO pbr_light_params{.label = "Params Lights Params SSBO"_str};

    SSBO ngon_vertices{.label = "Ngon Vertices SSBO"_str};

    SSBO ngon_indices{.label = "Ngon Indices SSBO"_str};

    SSBO ngon_params{.label = "Ngon Params SSBO"_str};

    SSBO rrect_params{.label = "RRect Params SSBO"_str};
  };

  InplaceVec<Resources, gpu::MAX_FRAME_BUFFERING> resources;

  PassContext passes;

  Vec<Dyn<RenderPipeline *>> pipelines;

  Renderer(AllocatorImpl allocator, PassContext passes) :
      resources{}, passes{std::move(passes)}, pipelines{allocator}
  {
  }

  static Renderer create(AllocatorImpl allocator)
  {
    PassContext passes = PassContext::create(allocator);
    return Renderer{allocator, std::move(passes)};
  }

  Renderer(Renderer const &)            = delete;
  Renderer(Renderer &&)                 = default;
  Renderer &operator=(Renderer const &) = delete;
  Renderer &operator=(Renderer &&)      = default;
  ~Renderer()                           = default;

  void acquire(GpuContext &ctx, AssetMap &assets)
  {
    passes.acquire(ctx, assets);

    for (Dyn<RenderPipeline *> const &p : pipelines)
    {
      p->acquire(ctx, passes, assets);
    }

    resources.resize_defaulted(ctx.buffering).unwrap();
  }

  void release(GpuContext &ctx, AssetMap &assets)
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
      p->release(ctx, passes, assets);
    }

    passes.release(ctx, assets);
  }

  void add_pass(GpuContext &ctx, Dyn<Pass *> pass, AssetMap &assets)
  {
    pass->acquire(ctx, assets);
    passes.all.push(std::move(pass)).unwrap();
  }

  void add_pipeline(GpuContext &ctx, Dyn<RenderPipeline *> pipeline,
                    AssetMap &assets)
  {
    pipeline->acquire(ctx, passes, assets);
    pipelines.push(std::move(pipeline)).unwrap();
  }

  void begin_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);

  void end_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);

  void render_frame(GpuContext &ctx, RenderTarget const &rt, Canvas &canvas);
};

}        // namespace ash
