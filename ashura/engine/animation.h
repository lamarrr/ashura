/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"
#include <concepts>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ash
{

/// [ ] https://create.roblox.com/docs/ui/animation#style
/// [ ] Procedural Animation https://www.youtube.com/watch?v=qlfh_rv6khY

// [x] Keyframes, for time interval x, move from a to b
// [ ] keyframe blending
// [x] play
// [x] reverse
// [x] cancel
// [ ] frame-rate customization
// [x] move back from point
// [x] loop n or forever
// [ ] stop after timepoint
// [ ] alternate
// [x] pause
// [x] resume in direction
// [x] reset animation state to beginning

// TODO: Support ash Span type instead of using std::string

// Animatable trait applicable to complex types like Color, Vec2, Vec3 etc
template <typename T>
concept Animatable = requires(T a, T b, T t) {
  { lerp(a, b, t) } -> std::convertible_to<T>;
};

template <Animatable T>
struct Animation;
template <Animatable T>
struct KeyFrame;
struct Timeline;
template <Animatable T>
struct Easing;

using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration  = std::chrono::duration<float>;

static inline Duration get_delta(const TimePoint &current,
                                 const TimePoint &previous)
{
  return std::chrono::duration_cast<Duration>(current - previous);
}

enum class CurveType
{
  Linear    = 0,
  EaseIn    = 1,
  EaseOut   = 2,
  EaseInOut = 3,
  Custom    = 4
};

template <Animatable T>
struct AnimationConfig
{
  Duration         duration{std::chrono::seconds(1)};
  Duration         delay{std::chrono::nanoseconds(0)};
  bool             loop   = false;
  Easing<T>        easing = Easing<T>{};
  Span<const char> label  = R"(default)"_span;
};

struct AnimationState
{
  bool     is_playing = false;
  Duration current_time{std::chrono::nanoseconds(0)};
  Duration delay{std::chrono::nanoseconds(0)};
};

struct TimelineOptions
{
  Duration    delay{std::chrono::nanoseconds(0)};
  bool        autoplay = true;
  bool        loop     = false;
  std::string label    = "default";
};

enum class GridDirection
{
  RowMajor    = 0,        // Left to right, top to bottom
  ColumnMajor = 1         // Top to bottom, left to right
};

struct StaggerPattern
{
  virtual ~StaggerPattern() = default;
  Duration start{std::chrono::nanoseconds(0)};
  Duration from{std::chrono::nanoseconds(0)};
  struct Grid
  {
    u32           rows      = 1;
    u32           cols      = 1;
    GridDirection direction = GridDirection::RowMajor;
  } grid;
  std::pair<u32, u32> grid_position(u32 index) const
  {
    if (this->grid.direction == GridDirection::RowMajor)
    {
      return {
          index / this->grid.cols,        // row
          index % this->grid.cols         // col
      };
    }

    return {
        index % this->grid.rows,        // row
        index / this->grid.rows         // col
    };
  }
  virtual Duration delay(u32 index, u32 total_count) const = 0;
};

struct StaggerPatternDefault : StaggerPattern
{
  Duration delay(u32 index, u32 total_count) const override
  {
    bool is_grid_based = this->grid.rows > 1 || this->grid.cols > 1;

    if (is_grid_based)
    {
      auto [row, col] = this->grid_position(index);
      // Default grid-based delay calculation
      f32 normalized_row = static_cast<f32>(row) / (this->grid.rows - 1);
      f32 normalized_col = static_cast<f32>(col) / (this->grid.cols - 1);

      switch (this->grid.direction)
      {
        case GridDirection::RowMajor:
        {
          f32 factor = normalized_row + (normalized_col * 0.5f);
          return std::chrono::duration_cast<Duration>(this->from * factor);
        }
        case GridDirection::ColumnMajor:
        {
          f32 factor = normalized_col + (normalized_row * 0.5f);
          return std::chrono::duration_cast<Duration>(this->from * factor);
        }
      }
    }

    auto factor = static_cast<f32>(index) / total_count;
    return std::chrono::duration_cast<Duration>(this->from * factor);
  }
};

struct IAnimation
{
  virtual ~IAnimation()           = default;
  virtual void tick()             = 0;
  virtual void play()             = 0;
  virtual void pause()            = 0;
  virtual void resume()           = 0;
  virtual void reset()            = 0;
  virtual bool is_playing() const = 0;
};

// Simple linear animation: perform basic interpolation on opacity, color,
// rotation etc.
// - play
// - pause
// - reset
// - set a callback function to track animation update
// ----
//
// ## Translate Position
// ```
// auto position = std::make_shared<Vec2>();
// Animation anim = Animation<f32>(
//  0.0f, // start
//  10.0f, // end
//  AnimationConfig{.duration = 2000ms, .loop = true, .easing =
//  CurveType::EaseInOut},
//);
// anim.play();
//
// // tick
// position->x = anim.tick(); // update only x axis
//
// ```
//

template <Animatable T>
struct Animation : IAnimation
{
private:
  bool is_reset;

public:
  T                  start;
  T                  end;
  AnimationConfig<T> config;
  AnimationState     state;
  TimePoint          last_update_time;
  T                  current_value;

  Animation(T start, T end, AnimationConfig<T> cfg) :
      start(start), end(end), config(cfg)
  {
    state.delay      = config.delay;
    is_reset         = false;
    last_update_time = Clock::now();
  }

  void tick() override
  {
    if (!state.is_playing)
      return;

    auto     current_time = Clock::now();
    Duration delta        = get_delta(current_time, last_update_time);
    last_update_time      = current_time;

    run(delta);
  }

  void play() override
  {
    state.is_playing = true;
    last_update_time = Clock::now();
  }

  void pause() override
  {
    state.is_playing = false;
  }

  void resume() override
  {
    if (!state.is_playing)
    {
      last_update_time = Clock::now();
      state.is_playing = true;
    }
  }

  void reset() override
  {
    is_reset = true;
  }

  bool is_playing() const override
  {
    return state.is_playing;
  }

  inline T value()
  {
    return current_value;
  }

  void run(Duration delta);
  void update(Duration _time, Duration delta);
};

template <Animatable T>
struct KeyFrame
{
  Duration    duration;
  std::string key;
  Easing<T>   easing;
  T           value;

  KeyFrame(Duration dur, std::string k, T val,
           const Easing<T> easing = Easing<T>{}) :
      duration(dur),
      key(std::move(k)),
      value(std::move(val)),
      easing(std::move(easing))
  {
  }
};

enum class SegmentType
{
  Basic,            // Single Animation
  KeyFramed,        // Multiple KeyFrames
  Parallel,         // Multiple parallel animations
  Staggered         // Staggered animations
};

struct TimelineSegment
{
  Span<const char> label = R"(default)"_span;
  Duration         start_time{std::chrono::nanoseconds(0)};
  Duration         duration{std::chrono::nanoseconds(0)};
  SegmentType      type = SegmentType::Basic;

  virtual ~TimelineSegment()                                     = default;
  virtual void             update(Duration time, Duration delta) = 0;
  virtual void             reset()                               = 0;
  virtual Span<const char> get_label()
  {
    return label;
  }
};

template <Animatable T>
struct BasicSegment : TimelineSegment
{
  Animation<T> animation;
  T            interpolated_value;

  BasicSegment(const T &start, const T &end, AnimationConfig<T> config) :
      animation{start, end, config}
  {
    this->duration = config.duration;
    this->type     = SegmentType::Basic;
  }

  T value() const
  {
    return this->interpolated_value;
  }

  void update(Duration current_time, Duration delta) override
  {
    animation.update(current_time, delta);
    this->interpolated_value = animation.current_value;
  }

  void reset() override
  {
    this->animation.reset();
  }
};

template <Animatable T>
struct KeyFrameSegment : TimelineSegment
{
  std::vector<KeyFrame<T>> keyframes;
  Duration                 current_time{std::chrono::seconds(1)};
  T                        interpolated_value;

  KeyFrameSegment(const std::vector<KeyFrame<T>> &kf) : keyframes(std::move(kf))
  {
    this->type = SegmentType::KeyFramed;
    calculate_duration();
  }

  void calculate_duration();

  void update(Duration _time, Duration delta) override;

  void reset() override
  {
    current_time = Duration::zero();
  }

  T value() const
  {
    return this->interpolated_value;
  }
};

// Timeline: A more robust way to manage keyframe or multiple animations
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
// std::vector<KeyFrame<Vec2>> keyframes = {
//     KeyFrame<Vec2>(500ms, "start", {0, 100.0f}, CurveType::EaseInOut),
//     KeyFrame<Vec2>(500ms, "middle", {150.0f, 50.0f}, CurveType::EaseInOut),
//     KeyFrame<Vec2>(500ms, "end", {0, 200.0f}, CurveType::EaseInOut)
// };
//
// timeline.add(keyframes);
// // Basic controls
// timeline.play();
// timeline.pause();
// timeline.reverse();
// timeline.seek(2000ms);
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
//     .add(keyframes)                           // KeyFrame sequence
//     .parallel(configs, values)                // Parallel animations
//     .stagger(start, end, config, count);      // Staggered animations
// ```

struct Timeline : IAnimation
{
  using SegmentPtr = std::shared_ptr<TimelineSegment>;

  std::vector<SegmentPtr> segments;
  TimelineOptions         options;
  Duration                current_time{Duration::zero()};
  Duration                total_duration{Duration::zero()};
  bool                    is_playing_    = false;
  f32                     time_direction = 1.0f;
  TimePoint               last_update_time;

  Timeline(const TimelineOptions &opts = TimelineOptions{}) : options(opts)
  {
    last_update_time = Clock::now();
    if (options.autoplay)
    {
      is_playing_ = true;
    }
  }

  ~Timeline()
  {
    clear();
  }

  Timeline &reserve(u32 size);

  void clear();

  template <Animatable T>
  Timeline &add(const T &start, const T &end, AnimationConfig<T> config);

  template <Animatable T>
  Timeline &add(const std::vector<KeyFrame<T>> &keyframes,
                const Span<const char>          label = R"(default)"_span);
  template <Animatable T>
  Timeline &parallel(const std::vector<AnimationConfig<T>>  &configs,
                     const std::vector<std::pair<f32, f32>> &values);
  template <Animatable T>
  Timeline &
      stagger(const f32 &start, const f32 &end, AnimationConfig<T> config,
              u32                   count,
              const StaggerPattern &stagger_opts = StaggerPatternDefault{});

  void tick() override;

  void play() override;

  void play_from_start();

  void play_from_end();

  void pause() override;
  void resume() override;

  void stop();
  void reset() override;

  bool is_playing() const override;

  void seek(Duration time);

  // Seek to percentage of duration
  void seek_percentage(f32 percentage)
  {
    percentage  = std::clamp(percentage, 0.0f, 1.0f);
    auto target = Duration{
        static_cast<Duration::rep>(total_duration.count() * percentage)};
    seek(target);
  }

  void reverse()
  {
    time_direction *= -1.0f;
  }
  // Force a direction: `1` is forward or `-1` is backward
  void set_direction(i32 direction)
  {
    time_direction = direction < 0 ? -1.0f : 1.0f;
  }

  Duration get_current_time() const
  {
    return current_time;
  };
  Duration get_total_duration() const
  {
    return total_duration;
  };
  f32 get_progress() const
  {
    if (total_duration.count() == 0)
      return 0.0f;
    return static_cast<float>(current_time.count()) /
           static_cast<float>(total_duration.count());
  }
  bool is_reversed() const
  {
    return time_direction < 0;
  }
  bool is_finished() const
  {
    return !options.loop &&
           ((time_direction > 0 && current_time >= total_duration) ||
            (time_direction < 0 && current_time <= Duration::zero()));
  };

  std::optional<SegmentPtr> get_segment(Span<const char> label);
  SegmentPtr                get_segment(u32 index)
  {
    return segments[index];
  }

private:
  void update_segments(Duration delta);
};

namespace anim
{

// Lerp implementations for different types
// Basic arithmetic types
template <typename T>
T lerp(const T &start, const T &end, T t)
  requires std::is_arithmetic_v<T>
{
  return (1 - t) * start + t * end;
}

// Vec2 lerp implementation
inline Vec2 lerp(const Vec2 &start, const Vec2 &end, f32 t)
{
  return start + (end - start) * Vec2::splat(t);
}

// Vec3 lerp implementation
inline Vec3 lerp(const Vec3 &start, const Vec3 &end, f32 t)
{
  return start + (end - start) * Vec3::splat(t);
}

// Vec4 lerp implementation
inline Vec4 lerp(const Vec4 &start, const Vec4 &end, f32 t)
{
  return start + (end - start) * Vec4::splat(t);
}

// Vec4U8 lerp implementation, useful for Colors
inline Vec4U8 lerp(const Vec4U8 &start, const Vec4U8 &end, f32 t)
{
  // Convert to float, interpolate, then convert back to u8
  Vec4 start_f = Vec4{static_cast<f32>(start.x), static_cast<f32>(start.y),
                      static_cast<f32>(start.z), static_cast<f32>(start.w)} /
                 255.0f;

  Vec4 end_f = Vec4{static_cast<f32>(end.x), static_cast<f32>(end.y),
                    static_cast<f32>(end.z), static_cast<f32>(end.w)} /
               255.0f;

  Vec4 result = start_f + (end_f - start_f) * Vec4::splat(t);

  return Vec4U8{
      static_cast<u8>(result.x * 255.0f), static_cast<u8>(result.y * 255.0f),
      static_cast<u8>(result.z * 255.0f), static_cast<u8>(result.w * 255.0f)};
}

template <Animatable T>
T linear_ease(const T &start, const T &end, f32 *params)
{
  f32 t = params[0];
  return lerp(start, end, t);
}

template <Animatable T>
T ease_in_quad_t(const T &start, const T &end, f32 *params)
{
  f32 t = params[0];
  return lerp(start, end, ash::ease_in(t));
}

template <Animatable T>
T ease_out_quad_t(const T &start, const T &end, f32 *params)
{
  f32 t = params[0];
  return lerp(start, end, ash::ease_out(t));
}

template <Animatable T>
T ease_in_out_quad_t(const T &start, const T &end, f32 *params)
{
  f32 t = params[0];
  return lerp(start, end, ash::ease_in_out(t));
}

template <Animatable T>
constexpr Easing<T> linear()
{
  return Easing{.params = {}, .easing = linear_ease<T>};
}

template <Animatable T>
constexpr Easing<T> ease_in_quad()
{
  return Easing{.params = {}, .easing = ease_in_quad_t<T>};
}

template <Animatable T>
constexpr Easing<T> ease_out_quad()
{
  return Easing{.params = {}, .easing = ease_out_quad_t<T>};
}

template <Animatable T>
constexpr Easing<T> ease_in_out_quad()
{
  return Easing{.params = {}, .easing = ease_in_out_quad_t<T>};
}

// Based on Robert Penner's elastic easing:
// http://robertpenner.com/easing/
// - amplitude: strength of the elastic effect (default 1.0)
// - period: length of the oscillation (default 0.3)
template <Animatable T>
constexpr Easing<T> elastic(f32 amplitude = 1.0f, f32 period = 0.3f)
{
  return Easing<T>{.params = {0.0f, amplitude, period},
                   .easing = [](const T &start, const T &end, f32 *params) {
                     f32 t         = params[0];
                     f32 amplitude = params[1];
                     f32 period    = params[2];

                     const f32 TWO_PI = 2.0f * M_PI;

                     if (t <= 0)
                       return 0.0f;
                     if (t >= 1)
                       return 1.0f;

                     f32 s = period / TWO_PI * std::asin(1 / amplitude);

                     f32 factor = amplitude * std::pow(2.0f, -10.0f * t) *
                                      std::sin((t - s) * TWO_PI / period) +
                                  1.0f;

                     return lerp(start, end, factor);
                   }};
}

// Based on Robert Penner's easeOutBounce:
// http://robertpenner.com/easing/
// - strength: strength of the bounce effect (default 1.0)
template <Animatable T>
constexpr Easing<T> bounce(f32 strength = 1.0)
{
  return Easing<T>{.params = {0.0f, strength},
                   .easing = [](const T &start, const T &end, f32 *params) {
                     f32 t = params[0];
                     f32 strength =
                         params[1];        // Bounce strength (default: 1.0)

                     // Inverse the time to create an ease-out effect
                     t = 1.0f - t;
                     float factor;

                     if (t < (1.0f / 2.75f))
                     {
                       factor = 1.0f - (7.5625f * t * t * strength);
                     }
                     else if (t < (2.0f / 2.75f))
                     {
                       t -= 1.5f / 2.75f;
                       factor = 1.0f - (7.5625f * t * t * strength + 0.75f);
                     }
                     else if (t < (2.5f / 2.75f))
                     {
                       t -= 2.25f / 2.75f;
                       factor = 1.0f - (7.5625f * t * t * strength + 0.9375f);
                     }
                     else
                     {
                       t -= 2.625f / 2.75f;
                       factor = 1.0f - (7.5625f * t * t * strength + 0.984375f);
                     }
                     return lerp(start, end, factor);
                   }};
}

// Spring-based elastic easing
// Based on simple harmonic motion with damping
// Reference: https://www.ryanjuckett.com/damped-springs/
// - mass: Oscillator mass (default: 1.0)
// - stiffness: Spring constant (default: 20.0)
// - damping: Damping coefficient (default: 10.0f)
//
// The default behavior is a tight spring effect, tune the parameters to give
// a desired effect.
template <Animatable T>
constexpr Easing<T> spring(f32 mass = 1.0f, f32 stiffness = 20.0f,
                           f32 damping = 10.0f)
{
  return Easing<T>{
      .params = {0.0f, mass, stiffness, damping},
      .easing = [](const T &start, const T &end, f32 *params) {
        f32 t         = params[0];        // current time
        f32 mass      = params[1];        // Oscillator mass
        f32 stiffness = params[2];        // Spring constant
        f32 damping   = params[3];        // Damping coefficient

        // Calculate critical damping factors
        f32 omega0           = std::sqrt(stiffness / mass);
        f32 critical_damping = 2.0f * std::sqrt(mass * stiffness);
        f32 zeta             = damping / critical_damping;

        if (zeta < 1.0f)
        {        // Underdamped
          f32 omega_d = omega0 * std::sqrt(1.0f - zeta * zeta);
          return 1.0f - std::exp(-zeta * omega0 * t) *
                            (std::cos(omega_d * t) +
                             (zeta * omega0 / omega_d) * std::sin(omega_d * t));
        }
        // Overdamped or critically damped
        f32  alpha = -zeta * omega0;
        f32  beta  = omega0 * std::sqrt(zeta * zeta - 1.0f);
        auto factor =
            1.0f -
            (std::exp(alpha * t) *
             (std::cosh(beta * t) + (alpha / beta) * std::sinh(beta * t)));

        return lerp(start, end, factor);
      }};
}

}        // namespace anim

template <Animatable T>
struct Easing
{
  typedef T (*Function)(const T &start, const T &end, f32 *params);
  f32      params[4] = {};
  Function easing    = nullptr;

  T evaluate(const T &start, const T &end, f32 t) const
  {
    if (easing)
    {
      f32 temp_params[4] = {t};
      for (int i = 1; i < 4; ++i)
      {
        temp_params[i] = params[i];
      }
      return easing(start, end, temp_params);
    }

    // Default to linear interpolation
    if constexpr (std::is_same_v<T, Vec2> || std::is_same_v<T, Vec3> ||
                  std::is_same_v<T, Vec4> || std::is_same_v<T, Vec4U8>)
    {
      return anim::lerp(start, end, t);        // Uses the splat version
    }
    else
    {
      return anim::lerp(start, end, T(t));        // Convert t to type T
    }
  }
};

// Animation utility to create and manage one or more animations or timelines.
// - play animations
// - pause animations
// - reset animations
// - staggered animations
// - set a callback function to track animation updates
// ```
// AnimationManager animator;
//
// auto anim1 = animator.create<Vec2>(
//     start_pos,
//     end_pos,
//     AnimationConfig{.duration = 1000ms, .loop = true, .easing =
//     CurveType::EaseInOut},
// );
//
// auto anim2 = animator.create<Vec2>(
//     start_pos,
//     end_pos,
//     AnimationConfig{.duration = 1000ms, .loop = true, .easing =
//     CurveType::EaseInOut},
// );
//
// // Play individual animation
// animator.get_animation(0).play();
// animator.get_animation(1).play();
// // Or play all the animations
// animator.play_all();
//
// // Get value from animation
// anim1_value = animator.get_animation(0).value();
// anim2_value = animator.get_animation(1).value();
// ```
//
//
// ### Staggered (Grid) animations
//--------
// ### Example: Ripple Pattern Animation
//```
// struct RipplePattern:StaggerPattern {
//  Duration delay(u32 index, u32 total_count) const override {
//    f32 center_row = 1.5f;
//    f32 center_col = 1.5f;
//    f32 distance = std::sqrt(
//         std::pow(row - center_row, 2) +
//         std::pow(col - center_col, 2)
//     );
//    return Duration{std::chrono::seconds(distance * 0.15f)};
//  }
//}
// animator.stagger<f32>(
//  0.0f, // start
//  1.0f, // end
//  AnimationConfig{.duration = 1000ms, .loop = true},
//  10, // total count
//  RipplePattern{.grid = {.rows = 4, .cols = 4}}
//);
//
// // Get the staggered animations
// auto anim1 = animator.get_animation(0); // get animation at index 0
// auto anim1_value = anim1.value(); // get the value at anim1
// auto anim2 = animator.get_animation(1); // get animation at index 1
// auto anim2_value = anim2.value(); // get the value at anim2
//```
struct AnimationManager
{
private:
  static constexpr usize    INITIAL_POOL_SIZE = 32;
  std::vector<IAnimation *> animations;
  u8                       *memory_pool = nullptr;
  usize                     pool_size   = 0;
  usize                     pool_offset = 0;

  void grow_pool()
  {
    usize new_size = pool_size * 2;
    u8   *new_pool = new u8[new_size];

    if (memory_pool)
    {
      std::memcpy(new_pool, memory_pool, pool_size);
      delete[] memory_pool;
    }

    memory_pool = new_pool;
    pool_size   = new_size;
  }

public:
  AnimationManager()
  {
    // Size for largest possible animation type?
    pool_size =
        INITIAL_POOL_SIZE * std::max(sizeof(Timeline), sizeof(Animation<Vec4>));
    memory_pool = new u8[pool_size];
    pool_offset = 0;
  }

  ~AnimationManager()
  {
    clear();
    delete[] memory_pool;
  }

  // Non-copyable
  AnimationManager(const AnimationManager &)            = delete;
  AnimationManager &operator=(const AnimationManager &) = delete;

  // Movable
  AnimationManager(AnimationManager &&other) noexcept;
  AnimationManager &operator=(AnimationManager &&other) noexcept;

  void reserve(u32 size);

  void clear();

  template <Animatable T>
  Animation<T> *create(T start, T end, AnimationConfig<T> config)
  {
    constexpr usize size = sizeof(Animation<T>);

    if (pool_offset + size > pool_size)
    {
      grow_pool();
    }

    void *memory = memory_pool + pool_offset;
    pool_offset += size;

    Animation<T> *anim = new (memory) Animation<T>(start, end, config);
    animations.push_back(anim);

    return anim;
  }

  Timeline *create(const TimelineOptions &options = TimelineOptions{})
  {
    constexpr usize size = sizeof(Timeline);

    if (pool_offset + size > pool_size)
    {
      grow_pool();
    }

    void *memory = memory_pool + pool_offset;
    pool_offset += size;

    Timeline *timeline = new (memory) Timeline(options);
    animations.push_back(timeline);

    return timeline;
  }

  void tick();

  void play_all();

  void pause_all();

  void reset_all();

  template <Animatable T>
  void stagger(const f32 &start, const f32 &end,
               const AnimationConfig<T> &base_config, usize count,
               const StaggerPattern &stagger_pattern = StaggerPatternDefault{})
  {
    for (u32 i = 0; i < count; ++i)
    {
      Duration base_delay = stagger_pattern.delay(i, count);

      auto config  = base_config;
      config.delay = stagger_pattern.start + base_delay;

      auto anim = create(start, end, config);
      anim->play();
    }
  }

  template <Animatable T>
  Animation<T> *get_animation(usize index)
  {
    if (index >= animations.size())
      return nullptr;
    return &animations[index];
  }
};

template struct Animation<f32>;
template struct Animation<Vec2>;
template struct Animation<Vec3>;
template struct Animation<Vec4>;
template struct Animation<Vec4U8>;
}        // namespace ash
