/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/passes.h"
#include "ashura/engine/systems.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct PassContext
{
  BloomPass *      bloom;
  BlurPass *       blur;
  NgonPass *       ngon;
  PBRPass *        pbr;
  RRectPass *      rrect;
  Vec<Dyn<Pass *>> all;

  static PassContext create(AllocatorRef allocator);

  PassContext(BloomPass & bloom, BlurPass & blur, NgonPass & ngon,
              PBRPass & pbr, RRectPass & rrect, Vec<Dyn<Pass *>> all) :
    bloom{&bloom},
    blur{&blur},
    ngon{&ngon},
    pbr{&pbr},
    rrect{&rrect},
    all{std::move(all)}
  {
  }

  PassContext(PassContext const &)             = delete;
  PassContext(PassContext &&)                  = default;
  PassContext & operator=(PassContext const &) = delete;
  PassContext & operator=(PassContext &&)      = default;
  ~PassContext()                               = default;

  void acquire();

  void release();
};

struct GpuPipeline
{
  virtual Span<char const> label() = 0;

  virtual void acquire(PassContext & passes) = 0;

  virtual void release(PassContext & passes) = 0;

  virtual void begin_frame(PassContext & passes, gpu::CommandEncoder & enc) = 0;

  virtual void end_frame(PassContext & passes, gpu::CommandEncoder & enc) = 0;

  virtual ~GpuPipeline() = 0;
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

  Vec<Dyn<GpuPipeline *>> pipelines;

  static Renderer create(AllocatorRef allocator);

  Renderer(AllocatorRef allocator, PassContext passes) :
    resources{},
    passes{std::move(passes)},
    pipelines{allocator}
  {
  }

  Renderer(Renderer const &)             = delete;
  Renderer(Renderer &&)                  = default;
  Renderer & operator=(Renderer const &) = delete;
  Renderer & operator=(Renderer &&)      = default;
  ~Renderer()                            = default;

  void acquire();

  void release();

  void add_pass(Dyn<Pass *> pass);

  void add_pipeline(Dyn<GpuPipeline *> pipeline);

  void begin_frame(Framebuffer const & fb, Canvas & canvas);

  void end_frame(Framebuffer const & fb, Canvas & canvas);

  void render_frame(Framebuffer const & fb, Canvas & canvas);
};

}    // namespace ash
