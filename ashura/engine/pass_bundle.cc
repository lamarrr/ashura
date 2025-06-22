/// SPDX-License-Identifier: MIT
#include "ashura/engine/pass_bundle.h"

namespace ash
{

PassBundle PassBundle::create(AllocatorRef allocator)
{
  Dyn blur            = dyn(allocator, BlurPass{allocator}).unwrap();
  Dyn contour_stencil = dyn(allocator, ContourStencilPass{allocator}).unwrap();
  Dyn ngon            = dyn(allocator, NgonPass{allocator}).unwrap();
  Dyn pbr             = dyn(allocator, PBRPass{allocator}).unwrap();
  Dyn sdf             = dyn(allocator, SdfPass{allocator}).unwrap();
  Dyn quad            = dyn(allocator, QuadPass{allocator}).unwrap();

  auto * pblur            = blur.get();
  auto * pngon            = ngon.get();
  auto * ppbr             = pbr.get();
  auto * psdf             = sdf.get();
  auto * pquad            = quad.get();
  auto * pcontour_stencil = contour_stencil.get();

  Vec<Dyn<Pass *>> all{allocator};

  all.push(cast<Pass *>(std::move(blur))).unwrap();
  all.push(cast<Pass *>(std::move(ngon))).unwrap();
  all.push(cast<Pass *>(std::move(pbr))).unwrap();
  all.push(cast<Pass *>(std::move(sdf))).unwrap();
  all.push(cast<Pass *>(std::move(contour_stencil))).unwrap();
  all.push(cast<Pass *>(std::move(quad))).unwrap();

  return PassBundle{*pblur, *pcontour_stencil, *pngon, *ppbr, *psdf,
                    *pquad, std::move(all)};
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
