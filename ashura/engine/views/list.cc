/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/list.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

List::List(Generator generator, Allocator allocator) :
  state_{.generator = generator, .items{allocator}},
  allocator_{allocator}
{
}

List & List::generator(Generator generator)
{
  state_.total_translation = 0;
  state_.view_extent       = 0;
  state_.first_item        = 0;
  state_.max_count         = USIZE_MAX;
  state_.num_loaded        = 0;
  state_.item_size         = none;
  state_.generator         = generator;
  state_.items.clear();
  return *this;
}

List & List::axis(Axis axis)
{
  style_.axis = axis;
  return *this;
}

List & List::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

List & List::item_frame(Frame frame)
{
  style_.item_frame = frame;
  return *this;
}

ui::State List::tick(Ctx const &, Events const & events, Fn<void(View &)> build)
{
  u32 const axis = style_.axis == Axis::X ? 0 : 1;

  if (events.scroll())
  {
    auto info                = events.scroll_info.unwrap();
    state_.total_translation = info.center[axis];
  }

  Slice visible = state_.visible().unwrap_or(Slice{0, 1})(state_.max_count);

  if (visible != state_.range())
  {
    auto old_range = state_.range();
    auto i         = visible.begin();

    for (; i < visible.end(); i++)
    {
      if (old_range.contains(i))
      {
        state_.items.push(std::move(state_.items[i])).unwrap();
      }
      else
      {
        if (auto item = state_.generator(allocator_, i); item.is_some())
        {
          state_.items.push(item.unwrap()).unwrap();
        }
        else
        {
          state_.max_count = i;
          break;
        }
      }
    }

    state_.items.erase(0, old_range.span);
    state_.first_item = visible.begin();
    state_.num_loaded = max(state_.range().end(), state_.num_loaded);
  }

  // [ ] ScrollBar: NEED TO GET SIZE INFO

  for (auto const & item : state_.items)
  {
    build(*item);
  }

  return ui::State{.scrollable = true, .viewport = true};
}

void List::size(f32x2 allocated, Span<f32x2> sizes)
{
  fill(sizes, style_.item_frame(style_.frame(allocated)));
}

Layout List::fit(f32x2 allocated, Span<f32x2 const> sizes, Span<f32x2> centers)
{
  auto      frame      = style_.frame(allocated);
  f32x2     extent     = {};
  u32 const axis       = style_.axis == Axis::X ? 0 : 1;
  u32 const cross_axis = style_.axis == Axis::X ? 1 : 0;

  // Calculate total extent along main axis
  for (auto const size : sizes)
  {
    extent[cross_axis] = max(extent[cross_axis], size[cross_axis]);
    extent[axis] += size[axis];
  }

  // Position items along main axis with translation
  auto first_item_offset = state_.first_item * state_.item_size.unwrap_or();

  f32 cursor = -0.5F * extent[axis];
  cursor += state_.total_translation;
  cursor -= first_item_offset;

  for (auto [center, size] : zip(centers, sizes))
  {
    center[axis]       = cursor + size[axis] * 0.5F;
    center[cross_axis] = 0;
    cursor += size[axis];
  }

  if (!sizes.is_empty())
  {
    state_.item_size = sizes[0][axis];
  }

  state_.view_extent = frame[axis];

  return {
    .extent          = frame,
    .viewport_extent = extent,
    .viewport_center = {-state_.total_translation, 0}
  };
}
}    // namespace ui

}    // namespace ash
