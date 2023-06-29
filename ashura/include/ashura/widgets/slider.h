#pragma once

#include "ashura/animation.h"
#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct SliderProps
{
  color      track_color  = material::BLUE_A700;
  f32        track_height = 5;
  f32        thumb_radius = 12;
  constraint width        = constraint{.scale = 1, .max = 250};
};

struct Slider : public Widget
{
  using Callback = stx::RcFn<void(Slider &, Context &, f32)>;

  static void default_on_changed(Slider &, Context &, f32)
  {}

  explicit Slider(Callback ion_changed      = stx::fn::rc::make_static(default_on_changed),
                  Callback ion_change_start = stx::fn::rc::make_static(default_on_changed),
                  Callback ion_change_end   = stx::fn::rc::make_static(default_on_changed),
                  f32 ivalue = 0, f32 imin = 0, f32 imax = 1,
                  SliderProps iprops = SliderProps{}) :
      on_changed{std::move(ion_changed)}, on_change_start{std::move(ion_change_start)}, on_change_end{std::move(ion_change_end)}, value{ivalue}, min{imin}, max{imax}, props{iprops}
  {
    __transition_radius(props.thumb_radius * 0.75f, props.thumb_radius * 0.5f);
  }

  STX_DISABLE_COPY(Slider)
  STX_DEFAULT_MOVE(Slider)

  virtual ~Slider() override
  {}

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return vec2{props.width.resolve(allocated_size.x), props.thumb_radius * 2};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    f32 percentage = (value - min) / (max - min);

    track_area = area;
    track_area.offset.x += props.thumb_radius;
    track_area.extent.x -= props.thumb_radius * 2;
    track_area.offset.y += props.thumb_radius;
    track_area.offset.y -= props.track_height / 2;
    track_area.extent.y = props.track_height;

    vec2 thumb_center{track_area.offset.x + percentage * track_area.extent.x, area.offset.y + area.extent.y / 2};
    f32  thumb_radius = thumb_animation.animate(thumb_animation_curve, thumb_tween);

    canvas
        .draw_round_rect_filled(track_area, vec4::splat(props.track_height / 2), 360, props.track_color)
        .draw_circle_filled(thumb_center, thumb_radius, 360, props.track_color);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    thumb_animation.tick(interval);
  }

  // TODO(lamarrr): mouse_position is already **inverted** by global zoom and pan matrix???
  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary)
    {
      on_change_start.handle(*this, ctx, value);
      f32 x_offset       = mouse_position.x - track_area.offset.x;
      f32 previous_value = value;
      value              = std::clamp(min + (x_offset / track_area.extent.x) * (max - min), min, max);
      if (value != previous_value)
      {
        on_changed.handle(*this, ctx, value);
      }
      is_changing = true;
    }
  }

  virtual void on_mouse_up(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && is_changing)
    {
      is_changing = false;
      on_change_end.handle(*this, ctx, value);
    }
  }

  virtual void on_mouse_move(Context &ctx, vec2 mouse_position, vec2 translation) override
  {
    if (is_changing)
    {
      f32 x_offset = mouse_position.x - track_area.offset.x;
      value        = std::clamp(min + (x_offset / track_area.extent.x) * (max - min), min, max);
      on_changed.handle(*this, ctx, value);
    }
  }

  virtual void on_mouse_enter(Context &ctx, vec2 mouse_position) override
  {
    __transition_radius(props.thumb_radius * 0.75f, props.thumb_radius);
  }

  virtual void on_mouse_leave(Context &ctx, stx::Option<vec2> mouse_position) override
  {
    __transition_radius(props.thumb_radius, props.thumb_radius * 0.75f);
  }

  void __transition_radius(f32 from, f32 to)
  {
    thumb_tween = Tween{from, to};
    thumb_animation.restart(milliseconds{200}, milliseconds{200}, 1);
  }

  Callback    on_changed;
  Callback    on_change_start;
  Callback    on_change_end;
  f32         value = 0, min = 0, max = 1;
  SliderProps props;
  rect        track_area;
  bool        is_changing = false;
  Animation   thumb_animation;
  Linear      thumb_animation_curve;
  Tween<f32>  thumb_tween;
};

}        // namespace ash