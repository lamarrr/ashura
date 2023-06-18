#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct CheckBoxProps
{
  color box_color         = material::BLUE_A700;
  color outline_color     = material::GRAY_600;
  color checkmark_color   = material::GRAY_300;
  f32   extent            = 20;
  f32   border_radius     = 2.5;
  f32   outline_thickness = 1;
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
    vertex checkmark_path[] = {
        {.position = {0.125, 0.5}, .color = props.checkmark_color.to_vec()},
        {.position = {0.374, 0.75}, .color = props.checkmark_color.to_vec()},
        {.position = {0.775, 0.25}, .color = props.checkmark_color.to_vec()}};

    if (value)
    {
      canvas.draw_round_rect_filled(area, vec4::splat(props.border_radius), 10, props.box_color)
          .save()
          .scale(props.extent, props.extent)
          .draw_path(checkmark_path, area, 0.125f, false)
          .restore();
    }
    else
    {
      canvas.draw_round_rect_stroke(area, vec4::splat(props.border_radius), 10, props.outline_color, props.outline_thickness);
    }
  }

  virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {
    if (button == MouseButton::Primary)
    {
      value = !value;
      on_changed(context, value);
    }
  }

  virtual void on_changed(Context &context, bool new_value)
  {}

  CheckBoxProps props;
  bool          value = false;
};

}        // namespace ash