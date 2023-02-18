#pragma once
#include <chrono>

#include "ashura/primitives.h"

namespace ash {

enum class AnimationStatus: u8{
Completed,
Forward,
Dismissed,
Reversed
};

// https://api.flutter.dev/flutter/animation/Animation-class.html
struct Animation {
  f32 delay = 0;
  usize nrepeats = 0;
  f32 speed = 1;
  bool auto_reverses = false;
  f32 value =0;

  virtual f32 tick(std::chrono::nanoseconds interval, f32 previous);
};

struct Linear : public Animation {};

struct EaseIn : public Animation {
  f32 duration = 0;
};

struct EaseOut : public Animation {
  f32 duration = 0;
};

struct EaseInOut : public Animation {
  f32 duration = 0;
};

struct SpringAnimation : public Animation {
  f32 response = 0;
  f32 damping = 0;
  f32 blend_duration = 0;
};

// https://api.flutter.dev/flutter/animation/Tween-class.html
template <typename T>
struct Tween {
  T begin;
  T end;
};

}  // namespace ash
