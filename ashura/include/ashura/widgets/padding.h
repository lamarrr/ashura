#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{
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

  virtual stx::Span<Widget *const> get_children(Context &context) override
  {
    return stx::Span{&child, 1};
  }

  virtual WidgetInfo get_info(Context &context) override
  {
    return WidgetInfo{.type = "Padding"};
  }

  virtual Layout layout(rect area)
  {
  }

  EdgeInsets edge_insets;
  Widget    *child = nullptr;
};

}        // namespace ash
