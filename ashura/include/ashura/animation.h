#pragma once
#include "ashura/primitives.h"

namespace ash
{

template <typename T>
struct Tween
{
  T begin;
  T end;

  constexpr T lerp(f32 percentage)
  {
    return ash::lerp(begin, end, percentage);
  }
};

namespace curves
{

constexpr f32 ease_in(f32 percentage)
{
  return percentage * percentage;
}

constexpr f32 ease_out(f32 percentage)
{
  return 1 - (1 - percentage) * (1 - percentage);
}

constexpr f32 ease_in_out(f32 percentage)
{
  return lerp(ease_in(percentage), ease_out(percentage), percentage);
}

};        // namespace curves

template <typename T>
struct AnimationCurve
{
  virtual ~AnimationCurve() override
  {}

  /// drive the animation.
  /// timepoint is the time passed since the animation started.
  /// time is tracked by an external clock source.
  virtual T forward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) = 0;

  virtual T backward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) = 0;
};

template <typename T>
struct Linear final : public AnimationCurve<T>
{
  virtual ~Linear() override
  {}

  virtual T forward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    return this->tween.lerp(percentage);
  }
};

template <typename T>
struct EaseIn final : public AnimationCurve<T>
{
  virtual ~EaseIn() override
  {}

  virtual T forward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    return this->tween.lerp(curves::ease_in(percentage));
  }
};

template <typename T>
struct EaseOut final : public AnimationCurve<T>
{
  virtual ~EaseOut() override
  {}

  virtual T forward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    return this->tween.lerp(curves::ease_out(percentage));
  }
};

template <typename T>
struct EaseInOut final : public AnimationCurve<T>
{
  virtual ~EaseInOut() override
  {}

  virtual T forward(Tween<T> tween, nanoseconds duration, nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    return this->tween.lerp(curves::ease_in_out(percentage));
  }
};

enum class AnimationDirection : u8
{
  Forward,
  Rewind,
  Static,
  Alternate
};

enum class AnimationState : u8
{
  Paused,
  Forward,
  Backward,
  Completed
};

struct AnimationProps
{
  nanoseconds        duration          = nanoseconds{0};
  nanoseconds        delay             = nanoseconds{0};
  usize              iterations        = 1;
  AnimationDirection direction         = AnimationDirection::Forward;
  nanoseconds        current_timepoint = nanoseconds{0};
  timepoint          begin;
};

// TODO(lamarrr): combined animations move from a to b then c to d -> combined duration, multiple tweens
// TODO(lamarrr): compound animations move from a to b then b to c -> total duration, single tween

template <typename T>
struct Animation
{
  AnimationProps               props;
  AnimationState               state           = AnimationState::Paused;
  usize                        iterations_done = 0;
  stx::Rc<AnimationCurve<T> *> curve;

  /// reset the animation
  void reset()
  {}

  /// pause the animation
  void pause()
  {}

  /// rewind the animation
  void rewind()
  {}

  /// drive the animation to completion
  void finish()
  {}

  T tick(nanoseconds timepoint);
};

namespace animation
{

template <typename T>
stx::Rc<Animation<T> *> make_linear(Tween<T> tween, nanoseconds duration)
{
  return stx::cast<Animation<T> *>(stx::rc::make_inplace<LinearAnimation<T>>(stx::os_allocator, tween, duration).unwrap());
}

}        // namespace animation

}        // namespace ash
