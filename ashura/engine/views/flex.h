/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{
namespace ui
{

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct Flex : View
{
  struct Style
  {
    Axis axis : 2 = Axis::X;

    bool wrap : 1 = true;

    MainAlign main_align : 3 = MainAlign::Start;

    f32 cross_align = 0;

    Frame frame = Frame{}.rel(1, 1);

    Frame item_frame = Frame{}.rel(1, 1);
  } style_;

  Vec<ref<View>> items_;

  Flex(AllocatorRef allocator = default_allocator);
  Flex(Flex const &)             = delete;
  Flex(Flex &&)                  = default;
  Flex & operator=(Flex const &) = delete;
  Flex & operator=(Flex &&)      = default;
  virtual ~Flex() override       = default;

  Flex & axis(Axis axis);

  Flex & wrap(bool wrap);

  Flex & main_align(MainAlign align);

  Flex & cross_align(f32 align);

  Flex & frame(Frame frame);

  Flex & item_frame(Frame frame);

  Flex & items(std::initializer_list<ref<View>> list);

  Flex & items(Span<ref<View> const> list);

  virtual State tick(Ctx const & ctx, Events const & events,
                     Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;
};

}    // namespace ui
}    // namespace ash
