

#pragma once
#include "ashura/math.h"
#include "ashura/types.h"

namespace ash
{

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
struct Curve
{
  virtual ~Curve()
  {
  }

  virtual f32 operator()(f32 t) = 0;
};

struct Linear final : public Curve
{
  virtual ~Linear() override
  {
  }

  virtual f32 operator()(f32 t) override
  {
    return t;
  }
};

struct EaseIn final : public Curve
{
  virtual ~EaseIn() override
  {
  }

  virtual f32 operator()(f32 t) override
  {
    return t * t;
  }
};

struct EaseOut final : public Curve
{
  virtual ~EaseOut() override
  {
  }

  virtual f32 operator()(f32 t) override
  {
    return 1 - (1 - t) * (1 - t);
  }
};

struct EaseInOut final : public Curve
{
  virtual ~EaseInOut() override
  {
  }

  virtual f32 operator()(f32 t) override
  {
    return lerp(t * t, 1 - (1 - t) * (1 - t), t);
  }
};

struct Quadratic final : public Curve
{
};

struct Cubic final : public Curve
{
};

struct QuadraticBezier final : public Curve
{
  f32 p0 = 0;
  f32 p1 = 0;
  f32 p2 = 0;

  virtual ~QuadraticBezier() override
  {
  }

  virtual f32 operator()(f32 t) override
  {
    return lerp(lerp(p0, p1, t), lerp(p1, p2, t), t);
  }
};
// TODO(lamarrr): Splines, Bezier Curves, Hermite Curves, Catmull-Rom curves,
// B-Spline

struct Spline final : public Curve
{
};

};        // namespace ash
