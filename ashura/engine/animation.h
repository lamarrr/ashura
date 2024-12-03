/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/dyn.h"
#include "ashura/std/error.h"
#include "ashura/std/math.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

namespace anim
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
// [x] restart animation state to beginning

/// @brief An object used to tween/interpolate between two values.
/// This is a separate object to allow users customize the definition of the
/// values depending on the context. And it isn't bundled with the interpolated
/// objects to allow for more efficient storage of the values.
struct Tweener
{
  constexpr Vec2 operator()(Vec2 const &low, Vec2 const &high, f32 t) const
  {
    return lerp(low, high, Vec2::splat(t));
  }

  constexpr Vec3 operator()(Vec3 const &low, Vec3 const &high, f32 t) const
  {
    return lerp(low, high, Vec3::splat(t));
  }

  constexpr Vec4 operator()(Vec4 const &low, Vec4 const &high, f32 t) const
  {
    return lerp(low, high, Vec4::splat(t));
  }

  constexpr Vec4U8 operator()(Vec4U8 const &low, Vec4U8 const &high,
                              f32 t) const
  {
    return as_vec4u8(lerp(as_vec4(low), as_vec4(high), Vec4::splat(t)));
  }
};

constexpr u32 NUM_EASE_PARAMETERS = 4;

/// @brief Easing function, the parameters are stored in an array
/// @param t the linear interpolator to be eased. guaranteed to be between 0
/// and 1.
typedef Fn<f32(Span<f32 const> params, f32 t)> EaseFn;

namespace easing
{

constexpr f32 linear(Span<f32 const>, f32 t)
{
  return t;
}

constexpr f32 in(Span<f32 const>, f32 t)
{
  return ease_in(t);
}

constexpr f32 out(Span<f32 const>, f32 t)
{
  return ease_out(t);
}

constexpr f32 in_out(Span<f32 const>, f32 t)
{
  return ease_in_out(t);
}

constexpr f32 bezier(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 3);

  return ash::bezier(params[0], params[1], params[2], t);
}

constexpr f32 cubic_bezier(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 4);

  return ash::cubic_bezier(params[0], params[1], params[2], params[3], t);
}

constexpr f32 catmull_rom(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 4);

  return ash::catmull_rom(params[0], params[1], params[2], params[3], t);
}

/// @brief Elastic Easing
/// @param amplitude strength of the elastic effect (default = 1.0)
/// @param period length of the oscillation (default = 0.3)
/// @note Based on Robert Penner's elastic easing
/// (http://robertpenner.com/easing/)
constexpr f32 elastic(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 2);

  constexpr f32 TWO_PI = 2.0F * PI;

  f32 const amplitude = params[0];
  f32 const period    = params[1];
  f32 const s         = (period / TWO_PI) * std::asin(1 / amplitude);
  f32 const factor    = amplitude * std::pow(2.0F, -10.0F * t) *
                         std::sin((t - s) * (TWO_PI / period)) +
                     1.0F;
  return factor;
}

/// @brief EaseOut Bounce
/// @param strength strength of the bounce effect (default = 1.0)
/// @note Based on Robert Penner's easeOutBounce
/// (http://robertpenner.com/easing/)
constexpr f32 bounce(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 1);

  f32 const strength = params[0];

  // Inverse the time to create an ease-out effect
  t = 1.0F - t;

  if (t < (1.0F / 2.75F))
  {
    return 1.0F - (7.5625F * t * t * strength);
  }
  else if (t < (2.0F / 2.75F))
  {
    t -= 1.5F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.75F);
  }
  else if (t < (2.5F / 2.75F))
  {
    t -= 2.25F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.9375F);
  }
  else
  {
    t -= 2.625F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.984375F);
  }
}

/// @brief Spring-based Elastic Easing based on simple harmonic motion with
/// damping
///
/// The default behavior is a tight spring effect, tune the parameters to give
/// a desired effect.
/// @param mass: Oscillator mass (default: 1.0)
/// @param stiffness: Spring constant (default: 20.0)
/// @param damping: Damping coefficient (default: 10.0F)
///
/// @note https://www.ryanjuckett.com/damped-springs/
///
constexpr f32 spring(Span<f32 const> params, f32 t)
{
  static_assert(NUM_EASE_PARAMETERS >= 3);

  f32 const mass      = params[0];
  f32 const stiffness = params[1];
  f32 const damping   = params[2];

  // Calculate critical damping factors
  f32 const omega0           = std::sqrt(stiffness / mass);
  f32 const critical_damping = 2.0F * std::sqrt(mass * stiffness);
  f32 const damping_ratio    = damping / critical_damping;

  // Underdamped
  if (damping_ratio < 1.0F)
  {
    f32 const omega_d =
        omega0 * std::sqrt(1.0F - damping_ratio * damping_ratio);
    return 1.0F -
           std::exp(-damping_ratio * omega0 * t) *
               (std::cos(omega_d * t) +
                (damping_ratio * omega0 / omega_d) * std::sin(omega_d * t));
  }

  // Overdamped or critically damped
  f32 const alpha = -damping_ratio * omega0;
  f32 const beta  = omega0 * std::sqrt(damping_ratio * damping_ratio - 1.0F);
  return 1.0F - (std::exp(alpha * t) *
                 (std::cosh(beta * t) + (alpha / beta) * std::sinh(beta * t)));
}

}        // namespace easing

struct Easing
{
  EaseFn ease                        = fn(easing::linear);
  f32    params[NUM_EASE_PARAMETERS] = {};

  constexpr f32 operator()(f32 t) const
  {
    return ease(span(params), t);
  }

  static constexpr Easing linear()
  {
    return Easing{.ease = fn(easing::linear)};
  }

  static constexpr Easing in()
  {
    return Easing{.ease = fn(easing::in)};
  }

  static constexpr Easing out()
  {
    return Easing{.ease = fn(easing::out)};
  }

  static constexpr Easing in_out()
  {
    return Easing{.ease = fn(easing::in_out)};
  }

  static constexpr Easing bezier(f32 p0, f32 p1, f32 p2)
  {
    return Easing{.ease = fn(easing::bezier), .params = {p0, p1, p2}};
  }

  static constexpr Easing cubic_bezier(f32 p0, f32 p1, f32 p2, f32 p3)
  {
    return Easing{.ease = fn(easing::cubic_bezier), .params = {p0, p1, p2, p3}};
  }

  static constexpr Easing catmull_rom(f32 p0, f32 p1, f32 p2, f32 p3)
  {
    return Easing{.ease = fn(easing::catmull_rom), .params = {p0, p1, p2, p3}};
  }

  static constexpr Easing elastic(f32 amplitude, f32 period)
  {
    return Easing{.ease = fn(easing::bounce), .params = {amplitude, period}};
  }

  static constexpr Easing bounce(f32 strength)
  {
    return Easing{.ease = fn(easing::elastic), .params = {strength}};
  }

  static constexpr Easing spring(f32 mass, f32 stiffness, f32 damping)
  {
    return Easing{.ease   = fn(easing::elastic),
                  .params = {mass, stiffness, damping}};
  }
};

struct AnimationConfig
{
  Span<const char> label    = {};
  nanoseconds      duration = 0ns;
  nanoseconds      delay    = 0ns;
  bool             loop     = false;
  Easing           easing   = {};
};

struct TimelineConfig
{
  Span<const char> label    = {};
  nanoseconds      delay    = 0ns;
  bool             autoplay = true;
  bool             loop     = false;
};

/// @brief Stagger delay of animation components
struct Stagger
{
  virtual ~Stagger() = default;

  /// @brief Perform stagger delay on a list of components
  /// @param item the index of the item
  /// @param count the total number of items to be staggered
  /// @return the stagger delay factor
  virtual f32 operator()(u32 item, u32 count) = 0;
};

/// @brief Grid-based delay calculation
/// @param rows number of rows the grid stagger represents
/// @param columns number of columns the grid stagger represents
/// @param t [0, 1], interpolator to determine which item gets delayed first
/// based on their column or row position
struct GridStagger : Stagger
{
  u32 rows;
  u32 columns;
  f32 t;

  constexpr GridStagger(u32 rows = 1, u32 columns = 1, f32 t = 0.5F) :
      rows{rows}, columns{columns}, t{t}
  {
  }

  constexpr GridStagger() = default;

  constexpr GridStagger(GridStagger const &) = default;

  constexpr GridStagger(GridStagger &&) = default;

  constexpr GridStagger &operator=(GridStagger const &) = default;

  constexpr GridStagger &operator=(GridStagger &&) = default;

  constexpr virtual ~GridStagger() override = default;

  constexpr u32 size() const
  {
    return rows * columns;
  }

  constexpr Tuple<u32, u32> pos(u32 index) const
  {
    return {index / rows, index % columns};
  }

  constexpr virtual f32 operator()(u32 item, u32) override
  {
    u32 const grid_size = size();

    f32 row_norm    = 1;
    f32 column_norm = 1;

    if (item < grid_size)
    {
      auto const [row, column] = pos(item);
      if (rows > 1)
      {
        row_norm = row / (f32) (rows - 1);
      }

      if (columns > 1)
      {
        column_norm = column / (f32) (columns - 1);
      }
    }

    return column_norm * t + (1 - t) * row_norm;
  }
};

struct RippleStagger : Stagger
{
  u32 width;
  u32 height;
  i32 direction;

  constexpr RippleStagger(u32 width = 1, u32 height = 1, i32 direction = 1) :
      width{width}, height{height}, direction{direction}
  {
  }

  constexpr RippleStagger() = default;

  constexpr RippleStagger(RippleStagger const &) = default;

  constexpr RippleStagger(RippleStagger &&) = default;

  constexpr RippleStagger &operator=(RippleStagger const &) = default;

  constexpr RippleStagger &operator=(RippleStagger &&) = default;

  constexpr virtual ~RippleStagger() override = default;

  constexpr Tuple<u32, u32> pos(u32 index) const
  {
    return {index / width, index % height};
  }

  constexpr virtual f32 operator()(u32 item, u32) override
  {
    u32 const size = width * height;

    f32 row_norm    = 1;
    f32 column_norm = 1;

    if (item < size)
    {
      auto const [row, column] = pos(item);
      if (rows > 1)
      {
        row_norm = row / (f32) (rows - 1);
      }

      if (columns > 1)
      {
        column_norm = column / (f32) (columns - 1);
      }
    }

    return column_norm * t + (1 - t) * row_norm;
  }
};

struct Animation
{
  virtual ~Animation() = default;

  virtual void tick(nanoseconds delta) = 0;

  virtual void play() = 0;

  virtual void pause() = 0;

  virtual void resume() = 0;

  virtual void restart() = 0;

  virtual bool is_playing() = 0;

  virtual f32 value() = 0;
};

/// @brief Tween Animation. Perform basic 2-point interpolation on properties.
/// i.e. opacity, color, rotation etc.
/// @brief Animation State
/// @param is_playing is the animation currently allowed to advance
/// @param start when the animation was started
/// @param delay currently applied delay on the animation once it reaches 0
/// while the animation is playing, it will proceed
/// @param time the amount of time that has passed so far (out of the
/// animation's duration)
///
/// ## Translate Position
///
/// ```
/// auto position = std::make_shared<Vec2>();
/// Animation anim = Animation<f32>(
///  0.0F, // start
///  10.0F, // end
///  AnimationConfig{.duration = 2000ms, .loop = true, .easing =
///  ash::anim::ease_in_out_quad<Vec2>()},
///);
/// anim.play();
///
/// // tick
/// position->x = anim.tick(); // update only x axis
///
/// ```
///
struct Tween : Animation
{
  AnimationConfig cfg;
  f32             t;
  bool            running;
  nanoseconds     delay;
  nanoseconds     time;

  constexpr Tween(AnimationConfig cfg) :
      cfg{cfg}, t{0}, running{false}, delay{cfg.delay}, time{0s}
  {
  }

  constexpr Tween(Tween const &) = default;

  constexpr Tween(Tween &&) = default;

  constexpr Tween &operator=(Tween const &) = default;

  constexpr Tween &operator=(Tween &&) = default;

  constexpr ~Tween() override = default;

  constexpr virtual void tick(nanoseconds delta) override
  {
    if (!running)
    {
      return;
    }

    if (delay >= delta)
    {
      delay -= delta;
      return;
    }
    else if (delay > 0s)
    {
      delta -= delay;
      delay = 0s;
    }

    time += delta;

    i64 const iterations = time.count() / cfg.duration.count();

    if (cfg.loop)
    {
      // using the modulo, get the remaining time after the finished iterations
      time = time - iterations * cfg.duration;
    }
    else
    {
      if (iterations >= 1)
      {
        time    = cfg.duration;
        running = false;
      }
    }

    t = cfg.easing((f32) time.count() / (f32) cfg.duration.count());
  }

  constexpr virtual void play() override
  {
    running = true;
  }

  constexpr virtual void pause() override
  {
    running = false;
  }

  constexpr virtual void resume() override
  {
    running = true;
  }

  constexpr virtual void restart() override
  {
    time  = 0s;
    delay = cfg.delay;
    t     = 0;
  }

  constexpr virtual bool is_playing() override
  {
    return running;
  }

  constexpr virtual f32 value() override
  {
    return t;
  }
};

struct KeyFrameInfo
{
  Span<char const> key      = {};
  nanoseconds      duration = 0s;
  Easing           easing   = Easing::linear();
};

struct SegmentInfo
{
  Span<const char> label    = {};
  nanoseconds      duration = 0s;
};

struct Segment
{
  virtual ~Segment() = default;

  virtual f32 value() = 0;

  virtual u32 count() = 0;

  virtual void tick(nanoseconds delta) = 0;

  virtual void restart() = 0;

  virtual SegmentInfo info() = 0;
};

struct TweenSegment : Segment
{
  Tween animation;

  constexpr TweenSegment(AnimationConfig cfg) : animation{cfg}
  {
  }

  constexpr TweenSegment(TweenSegment const &) = default;

  constexpr TweenSegment(TweenSegment &&) = default;

  constexpr TweenSegment &operator=(TweenSegment const &) = default;

  constexpr TweenSegment &operator=(TweenSegment &&) = default;

  constexpr ~TweenSegment() override = default;

  constexpr virtual f32 value() override
  {
    return animation.t;
  }

  virtual u32 count() override
  {
    return 2;
  }

  constexpr virtual void tick(nanoseconds delta) override
  {
    animation.tick(delta);
  }

  constexpr virtual void restart() override
  {
    animation.restart();
  }

  constexpr virtual SegmentInfo info() override
  {
    return SegmentInfo{.label    = animation.cfg.label,
                       .duration = animation.cfg.duration};
  }
};

// [ ] use run-end encoding for this
struct KeyFrameSegment : Segment
{
  Span<char const>  label = {};
  Vec<KeyFrameInfo> key_frames{};
  nanoseconds       time = 0ns;
  time_point        start_time{};
  f32               t = 0;

  KeyFrameSegment(Span<char const> label, Vec<KeyFrameInfo> key_frames) :
      label{label}, key_frames{std::move(key_frames)}
  {
  }

  KeyFrameSegment(KeyFrameSegment const &) = delete;

  KeyFrameSegment(KeyFrameSegment &&) = default;

  KeyFrameSegment &operator=(KeyFrameSegment const &) = delete;

  KeyFrameSegment &operator=(KeyFrameSegment &&) = default;

  ~KeyFrameSegment() = default;

  virtual u32 count() override
  {
    return key_frames.size32();
  }

  virtual void tick(nanoseconds delta) override
  {
    CHECK(key_frames.size() >= 2);

    time += delta;

    nanoseconds keyframes_time = 0ns;

    for (usize i = 0; i < key_frames.size() - 1; ++i)
    {
      KeyFrameInfo const &f0     = key_frames[i];
      KeyFrameInfo const &f1     = key_frames[i + 1];
      nanoseconds const   f0_end = keyframes_time + f0.duration;

      if (time <= f0_end)
      {
        f32 const segment_t =
            (time - keyframes_time).count() / (f32) f0.duration.count();
        t = f0.easing(segment_t);
        break;
      }

      keyframes_time = f0_end;
    }
  }

  virtual void restart() override
  {
    time = 0ns;
  }

  constexpr virtual SegmentInfo info() override
  {
    nanoseconds const duration =
        map_reduce(key_frames, nanoseconds{0},
                   [](KeyFrameInfo const &k) { return k.duration; });
    return SegmentInfo{.label = label, .duration = duration};
  }

  constexpr virtual f32 value() const
  {
    return t;
  }
};

// Timeline: A more robust way to manage keyframe or multiple
// animations
// -  Timeline animations or key_frames can be run in parallel or
// staggered.
// -  Set a callback to track each animation or keyframe update.
// -  Direction control
// -  Playback control
// ```
// TimelineConfig cfg{
//         .autoplay = true,
//         .loop = true
// };
// Timeline<Vec2> timeline(cfg);
//
// std::vector<KeyFrame<Vec2>> key_frames = {
//     KeyFrame<Vec2>(500ms, "start", {0, 100.0F},
//     ash::anim::ease_in_out_quad<Vec2>()), KeyFrame<Vec2>(500ms, "middle",
//     {150.0F, 50.0F}, ash::anim::ease_out_quad<Vec2>()), KeyFrame<Vec2>(500ms,
//     "end", {0, 200.0F}, ash::anim::ease_in_out_quad<Vec2>())
// };
//
// timeline.add(key_frames);
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
//     .add(start, end, cfg)                  // Simple animation
//     .add(key_frames)                           // KeyFrame sequence
//     .parallel(configs, values)                // Parallel animations
//     .stagger(start, end, cfg, count);      // Staggered animations
// ```
template <typename T, typename Tweener = ash::anim::Tweener>
struct Timeline : Animation
{
  AllocatorImpl       allocator;
  Tweener             tweener;
  Vec<Dyn<Segment *>> segments;
  Vec<nanoseconds>    segment_ends;
  Vec<T>              values;
  Vec<u32>            runs;
  TimelineConfig      cfg;
  nanoseconds         time           = 0ns;
  nanoseconds         duration       = 0ns;
  bool                playing        = false;
  i32                 time_direction = 1;

  constexpr Timeline(AllocatorImpl allocator, TimelineConfig cfg,
                     Tweener tweener = {}) :
      allocator{allocator},
      tweener{(Tweener &&) tweener},
      segments{allocator},
      segment_ends{allocator},
      values{allocator},
      runs{allocator},
      cfg{cfg}
  {
  }

  constexpr Timeline(Timeline const &) = delete;

  constexpr Timeline &operator=(Timeline const &) = delete;

  constexpr Timeline(Timeline &&) = default;

  constexpr Timeline &operator=(Timeline &&) = default;

  constexpr ~Timeline() = default;

  void clear()
  {
    // segments.clear();
  }

  Timeline &tween(AnimationConfig cfg, T start, T end)
  {
    Dyn segment = dyn_inplace<TweenSegment>(allocator, cfg).unwrap();
    duration += segment->animation.cfg.duration;
    segments.push(cast<Segment *>(std::move(segment)));
    segment_ends.push(duration).unwrap();
    values.push((T &&) start).unwrap();
    values.push((T &&) end).unwrap();
    runs.push(values.size32()).unwrap();
    return *this;
  }

  template <typename T>
  Timeline &key_frames(Span<char const> label, Span<KeyFrameInfo const> infos,
                       Span<T const> frames)
  {
    CHECK(infos.size() == frames.size());
    CHECK(infos.size() >= 2);
    Dyn segment =
        dyn_inplace<KeyFrameSegment>(
            allocator, label, vec<KeyFrameInfo>(allocator, data).unwrap())
            .unwrap();
    duration += segment->info().duration;
    segments.push_back(cast<Segment *>(std::move(segment)));
    segment_ends.push(duration).unwrap();
    values.extend_copy(frames).unwrap();
    runs.push(values.size32()).unwrap();
    return *this;
  }

  template <typename T>
  Timeline &parallel(const std::vector<AnimationConfig>     &configs,
                     const std::vector<std::pair<f32, f32>> &values)
  {
    nanoseconds max_duration = 0s;
    nanoseconds start_time   = total_duration;

    for (usize i = 0; i < configs.size(); ++i)
    {
      auto segment = std::make_shared<SingleSegment>(
          values[i].first, values[i].second, configs[i]);
      segment->label      = configs[i].label;
      segment->start_time = start_time;
      segments.push_back(segment);
      max_duration = std::max(max_duration, configs[i].duration);
    }

    total_duration += max_duration;
    return *this;
  }

  template <typename T>
  Timeline &stagger(const f32 &start, const f32 &end, AnimationConfig cfg,
                    u32                   count,
                    const StaggerPattern &stagger_cfg = StaggerPatternDefault{})
  {
    nanoseconds base_delay = 0s;
    nanoseconds start_time = total_duration;

    for (usize i = 0; i < count; ++i)
    {
      base_delay = stagger_pattern.delay(i, count);
    }

    nanoseconds delay = stagger_pattern.start + base_delay;

    auto segment_config  = cfg;
    segment_config.delay = delay;

    auto segment = std::make_shared<SingleSegment>(start, end, segment_config);
    segment->start_time = start_time;
    segment->label      = cfg.label;
    segments.push_back(segment);

    return *this;
  }

  void tick() override
  {
    if (!playing)
      return;

    auto        current_time_point = clock::now();
    nanoseconds delta =
        duration_cast<nanoseconds>(current_time_point - timestamp);
    timestamp = current_time_point;

    nanoseconds adjusted_delta = delta * time_direction;
    current_time += adjusted_delta;

    if (time_direction > 0 && current_time >= total_duration)
    {
      if (cfg.loop)
      {
        current_time = nanoseconds{
            std::fmod(current_time.count(), total_duration.count())};
        for (auto &segment : segments)
        {
          segment->restart();
        }
      }
      else
      {
        current_time = total_duration;
        playing      = false;
      }
    }
    else if (time_direction < 0 && current_time <= 0s)
    {
      if (cfg.loop)
      {
        current_time =
            total_duration + nanoseconds{std::fmod(current_time.count(),
                                                   total_duration.count())};
        for (auto &segment : segments)
        {
          segment->restart();
        }
      }
      else
      {
        current_time = 0s;
        playing      = false;
      }
    }

    update_segments(adjusted_delta);
  }

  void play() override
  {
    playing   = true;
    timestamp = clock::now();
  }

  void play_from_start()
  {
    for (auto &segment : segments)
    {
      segment->restart();
    }
    current_time   = 0s;
    time_direction = 1.0F;
    playing        = true;
    timestamp      = clock::now();
  }

  void play_from_end()
  {
    for (auto &segment : segments)
    {
      segment->restart();
    }
    current_time   = total_duration;
    time_direction = -1.0F;
    playing        = true;
    timestamp      = clock::now();
  }

  void pause() override
  {
    playing = false;
  }

  void resume() override
  {
    if (!playing)
    {
      play();
    }
  }

  void stop()
  {
    playing        = false;
    current_time   = 0s;
    time_direction = 1.0F;
    for (auto &segment : segments)
    {
      segment->restart();
    }
  }

  void restart() override
  {
    stop();
  }

  bool is_playing() const
  {
    return playing;
  }

  void seek(nanoseconds time)
  {
    nanoseconds target_time =
        std::clamp(time, nanoseconds::zero(), total_duration);

    if (target_time < current_time)
    {
      // Reset all segments if seeking backwards
      for (auto &segment : segments)
      {
        segment->restart();
      }
      current_time = 0s;
      update_segments(target_time);
    }
    else
    {
      nanoseconds delta = target_time - current_time;
      update_segments(delta);
    }

    current_time = target_time;
  }

  // Seek to percentage of duration
  void seek_percentage(f32 percentage)
  {
    percentage  = std::clamp(percentage, 0.0F, 1.0F);
    auto target = nanoseconds{
        static_cast<nanoseconds::rep>(total_duration.count() * percentage)};
    seek(target);
  }

  void reverse()
  {
    time_direction *= -1.0F;
  }
  // Force a direction: `1` is forward or `-1` is backward
  void set_direction(i32 direction)
  {
    time_direction = direction < 0 ? -1.0F : 1.0F;
  }

  nanoseconds current_time() const
  {
    return current_time;
  }

  nanoseconds total_duration() const
  {
    return total_duration;
  }

  f32 progress() const
  {
    if (total_duration.count() == 0)
      return 0.0F;
    return static_cast<float>(current_time.count()) /
           static_cast<float>(total_duration.count());
  }

  bool is_reversed() const
  {
    return time_direction < 0;
  }

  bool is_finished() const
  {
    return !cfg.loop &&
           ((time_direction > 0 && current_time >= total_duration) ||
            (time_direction < 0 && current_time <= nanoseconds::zero()));
  }

  std::optional<SegmentPtr> segment(Span<const char> label)
  {
    auto filter_segment = [&label](SegmentPtr seg) {
      return seg->label().data() == label.data();
    };
    if (auto result = std::ranges::find_if(segments, filter_segment);
        result != segments.end())
    {
      return *result;
    }
    return std::nullopt;
  }

  SegmentPtr segment(u32 index)
  {
    return segments[index];
  }

  void update_segments(nanoseconds delta)
  {
    for (auto &segment : segments)
    {
      if (time_direction > 0)
      {
        // Forward playback
        if (current_time >= segment->start_time &&
            current_time <= segment->start_time + segment->duration)
        {
          segment->update(current_time, delta);
        }
      }
      else
      {
        // Reverse playback
        nanoseconds segment_end = segment->start_time + segment->duration;
        if (current_time <= segment_end && current_time >= segment->start_time)
        {
          segment->update(current_time, delta);
        }
      }
    }
  }
};

}        // namespace anim

}        // namespace ash
