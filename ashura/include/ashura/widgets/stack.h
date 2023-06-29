#pragma once

#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/vec.h"

namespace ash
{




struct ZStack{

vec2 offset{2, -2};


};

/**
 * @brief 
 * 
 * 
 * 
 * 
 * 
 * 
 * ZStacks and offset
Finally, let’s take a quicker look at SwiftUI’s ZStack type, which enables us to stack a series of views in terms of depth, using a back-to-front order.

As an example, let’s say that we wanted to add support for displaying a small “verified badge” on top of our calendar view from before — by placing a checkmark icon at its top-trailing corner. To implement that in a slightly more generic way, let’s extend View with an API that lets us wrap any view within a ZStack (which in of itself won’t affect the view’s layout), that’ll also optionally contain our checkmark icon — like this:

extension View {
    func addVerifiedBadge(_ isVerified: Bool) -> some View {
        ZStack(alignment: .topTrailing) {
            self

            if isVerified {
                Image(systemName: "checkmark.circle.fill")
                    .offset(x: 3, y: -3)
            }
        }
    }
}
Note how a ZStack gives us full two-dimensional control over its alignment, which we can use to position our icon in the parent view’s top-trailing corner. We then also apply the .offset() modifier to our badge, which’ll move it slightly outside of the bounds of its parent view.
 * 
 */
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

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual WidgetDebugInfo get_debug_info(Context &ctx) override
  {
    return WidgetDebugInfo{.type = "Stack"};
  }

  // virtual Layout layout(Context &ctx, rect area) override;

  Alignment          alignment = Alignment::TopLeft;
  stx::Vec<Widget *> children;
};

}        // namespace ash
