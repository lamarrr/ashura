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
// [x] resume in direction
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

enum class GridDirection
{
  RowMajor,          // Left to right, top to bottom
  ColumnMajor        // Top to bottom, left to right
};

struct StaggerOptions
{
  f32 start = 0.0f;
  f32 from  = 0.0f;
  struct Grid
  {
    i32           rows      = 1;
    i32           cols      = 1;
    GridDirection direction = GridDirection::RowMajor;
  } grid;
  std::function<f32(i32 row, i32 col, i32 index, i32 total)> easing = nullptr;
};

static std::pair<i32, i32> grid_position(i32 index, i32 rows, i32 cols,
                                         GridDirection direction)
{
  if (direction == GridDirection::RowMajor)
  {
    return {
        index / cols,        // row
        index % cols         // col
    };
  }
  else
  {
    return {
        index % rows,        // row
        index / rows         // col
    };
  }
}

f32 stagger_grid_delay(i32 index, i32 count, const StaggerOptions &stagger_opts)
{
  auto [row, col] = animation::grid_position(index, stagger_opts.grid.rows,
                                             stagger_opts.grid.cols,
                                             stagger_opts.grid.direction);

  if (!stagger_opts.easing)
  {
    return stagger_opts.easing(row, col, index, count);
  }
  else
  {
    // Default grid-based delay calculation
    f32 normalized_row = static_cast<f32>(row) / (stagger_opts.grid.rows - 1);
    f32 normalized_col = static_cast<f32>(col) / (stagger_opts.grid.cols - 1);

    switch (stagger_opts.grid.direction)
    {
      case animation::GridDirection::RowMajor:
        return normalized_row + (normalized_col * 0.5f);
      case animation::GridDirection::ColumnMajor:
        return normalized_col + (normalized_row * 0.5f);
    }
  }
}

f32 stagger_linear_delay(i32 i, i32 count,
                         const StaggerOptions &stagger_opts = StaggerOptions{})
{
  return stagger_opts.easing ? stagger_opts.easing(0, 0, i, count) :
                               static_cast<f32>(i) / count;
}

f32 stagger_delay(i32 i, i32 count,
                  const StaggerOptions &stagger_opts = StaggerOptions{})
{
  bool is_grid_based = stagger_opts.grid.rows > 1 || stagger_opts.grid.cols > 1;

  if (is_grid_based)
  {
    return animation::stagger_grid_delay(i, count, stagger_opts);
  }

  return animation::stagger_linear_delay(i, count, stagger_opts);
}


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


template <Animatable T>
struct TimelineSegment
{
  std::string label;        // Useful for grouping multiple segments
  f32         start    = 0.0f;
  f32         duration = 0.0f;
  SegmentType type     = SegmentType::Basic;
  std::function<void(const T &)> on_update;

  virtual ~TimelineSegment()                   = default;
  virtual void update(f32 time, f32 deltaTime) = 0;
  virtual void reset()                         = 0;
};

template <Animatable T>
struct BasicSegment : TimelineSegment<T>
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
struct KeyframeSegment : TimelineSegment<T>
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

  void update(f32 time, f32 _delta) override
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
// - play multiple animations
// - pause all animations
// - reset all animations
// - grid based animations
// - set a callback function to track animation updates
// ```
// AnimationManager animator;
//
// auto anim1 = animator.create<Vec2>(
//     start_pos,
//     end_pos,
//     AnimationConfig{.duration = 1.0f, .loop = true},
//     [](const Vec2& pos) { /* update logic */ }
// );
//
// auto anim2 = animator.create<Vec2>(
//     start_pos,
//     end_pos,
//     AnimationConfig{.duration = 1.0f, .loop = true},
//     [](const Vec2& pos) { /* update logic */ }
// );
//
// // Play individual animation
// anim2.play();
// // Or play all the animations
// animator.play_all();
//
// ```
//
//
// ### Staggered (Grid) animations
//--------
// ### Example: Ripple Pattern Animation
//```
// animator.stagger<f32>(
//  0.0f, // start
//  1.0f, // end
//  AnimationConfig{.duration = 1.0f, .loop = true},
//  10, // total count
//  StaggerOptions {
//      .grid = {4, 4},
//      .easing = [](int row, int col, int index, int total) {
//          // Ripple effect
//          f32 center_row = 1.5f;
//          f32 center_col = 1.5f;
//          f32 distance = std::sqrt(
//              std::pow(row - center_row, 2) +
//              std::pow(col - center_col, 2)
//          );
//          return distance * 0.15f;
//  },
//  [](const f32& value, index) { /* update logic */ }
//);
//```
struct AnimationManager
{
private:
  std::vector<std::shared_ptr<IAnimation>> animations;

public:

  void reserve(u32 size){
    animations.reserve(size);
  }

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

  template <typename T>
  void stagger(const T &start, const T &end, const AnimationConfig &base_config,
               size_t                           count,
               const animation::StaggerOptions &stagger_opts =
                   animation::StaggerOptions{},
               std::function<void(const T &, size_t)> callback = nullptr)
  {
    for (size_t i = 0; i < count; ++i)
    {
      f32 base_delay = animation::stagger_delay(i, count, stagger_opts);

      auto config  = base_config;
      config.delay = stagger_opts.start + (base_delay * stagger_opts.from);

      // Create animation with grid index-aware callback
      create<T>(start, end, config, [callback, i](const T &value) {
        if (callback)
          callback(value, i);
      });

      play_all();
    }
  }
};

enum class CurveType
{
  Linear,
  EaseIn,
  EaseOut,
  EaseInOut,
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

// Timeline: A more robust way to manage multiple animations
// -  Timeline animations or keyframes can be run in parallel or staggered.
// -  Set a callback to track each animation or keyframe update.
// -  Direction control
// -  Playback control
// ```
// TimelineOptions options{
//         .autoplay = true,
//         .loop = true
// };
// Timeline<Vec2> timeline(options);
//
// std::vector<Keyframe<Vec2>> keyframes = {
//     Keyframe<Vec2>(0.5f, "start", {0, 100.0f}, CurveType::EaseInOut),
//     Keyframe<Vec2>(0.5f, "middle", {150.0f, 50.0f}, CurveType::EaseInOut),
//     Keyframe<Vec2>(0.5f, "end", {0, 200.0f}, CurveType::EaseInOut)
// };
//
// timeline.add(keyframes);
// // Basic controls
// timeline.play();
// timeline.pause();
// timeline.reverse();
// timeline.seek(2.0f);
// timeline.seek_percentage(0.5f);
//
//  // Playback controls
// timeline.play_from_start();  // Forward from start
// timeline.play_from_end();    // Reverse from end
// timeline.stop();          // Reset to start
// ```
// ---
//
// ### Chainable calls
// ```
// timeline
//     .add(start, end, config)                  // Simple animation
//     .add(keyframes)                           // Keyframe sequence
//     .parallel(configs, values)                // Parallel animations
//     .stagger(start, end, config, count);      // Staggered animations
// ```
template <typename T>
struct Timeline
{
  using SegmentPtr = std::shared_ptr<animation::timeline::TimelineSegment<T>>;

  std::vector<SegmentPtr> segments;
  TimelineOptions         options;
  f32                     current_time   = 0.0f;
  bool                    is_playing     = false;
  f32                     total_duration = 0.0f;
  f32                     time_direction = 1.0f;

  Timeline(const TimelineOptions &opts = TimelineOptions{}) : options(opts)
  {
    if (options.autoplay)
    {
      is_playing = true;
    }
  }

  Timeline &reserve(u32 size) {
    segments.reserve(size);
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
                    size_t                           count,
                    const animation::StaggerOptions &stagger_opts =
                        animation::StaggerOptions{})
  {
    f32 base_delay = 0.0f;
    f32 start_time = total_duration;

    for (size_t i = 0; i < count; ++i)
    {
      base_delay = animation::stagger_delay(i, count, stagger_opts);
    }

    f32 delay = stagger_opts.start + (base_delay * stagger_opts.from);

    auto segment_config  = config;
    segment_config.delay = delay;

    auto segment = std::make_shared<animation::timeline::BasicSegment<T>>(
        start, end, segment_config);
    segment->start_time = start_time;
    segment->type       = animation::timeline::SegmentType::Staggered;
    segments.push_back(segment);

    return *this;
  }

  void update(f32 delta)
  {
    if (!is_playing)
      return;

    f32 adjusted_delta = delta * time_direction;
    current_time += adjusted_delta;

    if (time_direction > 0 && current_time >= total_duration)
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
    else if (time_direction < 0 && current_time <= 0)
    {
      if (options.loop)
      {
        current_time = total_duration + std::fmod(current_time, total_duration);
        for (auto &segment : segments)
        {
          segment->reset();
        }
      }
      else
      {
        current_time = 0;
        is_playing   = false;
      }
    }

    update_segments(delta);
  }

  void play()
  {
    is_playing = true;
  }

  void play_from_start()
  {
    for (auto &segment : segments)
    {
      segment->reset();
    }
    current_time   = 0;
    time_direction = 1.0f;
    is_playing     = true;
  }

  void play_from_end()
  {
    for (auto &segment : segments)
    {
      segment->reset();
    }
    current_time   = total_duration;
    time_direction = -1.0f;
    is_playing     = true;
  }

  void pause()
  {
    is_playing = false;
  }

  void stop()
  {
    is_playing     = false;
    current_time   = 0;
    time_direction = 1.0f;
    for (auto &segment : segments)
    {
      segment->reset();
    }
  }

  void seek(f32 time)
  {
    f32 target_time = std::clamp(time, 0.0f, total_duration);
    f32 delta       = target_time - current_time;

    // Reset all segments if seeking backwards
    if (delta < 0)
    {
      for (auto &segment : segments)
      {
        segment->reset();
      }
      current_time = 0;
      update_segments(target_time);
    }
    else
    {
      update_segments(delta);
    }

    current_time = target_time;
  }

  // Seek to percentage of duration
  void seek_percentage(f32 percentage)
  {
    percentage = std::clamp(percentage, 0.0f, 1.0f);
    seek(total_duration * percentage);
  }

  void reverse()
  {
    time_direction *= -1.0f;
  }

  // Force a direction: `1` is forward or `-1` is backward
  void set_direction(i32 direction)
  {
    if (direction == -1)
    {
      time_direction = -1.0f;
      return;
    }
    if (direction >= 1)
    {
      time_direction = 1.0f;
      return;
    }
  }

  f32 get_current_time() const
  {
    return current_time;
  }
  f32 get_duration() const
  {
    return total_duration;
  }
  f32 get_progress() const
  {
    return current_time / total_duration;
  }
  bool is_reversed() const
  {
    return time_direction < 0;
  }
  bool is_finished() const
  {
    return !options.loop &&
           ((time_direction > 0 && current_time >= total_duration) ||
            (time_direction < 0 && current_time <= 0));
  }

  void set_callback(std::function<void(const T &)> callback)
  {
    for (auto &segment : segments)
    {
      segment->on_update = callback;
    }
  }

private:
  void update_segments(f32 delta)
  {
    for (auto &segment : segments)
    {
      if (time_direction > 0)
      {
        // Forward playback
        if (current_time >= segment->start_time &&
            current_time <= segment->start_time + segment->duration)
        {
          segment->update(current_time, std::abs(delta));
        }
      }
      else
      {
        // Reverse playback
        f32 segment_end = segment->startTime + segment->duration;
        if (current_time <= segment_end && current_time >= segment->start_time)
        {
          segment->update(current_time, std::abs(delta));
        }
      }
    }
  }
};

}        // namespace ash
