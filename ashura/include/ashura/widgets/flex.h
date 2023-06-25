#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include "ashura/widget.h"
#include "fmt/format.h"
#include "stx/vec.h"

namespace ash
{

struct FlexBoxProps
{
  Direction  direction   = Direction::Row;
  Wrap       wrap        = Wrap::Wrap;
  MainAlign  main_align  = MainAlign::Start;
  CrossAlign cross_align = CrossAlign::Start;
  Fit        main_fit    = Fit::Shrink;
  Fit        cross_fit   = Fit::Shrink;
  constraint width       = constraint{.scale = 1};
  constraint height      = constraint{.scale = 1};

  constexpr FlexProps to_flex_props() const
  {
    return FlexProps{
        .direction   = direction,
        .wrap        = wrap,
        .main_align  = main_align,
        .cross_align = cross_align,
        .main_fit    = main_fit,
        .cross_fit   = cross_fit};
  }
};

struct FlexBox : public Widget
{
  template <WidgetImpl... DerivedWidget>
  explicit FlexBox(FlexBoxProps iprops, DerivedWidget... ichildren) :
      props{iprops}
  {
    (children.push(new DerivedWidget{std::move(ichildren)}).unwrap(), ...);
  }

  STX_DISABLE_COPY(FlexBox)
  STX_DEFAULT_MOVE(FlexBox)

  virtual ~FlexBox() override
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

  virtual stx::Span<Widget *const> get_flex_children(Context &ctx) override
  {
    return children;
  }

  virtual WidgetInfo get_info(Context &ctx) override
  {
    return WidgetInfo{.type = "Flex"};
  }

  virtual Layout layout(Context &ctx, rect area) override
  {
    return Layout{.flex = props.to_flex_props(),
                  .area = rect{.offset = area.offset,
                               .extent = vec2{props.width.resolve(area.extent.x),
                                              props.height.resolve(area.extent.y)}}};
  }

  FlexBoxProps       props;
  stx::Vec<Widget *> children;
};

}        // namespace ash
