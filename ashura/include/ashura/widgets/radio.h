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
template <typename RadioValue>
struct RadioStateData
{
  RadioValue value;
};

template <typename RadioValue>
struct RadioState
{
  static_assert(stx::equality_comparable<RadioValue>);

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

struct RadioProps
{
  color color       = material::BLUE_A700;
  f32   width       = 20;
  f32   inner_width = 10;
  bool  disabled    = false;
};

template <typename RadioValue>
struct Radio : public Widget
{
  using Callback = stx::RcFn<void(Radio &, Context &, RadioValue const &)>;

  static void default_on_changed(Radio &, Context &, RadioValue const &)
  {}

  Radio(RadioValue ivalue, RadioState<RadioValue> iradio_state, Callback ion_changed = stx::fn::rc::make_static(default_on_changed), RadioProps iprops = {}) :
      on_changed{std::move(ion_changed)}, value{std::move(ivalue)}, state{std::move(iradio_state)}, props{iprops}
  {
    __restart_state_machine(state.data->value);
  }

  STX_DEFAULT_MOVE(Radio)
  STX_DISABLE_COPY(Radio)

  ~Radio()
  {
  }

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return vec2::splat(props.width);
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
    EaseIn curve;
    rect   outer_rect        = area;
    vec2   inner_rect_extent = vec2::splat(animation.animate(curve, is_active ? Tween{0.0f, props.inner_width} : Tween{props.inner_width, 0.0f}));
    rect   inner_rect        = rect{.offset = area.offset + (area.extent / 2) - inner_rect_extent / 2, .extent = inner_rect_extent};

    canvas
        .draw_rect_stroke(outer_rect, props.color, 1.5f)
        .draw_rect_filled(inner_rect, props.color);
  }

  virtual bool hit_test(Context &ctx, vec2 mouse_position) override
  {
    return true;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && !props.disabled)
    {
      state.data->value = value;
    }
  }

  void __restart_state_machine(RadioValue const &new_value)
  {
    if (new_value == value)
    {
      is_active = true;
    }
    else if (is_active)
    {
      is_active = false;
    }

    animation.restart(milliseconds{200}, milliseconds{200}, 1, false);
  }

  RadioValue             value;
  bool                   is_active = false;
  RadioState<RadioValue> state;
  RadioProps             props;
  Animation              animation;
  Callback               on_changed;
};

}        // namespace ash