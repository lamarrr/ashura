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
  /// by default, the `low` = 0% and `high` = 100% of the parent allotted
  /// extent. `low` and `high` must be in [0.0, 1.0] and `high` >= `low`.
  /// high must be <= 1.0 if in a constrained context.
  float low = 0.0f;
  float high = 1.0f;
};

/// Why this model? sizing can be
/// - relative (`scale` = relative size)
/// - absolute (`scale` = 0, `bias` = absolute size) or both
///
/// you can also automatically have contracting layout effects
/// - padding (+ve `bias`)
/// - absolute min/max (`low`, `high`)
/// - relative min/max (`clamp.low`, `clamp.high`)
///
struct Constrain {
  /// scaling the target size
  float scale = 1.0f;
  /// removing or deducting from the target size
  int32_t bias = 0;

  /// clamping the target size, i.e. should be between 20px and 600px
  uint32_t low = u32_min;
  uint32_t high = u32_max;

  /// clamping the relative values of the result
  OutputClamp clamp{};

  static constexpr Constrain relative(float scale) { return Constrain{scale}; }

  static constexpr Constrain absolute(int32_t value) {
    return Constrain{0.0f, value};
  }

  uint32_t resolve(uint32_t source) {
    VLK_DEBUG_ENSURE(high >= low);

    VLK_DEBUG_ENSURE(scale >= 0.0f);

    VLK_DEBUG_ENSURE(clamp.low >= 0.0f);
    VLK_DEBUG_ENSURE(clamp.low <= 1.0f);

    VLK_DEBUG_ENSURE(clamp.high >= 0.0f);

    VLK_DEBUG_ENSURE(clamp.high <= 1.0f);

    VLK_DEBUG_ENSURE(clamp.high >= clamp.low);

    int64_t const value_i64 = static_cast<int64_t>(scale * source) + bias;
    uint32_t const value_u32 = std::clamp(value_i64, static_cast<int64_t>(0),
                                          static_cast<int64_t>(u32_max));
    uint32_t const value = std::clamp(value_u32, low, high);
    uint32_t const min = static_cast<uint32_t>(std::floor(clamp.low * source));
    uint32_t const max = static_cast<uint32_t>(std::floor(clamp.high * source));

    return std::clamp(value, min, max);
  }
};

struct SelfExtent {
  Constrain width = Constrain{0.0f};
  Constrain height = Constrain{0.0f};

  Extent resolve(Extent const& allotment) {
    return Extent{width.resolve(allotment.width),
                  height.resolve(allotment.height)};
  }
};

/// usually marks the offset of the view relative to the view extent (usually a
/// resolved `SelfExtent`)
struct SelfOffset {
  Constrain x = Constrain{0.0f};
  Constrain y = Constrain{0.0f};

  Offset resolve(Extent const& self_extent) {
    return Offset{x.resolve(self_extent.width), y.resolve(self_extent.height)};
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
};

}  // namespace ui
}  // namespace vlk
