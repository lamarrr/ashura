#pragma once

#include "ashura/animation.h"
#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct SliderProps
{
  color track_color = material::BLUE_A700;
  color thumb_color = material::GRAY_300;
  f32   value       = 0;
  f32   min = 0, max = 1;
  f32   track_height = 5;
  f32   thumb_radius = 12;
};

struct Slider : public Widget
{
  Slider(SliderProps iprops = SliderProps{}) :
      props{iprops}
  {
    thumb_tween = Tween{props.thumb_radius * 0.75f, props.thumb_radius * 0.5f};
    thumb_animation.restart(milliseconds{200}, milliseconds{200}, 1);
    thumb_animation.finish();
  }

  virtual ~Slider() override
  {}

  virtual vec2 layout(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return vec2{allocated_size.x, props.thumb_radius * 2};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    f32 percentage = (props.value - props.min) / (props.max - props.min);

    track_area = area;
    track_area.offset.x += props.thumb_radius;
    track_area.extent.x -= props.thumb_radius * 2;
    track_area.offset.y += props.thumb_radius;
    track_area.offset.y -= props.track_height / 2;
    track_area.extent.y = props.track_height;

    canvas.draw_round_rect_filled(track_area, vec4::splat(props.track_height / 2), 360, props.track_color);

    f32 thumb_center_x     = track_area.offset.x + percentage * track_area.extent.x;
    f32 thumb_center_y     = area.offset.y + area.extent.y / 2;
    f32 outer_thumb_radius = props.thumb_radius;
    f32 inner_thumb_radius = thumb_animation.animate(thumb_animation_curve, thumb_tween);

    vec2 outer_thumb_offset{thumb_center_x - outer_thumb_radius, thumb_center_y - outer_thumb_radius};
    vec2 inner_thumb_offset{thumb_center_x - inner_thumb_radius, thumb_center_y - inner_thumb_radius};

    canvas.draw_circle_filled(outer_thumb_offset, outer_thumb_radius, 360, props.thumb_color);
    canvas.draw_circle_filled(inner_thumb_offset, inner_thumb_radius, 360, props.track_color);
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
      f32 x_offset  = mouse_position.x - track_area.offset.x;
      props.value   = std::clamp(props.min + (x_offset / track_area.extent.x) * (props.max - props.min), props.min, props.max);
      mouse_focused = true;
    }
  }

  virtual void on_mouse_up(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && mouse_focused)
    {
      mouse_focused = false;
    }
  }

  virtual void on_mouse_move(Context &ctx, vec2 mouse_position, vec2 translation) override
  {
    if (mouse_focused)
    {
      f32 x_offset = mouse_position.x - track_area.offset.x;
      props.value  = std::clamp(props.min + (x_offset / track_area.extent.x) * (props.max - props.min), props.min, props.max);
    }
  }

  virtual void on_mouse_enter(Context &ctx, vec2 mouse_position) override
  {
    thumb_tween = Tween{props.thumb_radius * 0.5f, props.thumb_radius * 0.75f};
    thumb_animation.restart(milliseconds{200}, milliseconds{200}, 1);
  }

  virtual void on_mouse_leave(Context &ctx, stx::Option<vec2> mouse_position) override
  {
    if (mouse_focused && mouse_position.is_none())
    {
      mouse_focused = false;
    }
    thumb_tween = Tween{props.thumb_radius * 0.75f, props.thumb_radius * 0.5f};
    thumb_animation.restart(milliseconds{200}, milliseconds{200}, 1);
  }

  virtual void on_changed(Context &ctx, f32 new_value)
  {}

  virtual void on_change_start(Context &ctx, f32 present_value)
  {}

  virtual void on_change_end(Context &ctx, f32 present_value)
  {}

  SliderProps props;
  rect        track_area;
  bool        mouse_focused = false;
  Animation   thumb_animation;
  Linear      thumb_animation_curve;
  Tween<f32>  thumb_tween;
};

}        // namespace ash