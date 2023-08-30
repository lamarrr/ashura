#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash
{

struct FlexProps
{
  Direction      direction   = Direction::H;                          // flex direction to layout children along
  Wrap           wrap        = Wrap::Wrap;                            /// wrap to a new block or not
  MainAlign      main_align  = MainAlign::Start;                      /// main-axis alignment. specifies how free space is used on the main axis
  CrossAlign     cross_align = CrossAlign::Start;                     /// cross-axis alignment. affects how free space is used on the cross axis
  SizeConstraint frame       = SizeConstraint::relative(1, 1);        /// frame size to use for layout. this is not same as the actual extent of the flex
};

struct Flex : public Widget
{
  template <WidgetImpl... DerivedWidget>
  explicit Flex(FlexProps iprops, DerivedWidget... ichildren) :
      props{iprops}
  {
    update_children(std::move(ichildren)...);
  }

  STX_DISABLE_COPY(Flex)
  STX_DEFAULT_MOVE(Flex)

  virtual ~Flex() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <WidgetImpl... DerivedWidget>
  void update_children(DerivedWidget... new_children)
  {
    for (Widget *child : children)
    {
      delete child;
    }
    children.clear();
    (children.push(new DerivedWidget{std::move(new_children)}).unwrap(), ...);
  }

  /// takes ownership of the children
  void update_children(stx::Span<Widget *const> new_children)
  {
    for (Widget *child : children)
    {
      delete child;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Flex"};
  }

  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation) override
  {
    children_allocation.fill(props.frame.resolve(allocated_size));
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    vec2 frame = props.frame.resolve(allocated_size);
    vec2 span;
    f32  cross_axis_cursor = 0;

    for (usize i = 0; i < children_sizes.size();)
    {
      usize iblock_end              = i + 1;
      f32   block_main_axis_extent  = 0;
      f32   block_cross_axis_extent = 0;
      f32   block_main_axis_spacing = 0;

      if (props.direction == Direction::H)
      {
        block_main_axis_extent  = children_sizes[i].x;
        block_cross_axis_extent = children_sizes[i].y;
        children_positions[i].y = cross_axis_cursor;

        for (; iblock_end < children_sizes.size(); iblock_end++)
        {
          if (props.wrap == Wrap::None || props.wrap == Wrap::Wrap && block_main_axis_extent + children_sizes[iblock_end].x <= frame.x)
          {
            children_positions[iblock_end].y = cross_axis_cursor;
            block_main_axis_extent += children_sizes[iblock_end].x;
            block_cross_axis_extent = std::max(block_cross_axis_extent, children_sizes[iblock_end].y);
          }
          else
          {
            break;
          }
        }

        if (props.main_align != MainAlign::Start)
        {
          block_main_axis_spacing = std::max(frame.x - block_main_axis_extent, 0.0f);
        }
      }
      else
      {
        block_main_axis_extent  = children_sizes[i].y;
        block_cross_axis_extent = children_sizes[i].x;
        children_positions[i].x = cross_axis_cursor;

        for (; iblock_end < children_sizes.size(); iblock_end++)
        {
          if (props.wrap == Wrap::None || props.wrap == Wrap::Wrap && block_main_axis_extent + children_sizes[iblock_end].y <= frame.y)
          {
            children_positions[iblock_end].x = cross_axis_cursor;
            block_main_axis_extent += children_sizes[iblock_end].y;
            block_cross_axis_extent = std::max(block_cross_axis_extent, children_sizes[iblock_end].x);
          }
          else
          {
            break;
          }
        }

        if (props.main_align != MainAlign::Start)
        {
          block_main_axis_spacing = std::max(frame.y - block_main_axis_extent, 0.0f);
        }
      }

      switch (props.cross_align)
      {
        case CrossAlign::Start:
          break;

        case CrossAlign::Center:
          if (props.direction == Direction::H)
          {
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y += (block_cross_axis_extent - children_sizes[iblock].y) / 2;
            }
          }
          else
          {
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x += (block_cross_axis_extent - children_sizes[iblock].x) / 2;
            }
          }
          break;

        case CrossAlign::End:
          if (props.direction == Direction::H)
          {
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y += block_cross_axis_extent - children_sizes[iblock].y;
            }
          }
          else
          {
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x += block_cross_axis_extent - children_sizes[iblock].x;
            }
          }
          break;

        default:
          break;
      }

      usize nblock_children = iblock_end - i;

      if (props.direction == Direction::H)
      {
        switch (props.main_align)
        {
          case MainAlign::Start:
          {
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].x;
            }
          }
          break;

          case MainAlign::SpaceAround:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children * 2);
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              main_axis_spacing_cursor += spacing;
              children_positions[iblock].x = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].x + spacing;
            }
          }
          break;

          case MainAlign::SpaceBetween:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children - 1);
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].x + spacing;
            }
          }
          break;

          case MainAlign::SpaceEvenly:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children + 1);
            f32 main_axis_spacing_cursor = spacing;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].x + spacing;
            }
          }
          break;

          case MainAlign::End:
          {
            f32 main_axis_spacing_cursor = block_main_axis_spacing;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].x = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].x;
            }
          }
          break;

          default:
            break;
        }
      }
      else
      {
        switch (props.main_align)
        {
          case MainAlign::Start:
          {
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].y;
            }
          }
          break;

          case MainAlign::SpaceAround:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children * 2);
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              main_axis_spacing_cursor += spacing;
              children_positions[iblock].y = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].y + spacing;
            }
          }
          break;

          case MainAlign::SpaceBetween:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children - 1);
            f32 main_axis_spacing_cursor = 0;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].y + spacing;
            }
          }
          break;

          case MainAlign::SpaceEvenly:
          {
            f32 spacing                  = block_main_axis_spacing / (nblock_children + 1);
            f32 main_axis_spacing_cursor = spacing;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].y + spacing;
            }
          }
          break;

          case MainAlign::End:
          {
            f32 main_axis_spacing_cursor = block_main_axis_spacing;
            for (usize iblock = i; iblock < iblock_end; iblock++)
            {
              children_positions[iblock].y = main_axis_spacing_cursor;
              main_axis_spacing_cursor += children_sizes[iblock].y;
            }
          }
          break;

          default:
            break;
        }
      }

      cross_axis_cursor += block_cross_axis_extent;

      if (props.direction == Direction::H)
      {
        span.x = std::max(span.x, block_main_axis_extent + block_main_axis_spacing);
        span.y = cross_axis_cursor;
      }
      else
      {
        span.x = cross_axis_cursor;
        span.y = std::max(span.y, block_main_axis_extent + block_main_axis_spacing);
      }

      i = iblock_end;
    }

    return span;
  }

  FlexProps          props;
  stx::Vec<Widget *> children;
};

}        // namespace ash
