#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{
struct Padding : public Widget
{
  template <Impl<Widget> DerivedWidget>
  Padding(EdgeInsets iedge_insets, DerivedWidget ichild) :
      edge_insets{iedge_insets}
  {
    children.push(new DerivedWidget{std::move(ichild)}).unwrap();
  }

  STX_DISABLE_COPY(Padding)
  STX_DEFAULT_MOVE(Padding)

  virtual ~Padding() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  template <Impl<Widget> DerivedWidget>
  void update_child(DerivedWidget widget)
  {
    update_child(new DerivedWidget{std::move(widget)});
  }

  void update_child(Widget *widget)
  {
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = widget;
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Padding"};
  }

  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             stx::Span<Vec2> children_allocation) override
  {
    Vec2 child_size = allocated_size - edge_insets.xy();
    child_size.x    = op::max(child_size.x, 0);
    child_size.y    = op::max(child_size.y, 0);
    children_allocation.fill(child_size);
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    children_positions[0] = edge_insets.top_left();
    Vec2 child_cover      = children_sizes[0] + edge_insets.xy();
    child_cover.x         = op::min(child_cover.x, allocated_size.x);
    child_cover.y         = op::min(child_cover.y, allocated_size.y);
    return child_cover;
  }

  EdgeInsets         edge_insets;
  stx::Vec<Widget *> children;
};

}        // namespace gui
}        // namespace ash
