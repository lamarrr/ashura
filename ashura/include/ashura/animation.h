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

enum class AnimationDirection : u8
{
  Forward,
  Reverse,
};

enum class AnimationState : u8
{
  Paused,
  Forward,
  Reversing,
  Completed
};

// TODO(lamarrr): Splines, Bezier Curves, Hermite Curves, Catmull-Rom curves, B-Spline

struct Animation
{
  /// CONFIGURATION
  nanoseconds        duration          = nanoseconds{0};
  nanoseconds        reverse_duration  = nanoseconds{0};
  usize              target_iterations = 1;
  AnimationDirection direction         = AnimationDirection::Forward;
  bool               alternate         = false;

  /// INTERNAL STATE
  usize iterations_done = 0;
  f32   t               = 0;
  f32   speed           = 1;        // higher spead means faster time to completion than specified duration

  constexpr void restart(nanoseconds duration, nanoseconds reverse_duration, usize iterations, bool alternate)
  {
    this->duration          = duration;
    this->reverse_duration  = reverse_duration;
    this->target_iterations = iterations;
    this->direction         = AnimationDirection::Forward;
    this->iterations_done   = 0;
    this->t                 = 0;
    this->speed             = 1;
    this->alternate         = alternate;
  }

  constexpr AnimationState get_state() const
  {
    switch (direction)
    {
      case AnimationDirection::Forward:
        if (epsilon_equal(t, 1) && iterations_done >= target_iterations)
        {
          return AnimationState::Completed;
        }
        else if (epsilon_equal(speed, 0))
        {
          return AnimationState::Paused;
        }
        else
        {
          return AnimationState::Forward;
        }
        break;

      case AnimationDirection::Reverse:
        if (epsilon_equal(t, 0) && iterations_done >= target_iterations)
        {
          return AnimationState::Completed;
        }
        else if (epsilon_equal(speed, 0))
        {
          return AnimationState::Paused;
        }
        else
        {
          return AnimationState::Reversing;
        }
        break;

      default:
        return AnimationState::Paused;
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

  /// reverse the animation's direction
  constexpr void reverse()
  {
    direction = AnimationDirection::Reverse;
  }

  /// drive the animation to completion
  constexpr void finish()
  {
    switch (direction)
    {
      case AnimationDirection::Forward:
        t = 1;
        break;

      case AnimationDirection::Reverse:
        t = 0;
        break;
    }
  }

  constexpr bool is_completed() const
  {
    return get_state() == AnimationState::Completed;
  }

  void tick(nanoseconds interval)
  {
    if (is_completed())
    {
      return;
    }

    f32 step_duration = (direction == AnimationDirection::Forward ? duration : reverse_duration).count();
    // TODO(lamarrr): speed negative might cause overflow
    f32   next_t          = t + (direction == AnimationDirection::Forward ? speed : -speed) * AS(f32, interval.count()) / epsilon_clamp(step_duration);        // if alternating, this would be squared by iterations?
    usize step_iterations = (usize) (i64) next_t;
    next_t                = next_t - (f32) (i64) next_t;

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
      //   case AnimationDirection::Reverse:
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
