#pragma once
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

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
struct AnimationCurve
{
  virtual ~AnimationCurve()
  {}

  virtual f32 tick(f32 t) = 0;
};

struct Linear final : public AnimationCurve
{
  virtual ~Linear() override
  {}

  virtual f32 tick(f32 t) override
  {
    return t;
  }
};

struct EaseIn final : public AnimationCurve
{
  virtual ~EaseIn() override
  {}

  virtual f32 tick(f32 t) override
  {
    return t * t;
  }
};

struct EaseOut final : public AnimationCurve
{
  virtual ~EaseOut() override
  {}

  virtual f32 tick(f32 t) override
  {
    return 1 - (1 - t) * (1 - t);
  }
};

struct EaseInOut final : public AnimationCurve
{
  virtual ~EaseInOut() override
  {}

  virtual f32 tick(f32 t) override
  {
    return lerp(t * t, 1 - (1 - t) * (1 - t), t);
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
  nanoseconds        duration         = nanoseconds{0};
  nanoseconds        reverse_duration = nanoseconds{0};
  usize              iterations       = 1;
  AnimationDirection direction        = AnimationDirection::Forward;

  /// INTERNAL STATE
  usize iterations_done = 0;
  f32   t               = 0;
  f32   speed           = 1;        // higher spead means faster time to completion than specified duration

  constexpr void restart(
      nanoseconds duration,
      nanoseconds reverse_duration,
      usize       iterations)
  {
    this->duration         = duration;
    this->reverse_duration = reverse_duration;
    this->iterations       = iterations;
    this->direction        = AnimationDirection::Forward;
    this->iterations_done  = 0;
    this->t                = 0;
    this->speed            = 1;
  }

  constexpr AnimationState get_state() const
  {
    switch (direction)
    {
      case AnimationDirection::Forward:
        if (epsilon_equal(t, 1) && iterations_done == iterations)
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
        if (epsilon_equal(t, 0) && iterations_done == iterations)
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

    f32   step_duration   = (direction == AnimationDirection::Forward ? duration : reverse_duration).count();
    f32   step            = speed * AS(f32, interval.count()) / epsilon_clamp(step_duration);
    usize step_iterations = AS(usize, step);
    step                  = step - AS(f32, AS(i64, step));

    if (iterations_done + step_iterations >= iterations)
    {
      finish();
      iterations_done = iterations;
    }
    else
    {
      switch (direction)
      {
        case AnimationDirection::Forward:
          t = std::min(t + step, 1.0f);
          break;

        case AnimationDirection::Reverse:
          t = std::max(t - step, 0.0f);
          break;
      }
      iterations_done += step_iterations;
    }
  }

  template <typename T>
  T animate(AnimationCurve &curve, Tween<T> const &tween) const
  {
    return tween.lerp(curve.tick(t));
  }
};

}        // namespace ash
