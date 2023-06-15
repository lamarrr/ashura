#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct SliderProps
{
  color track_color = material::BLUE_A700;
  color thumb_color = material::GRAY_100;
  f32   value       = 0;
  f32   min = 0, max = 1;
  f32   track_height = 5;
  f32   thumb_height = 15;
};

struct Slider : public Widget
{
  Slider(SliderProps iprops = SliderProps{}) :
      props{iprops}
  {}

  virtual ~Slider() override
  {}

  virtual Layout layout(Context &context, rect allotted) override
  {
    return Layout{.area = rect{.offset = allotted.offset, .extent = vec2{allotted.extent.x, props.thumb_height}}};
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area)
  {
    f32 percentage = (props.value - props.min) / (props.max - props.min);

    track_area = area;
    track_area.offset.x += props.thumb_height / 2;
    track_area.extent.x -= props.thumb_height;
    track_area.offset.y += props.thumb_height / 2;
    track_area.offset.y -= props.track_height / 2;
    track_area.extent.y = props.track_height;

    canvas.draw_round_rect_filled(track_area, vec4::splat(props.track_height / 2), 45, props.track_color);

    f32 thumb_center_pos = track_area.offset.x + percentage * track_area.extent.x;

    vec2 thumb_position{thumb_center_pos - props.thumb_height / 2, area.offset.y};

    canvas.draw_circle_filled(thumb_position, props.thumb_height / 2, 180, props.thumb_color);
  }

  // TODO(lamarrr): mouse_position is already **inverted** by global zoom and pan matrix???
  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {
    if (button == MouseButton::Primary)
    {
      f32 x_offset  = mouse_position.x - track_area.offset.x;
      props.value   = std::clamp(props.min + (x_offset / track_area.extent.x) * (props.max - props.min), props.min, props.max);
      mouse_focused = true;
    }
  }

  virtual void on_mouse_up(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {
    if (button == MouseButton::Primary && mouse_focused)
    {
      mouse_focused = false;
    }
  }

  constexpr virtual void on_mouse_move(Context &context, vec2 mouse_position, vec2 translation, quad quad)
  {
    if (mouse_focused)
    {
      f32 x_offset = mouse_position.x - track_area.offset.x;
      props.value  = std::clamp(props.min + (x_offset / track_area.extent.x) * (props.max - props.min), props.min, props.max);
    }
  }

  virtual void on_mouse_leave(Context &context, stx::Option<vec2> mouse_position)
  {
    if (mouse_focused && mouse_position.is_none())
    {
      mouse_focused = false;
    }
  }

  virtual void on_changed(Context &context, f32 new_value)
  {}

  virtual void on_change_start(Context &context, f32 present_value)
  {}

  virtual void on_change_end(Context &context, f32 present_value)
  {}

  SliderProps props;
  rect        track_area;
  bool        mouse_focused = false;
};

}        // namespace ash