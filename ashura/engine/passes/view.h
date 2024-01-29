#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{
namespace gfx
{

// needed because we need to be able to render a view that is part of another
// view without adding the elements of the view to the root view
//
// view pass should also check if the view has already been rendered for the
// current frame.
//
// TODO(lamarrr): should be able to request render of another view
//
//
struct ViewPass
{
  // render to view's frame buffer and then composite onto the present view
  // there must be no recursion happening here
  uid32 view;
};

}        // namespace gfx
}        // namespace ash

