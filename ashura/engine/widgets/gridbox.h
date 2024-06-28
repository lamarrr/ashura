#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

struct GridBoxItem
{
  u32  column_span = 1;
  u32  row_span    = 1;
  Vec2 alignment   = {0, 0};
};

struct GridBox : public Widget
{
  virtual GridBoxItem itemize(u32 child)
  {
    (void) child;
    return GridBoxItem{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame{width.resolve(allocated.x), height.resolve(allocated.y)};
    u32 const  num_children = (u32) sizes.size();
    u32        column       = 0;
    u32        row          = 0;

    {
      u32 child = 0;

      while (child < num_children)
      {
        // add to row based on span, otherwise add to column.
        child++;
      }
    }

    if (columns == 0)
    {
      columns = nchildren / rows + ((nchildren % rows) == 0 ? 0 : 1);
    }
    else if (rows == 0)
    {
      rows = nchildren / columns + ((nchildren % columns) == 0 ? 0 : 1);
    }

    f32 const  column_gap = (f32) (columns - 1) * column_gap;
    f32 const  row_gap    = (f32) (rows - 1) * row_gap;
    Vec2 const cell_size  = (self_extent - Vec2{column_gap, row_gap}) /
                           Vec2{(f32) columns, (f32) rows};

    if (items.is_empty())
    {
      children_allocation.fill(cell_size);
      return;
    }

    for (u32 i = 0; i < nchildren; i++)
    {
      GridItem const &item = items[i];
      Vec2            span_gap;

      if (item.column_span > 1)
      {
        span_gap.x = column_gap * (f32) (item.column_span - 1);
      }
      if (item.row_span > 1)
      {
        span_gap.y = row_gap * (f32) (item.row_span - 1);
      }

      children_allocation[i] =
          cell_size * Vec2{(f32) item.column_span, (f32) item.row_span} +
          span_gap;
    }
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    if (rows == 0 && columns == 0)
    {
      return Vec2{0, 0};
    }

    Vec2 const self_extent = frame.resolve(allocated_size);
    u32 const  nchildren   = (u32) children.size();
    u32        columns     = columns;
    u32        rows        = rows;

    if (columns == 0)
    {
      columns = nchildren / rows + ((nchildren % rows) == 0 ? 0 : 1);
    }
    else if (rows == 0)
    {
      rows = nchildren / columns + ((nchildren % columns) == 0 ? 0 : 1);
    }

    f32 const  column_gap = (f32) (columns - 1) * column_gap;
    f32 const  row_gap    = (f32) (rows - 1) * row_gap;
    Vec2 const cell_size  = (self_extent - Vec2{column_gap, row_gap}) /
                           Vec2{(f32) columns, (f32) rows};

    if (items.is_empty())
    {
      for (u32 i = 0; i < nchildren; i++)
      {
        u32 const  column   = i % columns;
        u32 const  row      = i / columns;
        Vec2 const position = (cell_size + Vec2{column_gap, row_gap}) *
                              Vec2{(f32) column, (f32) row};
        children_positions[i] =
            position + (cell_size - children_sizes[i]) * alignment;
      }
    }
    else
    {
      for (u32 i = 0; i < nchildren; i++)
      {
        Vec2 const position = (cell_size + Vec2{column_gap, row_gap}) *
                              Vec2{(f32) items[i].column, (f32) items[i].row};
        children_positions[i] =
            position +
            (children_allocations[i] - children_sizes[i]) * items[i].alignment;
      }
    }

    return self_extent;
  }

  u32            columns    = 0;
  u32            rows       = 0;
  f32            column_gap = 0;
  f32            row_gap    = 0;
  SizeConstraint width      = {};
  SizeConstraint height     = {};
};

}        // namespace ash