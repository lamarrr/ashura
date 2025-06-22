/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/contour_stencil.h"
#include "ashura/engine/passes/ngon.h"
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/passes/quad.h"
#include "ashura/engine/passes/sdf.h"
#include "ashura/std/dyn.h"

namespace ash
{

struct PassBundle
{
  ref<BlurPass>           blur;
  ref<ContourStencilPass> contour_stencil;
  ref<NgonPass>           ngon;
  ref<PBRPass>            pbr;
  ref<SdfPass>            sdf;
  ref<QuadPass>           quad;
  Vec<Dyn<Pass *>>        all;

  static PassBundle create(AllocatorRef allocator);

  PassBundle(BlurPass & blur, ContourStencilPass & contour_stencil,
             NgonPass & ngon, PBRPass & pbr, SdfPass & sdf, QuadPass & quad,
             Vec<Dyn<Pass *>> all) :
    blur{blur},
    contour_stencil{contour_stencil},
    ngon{ngon},
    pbr{pbr},
    sdf{sdf},
    quad{quad},
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
