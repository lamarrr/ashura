#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

// slider direction
struct Slider : public Widget
{
  virtual vec2 fit(Context &ctx, vec2 allocated_size,
                   stx::Span<vec2 const> children_allocations,
                   stx::Span<vec2 const> children_sizes,
                   stx::Span<vec2>       children_positions) override
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

    vec2 thumb_center{track_area.offset.x + percentage * track_area.extent.x,
                      area.offset.y + area.extent.y / 2};
    f32  thumb_radius =
        thumb_animation.animate(thumb_animation_curve, thumb_tween);

    canvas
        .draw_round_rect_filled(track_area, vec4::splat(props.track_height / 2),
                                45, props.track_color)
        .draw_circle_filled(thumb_center, thumb_radius, 360, props.track_color);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    thumb_animation.tick(interval);
  }

  virtual bool hit_test(Context &ctx, vec2 mouse_position) override
  {
    return true;
  }

  virtual stx::Option<DragData> on_drag_start(Context &ctx,
                                              vec2     mouse_position) override
  {
    return stx::Some(DragData{
        .type = "STUB",
        .data = stx::Unique{stx::Span<u8 const>{}, stx::manager_stub}});
  }

  virtual void on_drag_update(Context &ctx, vec2 mouse_position,
                              vec2            translation,
                              DragData const &drag_data) override
  {
    on_change_start.handle(*this, ctx, value);
    f32 diff = translation.x / track_area.extent.x;
    value    = std::clamp(value + diff * (max - min), min, max);
    on_changed.handle(*this, ctx, value);
  }

  virtual void on_mouse_enter(Context &ctx, vec2 mouse_position) override
  {
    __transition_radius(props.thumb_radius * 0.75f, props.thumb_radius);
  }

  virtual void on_mouse_leave(Context          &ctx,
                              stx::Option<vec2> mouse_position) override
  {
    __transition_radius(props.thumb_radius, props.thumb_radius * 0.75f);
  }

  void __transition_radius(f32 from, f32 to)
  {
    thumb_tween = Tween<f32>{from, to};
    thumb_animation.restart(milliseconds{200}, 1, AnimationCfg::Default, 1);
  }

  Fn<void(f32)> on_changed;
  Callback      on_change_start;
  Callback      on_change_end;
  f32           value = 0, min = 0, max = 1;
  color         track_color  = material::BLUE_A700;
  f32           track_height = 5;
  f32           thumb_radius = 10;
  constraint    width        = constraint{.scale = 1, .max = 250};
  bool          disabled     = false;
  rect          track_area;
  bool          is_changing = false;
  Animation     thumb_animation;
  Linear        thumb_animation_curve;
  Tween<f32>    thumb_tween;
};

}        // namespace ash