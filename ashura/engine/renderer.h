/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/contour_stencil.h"
#include "ashura/engine/passes/ngon.h"
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/passes/sdf.h"
#include "ashura/engine/systems.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct Canvas;

struct PassBundle
{
  ref<BlurPass>           blur;
  ref<NgonPass>           ngon;
  ref<PBRPass>            pbr;
  ref<SdfPass>            sdf;
  ref<ContourStencilPass> contour_stencil;
  Vec<Dyn<Pass *>>        all;

  static PassBundle create(AllocatorRef allocator);

  PassBundle(BlurPass & blur, NgonPass & ngon, PBRPass & pbr, SdfPass & sdf,
             ContourStencilPass & contour_stencil, Vec<Dyn<Pass *>> all) :
    blur{blur},
    ngon{ngon},
    pbr{pbr},
    sdf{sdf},
    contour_stencil{contour_stencil},
    all{std::move(all)}
  {
  }

  PassBundle(PassBundle const &)             = delete;
  PassBundle(PassBundle &&)                  = default;
  PassBundle & operator=(PassBundle const &) = delete;
  PassBundle & operator=(PassBundle &&)      = default;
  ~PassBundle()                              = default;

  void acquire();

  void release();

  void add_pass(Dyn<Pass *> pass);
};

struct BlurRenderParam
{
  // [ ] RRectShaderParam rrect         = {};
  RectU            area          = {};
  Vec2U            spread_radius = {};
  RectU            scissor       = {};
  gpu::Viewport    viewport      = {};
  Mat4             world_to_ndc  = {};
};

struct BlurRenderer
{
  static void render(FrameGraph & graph, Framebuffer const & fb,
                     Span<ColorTexture const>, Span<DepthStencilTexture const>,
                     PassBundle const & passes, BlurRenderParam const & param);
};

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
                     Framebuffer const & fb, Span<ColorTexture const>,
                     Span<DepthStencilTexture const>);
};

}    // namespace ash
