/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/space.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Space & Space::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

Layout Space::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  return Layout{.extent = style_.frame(allocated)};
}

}    // namespace ui

}    // namespace ash
