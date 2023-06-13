#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct CheckBoxProps
{
  color inactive_box_color     = material::GRAY_600;
  color active_box_color       = material::BLUE_A700;
  color active_checkmark_color = material::BLACK;
  f32   extent                 = 20;
  f32   border_radius          = 4;
};

struct CheckBox : public Widget
{
  CheckBox(CheckBoxProps iprops = CheckBoxProps{}) :
      props{iprops}
  {}

  virtual ~CheckBox() override
  {}

  virtual Layout layout(Context &context, rect allotted) override
  {
    return Layout{.area = allotted.with_extent(props.extent, props.extent)};
  }

  virtual void draw(Context &context, gfx::Canvas &canvas, rect area)
  {
    // TODO(lamarrr): change to normal rectangle and use black as checkmark color
    vertex checkmark_path[] = {
        {.position = {0, 0.5}, .color = props.active_box_color.to_vec()},
        {.position = {0.35, 1}, .color = props.active_box_color.to_vec()},
        {.position = {1, 0}, .color = props.active_box_color.to_vec()}};

    if (value)
    {
      canvas.draw_round_rect_stroke(area, vec4::splat(props.border_radius), props.active_box_color, 2, 2);
      canvas.save();
      canvas.scale(props.extent * .65f, props.extent * .65f);
      canvas.draw_path(checkmark_path, area.with_extent({1, 1}), 5 / props.extent, true);
      canvas.restore();
    }
    else
    {
      canvas.draw_round_rect_stroke(area, vec4::splat(props.border_radius), props.active_box_color, 2, 360);
    }
  }

  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {
    if (button == MouseButton::Primary)
    {
      value = !value;
    }
  }

  virtual void on_changed(bool new_value)
  {}

  CheckBoxProps props;
  bool          value = false;
};

}        // namespace ash