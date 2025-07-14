/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/stack.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Stack::Stack(AllocatorRef allocator) : items_{allocator}
{
}

Stack & Stack::reverse(bool r)
{
  style_.reverse = r;
  return *this;
}

Stack & Stack::align(f32x2 a)
{
  style_.alignment = a;
  return *this;
}

Stack & Stack::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Stack & Stack::items(std::initializer_list<ref<View>> list)
{
  items(span(list));
  return *this;
}

Stack & Stack::items(Span<ref<View> const> list)
{
  items_.extend(span(list)).unwrap();
  return *this;
}

i32 Stack::stack_item(i32 base, u32 i, u32 num)
{
  // sequential stacking
  if (!style_.reverse)
  {
    return base + (i32) i;
  }
  else
  {
    return base + (i32) (num - i);
  }
}

ui::State Stack::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ui::State{};
}

void Stack::size(f32x2 allocated, Span<f32x2> sizes)
{
  fill(sizes, style_.frame(allocated));
}

Layout Stack::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
  f32x2 span;

  for (auto style : sizes)
  {
    span.x = max(span.x, style.x);
    span.y = max(span.y, style.y);
  }

  for (auto [center, size] : zip(centers, sizes))
  {
    center = space_align(span, size, style_.alignment);
  }

  return {.extent = span};
}

i32 Stack::z_index(i32 allocated, Span<i32> indices)
{
  auto n = indices.size();
  for (auto [i, stack_index] : enumerate(indices))
  {
    stack_index = stack_item(allocated, i, n);
  }

  return allocated;
}

}    // namespace ui

}    // namespace ash
