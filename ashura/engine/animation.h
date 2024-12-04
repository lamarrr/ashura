/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/lambda.h"
#include "ashura/std/math.h"
#include "ashura/std/option.h"
#include "ashura/std/range.h"
#include "ashura/std/super.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// [ ] https://create.roblox.com/docs/ui/animation#style
/// [ ] Procedural Animation https://www.youtube.com/watch?v=qlfh_rv6khY

/// While we use nanoseconds as the unit of time for the animation API,
/// it is a virtual nanosecond, the application or target user can decide
/// what the nanoseconds interprets to, the animation API does not
/// manage or request operating system timestamps.

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

template <typename Tween, typename T>
concept Tweens = requires (Tween tween, T low, T high, f32 t) {
  { tween(low, high, t) } -> Convertible<T>;
};

/// @brief Easing function, the parameters are stored in an array
/// @param t the linear interpolator to be eased. guaranteed range [0, 1].
typedef Lambda<f32(f32 t)> Easing;

namespace easing
{

constexpr Easing linear()
{
  return Easing{[](f32 t) { return t; }};
}

constexpr Easing in()
{
  return Easing{[](f32 t) { return ash::ease_in(t); }};
}

constexpr Easing out()
{
  return Easing{[](f32 t) { return ash::ease_out(t); }};
}

constexpr Easing in_out()
{
  return Easing{[](f32 t) { return ash::ease_in_out(t); }};
}

constexpr Easing bezier(f32 p0, f32 p1, f32 p2)
{
  return Easing{[p0, p1, p2](f32 t) { return ash::bezier(p0, p1, p2, t); }};
}

constexpr Easing cubic_bezier(f32 p0, f32 p1, f32 p2, f32 p3)
{
  return Easing{
      [p0, p1, p2, p3](f32 t) { return ash::cubic_bezier(p0, p1, p2, p3, t); }};
}

constexpr Easing catmull_rom(f32 p0, f32 p1, f32 p2, f32 p3)
{
  return Easing{
      [p0, p1, p2, p3](f32 t) { return ash::catmull_rom(p0, p1, p2, p3, t); }};
}

constexpr Easing elastic(f32 amplitude, f32 period)
{
  return Easing{[amplitude, period](f32 t) {
    return ash::elastic(amplitude, period, t);
  }};
}

constexpr Easing bounce(f32 strength)
{
  return Easing{[strength](f32 t) { return ash::bounce(strength, t); }};
}

constexpr Easing spring(f32 mass, f32 stiffness, f32 damping)
{
  return Easing{[mass, stiffness, damping](f32 t) {
    return ash::spring(mass, stiffness, damping, t);
  }};
}

};        // namespace easing

template <typename T, Tweens<T> Tween = ash::Tween>
struct TimelineSpan
{
  Tween *                 tween = nullptr;
  Span<nanoseconds const> timestamps;
  Span<Easing const>      easings;
  Span<u64 const>         runs;
  Span<T const>           frames;

  constexpr TimelineSpan(Tween & tween, Span<nanoseconds const> timestamps,
                         Span<Easing const> easings, Span<u64 const> runs,
                         Span<T const> frames) :
      tween{&tween},
      timestamps{timestamps},
      easings{easings},
      runs{runs},
      frames{frames}
  {
  }

  constexpr TimelineSpan(TimelineSpan const &)             = default;
  constexpr TimelineSpan(TimelineSpan &&)                  = default;
  constexpr TimelineSpan & operator=(TimelineSpan const &) = default;
  constexpr TimelineSpan & operator=(TimelineSpan &&)      = default;
  constexpr ~TimelineSpan()                                = default;

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
/// @details We use a suffix sum encoding of the timestamps, this makes seeking
/// the entire timeline O(Log2N) as it enables us to use a binary search,
/// It also allows us to randomly start the animation from any point in the
/// timeline without modifying the timeline or having to persist the
/// timeline or the animation state.
///
/// ```
/// frames = [f0, f1, f2, f3]
/// durations = [5ns, 2ns, 3ns]
/// easings = [e0, e1, e2]
///
/// # Timestamps will be represented by their inclusive sums:
///
/// timeline.frames = [f0, f1, f2, f3]
/// timeline.timestamps = [0ns, 5ns, 7ns, 10ns]
/// timeline.runs = [0, 3]
/// timeline.easings = [e0, e1, e2]
///
/// # and we add another:
///
/// frames = [f4, f5, f6]
/// durations = [20ns, 8ns]
/// easings = [e3, e4]
///
/// timeline.frames = [f0, f1, f2, f3, f4, f5, f6]
/// timeline.timestamps = [0ns, 5ns, 7ns, 10ns, 30ns, 38ns]
/// timeline.easings = [e0, e1, e2, e3, e4]
/// ```
///
/// @param tween_ type-independent interpolator to use for animating
/// the provided frames
/// @param timestamps_ timestamp at which each animation sequence
/// ends (inclusive sum of the durations)
/// @param easings_ easing curve of each animation segment
/// @param runs_ inclusive sum of the number of frames of each segment
/// @param frames_ animation values of each segment
///
///
template <typename T, Tweens<T> Tween = ash::Tween>
struct Timeline
{
  Tween            tween_{};
  Vec<nanoseconds> timestamps_;
  Vec<Easing>      easings_;
  Vec<u64>         runs_;
  Vec<T>           frames_;

  constexpr bool is_empty() const
  {
    // we only need to check the timestamps, the invariant
    // is that it is either an empty or valid timeline
    return timestamps_.is_empty();
  }

  void clear()
  {
    timestamps_.clear();
    easings_.clear();
    runs_.clear();
    frames_.clear();
  }

  nanoseconds duration() const
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
    CHECK(frames.size() >= 2);
    CHECK(frames.size() < U32_MAX);
    CHECK(frames.size() == durations.size() + 1);
    CHECK(durations.size() == easings.size());

    if (timestamps_.is_empty()) [[unlikely]]
    {
      timestamps_.push(0ns).unwrap();
    }

    if (runs_.is_empty()) [[unlikely]]
    {
      runs_.push(0U).unwrap();
    }

    auto const times_offset = timestamps_.size();

    timestamps_.extend_uninit(durations.size()).unwrap();

    exclusive_scan(durations,
                   timestamps_.span().slice(times_offset, durations.size()),
                   duration());

    easings_.extend_move(easings).unwrap();

    runs_.push(runs_.last() + frames.size32()).unwrap();

    frames_.extend_move(frames).unwrap();

    return *this;
  }

  constexpr TimelineSpan<T, Tween> span() const
  {
    return TimelineSpan<T>{tween_, timestamps_, easings_, frames_, runs_};
  }
};

/// @param delay total delay remaining for the animation to start playing
/// @param time timestamp of the current animation
/// @param run_time total runtime of the animation
/// @param reversed to reverse the effect of the animation, i.e. move back in time
/// @param paused if the animation is currently paused
struct AnimationState
{
  nanoseconds delay_             = 0ns;
  nanoseconds time_              = 0ns;
  nanoseconds run_time_          = 0ns;
  nanoseconds timeline_duration_ = 0ns;
  bool        reversed_          = false;
  bool        paused_            = false;

  /// @brief The animation state needs to be re-targeted onto a new timeline. This is needed when the timeline's data changes
  template <typename T>
  constexpr void sync(TimelineSpan<T> const & timeline);

  constexpr bool is_completed() const
  {
    return reversed_ ? (time_ == 0ns) : (time_ == run_time_);
  }

  /// @brief Rush to completion
  constexpr AnimationState & complete()
  {
    time_ = reversed_ ? 0ns : run_time_;
    return *this;
  }

  constexpr AnimationState & resume()
  {
    paused_ = false;
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

  // sanitize timepoint
  constexpr AnimationState & seek(nanoseconds time_point);

  /// @param t relative timepoint to seek from. [0.0, 1.0]
  constexpr AnimationState & seek_relative(f64 t)
  {
    // uses f64 for better precision as nanoseconds is 64-bit and we could lose precision
    return seek(nanoseconds{
        static_cast<nanoseconds::rep>(timeline_duration_.count() * t)});
  }

  template <typename T>
  constexpr Option<T> animate(TimelineSpan<T> const & timeline) const
  {
    sync(timeline);

    if (timeline.is_empty()) [[unlikely]]
    {
      return None;
    }

    nanoseconds const time =
        (timeline_duration_ == 0ns) ? 0ns : (time_ % timeline_duration_);

    // get current frame segment (timestamps are sorted, perform binary
    // search to get current timepoint in the timeline)
    Span const timestamp_bound = upper_bound(timeline.timestamps, time);

    u64 const timestamp_idx =
        static_cast<u64>(timestamp_bound.data() - timeline.timestamps.data());

    Span const run_bound = upper_bound(timeline.runs, timestamp_idx);

    u64 const run_idx =
        static_cast<u64>(timestamp_bound.data() - timeline.timestamps.data());

    u64 const run_start = timeline.runs[run_idx - 1];

    u64 const run_end = timeline.runs[run_idx];

    u64 const run_length = run_end - run_start;

    // it's already at distance - pos

    // [ ] get current run interpolator: use

    return Some<T>{};
  }

  /// @param delta amount to advance animation by. to speed-up multiply a speed factor
  constexpr AnimationState & tick(nanoseconds delta)
  {
    if (timeline_duration_ == 0ns)
    {
      return *this;
    }

    if (paused_)
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
  /// @param item the index of the item
  /// @param count the total number of items to be staggered, must be greater than item
  /// @return the stagger delay factor. [0, 1]
  virtual f32 operator()(u64 width, u64 item, u64 count) = 0;
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

  constexpr virtual f32 operator()(u64 rows, u64 item, u64 count) override
  {
    u64 const columns = (rows == 0) ? 0U : (count / rows);

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

  constexpr RippleStagger() = default;

  constexpr RippleStagger(RippleStagger const &) = default;

  constexpr RippleStagger(RippleStagger &&) = default;

  constexpr RippleStagger & operator=(RippleStagger const &) = default;

  constexpr RippleStagger & operator=(RippleStagger &&) = default;

  constexpr virtual ~RippleStagger() override = default;

  constexpr auto pos(u64 rows, u64 index) const
  {
    return Tuple{index % rows, index / rows};
  }

  virtual f32 operator()(u64 rows, u64 item, u64 count) override
  {
    u64 const columns = (rows == 0) ? 0U : (count / rows);

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

// we'd ideally want to remain as data-independent as possible
struct AnimationGraph
{
  // blend other blended nodes
  struct BlendNodeInput
  {
    // type
    // index
  };

  struct BlendNode
  {
    u64 inputs     = 0;
    u64 num_inputs = 0;
    f32 weight     = 0;

    void run();
  };

  struct Clip
  {
    AnimationState state;
    u64            data = 0;
    // lerp? type-erased? referencable? removal?
    //
    // timeline reference
    //

    void run();
  };

  struct StaggerBase
  {
    Super<Stagger> stagger;
    u64            num_items = 0;
  };

  struct StaggerChild
  {
    u64 parent    = 0;
    u64 item      = 0;
    u64 animation = 0;
    // only applies to blend node or clip node?
  };

  //
  // if named, use hashmap to map to id
  //
  // use sparse vec
  //
  // store all timelines in one vec
  // stagger()
  // parallel()
  //
  // make this data-independent as much as possible
  //
  // weight(name, AnimationNode&)
  //
  //
  // add(clip_node, parent_node)
  //
  //
  //
};

}        // namespace ash
