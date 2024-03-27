#pragma once

#include "ashura/animation.h"
#include "ashura/color.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct SliderProps
{
  Color      track_color  = material::BLUE_A700;
  f32        track_height = 5;
  f32        thumb_radius = 10;
  Constraint width        = Constraint{.scale = 1, .max = 250};
  bool       disabled     = false;
};

struct Slider : public Widget
{
  using Callback = stx::UniqueFn<void(Slider &, Context &, f32)>;

  static void default_on_changed(Slider &, Context &, f32)
  {
  }

  explicit Slider(Callback ion_changed =
                      stx::fn::rc::make_unique_static(default_on_changed),
                  Callback ion_change_start =
                      stx::fn::rc::make_unique_static(default_on_changed),
                  Callback ion_change_end =
                      stx::fn::rc::make_unique_static(default_on_changed),
                  f32 ivalue = 0, f32 imin = 0, f32 imax = 1,
                  SliderProps iprops = SliderProps{}) :
      on_changed{std::move(ion_changed)},
      on_change_start{std::move(ion_change_start)},
      on_change_end{std::move(ion_change_end)},
      value{ivalue},
      min{imin},
      max{imax},
      props{iprops}
  {
    __transition_radius(props.thumb_radius * 0.75f, props.thumb_radius * 0.5f);
  }

  STX_DISABLE_COPY(Slider)
  STX_DEFAULT_MOVE(Slider)

  virtual ~Slider() override
  {
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    return Vec2{props.width.resolve(allocated_size.x), props.thumb_radius * 2};
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

    Vec2 thumb_center{track_area.offset.x + percentage * track_area.extent.x,
                      area.offset.y + area.extent.y / 2};
    f32  thumb_radius =
        thumb_animation.animate(thumb_animation_curve, thumb_tween);

    f32 r = props.track_height / 2;

    canvas
        .draw_round_rect_filled(track_area.offset, track_area.extent,
                                Vec4{r, r, r, r}, 45, props.track_color)
        .draw_circle_filled(thumb_center, thumb_radius, 360, props.track_color);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    thumb_animation.tick(interval);
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  virtual stx::Option<DragData> on_drag_start(Context &ctx,
                                              Vec2     mouse_position) override
  {
    return stx::Some(DragData{
        .type = "STUB",
        .data = stx::Unique{stx::Span<u8 const>{}, stx::manager_stub}});
  }

  virtual void on_drag_update(Context &ctx, Vec2 mouse_position,
                              Vec2            translation,
                              DragData const &drag_data) override
  {
    on_change_start.handle(*this, ctx, value);
    f32 diff = translation.x / track_area.extent.x;
    value    = std::clamp(value + diff * (max - min), min, max);
    on_changed.handle(*this, ctx, value);
  }

  virtual void on_mouse_enter(Context &ctx, Vec2 mouse_position) override
  {
    __transition_radius(props.thumb_radius * 0.75f, props.thumb_radius);
  }

  virtual void on_mouse_leave(Context          &ctx,
                              stx::Option<Vec2> mouse_position) override
  {
    __transition_radius(props.thumb_radius, props.thumb_radius * 0.75f);
  }

  void __transition_radius(f32 from, f32 to)
  {
    thumb_tween = Tween<f32>{from, to};
    thumb_animation.restart(Milliseconds{200}, 1, AnimationCfg::Default, 1);
  }

  Callback    on_changed;
  Callback    on_change_start;
  Callback    on_change_end;
  f32         value = 0, min = 0, max = 1;
  SliderProps props;
  Rect        track_area;
  bool        is_changing = false;
  Animation   thumb_animation;
  Linear      thumb_animation_curve;
  Tween<f32>  thumb_tween;
};

}        // namespace gui
}        // namespace ash
