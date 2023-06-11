#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct SliderProps
{
  color              color = material::BLUE_A700;
  f32                value = 0;
  f32                min = 0, max = 1;
  stx::Option<usize> divisions;
};


struct SliderKnob: public Widget{

};


struct SliderTrack: public Widget{

};

struct Slider : public Widget
{
  static constexpr f32 TRACK_HEIGHT = 2;
  static constexpr f32 KNOB_HEIGHT  = 15;

  Slider(SliderProps iprops = SliderProps{}) :
      props{iprops}
  {}

  virtual ~Slider() override
  {}

  virtual Layout layout(Context &context, rect allotted) override
  {
    return Layout{.area = rect{.offset = allotted.offset, .extent = vec2{allotted.extent.x, KNOB_HEIGHT}}};
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area)
  {
    f32 percentage = (props.value - props.min) / (props.max - props.min);

    rect track_area = area;
    track_area.offset.x += KNOB_HEIGHT / 2;
    track_area.extent.x -= KNOB_HEIGHT;
    track_area.offset.y += KNOB_HEIGHT / 2;
    track_area.offset.y -= TRACK_HEIGHT / 2;
    track_area.extent.y = TRACK_HEIGHT;

    canvas.draw_rect_filled(track_area, props.color);

    f32 knob_center_pos = track_area.offset.x + percentage * track_area.extent.x;

    vec2 knob_position{knob_center_pos - KNOB_HEIGHT / 2, area.offset.y};

    canvas.draw_circle_filled(knob_position, KNOB_HEIGHT / 2, 360, props.color);
  }

  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {

  }

  virtual void on_changed(Context &context, f32 new_value)
  {}

  virtual void on_change_start(Context &context, f32 present_value)
  {}

  virtual void on_change_end(Context &context, f32 present_value)
  {}

  SliderProps props;
};

}        // namespace ash