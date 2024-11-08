/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"
#include <chrono>
#include <concepts>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ash
{

/// [ ] https://create.roblox.com/docs/ui/animation#style
/// [ ] Procedural Animation https://www.youtube.com/watch?v=qlfh_rv6khY

// [x] Keyframes, for time interval x, move from a to b
// [ ] keyframe blending
// [x] play
// [x] reverse
// [ ] cancel
// [ ] frame-rate customization
// [ ] move back from point
// [x] loop n or forever
// [ ] stop after timepoint
// [ ] alternate
// [x] pause
// [ ] resume in direction
// [x] reset animation state to beginning

// TODO: Support ash Span type instead of using std::string

// Animatable trait applicable to complex types like Color, Vec2, Vec3 etc
template <typename T>
concept Animatable = requires(T a, T b, f32 t) {
  { lerp(a, b, t) } -> std::convertible_to<T>;
};

struct AnimationConfig
{
  f32                     duration = 1.0f;
  f32                     delay    = 0.0f;
  bool                    loop     = false;
  std::function<f32(f32)> easing   = linear<f32>;
};

struct AnimationState
{
  bool is_playing   = false;
  f32  current_time = 0.0f;
  f32  delay        = 0.0f;
};

struct TimelineOptions
{
  f32         delay    = 0.0f;
  bool        autoplay = true;
  bool        loop     = false;
  std::string label    = "default";
};

template <typename T>
struct Animation;
template <typename T>
struct Keyframe;
template <typename T>
struct Timeline;

namespace animation
{

/// @brief Timeline concept to compose and synchronize multiple animations
namespace timeline
{
// Segment types
enum class SegmentType
{
  Basic,            // Single Animation
  Keyframed,        // Multiple Keyframes
  Parallel,         // Multiple parallel animations
  Staggered         // Staggered animations
};

struct StaggerOptions
{
  f32                                start   = 0.0f;
  f32                                from    = 0.0f;
  std::string                        grid[2] = {"", ""};
  std::function<f32(size_t, size_t)> easing  = nullptr;
};

template <Animatable T>
struct AnimationSegment
{
  std::string label;        // Useful for grouping multiple segments
  f32         start    = 0.0f;
  f32         duration = 0.0f;
  SegmentType type     = SegmentType::Basic;
  std::function<void(const T &)> onUpdate;

  virtual ~AnimationSegment()                  = default;
  virtual void update(f32 time, f32 deltaTime) = 0;
  virtual void reset()                         = 0;
};

template <Animatable T>
struct BasicSegment : AnimationSegment<T>
{
  Animation<T> animation;

  BasicSegment(const T &start, const T &end, AnimationConfig config) :
      animation(start, end, config, this->onUpdate)
  {
    this->duration = config.duration;
    this->type     = SegmentType::Basic;
  }

  void update(f32 time, f32 delta) override
  {
    animation.update(delta);
  }

  void reset() override
  {
    animation.reset();
  }
};

template <Animatable T>
struct KeyframeSegment : AnimationSegment<T>
{
  std::vector<Keyframe<T>> keyframes;
  f32                      current_time = 0.0f;

  KeyframeSegment(const std::vector<Keyframe<T>> &kf) : keyframes(kf)
  {
    this->type = SegmentType::Keyframed;
    calculate_duration();
  }

  void calculate_duration()
  {
    this->duration = 0.0f;
    for (const auto &kf : keyframes)
    {
      this->duration += kf.duration;
    }
  }

  void update(f32 time, f32 delta) override
  {
    if (keyframes.size() < 2)
      return;

    current_time         = time - this->start;
    f32 accumulated_time = 0.0f;

    for (size_t i = 0; i < keyframes.size() - 1; ++i)
    {
      accumulated_time += keyframes[i].duration;

      if (current_time <= accumulated_time)
      {
        const Keyframe<T> &current_kf = keyframes[i];
        const auto        &next_kf    = keyframes[i + 1];

        f32 local_start_time = accumulated_time - current_kf.duration;
        f32 local_time =
            (current_time - local_start_time) / current_kf.duration;
        f32 eased_time = current_kf.get_ease_fn(local_time);

        if (this->on_update)
        {
          //   T interpolated_value;
          //   if constexpr (std::is_arithmetic_v<T>)
          //   {
          //     interpolated_value =
          //         lerp(current_kf.value, next_kf.value, eased_time);
          //   } else {
          //     interpolated_value = T::lerp(current_kf.value, next_kf.value,
          //     eased_time);
          //   }
          T interpolated_value =
              lerp(current_kf.value, next_kf.value, eased_time);

          this->on_update(interpolated_value);
        }
        break;
      }
    }
  }

  void reset() override
  {
    current_time = 0.0f;
  }
};
}        // namespace timeline
}        // namespace animation

struct IAnimation
{
  virtual ~IAnimation()           = default;
  virtual void update(f32 delta)  = 0;
  virtual void play()             = 0;
  virtual void pause()            = 0;
  virtual void reset()            = 0;
  virtual bool is_playing() const = 0;
};

template <typename T>
struct Animation : IAnimation
{
  T                              start;
  T                              end;
  AnimationConfig                config;
  AnimationState                 state;
  std::function<void(const T &)> on_update;

  Animation(T start, T end, AnimationConfig cfg,
            std::function<void(const T &)> callback) :
      start(start), end(end), config(cfg), on_update(callback)
  {
    state.delay = config.delay;
  }

  void update(f32 delta)
  {
    if (!state.is_playing)
      return;

    if (state.delay > 0)
    {
      state.delay -= delta;
      return;
    }

    state.current_time += delta;

    if (state.current_time >= config.duration)
    {
      if (config.loop)
      {
        state.current_time = std::fmod(state.current_time, config.duration);
      }
      else
      {
        state.current_time = config.duration;
        state.is_playing   = false;
      }
    }

    f32 t      = state.current_time / config.duration;
    f32 easedT = config.easing(t);

    T current_value = lerp(start, end, easedT);

    if (on_update)
    {
      on_update(current_value);
    }
  }

  void play()
  {
    state.is_playing = true;
  }

  void pause()
  {
    state.is_playing = false;
  }

  void reset()
  {
    state.current_time = 0;
    state.delay        = config.delay;
    if (on_update)
    {
      on_update(start);
    }
  }

  bool is_playing() const
  {
    return state.is_playing;
  }
};

// Manage multiple animations.
struct AnimationController
{
  std::vector<std::shared_ptr<IAnimation>> animations;

  template <typename T>
  std::shared_ptr<Animation<T>> create(T start, T end, AnimationConfig config,
                                       std::function<void(const T &)> callback)
  {
    auto anim = std::make_shared<Animation<T>>(start, end, config, callback);
    animations.push_back(anim);
    return anim;
  }

  void update(f32 delta)
  {
    for (auto &anim : animations)
    {
      if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
      {
        ianim->update(delta);
      }
    }
  }

  void play_all()
  {
    for (auto &anim : animations)
    {
      if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
      {
        ianim->play();
      }
    }
  }

  void pause_all()
  {
    for (auto &anim : animations)
    {
      if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
      {
        ianim->pause();
      }
    }
  }

  void reset_all()
  {
    for (auto &anim : animations)
    {
      if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
      {
        ianim->reset();
      }
    }
  }
};

enum class CurveType
{
  Linear,
  EaseIn,
  EaseOut,
  EaseInOut,
  //   Bezier,
  //   CubicBezier,
  Custom
};

struct KeyframeBase
{
  f32                     duration;
  std::string             key;
  CurveType               curve_type;
  std::function<f32(f32)> custom_easing;

  KeyframeBase(f32 dur, std::string k, CurveType curve = CurveType::Linear) :
      duration(dur), key(std::move(k)), curve_type(curve)
  {
  }

  KeyframeBase(f32 dur, std::string k, std::function<f32(f32)> easing) :
      duration(dur),
      key(std::move(k)),
      curve_type(CurveType::Custom),
      custom_easing(std::move(easing))
  {
  }

  f32 get_ease_fn(f32 t) const
  {
    if (curve_type == CurveType::Custom && custom_easing)
    {
      return custom_easing(t);
    }

    switch (curve_type)
    {
      case CurveType::Linear:
        return linear(t);
      case CurveType::EaseIn:
        return ease_in(t);
      case CurveType::EaseOut:
        return ease_out(t);
      case CurveType::EaseInOut:
        return ease_in_out(t);
      default:
        return t;
    }
  }
};

template <typename T>
struct Keyframe : KeyframeBase
{
  T value;

  Keyframe(f32 dur, std::string k, T val, CurveType curve = CurveType::Linear) :
      KeyframeBase(dur, std::move(k), curve), value(std::move(val))
  {
  }

  Keyframe(f32 dur, std::string k, T val, std::function<f32(f32)> easing) :
      KeyframeBase(dur, std::move(k), std::move(easing)), value(std::move(val))
  {
  }
};

template <typename T>
struct Timeline
{
  using SegmentPtr = std::shared_ptr<animation::timeline::AnimationSegment<T>>;

  std::vector<SegmentPtr> segments;
  TimelineOptions         options;
  f32                     current_time   = 0.0f;
  bool                    is_playing     = false;
  f32                     total_duration = 0.0f;

  Timeline(const TimelineOptions &opts = TimelineOptions{}) : options(opts)
  {
    if (options.autoplay)
    {
      is_playing = true;
    }
  }

  Timeline &add(const T &start, const T &end, AnimationConfig config)
  {
    auto segment = std::make_shared<animation::timeline::BasicSegment<T>>(
        start, end, config);
    segment->startTime = total_duration;
    total_duration += segment->duration;
    segments.push_back(segment);
    return *this;
  }

  Timeline &add(const std::vector<Keyframe<T>> &keyframes)
  {
    auto segment =
        std::make_shared<animation::timeline::KeyframeSegment<T>>(keyframes);
    segment->startTime = total_duration;
    total_duration += segment->duration;
    segments.push_back(segment);
    return *this;
  }

  Timeline &parallel(const std::vector<AnimationConfig> &configs,
                     const std::vector<std::pair<T, T>> &values)
  {
    f32 max_duration = 0.0f;
    f32 start_time   = total_duration;

    for (size_t i = 0; i < configs.size(); ++i)
    {
      auto segment = std::make_shared<animation::timeline::BasicSegment<T>>(
          values[i].first, values[i].second, configs[i]);
      segment->start = start_time;
      segment->type  = animation::timeline::SegmentType::Parallel;
      segments.push_back(segment);
      max_duration = std::max(max_duration, configs[i].duration);
    }

    total_duration += max_duration;
    return *this;
  }

  Timeline &stagger(const T &start, const T &end, AnimationConfig config,
                    size_t                                     count,
                    const animation::timeline::StaggerOptions &stagger_opts =
                        animation::timeline::StaggerOptions{})
  {
    f32 stagger_delay = config.duration / static_cast<f32>(count);
    f32 startTime     = total_duration;

    for (size_t i = 0; i < count; ++i)
    {
      f32 delay = stagger_opts.start +
                  (stagger_delay * (stagger_opts.easing ?
                                        stagger_opts.easing(i, count) :
                                        static_cast<f32>(i) / count));

      auto segment_config  = config;
      segment_config.delay = delay;

      auto segment = std::make_shared<animation::timeline::BasicSegment<T>>(
          start, end, segment_config);
      segment->startTime = startTime;
      segment->type      = animation::timeline::SegmentType::Staggered;
      segments.push_back(segment);
    }

    total_duration += config.duration + (stagger_delay * (count - 1));
    return *this;
  }

  void update(f32 delta)
  {
    if (!is_playing)
      return;

    current_time += delta;

    if (current_time >= total_duration)
    {
      if (options.loop)
      {
        current_time = std::fmod(current_time, total_duration);
        for (auto &segment : segments)
        {
          segment->reset();
        }
      }
      else
      {
        current_time = total_duration;
        is_playing   = false;
      }
    }

    for (auto &segment : segments)
    {
      segment->update(current_time, delta);
    }
  }

  void play()
  {
    is_playing = true;
  }
  void pause()
  {
    is_playing = false;
  }
  void reset()
  {
    current_time = 0.0f;
    for (auto &segment : segments)
    {
      segment->reset();
    }
  }
  void set_callback(std::function<void(const T &)> callback)
  {
    for (auto &segment : segments)
    {
      segment->onUpdate = callback;
    }
  }
};

}        // namespace ash
