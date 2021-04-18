#pragma once

#include <cstdint>
#include <numeric>
#include <variant>

#include "vlk/ui/primitives.h"

#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

struct Clamp {
  /// i.e. result should be between 50% and 75% of the parent allotted extent.
  /// by default, the `min` = 0% and `max` = 100% of the parent allotted
  /// extent. `min` and `max` must be in [0.0, 1.0] and `max` >= `min`.
  /// max must be <= 1.0 if in a constrained context.
  float min = 0.0f;
  float max = 1.0f;

  constexpr bool operator==(Clamp const& other) const {
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
  float scale = 0.0f;
  /// removing or deducting from the target size
  int64_t bias = 0;

  /// clipping the target size, i.e. should be between 20px and 600px
  int64_t min = i64_min;
  int64_t max = i64_max;

  /// clamping the relative values of the result
  Clamp clamp;

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

  constexpr bool operator==(Constrain const& other) const {
    return f32_eq(scale, other.scale) && bias == other.bias &&
           min == other.min && max == other.max && clamp == other.clamp;
  }
};

// TODO(lamarrr): helper functions for SelfExtent, ViewExtent, and Constrain

struct SelfExtent {
  Constrain width;
  Constrain height;

  static constexpr SelfExtent relative(float width, float height) {
    return SelfExtent{Constrain::relative(width), Constrain::relative(height)};
  }

  static constexpr SelfExtent absolute(int64_t width, int64_t height) {
    return SelfExtent{Constrain::absolute(width), Constrain::absolute(height)};
  }

  static constexpr SelfExtent absolute(Extent const& extent) {
    return SelfExtent::absolute(extent.width, extent.height);
  }

  Extent resolve(Extent const& allotment) const {
    auto const resolved_width = static_cast<uint32_t>(std::clamp<int64_t>(
        width.resolve(allotment.width, true), u32_min, u32_max));
    auto const resolved_height = static_cast<uint32_t>(std::clamp<int64_t>(
        height.resolve(allotment.height, true), u32_min, u32_max));
    return Extent{resolved_width, resolved_height};
  }

  constexpr bool operator==(SelfExtent const& other) const {
    return width == other.width && height == other.height;
  }
};

using Padding = Edges;
// using Border =
//    Edges;  // TODO(lamarrr): do we add more properties or more widgets?
//    widgets
// will ensure they are used on a per-need basis
// using Margin = Edges;  // ?

/// this can exceed the parent allotted size. especially in cases where we might
/// need partially or wholly constrained/unconstrained views. (i.e. constrained
/// to parent's alloted extent along width but unconstrained along height).
struct ViewExtent {
  Constrain width;
  Constrain height;

  static constexpr ViewExtent relative(float width, float height) {
    return ViewExtent{Constrain::relative(width), Constrain::relative(height)};
  }

  static constexpr ViewExtent absolute(int64_t width, int64_t height) {
    return ViewExtent{Constrain::absolute(width), Constrain::absolute(height)};
  }

  static constexpr ViewExtent absolute(Extent const& extent) {
    return ViewExtent::absolute(extent.width, extent.height);
  }

  Extent resolve(Extent const& allotment) const {
    auto const resolved_width = static_cast<uint32_t>(std::clamp<int64_t>(
        width.resolve(allotment.width, false), u32_min, u32_max));
    auto const resolved_height = static_cast<uint32_t>(std::clamp<int64_t>(
        height.resolve(allotment.height, false), u32_min, u32_max));
    return Extent{resolved_width, resolved_height};
  }

  constexpr bool operator==(ViewExtent const& other) const {
    return width == other.width && height == other.height;
  }
};

/// marks the offset of the view relative to the view extent (usually
/// a resolved `SelfExtent`)
struct ViewOffset {
  Constrain x;
  Constrain y;

  static constexpr ViewOffset relative(float x, float y) {
    return ViewOffset{Constrain::relative(x), Constrain::relative(y)};
  }

  static constexpr ViewOffset absolute(int64_t x, int64_t y) {
    return ViewOffset{Constrain::absolute(x), Constrain::absolute(y)};
  }

  static constexpr ViewOffset absolute(IOffset const& offset) {
    return ViewOffset::absolute(offset.x, offset.y);
  }

  static constexpr ViewOffset absolute(Offset const& offset) {
    return ViewOffset::absolute(offset.x, offset.y);
  }

  IOffset resolve(Extent const& content_extent) const {
    return IOffset{x.resolve(content_extent.width, false),
                   y.resolve(content_extent.height, false)};
  }

  constexpr bool operator==(ViewOffset const& other) const {
    return x == other.x && y == other.y;
  }
};

enum class Direction : uint8_t { Row, Column };

enum class Wrap : uint8_t { None, Wrap };

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

/// cross-axis alignment
/// affects how free space is used on the cross axis
/// cross-axis for row flex is y
/// cross-axis for column flex is x
enum class CrossAlign : uint8_t { Start, End, Center, Stretch };

enum class Fit : uint8_t { Shrink, Expand };

struct Flex {
  Direction direction = Direction::Row;

  Wrap wrap = Wrap::Wrap;

  MainAlign main_align = MainAlign::Start;

  CrossAlign cross_align = CrossAlign::Start;

  Fit main_fit = Fit::Shrink;

  Fit cross_fit = Fit::Shrink;
};

/// used to fit the widget's self_extent to its view_extent (if it has enough
/// space to accomodate it)
enum class ViewFit : uint8_t { None = 0, Width = 1, Height = 2 };

constexpr ViewFit operator|(ViewFit a, ViewFit b) { return vlk::enum_or(a, b); }

constexpr ViewFit operator&(ViewFit a, ViewFit b) {
  return vlk::enum_and(a, b);
}

}  // namespace ui
}  // namespace vlk
