#pragma once

#include "ashura/animation.h"
#include "ashura/color.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "stx/common.h"
#include "stx/fn.h"
#include "stx/rc.h"

namespace ash
{
namespace gui
{

template <typename RadioValue>
struct RadioCtx
{
  static_assert(stx::equality_comparable<RadioValue>);

  explicit RadioCtx(RadioValue value) :
      data{stx::rc::make(stx::os_allocator, std::move(value)).unwrap()}
  {
  }

  RadioCtx(RadioCtx const &other) : data{other.data.share()}
  {
  }

  RadioCtx &operator=(RadioCtx const &other)
  {
    data = other.data.share();
    return *this;
  }

  STX_DEFAULT_MOVE(RadioCtx)

  stx::Rc<RadioValue *> data;
};

struct RadioProps
{
  Color color       = material::BLUE_A700;
  f32   width       = 20;
  f32   inner_width = 10;
  bool  disabled    = false;
};

template <typename RadioValue>
struct Radio : public Widget
{
  using Callback = stx::UniqueFn<void(Radio &, Context &, RadioValue const &)>;

  static void default_on_changed(Radio &, Context &, RadioValue const &)
  {
  }

  Radio(RadioValue ivalue, RadioCtx<RadioValue> iradio_ctx,
        Callback ion_changed =
            stx::fn::rc::make_unique_static(default_on_changed),
        RadioProps iprops = {}) :
      on_changed{std::move(ion_changed)},
      value{std::move(ivalue)},
      radio_ctx{std::move(iradio_ctx)},
      props{iprops}
  {
    __restart_state_machine(*radio_ctx.data);
  }

  STX_DEFAULT_MOVE(Radio)
  STX_DISABLE_COPY(Radio)

  ~Radio()
  {
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    return uniform_vec2(props.width);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    if (!ctx.key_events.span()
             .which([](KeyEvent e) {
               return e.action == KeyAction::Press && e.key == ESCAPE_Key;
             })
             .is_empty())
    {
      // TODO(lamarrr): key debouncing
      fmt::println("esc down!");
    }

    if (!ctx.key_events.span()
             .which([](KeyEvent e) {
               return e.action == KeyAction::Release && e.key == ESCAPE_Key;
             })
             .is_empty())
    {
      // TODO(lamarrr): key debouncing
      fmt::println("esc up!");
    }

    if (!ctx.key_events.span()
             .which([](KeyEvent e) {
               return e.action == KeyAction::Press && e.key == m_Key;
             })
             .is_empty())
    {
      // TODO(lamarrr): key debouncing
      fmt::println("m down!");
    }

    if (!ctx.key_events.span()
             .which([](KeyEvent e) {
               return e.action == KeyAction::Release && e.key == m_Key;
             })
             .is_empty())
    {
      // TODO(lamarrr): key debouncing
      fmt::println("m up!");
    }

    if (*radio_ctx.data == value && !is_active)
    {
      on_changed.handle(*this, ctx, *radio_ctx.data);
      __restart_state_machine(*radio_ctx.data);
    }
    else if (*radio_ctx.data != value && is_active)
    {
      on_changed.handle(*this, ctx, *radio_ctx.data);
      __restart_state_machine(*radio_ctx.data);
    }

    animation.tick(interval);
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    EaseIn curve;
    Rect   outer_rect        = area;
    Vec2   inner_rect_extent = uniform_vec2(animation.animate(
        curve, is_active ? Tween<f32>{0.0f, props.inner_width} :
                             Tween<f32>{props.inner_width, 0.0f}));
    Rect   inner_rect =
        Rect{.offset = area.offset + (area.extent / 2) - inner_rect_extent / 2,
             .extent = inner_rect_extent};

    canvas
        .draw_rect_stroke(outer_rect.offset, outer_rect.extent, props.color,
                          1.5f)
        .draw_rect_filled(inner_rect.offset, inner_rect.extent, props.color);
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button,
                             Vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && !props.disabled)
    {
      *radio_ctx.data = value;
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

    animation.restart(Milliseconds{200}, 1, AnimationCfg::Default, 1);
  }

  Callback             on_changed;
  RadioValue           value;
  bool                 is_active = false;
  RadioCtx<RadioValue> radio_ctx;
  RadioProps           props;
  Animation            animation;
};

}        // namespace gui
}        // namespace ash