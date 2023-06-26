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

  virtual vec2 layout(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return vec2{props.extent, props.extent};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    vertex checkmark_path[] = {
        {.position = {0.125f, 0.5f}, .color = props.checkmark_color.to_vec()},
        {.position = {0.374f, 0.75f}, .color = props.checkmark_color.to_vec()},
        {.position = {0.775f, 0.25f}, .color = props.checkmark_color.to_vec()}};

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

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary)
    {
      value = !value;
      on_changed(ctx, value);
    }
  }

  virtual void on_changed(Context &ctx, bool new_value)
  {}

  CheckBoxProps props;
  bool          value = false;
};

}        // namespace ash