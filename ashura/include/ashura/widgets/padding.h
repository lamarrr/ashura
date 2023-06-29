#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

// TODO(lamarrr): replace single element with vector
struct Padding : public Widget
{
  template <typename DerivedWidget>
  Padding(EdgeInsets iedge_insets, DerivedWidget ichild) :
      edge_insets{iedge_insets}, child{new DerivedWidget{std::move(ichild)}}
  {}

  virtual ~Padding() override
  {
    delete child;
  }

  template <typename DerivedWidget>
  void update_child(DerivedWidget ichild)
  {
    delete child;
    child = new DerivedWidget{std::move(ichild)};
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return stx::Span{&child, 1};
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Padding"};
  }

  // virtual Layout layout(rect area)override
  // {
  // }

  EdgeInsets edge_insets;
  Widget    *child = nullptr;
};

}        // namespace ash
