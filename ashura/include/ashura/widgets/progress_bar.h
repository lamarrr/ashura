

#pragma once

#include "ashura/animation.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"

namespace ash
{

struct ProgressBarProps
{
  SizeConstraint size          = SizeConstraint::absolute(200, 20);
  bool           indeterminate = true;
  color          bar_color     = material::BLUE_A700;
  color          track_color   = material::GRAY_500;
};

struct ProgressBar : public Widget
{
  explicit ProgressBar(ProgressBarProps iprops = {}, f32 initial_value = 0) :
      props{iprops}, value{initial_value}
  {
    animation.restart(milliseconds{500}, 0, AnimationCfg::Loop | AnimationCfg::Alternate, 1);
  }

  STX_DISABLE_COPY(ProgressBar)
  STX_DEFAULT_MOVE(ProgressBar)

  virtual ~ProgressBar() override
  {}

  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) override
  {
    return props.size.resolve(allocated_size);
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
    canvas.draw_rect_filled(area, props.track_color);

    if (props.indeterminate)
    {
      Linear curve;
      f32    pos = animation.animate(curve, Tween{0.0f, area.extent.x});
      canvas.draw_rect_filled(area.with_extent(pos, area.extent.y), props.bar_color);
    }
    else
    {
      f32 pos = std::clamp(value * area.extent.x, 0.0f, area.extent.x);
      canvas.draw_rect_filled(area.with_extent(pos, area.extent.y), props.bar_color);
    }
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
    animation.tick(interval);
  }

  virtual bool hit_test(Context &ctx, vec2 mouse_position) override
  {
    return true;
  }

  ProgressBarProps props;
  f32              value = 0;
  Animation        animation;
};

}        // namespace ash
