#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct EdgeInsets
{
  f32 left = 0, top = 0, right = 0, bottom = 0;
};

struct Padding : public Widget
{
  template <typename DerivedWidget>
  constexpr Padding(EdgeInsets iedge_insets, DerivedWidget ichild) :
      edge_insets{iedge_insets}, child{new DerivedWidget{std::move(ichild)}}
  {}

  constexpr virtual ~Padding() override
  {
    delete child;
  }

  template <typename DerivedWidget>
  constexpr void update_child(DerivedWidget ichild)
  {
    delete child;
    child = new DerivedWidget{std::move(ichild)};
  }

  constexpr virtual stx::Span<Widget *const> get_children() override
  {
    return stx::Span{&child, 1};
  }

  constexpr virtual WidgetInfo get_info() override
  {
    return WidgetInfo{.type = "Padding"};
  }

  constexpr virtual Layout layout(rect area);

  virtual simdjson::dom::element save(Context         &context,
                                      simdjson::dom::parser &parser);

  virtual void restore(Context &context, simdjson::dom::element const &element);

  EdgeInsets edge_insets;
  Widget    *child = nullptr;
};

}        // namespace ash
