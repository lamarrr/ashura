#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash
{
namespace gui
{

struct GridItem
{
  u32       column      = 0;
  u32       column_span = 1;
  u32       row         = 0;
  u32       row_span    = 1;
  Alignment alignment   = ALIGN_LEFT_CENTER;
};

struct GridProps
{
  u32                columns    = 2;
  u32                rows       = 0;
  f32                column_gap = 0;
  f32                row_gap    = 0;
  Alignment          alignment  = ALIGN_TOP_LEFT;
  stx::Vec<GridItem> items;
  Constraint2D       frame = Constraint2D::relative(1, 1);
};

struct Grid : public Widget
{
  template <Impl<Widget>... DerivedWidget>
  explicit Grid(GridProps iprops, DerivedWidget... ichildren) :
      props{std::move(iprops)}
  {
    update_children(std::move(ichildren)...);
  }

  STX_DISABLE_COPY(Grid)
  STX_DEFAULT_MOVE(Grid)

  virtual ~Grid() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <Impl<Widget>... DerivedWidget>
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
    return WidgetDebugInfo{.type = "Grid"};
  }

  virtual void allocate_size(Context &ctx, Vec2 allocated_size, stx::Span<Vec2> children_allocation) override
  {
    ASH_CHECK(props.items.is_empty() || props.items.size() == children.size());

    if (props.rows == 0 && props.columns == 0)
    {
      return;
    }

    Vec2 const self_extent = props.frame.resolve(allocated_size);
    u32 const  nchildren   = (u32) children.size();
    u32        columns     = props.columns;
    u32        rows        = props.rows;

    if (columns == 0)
    {
      columns = nchildren / props.rows + ((nchildren % props.rows) == 0 ? 0 : 1);
    }
    else if (rows == 0)
    {
      rows = nchildren / props.columns + ((nchildren % props.columns) == 0 ? 0 : 1);
    }

    f32 const  column_gap = (f32) (columns - 1) * props.column_gap;
    f32 const  row_gap    = (f32) (rows - 1) * props.row_gap;
    Vec2 const cell_size  = (self_extent - Vec2{column_gap, row_gap}) / Vec2{(f32) columns, (f32) rows};

    if (props.items.is_empty())
    {
      children_allocation.fill(cell_size);
      return;
    }

    for (u32 i = 0; i < nchildren; i++)
    {
      GridItem const &item = props.items[i];
      Vec2            span_gap;

      if (item.column_span > 1)
      {
        span_gap.x = props.column_gap * (f32) (item.column_span - 1);
      }
      if (item.row_span > 1)
      {
        span_gap.y = props.row_gap * (f32) (item.row_span - 1);
      }

      children_allocation[i] = cell_size * Vec2{(f32) item.column_span, (f32) item.row_span} + span_gap;
    }
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size, stx::Span<Vec2 const> children_allocations, stx::Span<Vec2 const> children_sizes, stx::Span<Vec2> children_positions) override
  {
    ASH_CHECK(props.items.is_empty() || props.items.size() == children.size());

    if (props.rows == 0 && props.columns == 0)
    {
      return Vec2{0, 0};
    }

    Vec2 const self_extent = props.frame.resolve(allocated_size);
    u32 const  nchildren   = (u32) children.size();
    u32        columns     = props.columns;
    u32        rows        = props.rows;

    if (columns == 0)
    {
      columns = nchildren / props.rows + ((nchildren % props.rows) == 0 ? 0 : 1);
    }
    else if (rows == 0)
    {
      rows = nchildren / props.columns + ((nchildren % props.columns) == 0 ? 0 : 1);
    }

    f32 const  column_gap = (f32) (columns - 1) * props.column_gap;
    f32 const  row_gap    = (f32) (rows - 1) * props.row_gap;
    Vec2 const cell_size  = (self_extent - Vec2{column_gap, row_gap}) / Vec2{(f32) columns, (f32) rows};

    if (props.items.is_empty())
    {
      for (u32 i = 0; i < nchildren; i++)
      {
        u32 const  column     = i % props.columns;
        u32 const  row        = i / props.columns;
        Vec2 const position   = (cell_size + Vec2{props.column_gap, props.row_gap}) * Vec2{(f32) column, (f32) row};
        children_positions[i] = position + (cell_size - children_sizes[i]) * props.alignment;
      }
    }
    else
    {
      for (u32 i = 0; i < nchildren; i++)
      {
        Vec2 const position   = (cell_size + Vec2{props.column_gap, props.row_gap}) * Vec2{(f32) props.items[i].column, (f32) props.items[i].row};
        children_positions[i] = position + (children_allocations[i] - children_sizes[i]) * props.items[i].alignment;
      }
    }

    return self_extent;
  }

  GridProps          props;
  stx::Vec<Widget *> children;
  stx::Vec<f32>      row_heights;
  stx::Vec<Vec2>     column_widths;
};

}        // namespace gui
}        // namespace ash
