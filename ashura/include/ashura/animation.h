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

template <typename T>
struct Animation
{
  virtual ~Animation()
  {}

  /// drive the animation.
  /// timepoint is the time passed since the animation started.
  /// time is tracked by an external clock source.
  virtual T tick(nanoseconds timepoint) = 0;
};

template <typename T>
struct TweenAnimation : public virtual Animation<T>
{
  TweenAnimation(Tween<T> itween, nanoseconds iduration) :
      tween{itween}, duration{iduration}
  {}

  virtual ~TweenAnimation() override
  {}

  virtual T tick( Tween<T> tween, nanoseconds duration, nanoseconds timepoint );

  Tween<T>    tween;
  nanoseconds duration{0};
};

template <typename T>
struct LinearAnimation final : public TweenAnimation<T>
{
  explicit LinearAnimation(Tween<T> itween, nanoseconds iduration) :
      TweenAnimation<T>{itween, iduration}
  {}

  virtual ~LinearAnimation() override
  {}

  virtual T tick(nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    return this->tween.lerp(percentage);
  }
};

template <typename T>
struct EaseInAnimation final : public TweenAnimation<T>
{
  explicit EaseInAnimation(Tween<T> itween, nanoseconds iduration) :
      TweenAnimation<T>{itween, iduration}
  {}

  virtual ~EaseInAnimation() override
  {}

  virtual T tick(nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    f32 ease_in    = percentage * percentage;
    return this->tween.lerp(ease_in);
  }
};

template <typename T>
struct EaseOutAnimation final : public TweenAnimation<T>
{
  explicit EaseOutAnimation(Tween<T> itween, nanoseconds iduration) :
      TweenAnimation<T>{itween, iduration}
  {}

  virtual ~EaseOutAnimation() override
  {}

  virtual T tick(nanoseconds timepoint) override
  {
    f32 percentage = AS(f32, timepoint.count()) / this->duration.count();
    f32 ease_out   = 1 - ((1 - percentage) * (1 - percentage));
    return this->tween.lerp(ease_out);
  }
};

template <typename T>
struct EaseInOutAnimation final : public TweenAnimation<T>
{
  explicit EaseInOutAnimation(Tween<T> itween, nanoseconds iduration) :
      TweenAnimation<T>{itween, iduration}
  {}

  virtual ~EaseInOutAnimation() override
  {}

  virtual T tick(nanoseconds timepoint) override
  {
    f32 percentage  = AS(f32, timepoint.count()) / this->duration.count();
    f32 ease_in     = percentage * percentage;
    f32 ease_out    = 1 - ((1 - percentage) * (1 - percentage));
    f32 ease_in_out = lerp(ease_in, ease_out, percentage);
    return this->tween.lerp(ease_in_out);
  }
};

enum class AnimationDirection : u8
{
  Forward,
  Backwards,
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
  nanoseconds        delay              = nanoseconds{0};
  nanoseconds        backwards_duration = nanoseconds{0};
  usize              iterations         = 1;
  AnimationDirection direction          = AnimationDirection::Forward;
};

template <typename T>
struct AnimationController
{
  Twe
  AnimationProps props;
  AnimationState    state     = AnimationState::Paused;
  usize             iteration = 1;

  /// reset the animation
    void reset(){}

  /// pause the animation
    void pause() {}

  /// reverse the animation
    void reverse() {}

  /// drive the animation to completion
    void finish() {}

};

namespace animation
{

template <typename T>
stx::Rc<TweenAnimation<T> *> make_linear(Tween<T> tween, nanoseconds duration)
{
  return stx::cast<TweenAnimation<T> *>(stx::rc::make_inplace<LinearAnimation<T>>(stx::os_allocator, tween, duration).unwrap());
}

}        // namespace animation

}        // namespace ash
