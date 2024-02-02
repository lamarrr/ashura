

#pragma once

#include "ashura/animation.h"
#include "ashura/primitives.h"
#include "ashura/time.h"
#include "ashura/widget.h"

namespace ash
{
namespace gui
{

struct ProgressBarProps
{
  Constraint2D size          = Constraint2D::absolute(300, 15);
  bool         indeterminate = true;
  Color        bar_color     = material::BLUE_A700;
  Color        track_color   = material::GRAY_500;
};

struct ProgressBar : public Widget
{
  explicit ProgressBar(ProgressBarProps iprops = {}, f32 initial_value = 0) :
      props{iprops}, value{initial_value}
  {
    animation.restart(Milliseconds{500}, 0,
                      AnimationCfg::Loop | AnimationCfg::Alternate, 1);
  }

  STX_DISABLE_COPY(ProgressBar)
  STX_DEFAULT_MOVE(ProgressBar)

  virtual ~ProgressBar() override
  {
  }

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions) override
  {
    return props.size.resolve(allocated_size);
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    canvas.draw_rect_filled(area.offset, area.extent, props.track_color);

    if (props.indeterminate)
    {
      Linear curve;
      f32    pos = animation.animate(curve, Tween<f32>{0.0f, area.extent.x});
      canvas.draw_rect_filled(area.offset, {pos, area.extent.y},
                              props.bar_color);
    }
    else
    {
      f32 pos = std::clamp(value * area.extent.x, 0.0f, area.extent.x);
      canvas.draw_rect_filled(area.offset, {pos, area.extent.y},
                              props.bar_color);
    }

    canvas.draw_rect_stroke(area.offset, area.extent, props.bar_color, 1);
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    animation.tick(interval);
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  ProgressBarProps props;
  f32              value = 0;
  Animation        animation;
};

}        // namespace gui
}        // namespace ash
