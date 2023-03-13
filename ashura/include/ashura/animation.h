#pragma once
#include <chrono>

#include "ashura/primitives.h"

namespace ash
{

struct Animation
{
  f32 duration = 0;
  f32 begin = 0, end = 0;

  virtual f32 tick(f32 timepoint)
  {
    return 0;
  }
};

struct Linear : public Animation
{
  virtual f32 tick(f32 timepoint) override
  {
    return Animation::end * timepoint / Animation::duration + Animation::begin;
  }
};

struct EaseIn : public Animation
{
  virtual f32 tick(f32 timepoint) override
  {
    return Animation::end * (timepoint /= Animation::duration) * timepoint +
           Animation::begin;
  }
};

struct EaseOut : public Animation
{
  virtual f32 tick(f32 timepoint) override
  {
    return -Animation::end * (timepoint /= Animation::duration) *
               (timepoint - 2) +
           Animation::begin;
  }
};

struct EaseInOut : public Animation
{
  virtual f32 tick(f32 timepoint) override
  {
    if ((timepoint /= Animation::duration / 2) < 1)
      return Animation::end / 2 * timepoint * timepoint + Animation::begin;
    return -Animation::end / 2 * ((--timepoint) * (timepoint - 2) - 1) +
           Animation::begin;
  }
};

}        // namespace ash
