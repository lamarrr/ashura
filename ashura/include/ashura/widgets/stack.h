#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash {

enum class Alignment : u8 {
  TopLeft,
  TopCenter,
  TopRight,
  CenterLeft,
  Center,
  CenterRight,
  BottomLeft,
  BottomCenter,
  BottomRight
};

struct Stack : public Widget {
  template <typename... DerivedWidget>
  explicit Stack(Alignment ialignment, DerivedWidget... ichildren);

  virtual ~Stack() override {
    for (Widget* child : children) {
      delete child;
    }
  }

// update_children(DerivedWidget...)

  void update_children(stx::Span<Widget* const> new_children) {
    for (Widget* child : children) {
      delete child;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual stx::Span<Widget* const> get_children() override { return children; }

  constexpr virtual WidgetInfo get_info() override {
    return WidgetInfo{.type = "Stack", .id = Widget::id};
  }

  constexpr virtual Layout layout(rect area);

  virtual simdjson::dom::element save(WidgetContext & context,simdjson::dom::parser& parser);

  virtual void restore(WidgetContext & context,simdjson::dom::element const& element);

  Alignment alignment = Alignment::TopLeft;
  stx::Vec<Widget*> children{stx::os_allocator};
};

}  // namespace ash
