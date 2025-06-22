/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass_bundle.h"
#include "ashura/engine/systems.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct Canvas;

struct Renderer
{
  Dyn<PassBundle *> passes_;

  static Renderer create(AllocatorRef allocator);

  Renderer(Dyn<PassBundle *> passes) : passes_{std::move(passes)}
  {
  }

  Renderer(Renderer const &)             = delete;
  Renderer(Renderer &&)                  = default;
  Renderer & operator=(Renderer const &) = delete;
  Renderer & operator=(Renderer &&)      = default;
  ~Renderer()                            = default;

  void acquire();

  void release();

  void render_canvas(FrameGraph & graph, Canvas const & canvas,
                     Framebuffer const &             framebuffer,
                     Span<ColorTexture const>        color_textures,
                     Span<DepthStencilTexture const> depth_stencil_textures);
};

}    // namespace ash
