#pragma once

#include <cstdint>
#include <numeric>
#include <variant>

#include "vlk/ui/primitives.h"

#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

struct OutputClamp {
  /// i.e. result should be between 50% and 75% of the parent allotted extent.
  /// by default, the `min` = 0% and `max` = 100% of the parent allotted
  /// extent. `min` and `max` must be in [0.0, 1.0] and `max` >= `min`.
  /// max must be <= 1.0 if in a constrained context.
  float min = 0.0f;
  float max = 1.0f;

  constexpr bool operator==(OutputClamp const& other) const {
    return f32_eq(min, other.min) && f32_eq(max, other.max);
  }
};

/// Why this model? sizing can be
/// - relative (`scale` = relative size)
/// - absolute (`scale` = 0, `bias` = absolute size) or both
///
/// you can also automatically have contracting layout effects
/// - padding (+ve `bias`)
/// - absolute min/max (`min`, `max`)
/// - relative min/max (`clamp.min`, `clamp.max`)
///
/// how do we achieve padding/margin effect? we allot an extent and only draw
/// over a specific portion of it, the implementation of the widget itself is
/// left to determine how this will work.
///
struct Constrain {
  /// scaling the target size
  float scale = 1.0f;
  /// removing or deducting from the target size
  int64_t bias = 0;

  /// clipping the target size, i.e. should be between 20px and 600px
  int64_t min = i64_min;
  int64_t max = i64_max;

  /// clamping the relative values of the result
  OutputClamp clamp{};

  static constexpr Constrain relative(float scale) { return Constrain{scale}; }

  static constexpr Constrain absolute(int64_t value) {
    return Constrain{0.0f, value};
  }

  int64_t resolve(int64_t source, bool is_restricted) const {
    VLK_ENSURE(max >= min);
    VLK_ENSURE(scale >= 0.0f);

    if (is_restricted) {
      VLK_ENSURE(clamp.min >= 0.0f);
      VLK_ENSURE(clamp.min <= 1.0f);

      VLK_ENSURE(clamp.max >= 0.0f);
      VLK_ENSURE(clamp.max <= 1.0f);
    }

    VLK_ENSURE(clamp.max >= clamp.min);

    int64_t const value_i64 = static_cast<int64_t>(scale * source) + bias;
    int64_t const value = std::clamp(value_i64, min, max);
    auto const min = static_cast<int64_t>(clamp.min * source);
    auto const max = static_cast<int64_t>(clamp.max * source);

    return std::clamp(value, min, max);
  }
};

struct SelfExtent {
  Constrain width = Constrain{0.0f};
  Constrain height = Constrain{0.0f};

  Extent resolve(Extent const& allotment) const {
    auto const resolved_width = static_cast<uint32_t>(std::clamp(
        width.resolve(allotment.width, true), static_cast<int64_t>(u32_min),
        static_cast<int64_t>(u32_max)));
    auto const resolved_height = static_cast<uint32_t>(std::clamp(
        height.resolve(allotment.height, true), static_cast<int64_t>(u32_min),
        static_cast<int64_t>(u32_max)));
    return Extent{resolved_width, resolved_height};
  }
};

using Padding = Edges;

/// this can exceed the parent allotted size. especially in cases where we might
/// need partially or wholly constrained/unconstrained views. (i.e. constrained
/// to parent's alloted extent along width but unconstrained along height).
struct ViewExtent {
  Constrain width = Constrain{0.0f};
  Constrain height = Constrain{0.0f};

  Extent resolve(Extent const& allotment) const {
    auto const resolved_width = static_cast<uint32_t>(std::clamp(
        width.resolve(allotment.width, false), static_cast<int64_t>(u32_min),
        static_cast<int64_t>(u32_max)));
    auto const resolved_height = static_cast<uint32_t>(std::clamp(
        height.resolve(allotment.height, false), static_cast<int64_t>(u32_min),
        static_cast<int64_t>(u32_max)));
    return Extent{resolved_width, resolved_height};
  }
};

/// marks the offset of the view relative to the view extent (usually
/// a resolved `SelfExtent`)
struct ViewOffset {
  Constrain x = Constrain{0.0f};
  Constrain y = Constrain{0.0f};

  IOffset resolve(Extent const& content_extent) const {
    return IOffset{x.resolve(content_extent.width, false),
                   y.resolve(content_extent.height, false)};
  }
};

struct Flex {
  enum class Direction : uint8_t { Row, Column };

  Direction direction = Direction::Row;

  enum class Wrap : uint8_t { None, Wrap };

  Wrap wrap = Wrap::Wrap;

  /// main-axis alignment
  /// affects how free space is used on the main axis
  /// main-axis for row flex is x
  /// main-axis for column flex is y
  enum class MainAlign : uint8_t {
    Start,
    End,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };

  MainAlign main_align = MainAlign::Start;

  /// cross-axis alignment
  /// affects how free space is used on the cross axis
  /// cross-axis for row flex is y
  /// cross-axis for column flex is x
  enum class CrossAlign : uint8_t { Start, End, Center, Stretch };

  CrossAlign cross_align = CrossAlign::Start;

  enum class Fit : uint8_t { Shrink = 0, Expand = 1 };

  Fit main_fit = Fit::Shrink;

  Fit cross_fit = Fit::Shrink;
};

constexpr Flex::Fit operator|(Flex::Fit const& a, Flex::Fit const& b) {
  return vlk::enum_or(a, b);
}

constexpr Flex::Fit operator&(Flex::Fit const& a, Flex::Fit const& b) {
  return vlk::enum_and(a, b);
}

}  // namespace ui
}  // namespace vlk
