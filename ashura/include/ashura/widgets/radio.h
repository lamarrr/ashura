#pragma once

#include "ashura/animation.h"
#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/common.h"
#include "stx/fn.h"
#include "stx/rc.h"

namespace ash
{
// TODO(lamarrr): disabling
template <typename RadioValue>
struct RadioContextData
{
  RadioValue value;
};

template <typename RadioValue>
struct RadioContext
{
  explicit RadioContext(RadioValue value) :
      data{stx::rc::make(stx::os_allocator, RadioContextData{.value = std::move(value)}).unwrap()}
  {}

  RadioContext(RadioContext const &other) :
      data{other.data.share()}
  {}

  RadioContext &operator=(RadioContext const &other)
  {
    data = other.data.share();
    return *this;
  }

  STX_DEFAULT_MOVE(RadioContext)

  stx::Rc<RadioContextData<RadioValue> *> data;
};

struct RadioProps
{
  color active_color   = material::GRAY_300;
  color inactive_color = material::BLUE_A700;
  f32   radius         = 12.5;
};

template <typename RadioValue>
struct Radio : public Widget
{
  static_assert(stx::equality_comparable<RadioValue>);

  Radio(RadioValue ivalue, RadioContext<RadioValue> iradio_context, RadioProps iprops = {}) :
      value{std::move(ivalue)}, radio_context{std::move(iradio_context)}, props{iprops}
  {
    if (radio_context.data->value == value)
    {
      is_active = true;
      tween     = Tween{props.inactive_color, props.active_color};
      animation.restart(milliseconds{200}, milliseconds{200}, 1);
      animation.finish();
    }
    else
    {
      is_active = false;
      tween     = Tween{props.active_color, props.inactive_color};
      animation.restart(milliseconds{200}, milliseconds{200}, 1);
      animation.finish();
    }
  }

  STX_DEFAULT_MOVE(Radio)
  STX_DISABLE_COPY(Radio)

  ~Radio()
  {
  }

  virtual Layout layout(Context &context, rect allotted) override
  {
    return Layout{.area = allotted.with_extent(props.radius * 2, props.radius * 2)};
  }

  virtual void tick(Context &context, std::chrono::nanoseconds interval) override
  {
    if (radio_context.data->value == value && !is_active)
    {
      on_changed(context, radio_context.data->value);
    }
    else if (radio_context.data->value != value && is_active)
    {
      on_changed(context, radio_context.data->value);
    }

    animation.tick(interval);
  }

  virtual void draw(Context &context, gfx::Canvas &canvas) override
  {
    f32 inner_radius = props.radius * 0.6f;
    canvas.draw_circle_filled(area.offset, props.radius, 180, props.inactive_color)
        .draw_circle_filled(area.offset + props.radius - inner_radius, inner_radius, 180, animation.animate(color_curve, tween));
  }

  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary)
    {
      radio_context.data->value = value;
    }
  }

  virtual void on_changed(Context &context, RadioValue const &new_value)
  {
    if (new_value == value)
    {
      is_active = true;
      on_selected(context);
      tween = Tween{props.inactive_color, props.active_color};
    }
    else if (is_active)
    {
      is_active = false;
      on_deselected(context);
      tween = Tween{props.active_color, props.inactive_color};
    }

    animation.restart(milliseconds{200}, milliseconds{200}, 1);
  }

  virtual void on_selected(Context &context)
  {}

  virtual void on_deselected(Context &context)
  {}

  RadioValue               value;
  bool                     is_active = false;
  RadioContext<RadioValue> radio_context;
  RadioProps               props;
  EaseIn                   radius_curve;
  EaseIn                   color_curve;
  Tween<color>             tween;
  Animation                animation;
};

}        // namespace ash