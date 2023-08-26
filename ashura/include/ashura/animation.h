#pragma once
#include "ashura/curve.h"
#include "ashura/primitives.h"

namespace ash
{

template <typename T>
struct Tween
{
  T a;
  T b;

  constexpr T lerp(f32 t) const
  {
    return ash::lerp(a, b, t);
  }
};

enum class AnimationState : u8
{
  Paused,
  Running,
  Completed
};

enum class AnimationCfg : u8
{
  Default   = 0,
  Loop      = 1,
  Alternate = 2
};

STX_DEFINE_ENUM_BIT_OPS(AnimationCfg)

/// higher spead means faster time to completion than specified duration
struct Animation
{
  nanoseconds  duration        = milliseconds{256};
  u64          iterations      = 1;
  AnimationCfg cfg             = AnimationCfg::Default;
  f32          speed           = 1;
  nanoseconds  passed_duration = nanoseconds{0};
  u64          iterations_done = 0;
  f32          t               = 0;

  void restart(nanoseconds duration, u64 iterations, AnimationCfg cfg, f32 speed)
  {
    ASH_CHECK(duration.count() > 0);
    ASH_CHECK(speed >= 0);
    this->duration        = duration;
    this->iterations      = iterations;
    this->cfg             = cfg;
    this->speed           = 1;
    this->iterations_done = 0;
    this->t               = 0;
  }

  constexpr AnimationState get_state() const
  {
    if (epsilon_equal(t, 1) && !loop && iterations_done >= target_iterations)
    {
      return AnimationState::Completed;
    }
    else if (speed == 0)
    {
      return AnimationState::Paused;
    }
    else
    {
      return AnimationState::Running;
    }
  }

  /// pause the animation
  constexpr void pause()
  {
    speed = 0;
  }

  /// resume the animation
  constexpr void resume()
  {
    if (epsilon_equal(speed, 0))
    {
      speed = 1;
    }
  }

  /// drive the current animation iteration to completion
  constexpr void finish()
  {
    switch (direction)
    {
      case AnimationDirection::Forward:
        t = 1;
        break;

      case AnimationDirection::Backward:
        t = 0;
        break;
    }
  }

  constexpr bool is_completed() const
  {
    return (!loop) && get_state() == AnimationState::Completed;
  }

  void tick(nanoseconds interval)
  {
    if (is_completed())
    {
      return;
    }

    nanoseconds step_duration = nanoseconds{(i64) ((f64) interval.count() * (f64) speed)};
    passed_duration += step_duration;

    if (((cfg & AnimationCfg::Loop) != AnimationCfg::Loop) || iterations >= iterations_done)
    {
      return;
    }

    f32 const t_step          = (((f32) interval.count()) / (f32) duration.count()) * speed;
    u64 const step_iterations = (u64) t_step;

    if (alternate)
    {
      bool const is_even = (step_iterations % 2) == 0;
      if (is_even)
      {
        direction = (direction == AnimationDirection::Forward) ? AnimationDirection::Backward : AnimationDirection::Forward;
      }
    }
    else
    {
    }

    if (iterations_done + step_iterations >= target_iterations)
    {
      finish();
      iterations_done = target_iterations;
    }
    else
    {
      // switch (direction)
      // {
      //   case AnimationDirection::Forward:
      //     t = std::min(t + step, 1.0f);
      //     break;
      //
      //   case AnimationDirection::Backward:
      //     t = std::max(t - step, 0.0f);
      //     break;
      // }
      iterations_done += step_iterations;
      t = next_t;
    }
  }

  template <typename T>
  T animate(Curve &curve, Tween<T> const &tween) const
  {
    return tween.lerp(curve(t));
  }
};

}        // namespace ash
