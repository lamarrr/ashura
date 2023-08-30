
#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct ScrollBoxProps
{
  constraint     scroll_x  = constraint::absolute(0);
  constraint     scroll_y  = constraint::absolute(0);
  bool           disabled  = false;
  color          bar_color = material::BLUE_A700;
  f32            bar_width = 20;
  SizeConstraint frame     = SizeConstraint::absolute(200, 200);
};

struct ScrollBox : public Widget
{
  template <WidgetImpl DerivedWidget>
  ScrollBox(ScrollBoxProps iprops, DerivedWidget child) :
      props{iprops}
  {
    children.push(new DerivedWidget{std::move(child)}).unwrap();
  }

  ScrollBox(ScrollBoxProps iprops, Widget *child) :
      props{iprops}
  {
    children.push_inplace(child).unwrap();
  }

  STX_DISABLE_COPY(ScrollBox)
  STX_DEFAULT_MOVE(ScrollBox)

  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation) override
  {
    children_allocation.fill(allocated_size - props.bar_width);
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    vec2 frame      = props.frame.resolve(allocated_size);
    vec2 child_size = children_sizes[0];
    bool scroll_x   = child_size.x > frame.x;
    bool scroll_y   = child_size.y > frame.y;
    vec2 size;
    size.x = scroll_x ? frame.x : child_size.x;
    size.y = scroll_y ? frame.y : child_size.y;
    vec2 translation;
    translation.x = scroll_x ? props.scroll_x.resolve(child_size.x - size.x) : 0;
    translation.y = scroll_y ? props.scroll_y.resolve(child_size.y - size.y) : 0;
    children_positions.fill(translation);
     return size;
  }

  virtual rect clip(Context &ctx, rect allocated_clip, stx::Span<rect> children_allocation) override
  {
    rect clip = area;
    clip.extent.x -= props.bar_width;
    clip = clip.intersect(allocated_clip);
    children_allocation.fill(clip);
    return area;
  }

  virtual stx::Span<Widget *const> get_children(Context &ctx) override
  {
    return children;
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    rect bar_area;
    bar_area.offset = vec2{area.offset.x + area.extent.x - props.bar_width, area.offset.y + props.bar_width};
    bar_area.extent = vec2{props.bar_width, area.extent.y - props.bar_width * 2};
    rect up_button_area;
    up_button_area.offset = vec2{area.offset.x + area.extent.x - props.bar_width, area.offset.y};
    up_button_area.extent = vec2::splat(props.bar_width);
    rect down_button_area;
    down_button_area.offset = vec2{area.offset.x + area.extent.x - props.bar_width, area.offset.y + area.extent.y - props.bar_width};
    down_button_area.extent = vec2::splat(props.bar_width);

    canvas
        .draw_rect_filled(up_button_area, props.bar_color)
        .draw_rect_filled(down_button_area, props.bar_color)
        .draw_rect_stroke(bar_area, props.bar_color, 1);
  }

  virtual ~ScrollBox() override
  {}

  template <WidgetImpl DerivedWidget>
  void update_child(DerivedWidget widget)
  {
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = new DerivedWidget{std::move(widget)};
  }

  void update_child(Widget *widget)
  {
    ASH_CHECK(children.size() == 1);
    delete children[0];
    children[0] = widget;
  }

  virtual bool hit_test(Context &ctx, vec2 mouse_position) override
  {
    return true;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && !props.disabled)
    {
      scroll_translation.y += 10;
    }
  }

  stx::Vec<Widget *> children;
  ScrollBoxProps     props;
  vec2               scroll_translation;
};

};        // namespace ash