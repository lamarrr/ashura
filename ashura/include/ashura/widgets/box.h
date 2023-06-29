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


    virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation)override
  {
    children_allocation.fill(allocated_size); // TODO(lamarrr): border + padding
  }

  virtual vec2   fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    // return Layout{.area = allotted.with_extent(props.width.resolve(allotted.extent.x) + props.border_thickness * 2, props.height.resolve(allotted.extent.y) + props.border_thickness * 2)};
    children_positions[0] = {0, 0};
    return children_sizes[0];
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    canvas
        .draw_round_rect_filled(rect{.offset = area.offset + props.border_thickness, .extent = area.extent - props.border_thickness}, props.border_radius, 360, props.background_color)
        .draw_round_rect_stroke(area, props.border_radius, 360, props.border_color, props.border_thickness);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
  }

  virtual ~Box() override
  {}

  stx::Vec<Widget *> children;
  BoxProps           props;
};

};        // namespace ash