#pragma once

#include <algorithm>
#include <string>
#include <utility>

#include "ashura/widget.h"
#include "fmt/format.h"
#include "stx/vec.h"

namespace ash
{

struct Flex : public Widget
{
  template <WidgetImpl... DerivedWidget>
  explicit Flex(FlexProps iprops, DerivedWidget... ichildren) :
      props{iprops}
  {
    (children.push(new DerivedWidget{std::move(ichildren)}).unwrap(), ...);
  }

  STX_DISABLE_COPY(Flex)
  STX_DEFAULT_MOVE(Flex)

  virtual ~Flex() override
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
    return WidgetInfo{.type = "Flex"};
  }

  virtual Layout layout(Context &context, rect area) override
  {
    return Layout{.flex = props,
                  .area = rect{.offset = area.offset,
                               .extent = vec2{props.width.resolve(area.extent.x),
                                              props.height.resolve(area.extent.y)}}};
  }

  FlexProps          props;
  stx::Vec<Widget *> children;
};

}        // namespace ash
