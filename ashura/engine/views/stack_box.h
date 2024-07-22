/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct StackBox : public View
{
  bool  reverse   = false;
  Vec2  alignment = {0, 0};
  Frame frame     = {};

  virtual View *child(u32 i) const override final
  {
    return item(i);
  }

  virtual View *item(u32 i) const
  {
    (void) i;
    return nullptr;
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) const override
  {
    fill(sizes, frame(allocated));
  }

  virtual Vec2 fit(Vec2, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) const override
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
      offsets[i] = space_align(span, sizes[i], alignment);
    }

    return span;
  }

  virtual i32 stack(i32 z_index, Span<i32> allocation) const override
  {
    if (!reverse)
    {
      iota(allocation, z_index + 1);
    }
    else
    {
      riota(allocation, z_index + 1);
    }
    return z_index;
  }
};

}        // namespace ash