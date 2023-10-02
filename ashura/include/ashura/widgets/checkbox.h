#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct CheckBoxProps
{
  Color box_color         = material::BLUE_A700;
  Color checkmark_color   = material::GRAY_300;
  f32   extent            = 20;
  f32   outline_thickness = 1;
  bool  disabled          = false;
};

struct CheckBox : public Widget
{
  using Callback = stx::UniqueFn<void(CheckBox &, Context &, bool)>;

  static void default_on_changed(CheckBox &checkbox, Context &ctx, bool new_value)
  {}

  CheckBox(Callback ion_changed = stx::fn::rc::make_unique_static(default_on_changed), bool default_value = false, CheckBoxProps iprops = CheckBoxProps{}) :
      on_changed{std::move(ion_changed)}, value{default_value}, props{iprops}
  {}

  STX_DISABLE_COPY(CheckBox)
  STX_DEFAULT_MOVE(CheckBox)

  virtual ~CheckBox() override
  {}

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size, stx::Span<Vec2 const> children_allocations, stx::Span<Vec2 const> children_sizes, stx::Span<Vec2> children_positions) override
  {
    return Vec2{props.extent, props.extent};
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    Vertex2d checkmark_path[] = {
        {.position = {0.125f, 0.5f}, .color = props.checkmark_color.to_normalized_vec()},
        {.position = {0.374f, 0.75f}, .color = props.checkmark_color.to_normalized_vec()},
        {.position = {0.775f, 0.25f}, .color = props.checkmark_color.to_normalized_vec()}};

    if (value)
    {
      canvas
          .draw_rect_filled(area, props.box_color)
          .save()
          .scale(props.extent, props.extent)
          .draw_path(checkmark_path, area.offset, area.extent, 0.125f, false)
          .restore();
    }
    else
    {
      canvas.draw_rect_stroke(area, props.box_color, props.outline_thickness);
    }
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, Vec2 mouse_position, u32 nclicks) override
  {
    if (button == MouseButton::Primary && !props.disabled)
    {
      value = !value;
      on_changed.handle(*this, ctx, value);
    }
  }

  Callback      on_changed;
  bool          value = false;
  CheckBoxProps props;
};

}        // namespace gui
}        // namespace ash
