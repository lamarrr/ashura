#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

// needed because we need to be able to render a view that is part of another
// view without adding the elements of the view to the root view
//
// view pass should also check if the view has already been rendered for the
// current frame.
//
// TODO(lamarrr): should be able to request render of another view
//
// get metadata for another pass belonging to a view
// i.e. get color attachment for view, get depth attachment for view, get
// framebuffer, renderpass. named tagged and used for a specific purpose,
// possibly referenced by all passes if they need to modify or add data atop of
// it but what if another view modifies it?
//
struct ViewPassNode
{
  u32 view = UID32_INVALID;
};

struct ViewPass
{
  // render to view's frame buffer and then composite onto the present view
  // there must be no recursion happening here
  SparseVec<u32> id_map = {};

  uid32 add_view()
  {
  }

  void remove_view(uid32);

  static void encode();
};

}        // namespace ash
