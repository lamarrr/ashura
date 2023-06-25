

#pragma once
#include "ashura/primitives.h"

namespace ash
{

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
struct Curve
{
  virtual ~Curve()
  {}

  virtual f32 operator()(f32 t) = 0;
};

struct Linear final : public Curve
{
  virtual ~Linear() override
  {}

  virtual f32 operator()(f32 t) override
  {
    return t;
  }
};

struct EaseIn final : public Curve
{
  virtual ~EaseIn() override
  {}

  virtual f32 operator()(f32 t) override
  {
    return t * t;
  }
};

struct EaseOut final : public Curve
{
  virtual ~EaseOut() override
  {}

  virtual f32 operator()(f32 t) override
  {
    return 1 - (1 - t) * (1 - t);
  }
};

struct EaseInOut final : public Curve
{
  virtual ~EaseInOut() override
  {}

  virtual f32 operator()(f32 t) override
  {
    return lerp(t * t, 1 - (1 - t) * (1 - t), t);
  }
};

struct Quadratic;
struct Cubic;
struct Bezier;

};        // namespace ash
