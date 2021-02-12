#pragma once

#include <cstdint>
#include <variant>

#include "vlk/utils/limits.h"

// TODO(lamarrr): rename to layout.h?

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
/// - absolute (`scale` = 0, `bias` = absolute size)
///
/// you can also automatically have contracting layout effects
/// - padding (+ve `bias`)
/// - absolute min/max (`low`, `high`)
/// - relative min/max (`clamp.low`, `clamp.high`)
///
struct IndependentParameters {
  /// scaling the target size
  float scale = 1.0f;
  /// removing or deducting from the target size
  int32_t bias = 0;

  /// clamping the target size, i.e. should be between 20px and 600px
  uint32_t low = vlk::u32_min;
  uint32_t high = vlk::u32_max;

  /// clamping the relative values of the result
  OutputClamp clamp{};
};

/// we query the child's sizing first by giving it the max allottable
/// extent determined by `children_allocation`.
/// using the max children's spatial span (maximum of two extreme ends), we
/// determine the widget's extent from the child's using `self_allocation`.
struct DependentParameters {
  IndependentParameters self_allocation;
  IndependentParameters children_allocation;
};

using Parameters = std::variant<IndependentParameters, DependentParameters>;

struct SelfLayout {
  Parameters width;
  Parameters height;
};

// position children relative to own dimensions
// problem here is that there's no way to get the children's dimensions and lay
// them out relative to that.

struct Flex {
  // main axis for row flex is x
  // main axis for column flex y
  enum class Direction : uint8_t { Row, Column };

  Direction direction = Direction::Row;

  // if wrap is None do we make it a view?
  enum class Wrap : uint8_t { None, Wrap };

  Wrap wrap = Wrap::Wrap;

  /// main-axis alignment
  /// how free space is used on the main axis
  ///
  enum class MainAlign : uint8_t {
    Start,
    End,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };

  MainAlign main_align = MainAlign::Start;

  /// cross-axis alignment
  /// how free space is used on the cross axis
  ///
  enum class CrossAlign : uint8_t { Start, End, Center, Stretch };

  CrossAlign cross_align = CrossAlign::Start;
};

struct ChildLayout {
  IndependentParameters x;
  IndependentParameters y;
  IndependentParameters width;
  IndependentParameters height;
};

/// marks the offset of the view relative to the view extent
struct ViewOffset {
  IndependentParameters x = IndependentParameters{0.0f};
  IndependentParameters y = IndependentParameters{0.0f};
};

/// marks the inner extent of the view which could depend on its children's
/// layout
struct ViewExtent {
  Parameters width = DependentParameters{// self allocation
                                         IndependentParameters{1.0f},
                                         // child allocation
                                         IndependentParameters{1.0f}};
  Parameters height = DependentParameters{// self allocation
                                          IndependentParameters{1.0f},
                                          // child allocation
                                          IndependentParameters{1.0f}};
};

}  // namespace ui
}  // namespace vlk
