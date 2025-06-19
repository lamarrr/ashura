/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/engine/views/space.h"

namespace ash
{

namespace ui
{

Space & Space::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

Layout Space::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{.extent = style_.frame(allocated)};
}

}    // namespace ui

}    // namespace ash
