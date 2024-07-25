/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct StackBox : public View
{
  bool  reverse   = false;
  Vec2  alignment = {0, 0};
  Frame frame     = {};

  virtual View *child(u32 i) override final
  {
    return item(i);
  }

  virtual View *item(u32 i)
  {
    (void) i;
    return nullptr;
  }

  virtual Vec2 align_item(u32 i)
  {
    (void) i;
    return alignment;
  }

  virtual i32 stack_item(i32 base, u32 i, u32 num)
  {
    i64 z = base + 1;
    if (!reverse)
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
    fill(sizes, frame(allocated));
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

  virtual i32 stack(i32 allocated, Span<i32> indices) override
  {
    for (u32 i = 0; i < indices.size32(); i++)
    {
      indices[i] = stack_item(allocated, i, indices.size32());
    }
    return allocated;
  }
};

}        // namespace ash