#pragma once

#include "ashura/engine/widget.h"

namespace ash
{

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct FlexBox : public Widget
{
  Axis           axis        = Axis::Horizontal;
  bool           wrap        = true;
  MainAlign      main_align  = MainAlign::Start;
  CrossAlign     cross_align = CrossAlign::Start;
  SizeConstraint width       = {};
  SizeConstraint height      = {};

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame{width.resolve(allocated.x), height.resolve(allocated.y)};
    fill(sizes, frame);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    u32 const  num_children = (u32) sizes.size();
    Vec2 const frame{width.resolve(allocated.x), height.resolve(allocated.y)};
    Vec2       span;
    f32        cross_cursor = 0;
    u8 const   main_axis    = (axis == Axis::Horizontal) ? 0 : 1;
    u8 const   cross_axis   = (axis == Axis::Horizontal) ? 1 : 0;

    for (u32 i = 0; i < num_children;)
    {
      u32 first                  = i++;
      f32 main_extent            = sizes[first][main_axis];
      f32 cross_extent           = sizes[first][cross_axis];
      f32 main_spacing           = 0;
      offsets[first][cross_axis] = cross_cursor;

      while (i < num_children &&
             !(wrap && (main_extent + sizes[i][main_axis]) > frame[main_axis]))
      {
        offsets[i][main_axis] = cross_cursor;
        main_extent += sizes[i][main_axis];
        cross_extent = max(cross_extent, sizes[i][cross_axis]);
        i++;
      }

      u32 const count = i - first;

      if (main_align != MainAlign::Start)
      {
        main_spacing = max(frame[main_axis] - main_extent, 0.0f);
      }

      switch (cross_align)
      {
        case CrossAlign::Start:
          break;

        case CrossAlign::Center:
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][cross_axis] += (cross_extent - sizes[b][cross_axis]) / 2;
          }
          break;

        case CrossAlign::End:
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][cross_axis] += cross_extent - sizes[b][cross_axis];
          }
          break;

        default:
          break;
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

      cross_cursor += cross_extent;

      span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
      span[cross_axis] = cross_cursor;
    }

    return span;
  }
};

}        // namespace ash