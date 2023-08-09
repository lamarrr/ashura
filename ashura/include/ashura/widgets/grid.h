#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash
{

struct GridItem
{
  u32       column       = 0;
  u32       column_span  = 1;
  u32       row          = 0;
  u32       row_span     = 1;
  Alignment alignment    = ALIGN_TOP_LEFT;
  bool      expand_cells = false;
};

struct GridProps
{
  u32                columns    = 3;
  f32                column_gap = 0;
  f32                row_gap    = 0;
  Alignment          alignment  = ALIGN_TOP_LEFT;
  stx::Vec<GridItem> items;
  SizeConstraint     frame = SizeConstraint::relative(1, 1);
};

struct Grid : public Widget
{
  template <WidgetImpl... DerivedWidget>
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
    return WidgetDebugInfo{.type = "Grid"};
  }

  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation) override
  {
    ASH_CHECK(props.items.is_empty() || props.items.size() == children.size());
    // TODO(lamarrr): handle props.columns == 0
    vec2 const self_extent = props.frame.resolve(allocated_size);
    u32 const  rows        = (u32) (props.columns == 0 ? 0 : ((children.size() / props.columns) + (((children.size() % props.columns) == 0) ? 0 : 1)));
    f32 const  column_gap  = props.columns <= 1 ? 0.0f : ((props.columns - 1) * props.column_gap);
    f32 const  row_gap     = rows <= 1 ? 0.0f : ((rows - 1) * props.row_gap);        // TODO(lamarrr): handle rows == 0
    vec2 const cell_size   = (self_extent - vec2{column_gap, row_gap}) / vec2{(f32) props.columns, (f32) rows};

    if (props.items.is_empty())
    {
      children_allocation.fill(cell_size);
      return;
    }

    for (u32 i = 0; i < (u32) children.size(); i++)
    {
      children_allocation[i] = vec2{cell_size.x * props.items[i].column_span, cell_size.y * props.items[i].row_span};
    }
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    vec2 const self_extent = props.frame.resolve(allocated_size);
    u32 const  rows        = (u32) (props.columns == 0 ? 0 : ((children.size() / props.columns) + (((children.size() % props.columns) == 0) ? 0 : 1)));
    f32 const  column_gap  = props.columns <= 1 ? 0.0f : ((props.columns - 1) * props.column_gap);
    f32 const  row_gap     = rows <= 1 ? 0.0f : ((rows - 1) * props.row_gap);
    vec2 const cell_size   = (self_extent - vec2{column_gap, row_gap}) / vec2{(f32) props.columns, (f32) rows};
// spacing
    if (props.items.is_empty())
    {
      for (u32 i = 0; i < (u32) children.size(); i++)
      {
        vec2 const position   = vec2{(i % props.columns) * cell_size.x, (i / props.columns) * cell_size.y};
        children_positions[i] = position + (cell_size - children_sizes[i]) * props.alignment;
      }
    }
    else
    {
      for (u32 i = 0; i < (u32) children.size(); i++)
      {
        vec2 const position   = vec2{props.items[i].column * cell_size.x, props.items[i].row * cell_size.y};
        children_positions[i] = position + (children_allocations[i] - children_sizes[i]) * props.items[i].alignment;
      }
    }

    return self_extent;
  }

  GridProps          props;
  stx::Vec<Widget *> children;
  stx::Vec<f32>      row_heights;
  stx::Vec<vec2>     column_widths;
};

}        // namespace ash
