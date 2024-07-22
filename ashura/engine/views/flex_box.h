/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"

namespace ash
{

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct FlexBox : public View
{
  Axis      axis        = Axis::X;
  bool      wrap        = true;
  bool      reverse     = false;
  MainAlign main_align  = MainAlign::Start;
  f32       cross_align = 0;
  Frame     frame       = {};

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
    Vec2 const frame = this->frame(allocated);
    fill(sizes, frame);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) const override
  {
    u32 const  num_children = sizes.size32();
    Vec2 const frame        = this->frame(allocated);
    Vec2       span;
    f32        cross_cursor = 0;
    u8 const   main_axis    = (axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (axis == Axis::X) ? 1 : 0;

    for (u32 i = 0; i < num_children;)
    {
      u32 first        = i++;
      f32 main_extent  = sizes[first][main_axis];
      f32 cross_extent = sizes[first][cross_axis];
      f32 main_spacing = 0;

      while (i < num_children &&
             !(wrap && (main_extent + sizes[i][main_axis]) > frame[main_axis]))
      {
        main_extent += sizes[i][main_axis];
        cross_extent = max(cross_extent, sizes[i][cross_axis]);
        i++;
      }

      u32 const count = i - first;

      if (main_align != MainAlign::Start)
      {
        main_spacing = max(frame[main_axis] - main_extent, 0.0f);
      }

      for (u32 b = first; b < first + count; b++)
      {
        offsets[b][cross_axis] =
            cross_cursor +
            space_align(frame[cross_axis], sizes[b][cross_axis], cross_align);
      }

      switch (main_align)
      {
        case MainAlign::Start:
        {
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis];
          }
        }
        break;

        case MainAlign::SpaceAround:
        {
          f32 spacing             = main_spacing / (count * 2);
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            main_spacing_cursor += spacing;
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::SpaceBetween:
        {
          f32 spacing             = main_spacing / (count - 1);
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::SpaceEvenly:
        {
          f32 spacing             = main_spacing / (count + 1);
          f32 main_spacing_cursor = spacing;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::End:
        {
          f32 main_spacing_cursor = main_spacing;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis];
          }
        }
        break;

        default:
          break;
      }

      if (reverse)
      {
        for (u32 b0 = first, b1 = first + count - 1; b0 < b1; b0++, b1--)
        {
          swap(offsets[b0][main_axis], offsets[b1][main_axis]);
        }
      }

      cross_cursor += cross_extent;

      span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
      span[cross_axis] = cross_cursor;
    }

    return span;
  }
};

}        // namespace ash