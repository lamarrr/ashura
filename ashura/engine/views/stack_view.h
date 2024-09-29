/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct StackView : public View
{
  struct Style
  {
    bool  reverse : 1 = false;
    Vec2  alignment   = {0, 0};
    Frame frame       = {.width = {.scale = 1}, .height = {.scale = 1}};
  } style;

  virtual Vec2 align_item(u32 i)
  {
    (void) i;
    return style.alignment;
  }

  virtual i32 stack_item(i32 base, u32 i, u32 num)
  {
    i64 z = base + 1;
    if (!style.reverse)
    {
      z += (i32) i;
    }
    else
    {
      z += (i32) (num - (i + 1));
    }
    return z;
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override final
  {
    fill(sizes, style.frame(allocated));
  }

  virtual Vec2 fit(Vec2, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override final
  {
    Vec2      span;
    u32 const num_children = sizes.size32();

    for (Vec2 s : sizes)
    {
      span.x = max(span.x, s.x);
      span.y = max(span.y, s.y);
    }

    for (u32 i = 0; i < num_children; i++)
    {
      offsets[i] = space_align(span, sizes[i], align_item(i));
    }

    return span;
  }

  virtual i32 z_index(i32 allocated, Span<i32> indices) override
  {
    for (u32 i = 0; i < indices.size32(); i++)
    {
      indices[i] = stack_item(allocated, i, indices.size32());
    }
    return allocated;
  }
};

}        // namespace ash