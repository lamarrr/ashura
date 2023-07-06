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
struct RadioStateData
{
  RadioValue value;
};

template <typename RadioValue>
struct RadioState
{
  explicit RadioState(RadioValue value) :
      data{stx::rc::make(stx::os_allocator, RadioStateData{.value = std::move(value)}).unwrap()}
  {}

  RadioState(RadioState const &other) :
      data{other.data.share()}
  {}

  RadioState &operator=(RadioState const &other)
  {
    data = other.data.share();
    return *this;
  }

  STX_DEFAULT_MOVE(RadioState)

  stx::Rc<RadioStateData<RadioValue> *> data;
};

// TODO(lamarrr): make this a rect instead
struct RadioProps
{
  color color  = material::BLUE_A700;
  f32   radius = 12.5;
};

template <typename RadioValue>
struct Radio : public Widget
{
  static_assert(stx::equality_comparable<RadioValue>);

  using Callback = stx::RcFn<void(Radio &, Context &, RadioValue const &)>;

  static void default_on_changed(Radio &radio, Context &ctx, RadioValue const &new_value)
  {}

  Radio(RadioValue ivalue, RadioState<RadioValue> iradio_state, Callback ion_changed = stx::fn::rc::make_static(default_on_changed), RadioProps iprops = {}) :
      value{std::move(ivalue)}, state{std::move(iradio_state)}, on_changed{std::move(ion_changed)}, props{iprops}
  {
    __restart_state_machine(state.data->value);
  }

  STX_DEFAULT_MOVE(Radio)
  STX_DISABLE_COPY(Radio)

  ~Radio()
  {
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return vec2{props.radius * 2, props.radius * 2};
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    if (state.data->value == value && !is_active)
    {
      on_changed.handle(*this, ctx, state.data->value);
      __restart_state_machine(state.data->value);
    }
    else if (state.data->value != value && is_active)
    {
      on_changed.handle(*this, ctx, state.data->value);
      __restart_state_machine(state.data->value);
    }

    animation.tick(interval);
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    f32  inner_radius = props.radius * 0.6f;
    vec2 center       = area.offset + props.radius;

    canvas
        .draw_circle_stroke(center, props.radius, 360, props.color, 1.5)
        .draw_circle_filled(center, inner_radius, 360, animation.animate(color_curve, tween));
  }

  virtual bool hit_test(Context &ctx, vec2 mouse_position) override
  {
    return true;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary)
    {
      state.data->value = value;
    }
  }

  void __restart_state_machine(RadioValue const &new_value)
  {
    if (new_value == value)
    {
      is_active = true;
      tween     = Tween{props.color.with_alpha(10), props.color};
    }
    else if (is_active)
    {
      is_active = false;
      tween     = Tween{props.color, props.color.with_alpha(10)};
    }

    animation.restart(milliseconds{200}, milliseconds{200}, 1);
  }

  RadioValue             value;
  bool                   is_active = false;
  RadioState<RadioValue> state;
  RadioProps             props;
  EaseIn                 radius_curve;
  EaseIn                 color_curve;
  Tween<color>           tween;
  Animation              animation;
  Callback               on_changed;
};

}        // namespace ash