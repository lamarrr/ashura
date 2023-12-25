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
struct SwitchProps
{
  Color active_track_color   = material::BLUE_A700;
  Color inactive_track_color = material::GRAY_500;
  Color thumb_color          = material::WHITE;
  f32   height               = 20;
  bool  disabled             = false;
};

struct Switch : public Widget
{
  using Callback = stx::UniqueFn<void(Switch &, Context &, bool)>;

  static void default_on_changed(Switch &, Context &, bool new_state)
  {
  }

  explicit Switch(Callback ion_changed =
                      stx::fn::rc::make_unique_static(default_on_changed),
                  bool istate = false, SwitchProps iprops = {}) :
      on_changed{std::move(ion_changed)}, state{istate}, props{iprops}
  {
  }

  STX_DEFAULT_MOVE(Switch)
  STX_DISABLE_COPY(Switch)

  ~Switch()
  {
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    return Vec2{props.height * 1.75f, props.height};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    f32 const padding       = 1.75f;
    f32 const thumb_radius  = std::max(props.height / 2 - padding, 0.0f);
    f32 const thumb_begin_x = padding + thumb_radius;
    f32 const thumb_end_x =
        std::max(area.extent.x - padding - thumb_radius, 0.0f);
    Tween color_tween =
        state ?
            Tween<Color>{props.inactive_track_color, props.active_track_color} :
            Tween<Color>{props.active_track_color, props.inactive_track_color};
    Tween     thumb_position_tween = state ?
                                         Tween<f32>{thumb_begin_x, thumb_end_x} :
                                         Tween<f32>{thumb_end_x, thumb_begin_x};
    EaseIn    curve;
    Color     color          = animation.animate(curve, color_tween);
    f32 const thumb_position = animation.animate(curve, thumb_position_tween);

    canvas
        .draw_round_rect_filled(area.offset, area.extent,
                                uniform_vec4(props.height / 2), 90, color)
        .draw_circle_filled(area.offset +
                                Vec2{thumb_position, 1.5f + thumb_radius},
                            thumb_radius, 180, props.thumb_color);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    animation.tick(interval);
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button,
                             Vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary)
    {
      state = !state;
      animation.restart(Milliseconds{200}, 1, AnimationCfg::Default, 1);
      on_changed.handle(*this, ctx, state);
    }
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  Callback    on_changed;
  bool        state = true;
  SwitchProps props;
  Animation   animation;
};
}        // namespace gui
}        // namespace ash