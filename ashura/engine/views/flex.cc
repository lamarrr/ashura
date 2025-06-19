/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/engine/views/flex.h"                                    

namespace ash
{

namespace ui
{

Flex::Flex(AllocatorRef allocator) : items_{allocator}
{
}

Flex & Flex::axis(Axis a)
{
  style_.axis = a;
  return *this;
}

Flex & Flex::wrap(bool w)
{
  style_.wrap = w;
  return *this;
}

Flex & Flex::main_align(MainAlign align)
{
  style_.main_align = align;
  return *this;
}

Flex & Flex::cross_align(f32 align)
{
  style_.cross_align = align;
  return *this;
}

Flex & Flex::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Flex & Flex::item_frame(Frame f)
{
  style_.item_frame = f;
  return *this;
}

Flex & Flex::items(std::initializer_list<ref<View>> list)
{
  return items(span(list));
}

Flex & Flex::items(Span<ref<View> const> list)
{
  items_.extend(list).unwrap();
  return *this;
}

ui::State Flex::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ui::State{};
}

void Flex::size(Vec2 allocated, Span<Vec2> sizes)
{
  auto const frame = style_.frame(allocated);
  fill(sizes, style_.item_frame(frame));
}

Layout Flex::fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  auto const n            = sizes.size();
  auto const frame        = style_.frame(allocated);
  u32 const  main_axis    = (style_.axis == Axis::X) ? 0 : 1;
  u32 const  cross_axis   = (style_.axis == Axis::X) ? 1 : 0;
  Vec2       span         = {};
  f32        cross_cursor = 0;

  for (usize i = 0; i < n;)
  {
    auto first        = i++;
    f32  main_extent  = sizes[first][main_axis];
    f32  cross_extent = sizes[first][cross_axis];
    f32  main_spacing = 0;

    while (i < n && !(style_.wrap &&
                      (main_extent + sizes[i][main_axis]) > frame[main_axis]))
    {
      main_extent += sizes[i][main_axis];
      cross_extent = max(cross_extent, sizes[i][cross_axis]);
      i++;
    }

    auto const count = i - first;

    if (style_.main_align != MainAlign::Start)
    {
      main_spacing = max(frame[main_axis] - main_extent, 0.0F);
    }

    for (auto [center, size] :
         zip(centers.slice(first, count), sizes.slice(first, count)))
    {
      f32 const pos =
        space_align(cross_extent, size[cross_axis], style_.cross_align);
      center[cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
    }

    switch (style_.main_align)
    {
      case MainAlign::Start:
      {
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis];
        }
      }
      break;

      case MainAlign::SpaceAround:
      {
        f32 spacing             = main_spacing / (count * 2);
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          main_spacing_cursor += spacing;
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::SpaceBetween:
      {
        f32 spacing             = main_spacing / (count - 1);
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::SpaceEvenly:
      {
        f32 spacing             = main_spacing / (count + 1);
        f32 main_spacing_cursor = spacing;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::End:
      {
        f32 main_spacing_cursor = main_spacing;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis];
        }
      }
      break;

      default:
        break;
    }

    cross_cursor += cross_extent;

    span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
    span[cross_axis] = cross_cursor;
  }

  // convert from cursor space [0, w] to parent space [-0.5w, 0.5w]
  for (auto & center : centers)
  {
    center -= span * 0.5F;
  }

  return {.extent = span};
}

}    // namespace ui

}    // namespace ash
