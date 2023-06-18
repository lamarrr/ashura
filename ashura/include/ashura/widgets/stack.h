#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash
{

enum class Alignment : u8
{
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

struct Stack : public Widget
{
  template <typename... DerivedWidget>
  explicit Stack(Alignment ialignment, DerivedWidget... ichildren);

  virtual ~Stack() override
  {
    for (Widget *child : children)
    {
      delete child;
    }
  }

  // update_children(DerivedWidget...)

  void update_children(stx::Span<Widget *const> new_children)
  {
    for (Widget *child : children)
    {
      delete child;
    }

    children.clear();
    children.extend(new_children).unwrap();
  }

  virtual stx::Span<Widget *const> get_children(Context &context) override
  {
    return children;
  }

  virtual WidgetInfo get_info(Context &context) override
  {
    return WidgetInfo{.type = "Stack"};
  }

  virtual Layout layout(Context &context, rect area);

  Alignment          alignment = Alignment::TopLeft;
  stx::Vec<Widget *> children;
};

}        // namespace ash
