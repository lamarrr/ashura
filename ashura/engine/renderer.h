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
  BloomPass       *bloom = nullptr;
  BlurPass        *blur  = nullptr;
  NgonPass        *ngon  = nullptr;
  PBRPass         *pbr   = nullptr;
  RRectPass       *rrect = nullptr;
  Vec<Dyn<Pass *>> all   = {};

  void init(GpuContext &ctx)
  {
    for (auto const &p : all)
    {
      logger->info("Initializing Pass: ", p->id());
      p->init(ctx);
    };
  }

  void uninit(GpuContext &ctx)
  {
    for (auto const &p : all)
    {
      logger->info("Unintializing Pass: ", p->id());
      p->uninit(ctx);
      p.uninit();
    };
  }
};

struct RenderPipeline
{
  virtual Span<char const> id() = 0;

  virtual void acquire(GpuContext &ctx, PassContext &) = 0;

  virtual void release(GpuContext &ctx, PassContext &) = 0;

  virtual void begin_frame(GpuContext &ctx, PassContext &,
                           gpu::CommandEncoderImpl const &) = 0;

  virtual void end_frame(GpuContext &ctx, PassContext &,
                         gpu::CommandEncoderImpl const &) = 0;

  virtual ~RenderPipeline() = 0;
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
    SSBO pbr_params       = {.label = "PBR Params SSBO"_span};
    SSBO pbr_light_params = {.label = "Params Lights Params SSBO"_span};
    SSBO ngon_vertices    = {.label = "Ngon Vertices SSBO"_span};
    SSBO ngon_indices     = {.label = "Ngon Indices SSBO"_span};
    SSBO ngon_params      = {.label = "Ngon Params SSBO"_span};
    SSBO rrect_params     = {.label = "RRect Params SSBO"_span};
  };

  InplaceVec<Resources, gpu::MAX_FRAME_BUFFERING> resources;

  PassContext passes{};

  Canvas *canvas = nullptr;

  Vec<Dyn<RenderPipeline *>> pipelines{};

  void init(GpuContext &ctx)
  {
    passes.init(ctx);
    for (auto &p : pipelines)
    {
      p->init(ctx, passes);
      logger->info("Initialized Pipeline: ", p->id());
    }
    resources.resize_defaulted(ctx.buffering).unwrap();
  }

  void uninit(GpuContext &ctx)
  {
    for (auto &p : pipelines)
    {
      logger->info("Uninitializing Pipeline: ", p->id());
      p->uninit(ctx, passes);
    }
    pipelines.uninit();
    for (Resources &r : resources)
    {
      r.pbr_params.uninit(ctx);
      r.pbr_light_params.uninit(ctx);
      r.ngon_vertices.uninit(ctx);
      r.ngon_indices.uninit(ctx);
      r.ngon_params.uninit(ctx);
      r.rrect_params.uninit(ctx);
    }
    resources.uninit();
    passes.uninit(ctx);
  }

  void register_pass();
  void register_pipeline();

  void begin_frame(GpuContext &ctx, RenderTarget const &);

  void end_frame(GpuContext &ctx, RenderTarget const &);

  void render_frame(GpuContext &ctx, RenderTarget const &rt);
};

}        // namespace ash
