/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/passes/bezier_stencil.h"
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/fill_stencil.h"
#include "ashura/engine/passes/ngon.h"
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/passes/quad.h"
#include "ashura/engine/passes/sdf.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct PassBundle
{
  ref<SdfPass>           sdf;
  ref<QuadPass>          quad;
  ref<NgonPass>          ngon;
  ref<FillStencilPass>   fill_stencil;
  ref<BezierStencilPass> bezier_stencil;
  ref<BlurPass>          blur;
  ref<PBRPass>           pbr;
  Vec<Dyn<Pass *>>       all;

  static PassBundle create(AllocatorRef allocator);

  PassBundle(SdfPass & sdf, QuadPass & quad, NgonPass & ngon,
             FillStencilPass & fill_stencil, BezierStencilPass & bezier_stencil,
             BlurPass & blur, PBRPass & pbr, Vec<Dyn<Pass *>> all) :
    sdf{sdf},
    quad{quad},
    ngon{ngon},
    fill_stencil{fill_stencil},
    bezier_stencil{bezier_stencil},
    blur{blur},
    pbr{pbr},
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

}    // namespace ash
