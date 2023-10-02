#pragma once

#include "ashura/primitives.h"
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

  virtual void allocate_size(Context &ctx, Vec2 allocated_size, stx::Span<Vec2> children_allocation) override
  {
    children_allocation.fill(max(allocated_size - edge_insets.xy(), Vec2{0, 0}));
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size, stx::Span<Vec2 const> children_allocations, stx::Span<Vec2 const> children_sizes, stx::Span<Vec2> children_positions) override
  {
    children_positions[0] = edge_insets.top_left();
    return min(children_sizes[0] + edge_insets.xy(), allocated_size);
  }

  EdgeInsets         edge_insets;
  stx::Vec<Widget *> children;
};

}        // namespace gui
}        // namespace ash
