#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

// object-clip space blur
//
// - capture scene at object's screen-space area, dilate by the blur extent
// - reserve scratch stencil image with at least size of the dilated area
// - blur captured area
// - render object to offscreen scratch image stencil only
// - using rendered stencil, directly-write (without blending) onto scene again
struct BlurPass
{
};

}        // namespace ash
