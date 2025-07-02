/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

// [ ] segmentation
// [ ] measure function
/// @brief An infinitely scrollable List of elements.
struct List : View
{
  typedef Fn<Option<Dyn<View *>>(AllocatorRef, usize i)> Generator;

  static constexpr auto DEFAULT_GENERATOR =
    [](AllocatorRef, usize) -> Option<Dyn<View *>> { return none; };

  struct State
  {
    /// @brief effective translation of the entire list
    f32 total_translation = 0;

    /// @brief the view extent of the viewport
    f32 view_extent = 0;

    /// @brief the first of the currently active subset
    usize first_item = 0;

    /// @brief determined upper bound
    usize max_count = USIZE_MAX;

    usize num_loaded = 0;

    Option<f32> item_size = none;

    /// @brief the item generator
    Generator generator = DEFAULT_GENERATOR;

    Vec<Dyn<View *>> items;

    Slice range() const
    {
      return Slice{first_item, items.size()};
    }

    Option<Slice> visible() const
    {
      return item_size.map([&](f32 s) {
        auto first =
          static_cast<usize>(std::abs(std::floor((-total_translation) / s)));
        auto count = static_cast<usize>(std::abs(std::ceil(view_extent / s)));
        return Slice{first, count};
      });
    }

  } state_;

  struct Style
  {
    Axis axis = Axis::X;

    Frame frame = Frame{}.abs(1, 1);

    Frame item_frame = Frame{}.abs(1, 1);
  } style_;

  AllocatorRef allocator_;

  List(Generator    generator = DEFAULT_GENERATOR,
       AllocatorRef allocator = default_allocator);

  List & generator(Generator generator);

  List & axis(Axis axis);

  List & frame(Frame frame);

  List & item_frame(Frame frame);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;
};

}    // namespace ui

}    // namespace ash
