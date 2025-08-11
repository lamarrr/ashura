/// SPDX-License-Identifier: MIT
#include "ashura/engine/pass_bundle.h"

namespace ash
{

PassBundle PassBundle::create(Allocator allocator)
{
  Dyn sdf            = dyn(allocator, SdfPass{allocator}).unwrap();
  Dyn quad           = dyn(allocator, QuadPass{allocator}).unwrap();
  Dyn ngon           = dyn(allocator, NgonPass{allocator}).unwrap();
  Dyn fill_stencil   = dyn(allocator, FillStencilPass{allocator}).unwrap();
  Dyn bezier_stencil = dyn(allocator, BezierStencilPass{allocator}).unwrap();
  Dyn blur           = dyn(allocator, BlurPass{allocator}).unwrap();
  Dyn pbr            = dyn(allocator, PBRPass{allocator}).unwrap();

  auto * psdf            = sdf.get();
  auto * pquad           = quad.get();
  auto * pngon           = ngon.get();
  auto * pfill_stencil   = fill_stencil.get();
  auto * pbezier_stencil = bezier_stencil.get();
  auto * pblur           = blur.get();
  auto * ppbr            = pbr.get();

  Vec<Dyn<Pass *>> all{allocator};

  all.push(cast<Pass *>(std::move(sdf))).unwrap();
  all.push(cast<Pass *>(std::move(quad))).unwrap();
  all.push(cast<Pass *>(std::move(ngon))).unwrap();
  all.push(cast<Pass *>(std::move(fill_stencil))).unwrap();
  all.push(cast<Pass *>(std::move(bezier_stencil))).unwrap();
  all.push(cast<Pass *>(std::move(blur))).unwrap();
  all.push(cast<Pass *>(std::move(pbr))).unwrap();

  return PassBundle{*psdf,          *pquad,           *pngon,
                    *pfill_stencil, *pbezier_stencil, *pblur,
                    *ppbr,          std::move(all)};
}

void PassBundle::acquire()
{
  for (auto const & p : all)
  {
    p->acquire();
  };
}

void PassBundle::release()
{
  for (auto const & p : all)
  {
    p->release();
  };
}

void PassBundle::add_pass(Dyn<Pass *> pass)
{
  pass->acquire();
  all.push(std::move(pass)).unwrap();
}

}    // namespace ash
