#pragma once
#include "ashura/math.h"
#include "ashura/primitives.h"
#include "ashura/types.h"
#include "ashura/utils.h"

namespace ash
{

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
constexpr f32 linear(f32 t)
{
  return t;
}

constexpr f32 ease_in(f32 t)
{
  return t * t;
}

constexpr f32 ease_out(f32 t)
{
  return 1 - (1 - t) * (1 - t);
}

constexpr f32 ease_in_out(f32 t)
{
  return lerp(t * t, 1 - (1 - t) * (1 - t), t);
}

constexpr f32 quadratic_bezier(f32 p0, f32 p1, f32 p2, f32 t)
{
  return lerp(lerp(p0, p1, t), lerp(p1, p2, t), t);
}

constexpr f32 step(f32 a, f32 t)
{
  return t < a ? 0.0f : 1.0f;
}

constexpr f32 smoothstep(f32 a, f32 b, f32 t)
{
  t = (t - a) / (b - a);
  return t * t * (3.0f - 2.0f * t);
}

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

/// @speed: higher spead means faster time to completion than specified duration
struct Animation
{
  Nanoseconds  duration         = Milliseconds{256};
  u64          iterations       = 1;
  AnimationCfg cfg              = AnimationCfg::Default;
  f32          speed            = 1;
  Nanoseconds  elapsed_duration = Nanoseconds{0};
  u64          iterations_done  = 0;
  f32          t                = 0;

  void restart(Nanoseconds duration, u64 iterations, AnimationCfg cfg,
               f32 speed)
  {
    ASH_CHECK(duration.count() > 0);
    ASH_CHECK(speed >= 0);
    this->duration         = duration;
    this->iterations       = iterations;
    this->cfg              = cfg;
    this->speed            = 1;
    this->elapsed_duration = Nanoseconds{0};
    this->iterations_done  = 0;
    this->t                = 0;
  }

  constexpr AnimationState get_state() const
  {
    if (!has_bits(cfg, AnimationCfg::Loop) && iterations_done >= iterations)
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

  constexpr void pause()
  {
    speed = 0;
  }

  constexpr void resume()
  {
    speed = 1;
  }

  constexpr void complete()
  {
    cfg &= ~AnimationCfg::Loop;
    iterations_done = iterations;
    t               = has_bits(cfg, AnimationCfg::Alternate) ?
                          (((iterations % 2) == 0) ? 0.0 : 1.0) :
                          1.0;
  }

  void tick(Nanoseconds interval)
  {
    if (get_state() == AnimationState::Completed)
    {
      return;
    }

    Nanoseconds const tick_duration =
        Nanoseconds{(i64) ((f32) interval.count() * (f32) speed)};
    Nanoseconds const total_elapsed_duration = elapsed_duration + tick_duration;
    f32 const         t_total =
        (((f32) total_elapsed_duration.count()) / (f32) duration.count());
    u64 const t_iterations = (u64) t_total;

    if (!has_bits(cfg, AnimationCfg::Loop) && t_iterations >= iterations)
    {
      elapsed_duration = total_elapsed_duration;
      iterations_done  = iterations;
      t                = has_bits(cfg, AnimationCfg::Alternate) ?
                             (((iterations % 2) == 0) ? 0.0 : 1.0) :
                             1.0;
    }
    else
    {
      f32 const t_unsigned = t_total - (f32) t_iterations;
      f32 const t_signed =
          has_bits(cfg, AnimationCfg::Alternate) ?
              ((t_iterations % 2) == 0 ? t_unsigned : (1.0 - t_unsigned)) :
              t_unsigned;
      elapsed_duration = total_elapsed_duration;
      iterations_done  = t_iterations;
      t                = t_signed;
    }
  }

  template <typename T>
  T animate(Curve &curve, Tween<T> const &tween) const
  {
    return tween.lerp(curve((f32) t));
  }
};

}        // namespace ash
