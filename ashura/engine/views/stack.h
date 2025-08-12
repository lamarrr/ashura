/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct Stack : View
{
  struct Style
  {
    bool reverse = false;

    f32x2 alignment = ALIGNMENT_CENTER_CENTER;

    Frame frame = Frame{}.rel(1, 1);
  } style_;

  Vec<ref<View>> items_;

  Stack(Allocator allocator = default_allocator);
  Stack(Stack const &)             = delete;
  Stack(Stack &&)                  = default;
  Stack & operator=(Stack const &) = delete;
  Stack & operator=(Stack &&)      = default;
  virtual ~Stack() override        = default;

  Stack & reverse(bool reverse);

  Stack & align(f32x2 alignment);

  Stack & frame(Frame frame);

  Stack & items(std::initializer_list<ref<View>> list);

  Stack & items(Span<ref<View> const> list);

  virtual i32 stack_item(i32 base, u32 index, u32 num);

  virtual State tick(Ctx const & ctx, Events const & events,
                     Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual i32 z_index(i32 allocated, Span<i32> indices) override;
};

}    // namespace ui

}    // namespace ash
