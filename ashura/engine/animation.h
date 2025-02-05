/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/lambda.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
#include "ashura/std/super.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// While we use nanoseconds as the unit of time for the animation API,
/// it is a virtual nanosecond, the application or target user can decide
/// what the nanoseconds interprets to, the animation API does not
/// manage or request operating system timestamps.
/// i64 precision is important for externally synchronized animations.

/// @brief An object used to tween/interpolate between two values.
/// This is a separate object to allow users customize the definition of the
/// values depending on the context. And it isn't bundled with the interpolated
/// objects to allow for more efficient storage of the values.
struct Tween
{
  constexpr f32 operator()(f32 low, f32 high, f32 t) const
  {
    return lerp(low, high, t);
  }

  constexpr f32 operator()(f64 low, f64 high, f32 t) const
  {
    return lerp((f32) low, (f32) high, t);
  }

  constexpr Vec2 operator()(Vec2 const & low, Vec2 const & high, f32 t) const
  {
    return lerp(low, high, Vec2::splat(t));
  }

  constexpr Vec3 operator()(Vec3 const & low, Vec3 const & high, f32 t) const
  {
    return lerp(low, high, Vec3::splat(t));
  }

  constexpr Vec4 operator()(Vec4 const & low, Vec4 const & high, f32 t) const
  {
    return lerp(low, high, Vec4::splat(t));
  }

  constexpr Vec4U8 operator()(Vec4U8 const & low, Vec4U8 const & high,
                              f32 t) const
  {
    return as_vec4u8(lerp(as_vec4(low), as_vec4(high), Vec4::splat(t)));
  }
};

template <typename T>
using Tweener = Lambda<T(T, T, f32)>;

/// @brief Easing function
/// @param t the linear interpolator to be eased.
typedef Lambda<f32(f32 t)> Easing;

namespace easing
{

inline Easing linear()
{
  return Easing{[](f32 t) { return t; }};
}

inline Easing in()
{
  return Easing{[](f32 t) { return ash::ease_in(t); }};
}

inline Easing out()
{
  return Easing{[](f32 t) { return ash::ease_out(t); }};
}

inline Easing in_out()
{
  return Easing{[](f32 t) { return ash::ease_in_out(t); }};
}

inline Easing bezier(f32 p0, f32 p1, f32 p2)
{
  return Easing{[p0, p1, p2](f32 t) { return ash::bezier(p0, p1, p2, t); }};
}

inline Easing cubic_bezier(f32 p0, f32 p1, f32 p2, f32 p3)
{
  return Easing{
    [p0, p1, p2, p3](f32 t) { return ash::cubic_bezier(p0, p1, p2, p3, t); }};
}

inline Easing catmull_rom(f32 p0, f32 p1, f32 p2, f32 p3)
{
  return Easing{
    [p0, p1, p2, p3](f32 t) { return ash::catmull_rom(p0, p1, p2, p3, t); }};
}

inline Easing elastic(f32 amplitude, f32 period)
{
  return Easing{
    [amplitude, period](f32 t) { return ash::elastic(amplitude, period, t); }};
}

inline Easing bounce(f32 strength)
{
  return Easing{[strength](f32 t) { return ash::bounce(strength, t); }};
}

inline Easing spring(f32 mass, f32 stiffness, f32 damping)
{
  return Easing{[mass, stiffness, damping](f32 t) {
    return ash::spring(mass, stiffness, damping, t);
  }};
}

inline Easing in_cubic()
{
  return Easing{[](f32 t) { return t * t * t; }};
}

inline Easing out_cubic()
{
  return Easing{[](f32 t) { return 1 - pow3(1 - t); }};
}

inline Easing in_out_cubic()
{
  return Easing{[](f32 t) {
    return t < 0.5F ? 4 * t * t * t : 1 - pow3(-2 * t + 2) * 0.5F;
  }};
}

inline Easing in_quart()
{
  return Easing{[](f32 t) { return pow4(t); }};
}

inline Easing out_quart()
{
  return Easing{[](f32 t) { return 1 - pow4(1 - t); }};
}

inline Easing in_out_quart()
{
  return Easing{
    [](f32 t) { return t < 0.5F ? 8 * pow4(t) : 1 - pow4(-2 * t + 2) * 0.5F; }};
}

inline Easing in_quint()
{
  return Easing{[](f32 t) { return pow5(t); }};
}

inline Easing out_quint()
{
  return Easing{[](f32 t) { return 1 - pow5(1 - t); }};
}

inline Easing in_out_quint()
{
  return Easing{[](f32 t) {
    return t < 0.5F ? 16 * t * t * t * t * t : 1 - pow5(-2 * t + 2) * 0.5F;
  }};
}

inline Easing in_sine()
{
  return Easing{[](f32 t) { return 1 - cos(t * PI * 0.5F); }};
}

inline Easing out_sine()
{
  return Easing{[](f32 t) { return sin(t * PI * 0.5F); }};
}

inline Easing in_out_sine()
{
  return Easing{[](f32 t) { return -(cos(PI * t) - 1) * 0.5F; }};
}

inline Easing in_expo()
{
  return Easing{[](f32 t) { return t == 0 ? 0 : powf(2, 10 * t - 10); }};
}

inline Easing out_expo()
{
  return Easing{[](f32 t) { return t == 1 ? 1 : 1 - powf(2, -10 * t); }};
}

inline Easing in_out_expo()
{
  return Easing{[](f32 t) {
    return t == 0  ? 0 :
           t == 1  ? 1 :
           t < 0.5 ? powf(2, 20 * t - 10) * 0.5F :
                     (2 - powf(2, -20 * t + 10)) * 0.5F;
  }};
}

inline Easing in_circ()
{
  return Easing{[](f32 t) { return 1 - sqrt(1 - pow2(t)); }};
}

inline Easing out_circ()
{
  return Easing{[](f32 t) { return sqrt(1 - pow2(t - 1)); }};
}

inline Easing in_out_circ()
{
  return Easing{[](f32 t) {
    return t < 0.5F ? (1 - sqrt(1 - pow2(2 * t))) * 0.5F :
                      (sqrt(1 - pow2(-2 * t + 2)) + 1) * 0.5F;
  }};
}

};    // namespace easing

template <typename T>
struct TimelineView
{
  Tweener<T> const *      tweener = nullptr;
  Span<nanoseconds const> timestamps;
  Span<Easing const>      easings;
  Span<T const>           frames;

  constexpr TimelineView(Tweener<T> const &      tweener,
                         Span<nanoseconds const> timestamps,
                         Span<Easing const> easings, Span<T const> frames) :
    tweener{&tweener},
    timestamps{timestamps},
    easings{easings},
    frames{frames}
  {
  }

  constexpr TimelineView(TimelineView const &)             = default;
  constexpr TimelineView(TimelineView &&)                  = default;
  constexpr TimelineView & operator=(TimelineView const &) = default;
  constexpr TimelineView & operator=(TimelineView &&)      = default;
  constexpr ~TimelineView()                                = default;

  constexpr bool is_empty() const
  {
    return timestamps.is_empty();
  }

  constexpr nanoseconds duration() const
  {
    return timestamps.is_empty() ? 0ns : timestamps.last();
  }
};

/// @brief An animtion Timeline containing timestamps, values, and easing
/// functions needed to execute an animation.
/// This is well optimized for serialization, deserialization,
/// and dynamic updates. The associated keyframe data is also dynamic and not
/// forcefully needed to be owned by the timeline. but only added for ease of use.
///
///
/// @details We use a prefix sum encoding of the timestamps, this makes seeking
/// the entire timeline O(Log2N) as it enables us to use a binary search,
/// It also allows us to randomly start the animation from any point in the
/// timeline without modifying the timeline or having to persist the
/// timeline or the animation state.
///
/// ```
/// frames = [a_0, a_1, b_0, b_1]
/// durations = [5ns, 3ns]
/// easings = [e0, e1]
///
/// # Timestamps will be represented by their inclusive sums:
///
/// timeline.frames = [a_0, a_1, b_0, b_1]
/// timeline.timestamps = [0ns, 5ns, 8ns]
/// timeline.easings = [e0, e1]
///
/// # and we add another:
///
/// frames = [c_0, c_1, d_0, d_1]
/// durations = [200ns, 1000ns]
/// easings = [e2, e3]
///
/// timeline.frames = [a_0, a_1, b_0, b_1, c_0, c_1, d_0, d_1]
/// timeline.timestamps = [0ns, 5ns, 8ns, 208ns, 1208ns]
/// timeline.easings = [e0, e1, e2, e3]
/// ```
///
/// @param tween_ type-independent interpolator to use for animating
/// the provided frames
/// @param timestamps_ timestamp at which each animation sequence
/// ends (inclusive sum of the durations)
/// @param easings_ easing curve of each animation segment
/// @param frames_ animation values of each segment
///
///
template <typename T>
struct Timeline
{
  Tweener<T>       tweener_{Tween{}};
  Vec<nanoseconds> timestamps_;
  Vec<Easing>      easings_;
  Vec<T>           frames_;

  constexpr bool is_empty() const
  {
    // we only need to check the timestamps, the invariant
    // is that it is either an empty or valid timeline
    return timestamps_.is_empty();
  }

  constexpr void clear()
  {
    timestamps_.clear();
    easings_.clear();
    frames_.clear();
  }

  constexpr nanoseconds duration() const
  {
    if (timestamps_.is_empty()) [[unlikely]]
    {
      return 0ns;
    }
    return timestamps_.last();
  }

  /// @brief Add a tween key frame
  Timeline & frame(T start, T end, nanoseconds duration, Easing easing)
  {
    T frames[] = {std::move(start), std::move(end)};

    return key_frame(frames, span<nanoseconds>({duration}), {&easing, 1});
  }

  /// @brief Add a multi-point key-frame
  Timeline & key_frame(Span<T> frames, Span<nanoseconds const> durations,
                       Span<Easing> easings)
  {
    CHECK(frames.size() == (durations.size() * 2), "");
    CHECK(durations.size() == easings.size(), "");

    if (timestamps_.is_empty()) [[unlikely]]
    {
      timestamps_.push(0ns).unwrap();
    }

    auto const times_offset = timestamps_.size();

    auto const run_time = timestamps_.last();

    timestamps_.extend_uninit(durations.size()).unwrap();

    inclusive_scan(durations,
                   timestamps_.view().slice(times_offset, durations.size()),
                   run_time);

    easings_.extend_move(easings).unwrap();

    frames_.extend_move(frames).unwrap();

    return *this;
  }

  constexpr TimelineView<T> view() const
  {
    return TimelineView<T>{tweener_, timestamps_, easings_, frames_};
  }
};

/// @param delay_ total delay remaining for the animation to start playing
/// @param time_ timestamp of the current animation
/// @param run_delay_ total runtime of the animation
/// @param run_time_ total runtime of the animation
/// @param reversed_ to reverse the effect of the animation, i.e. move back in time
/// @param paused_ if the animation is currently paused
struct AnimationState
{
  nanoseconds delay_     = 0ns;
  nanoseconds time_      = 0ns;
  bool        reversed_  = false;
  bool        paused_    = false;
  nanoseconds run_delay_ = 0ns;
  nanoseconds run_time_  = nanoseconds::max();

  /// @brief is the animation a delayed type of animation
  constexpr bool is_delayed() const
  {
    return run_delay_ != 0ns;
  }

  /// @brief is the animation pending execution due to a delay
  constexpr bool is_pending() const
  {
    return delay_ != 0ns;
  }

  constexpr bool is_completed() const
  {
    return reversed_ ? (time_ == 0ns) : (time_ == run_time_);
  }

  constexpr bool is_reversed() const
  {
    return reversed_;
  }

  constexpr bool is_paused() const
  {
    return paused_;
  }

  constexpr AnimationState & restart()
  {
    delay_    = run_delay_;
    time_     = 0ns;
    reversed_ = false;
    return *this;
  }

  /// @brief Rush to completion
  constexpr AnimationState & complete()
  {
    delay_ = 0ns;
    time_  = reversed_ ? 0ns : run_time_;
    return *this;
  }

  constexpr AnimationState & cancel()
  {
    delay_  = 0ns;
    time_   = 0ns;
    paused_ = true;
    return *this;
  }

  constexpr AnimationState & resume()
  {
    delay_  = 0ns;
    paused_ = false;
    return *this;
  }

  constexpr AnimationState & delay(nanoseconds delay)
  {
    delay_     = delay;
    run_delay_ = delay;
    return *this;
  }

  constexpr AnimationState & reverse()
  {
    reversed_ = !reversed_;
    return *this;
  }

  constexpr AnimationState & pause()
  {
    paused_ = true;
    return *this;
  }

  constexpr AnimationState & loop()
  {
    run_time_ = nanoseconds::max();
    return *this;
  }

  constexpr AnimationState & limit(nanoseconds run_time)
  {
    run_time_ = run_time;
    return *this;
  }

  constexpr AnimationState & seek(nanoseconds time_point)
  {
    time_ = clamp(time_point, 0ns, run_time_);
    return *this;
  }

  template <typename T>
  constexpr AnimationState & iterations(TimelineView<T> const & timeline,
                                        i64                     num_iterations)
  {
    run_time_ = timeline.duration() * num_iterations;
    return *this;
  }

  /// @brief Synchronize with the timeline and get the current animated value
  /// @returns None, if there's no animation data, otherwise the animated value.
  template <typename T>
  T animate(TimelineView<T> const & timeline)
  {
    CHECK(!timeline.is_empty(), "");

    /// add 1ns so result of modulo operation would be between 0ns and timeline-duration
    auto const timeline_end = timeline.duration() + 1ns;

    auto const time =
      (timeline.duration() == 0ns) ? 0ns : (time_ % timeline_end);

    auto const timestamps = timeline.timestamps;

    // get current frame segment (timestamps are sorted, perform binary
    // search to get current timepoint in the timeline)
    Span const span = binary_find(timestamps.slice(1), geq, time);

    CHECK(!span.is_empty(), "");

    u64 const end_idx = static_cast<u64>(span.pbegin() - timestamps.pbegin());

    u64 const ease_idx  = end_idx - 1;
    u64 const frame_idx = ease_idx * 2;

    nanoseconds const start    = timestamps[end_idx - 1];
    nanoseconds const end      = timestamps[end_idx];
    nanoseconds const duration = end - start;
    nanoseconds const offset   = time - start;

    f32 const t = (f32) (((f64) offset.count()) / (f64) duration.count());

    Easing const & easing = timeline.easings[ease_idx];

    return (*timeline.tweener)(timeline.frames[frame_idx],
                               timeline.frames[frame_idx + 1], easing(t));
  }

  /// @brief Drive the animation. Cheap enough to be called every frame
  /// @param delta amount to advance animation by. to speed-up
  /// multiply a speed factor.
  constexpr AnimationState & tick(nanoseconds delta)
  {
    if (is_paused() || is_completed())
    {
      return *this;
    }

    if (delay_ >= delta)
    {
      delay_ -= delta;
      return *this;
    }
    else if (delay_ < delta)
    {
      delta -= delay_;
    }

    delta *= reversed_ ? -1 : 1;

    time_ += delta;

    time_ = clamp(time_, 0ns, run_time_);

    return *this;
  }
};

/// @brief Stagger delay of animation components
struct Stagger
{
  virtual ~Stagger() = default;

  /// @brief Perform stagger delay on a list of components
  /// @param width the dimension of the stagger pattern, i.e. the number of rows.
  /// Affects the pattern's granularity
  /// @param num_items the total number of items to be staggered, must be greater than item
  /// @return the stagger delay factor. [0, 1]
  /// @param item the index of the item
  virtual f32 operator()(u64 width, u64 num_items, u64 item = 0) = 0;
};

struct Unstaggered final : Stagger
{
  constexpr Unstaggered() = default;

  constexpr Unstaggered(Unstaggered const &) = default;

  constexpr Unstaggered(Unstaggered &&) = default;

  constexpr Unstaggered & operator=(Unstaggered const &) = default;

  constexpr Unstaggered & operator=(Unstaggered &&) = default;

  virtual ~Unstaggered() override = default;

  virtual f32 operator()(u64, u64, u64) override
  {
    return 0;
  }
};

/// @brief Grid-based delay calculation
/// @param row_weight weight controlling the relative influence of the row position to the column position [0, 1]
/// @param reverse_row reverse the stagger direction in the row axis, i.e. the elements at the end of the row will animate first
/// @param reverse_column reverse the stagger direction in the column axis, i.e. the elements at the end of the column will animate first
struct GridStagger final : Stagger
{
  bool reverse_row;
  bool reverse_column;
  f32  row_weight;

  constexpr GridStagger(bool reverse_row = false, bool reverse_column = false,
                        f32 row_weight = 0.5F) :
    reverse_row{reverse_row},
    reverse_column{reverse_column},
    row_weight{row_weight}
  {
  }

  constexpr GridStagger() = default;

  constexpr GridStagger(GridStagger const &) = default;

  constexpr GridStagger(GridStagger &&) = default;

  constexpr GridStagger & operator=(GridStagger const &) = default;

  constexpr GridStagger & operator=(GridStagger &&) = default;

  constexpr virtual ~GridStagger() override = default;

  constexpr auto pos(u64 rows, u64 index) const
  {
    return Tuple{index % rows, index / rows};
  }

  constexpr virtual f32 operator()(u64 rows, u64 num_items, u64 item) override
  {
    rows              = max(rows, (u64) 1);
    u64 const columns = num_items / rows;

    f32 row_norm    = 1;
    f32 column_norm = 1;

    auto const [row, column] = pos(rows, item);

    if (rows > 1)
    {
      row_norm = row / (f32) (rows - 1);
    }

    if (columns > 1)
    {
      column_norm = column / (f32) (columns - 1);
    }

    if (reverse_row)
    {
      row_norm = 1 - row_norm;
    }

    if (reverse_column)
    {
      column_norm = 1 - column_norm;
    }

    return lerp(row_norm, column_norm, row_weight);
  }
};

/// @param inwards should the ripple effect occur with the outer part
/// animating first
struct RippleStagger final : Stagger
{
  bool inwards;

  constexpr RippleStagger(bool inwards = false) : inwards{inwards}
  {
  }

  constexpr RippleStagger(RippleStagger const &) = default;

  constexpr RippleStagger(RippleStagger &&) = default;

  constexpr RippleStagger & operator=(RippleStagger const &) = default;

  constexpr RippleStagger & operator=(RippleStagger &&) = default;

  constexpr virtual ~RippleStagger() override = default;

  constexpr auto pos(u64 rows, u64 index) const
  {
    return Tuple{index % rows, index / rows};
  }

  virtual f32 operator()(u64 rows, u64 num_items, u64 item) override
  {
    rows              = max(rows, (u64) 1);
    u64 const columns = num_items / rows;

    f32 row_norm    = 0.5F;
    f32 column_norm = 0.5F;

    auto const [row, column] = pos(rows, item);

    if (rows > 1)
    {
      row_norm = (f32) row / (f32) (rows - 1);
    }

    if (columns > 1)
    {
      column_norm = (f32) column / (f32) (columns - 1);
    }

    row_norm    = row_norm - 0.5F;
    column_norm = column_norm - 0.5F;

    f32 radius_norm = sqrt(row_norm * row_norm + column_norm * column_norm);

    radius_norm = inwards ? (1 - radius_norm) : radius_norm;

    // sqrt is in-exact
    return clamp(radius_norm, 0.0F, 1.0F);
  }
};

template <typename... T>
struct StaggeredAnimation
{
  Vec<AnimationState>   states_{};
  Super<Stagger>        stagger_{Unstaggered{}};
  u64                   stagger_width_ = 1;
  Tuple<Timeline<T>...> timelines_{};
  nanoseconds           delay_ = 0ns;

  StaggeredAnimation() = default;

  StaggeredAnimation(Vec<AnimationState> states, Super<Stagger> stagger,
                     u64 stagger_width, Tuple<Timeline<T>...> timelines) :
    states_{std::move(states)},
    stagger_{std::move(stagger)},
    stagger_width_{stagger_width},
    timelines_{std::move(timelines)}
  {
  }

  StaggeredAnimation(StaggeredAnimation &&)             = default;
  StaggeredAnimation & operator=(StaggeredAnimation &&) = default;

  StaggeredAnimation(StaggeredAnimation const &)             = delete;
  StaggeredAnimation & operator=(StaggeredAnimation const &) = delete;

  ~StaggeredAnimation() = default;

  static auto make(u64 stagger_width = 0, u64 num_items = 0,
                   Super<Stagger> stagger = Unstaggered{})
  {
    Vec<AnimationState> states{};
    states.resize(num_items).unwrap();

    Tuple<Timeline<T>...> timelines;

    return StaggeredAnimation<T...>{std::move(states), std::move(stagger),
                                    stagger_width, std::move(timelines)};
  }

  StaggeredAnimation & delay(nanoseconds delay)
  {
    delay_ = delay;
    for (auto [item, state] : enumerate<u64>(states_))
    {
      f32 const delay_factor =
        stagger_.get()(stagger_width_, states_.size64(), item);
      nanoseconds item_delay = nanoseconds{static_cast<nanoseconds::rep>(
        static_cast<f64>(delay.count()) * delay_factor)};
      state.delay(item_delay);
    }
    return *this;
  }

  StaggeredAnimation & complete()
  {
    for (AnimationState & s : states_)
    {
      s.complete();
    }
    return *this;
  }

  StaggeredAnimation & cancel()
  {
    for (AnimationState & s : states_)
    {
      s.cancel();
    }
    return *this;
  }

  StaggeredAnimation & resume()
  {
    for (AnimationState & s : states_)
    {
      s.resume();
    }
    return *this;
  }

  StaggeredAnimation & restart()
  {
    for (AnimationState & s : states_)
    {
      s.restart();
    }
    return *this;
  }

  StaggeredAnimation & reverse()
  {
    for (AnimationState & s : states_)
    {
      s.reverse();
    }
    return *this;
  }

  StaggeredAnimation & pause()
  {
    for (AnimationState & s : states_)
    {
      s.pause();
    }
    return *this;
  }

  StaggeredAnimation & loop()
  {
    for (AnimationState & s : states_)
    {
      s.loop();
    }
    return *this;
  }

  u64 width() const
  {
    return stagger_width_;
  }

  StaggeredAnimation & width(u64 extent)
  {
    stagger_width_ = extent;
    return *this;
  }

  StaggeredAnimation & stagger(Super<Stagger> stagger = Unstaggered{})
  {
    stagger_ = std::move(stagger);
    return *this;
  }

  AnimationState & state(u64 item)
  {
    CHECK(states_.size64() > item, "");
    return states_[item];
  }

  Tuple<Timeline<T> &...> timelines()
  {
    return apply(
      [](auto &... timelines) {
        return Tuple<decltype(timelines)...>{timelines...};
      },
      timelines_);
  }

  Tuple<T...> animate(u64 item)
  {
    CHECK(states_.size64() > item, "");

    AnimationState & state = states_[item];

    return apply(
      [&](auto &... timeline) {
        return Tuple<T...>{state.animate(timeline.view())...};
      },
      timelines_);
  }

  void tick(nanoseconds delta)
  {
    for (AnimationState & state : states_)
    {
      // update the total runtime
      state.run_time_ = timelines_.v0.duration();
      state.tick(delta);
    }
  }
};

}    // namespace ash
