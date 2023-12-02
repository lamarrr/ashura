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

// TODO(lamarrr): support CSS-like Animation Keyframes?
// TODO(lamarrr): animation resolution for anime-like frames

struct Animation
{
  nanoseconds  duration   = milliseconds{256};
  u64          iterations = 1;
  AnimationCfg cfg        = AnimationCfg::Default;
  f64 speed = 1;        // higher spead means faster time to completion than specified duration

  nanoseconds elapsed_duration = nanoseconds{0};
  u64         iterations_done  = 0;
  f64         t                = 0;

  void restart(nanoseconds duration, u64 iterations, AnimationCfg cfg, f64 speed)
  {
    ASH_CHECK(duration.count() > 0);
    ASH_CHECK(speed >= 0);
    this->duration         = duration;
    this->iterations       = iterations;
    this->cfg              = cfg;
    this->speed            = 1;
    this->elapsed_duration = nanoseconds{0};
    this->iterations_done  = 0;
    this->t                = 0;
  }

  constexpr AnimationState get_state() const
  {
    if (has_bits(cfg, AnimationCfg::Loop) && iterations_done >= iterations)
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
    t = has_bits(cfg, AnimationCfg::Alternate) ? (((iterations % 2) == 0) ? 0.0 : 1.0) : 1.0;
  }

  void tick(nanoseconds interval)
  {
    if (get_state() == AnimationState::Completed)
    {
      return;
    }

    nanoseconds const tick_duration = nanoseconds{(i64) ((f64) interval.count() * (f64) speed)};
    nanoseconds const total_elapsed_duration = elapsed_duration + tick_duration;
    f64 const         t_total = (((f64) total_elapsed_duration.count()) / (f64) duration.count());
    u64 const         t_iterations = (u64) t_total;

    if (has_bits(cfg, AnimationCfg::Loop) && t_iterations >= iterations)
    {
      elapsed_duration = total_elapsed_duration;
      iterations_done  = iterations;
      t = has_bits(cfg, AnimationCfg::Alternate) ? (((iterations % 2) == 0) ? 0.0 : 1.0) : 1.0;
    }
    else
    {
      f64 const t_unsigned = t_total - (f64) t_iterations;
      f64 const t_signed   = has_bits(cfg, AnimationCfg::Alternate) ?
                                 ((t_iterations % 2) == 0 ? t_unsigned : (1.0 - t_unsigned)) :
                                 t_unsigned;
      elapsed_duration     = total_elapsed_duration;
      iterations_done      = t_iterations;
      t                    = t_signed;
    }
  }

  template <typename T>
  T animate(Curve &curve, Tween<T> const &tween) const
  {
    return tween.lerp(curve((f32) t));
  }
};

}        // namespace ash
