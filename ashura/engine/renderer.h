/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/passes.h"
#include "ashura/engine/systems.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct Canvas;

struct PassContext
{
  ref<BloomPass>    bloom;
  ref<BlurPass>     blur;
  ref<NgonPass>     ngon;
  ref<PBRPass>      pbr;
  ref<RRectPass>    rrect;
  ref<SquirclePass> squircle;
  Vec<Dyn<Pass *>>  all;

  static PassContext create(AllocatorRef allocator);

  PassContext(BloomPass & bloom, BlurPass & blur, NgonPass & ngon,
              PBRPass & pbr, RRectPass & rrect, SquirclePass & squircle,
              Vec<Dyn<Pass *>> all) :
    bloom{bloom},
    blur{blur},
    ngon{ngon},
    pbr{pbr},
    rrect{rrect},
    squircle{squircle},
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

  void add_pass(Dyn<Pass *> pass);
};

struct BlurRenderParam
{
  RectU         area          = {};
  Vec2U         spread_radius = {};
  Vec4          corner_radii  = {};
  Mat4          transform     = Mat4::IDENTITY;
  f32           aspect_ratio  = 1;
  RectU         scissor       = {};
  gpu::Viewport viewport      = {};
  Mat4          world_to_view = {};
};

struct BlurRenderer
{
  static void render(PassContext const & passes, FrameGraph & graph,
                     Framebuffer const & fb, BlurRenderParam const & param);
};

struct Renderer
{
  Dyn<PassContext *> passes_;

  static Renderer create(AllocatorRef allocator);

  Renderer(Dyn<PassContext *> passes) : passes_{std::move(passes)}
  {
  }

  Renderer(Renderer const &)             = delete;
  Renderer(Renderer &&)                  = default;
  Renderer & operator=(Renderer const &) = delete;
  Renderer & operator=(Renderer &&)      = default;
  ~Renderer()                            = default;

  void acquire();

  void release();

  void render_canvas(Framebuffer const & fb, FrameGraph & graph,
                     Canvas const & canvas);
};

}    // namespace ash
