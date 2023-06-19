#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

namespace ash
{

struct BoxProps
{
  constraint width;
  constraint height;
  color      background_color;
  EdgeInsets padding;
  f32        border_thickness = 0;
  color      border_color     = colors::BLACK;
  vec4       border_radius;
};

struct Box : public Widget
{
  template <WidgetImpl DerivedWidget>
  Box(BoxProps iprops, DerivedWidget child) :
      props{iprops}
  {
    children.push(new DerivedWidget{std::move(child)}).unwrap();
  }

  explicit Box(BoxProps iprops) :
      props{iprops}
  {}

  STX_DISABLE_COPY(Box)
  STX_DEFAULT_MOVE(Box)

  virtual Layout layout(Context &context, rect allotted) override
  {
    return Layout{.flex = FlexProps{.main_fit = Fit::Expand, .cross_fit = Fit::Expand}, .area = allotted.with_extent(props.width.resolve(allotted.extent.x) + props.border_thickness * 2, props.height.resolve(allotted.extent.y) + props.border_thickness * 2)};
  }

  virtual stx::Span<Widget *const> get_children(Context &context) override
  {
    return children;
  }

  virtual void draw(Context &context, gfx::Canvas &canvas) override
  {
    canvas        //.draw_round_rect_filled(rect{.offset = area.offset + props.border_thickness, .extent = area.extent - props.border_thickness}, props.border_radius, 360, props.background_color)
        .draw_round_rect_stroke(area, props.border_radius, 360, props.border_color, props.border_thickness);
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {
  }

  virtual ~Box() override
  {}

  stx::Vec<Widget *> children;
  BoxProps           props;
};

};        // namespace ash