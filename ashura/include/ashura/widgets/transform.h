#pragma once
#include "ashura/primitives.h"
#include "ashura/widget.h"

struct Transform : public Widget {
  template <typename DerivedWidget>
  constexpr Transform(mat4 itransform, DerivedWidget ichild)
      : transform{itransform}, child{new DerivedWidget{std::move(ichild)}} {}

  constexpr ~Transform() override { delete child; }

  template <typename DerivedWidget>
  constexpr void update_child(DerivedWidget ichild);

  constexpr virtual stx::Span<Widget* const> get_children() override {
    return stx::Span{&child, 1};
  }

  constexpr virtual WidgetInfo get_info() override {
    return WidgetInfo{.type = "Transform", .id = Widget::id};
  }

  constexpr virtual Layout layout(rect area);

  virtual simdjson::dom::element save(simdjson::dom::parser& parser);

  virtual void restore(simdjson::dom::element const& element);

  mat4 transform;
  Widget* child = nullptr;
};