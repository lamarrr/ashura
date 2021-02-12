#pragma once

#include <algorithm>
#include <cmath>

#include "vlk/ui/constraints.h"

#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

inline uint32_t resolve_eqn(uint32_t source, float scale, uint32_t bias,
                            uint32_t low, uint32_t high,
                            OutputClamp const &clamp, bool is_constrained) {
  // future-TODO[1]
  VLK_DEBUG_ENSURE(high >= low);

  VLK_DEBUG_ENSURE(scale >= 0.0f);

  VLK_DEBUG_ENSURE(clamp.low >= 0.0f);
  VLK_DEBUG_ENSURE(clamp.low <= 1.0f);

  VLK_DEBUG_ENSURE(clamp.high >= 0.0f);
  if (is_constrained) {
    VLK_DEBUG_ENSURE(clamp.high <= 1.0f);
  }

  VLK_DEBUG_ENSURE(clamp.high >= clamp.low);

  int64_t const value_i64 = static_cast<int64_t>(scale * source) + bias;
  uint32_t const value_u32 = std::clamp(value_i64, static_cast<int64_t>(0),
                                        static_cast<int64_t>(vlk::u32_max));
  uint32_t value = std::clamp(value_u32, low, high);
  uint32_t min = static_cast<uint32_t>(std::floor(clamp.low * source));
  uint32_t max = static_cast<uint32_t>(std::floor(clamp.high * source));

  return std::clamp(value, min, max);
}

inline uint32_t resolve_eqn_dependent(uint32_t source, uint32_t allotted,
                                      float scale, uint32_t bias, uint32_t low,
                                      uint32_t high, OutputClamp const &clamp,
                                      bool is_constrained) {
  // future-TODO[1]
  VLK_DEBUG_ENSURE(high >= low);

  VLK_DEBUG_ENSURE(scale >= 0.0f);

  VLK_DEBUG_ENSURE(clamp.low >= 0.0f);
  VLK_DEBUG_ENSURE(clamp.low <= 1.0f);

  VLK_DEBUG_ENSURE(clamp.high >= 0.0f);
  if (is_constrained) {
    VLK_DEBUG_ENSURE(clamp.high <= 1.0f);
  }

  VLK_DEBUG_ENSURE(clamp.high >= clamp.low);

  int64_t const value_i64 = static_cast<int64_t>(scale * source) + bias;
  uint32_t const value_u32 = std::clamp(value_i64, static_cast<int64_t>(0),
                                        static_cast<int64_t>(vlk::u32_max));
  uint32_t value = std::clamp(value_u32, low, high);
  uint32_t min = static_cast<uint32_t>(std::floor(clamp.low * allotted));
  uint32_t max = static_cast<uint32_t>(std::floor(clamp.high * allotted));

  return std::clamp(value, min, max);
}

inline uint32_t resolve_self_layout(IndependentParameters const &param,
                                    uint32_t allotted_extent) {
  return resolve_eqn(allotted_extent, param.scale, param.bias, param.low,
                     param.high, param.clamp, true);
}

/// the child's extent has already been calculated using
/// `param.child_allocation`
inline uint32_t resolve_self_layout(DependentParameters const &param,
                                    uint32_t child_extent,
                                    uint32_t parent_allotted_extent) {
  auto const &dparam = param.self_allocation;
  return resolve_eqn_dependent(child_extent, parent_allotted_extent,
                               dparam.scale, dparam.bias, dparam.low,
                               dparam.high, dparam.clamp, true);
}

/// calculate own layout using the independent parameters. this means this
/// widget does not depend on its child extent.

/// how do we prevent the parent from taking more than the allotted layout
/// whilst determining its own layout? the parent's allocation is clamped by
/// clamp.low and clamp.high (between 0% and 100%) so it can't allot more than
/// that, and the child's self layout is clamped by equivalent factors.
inline uint32_t resolve_child_allotted_layout(
    IndependentParameters const &child_allocation_param,
    uint32_t parent_allotted_extent) {
  auto const &param = child_allocation_param;
  return resolve_eqn(parent_allotted_extent, param.scale, param.bias, param.low,
                     param.high, param.clamp, true);
}

inline uint32_t resolve_view_child_allotted_layout(
    IndependentParameters const &param, uint32_t parent_allotted_extent) {
  return resolve_eqn(parent_allotted_extent, param.scale, param.bias, param.low,
                     param.high, param.clamp, false);
}

// a view's extent is not constrained to the parent's allotted extent
inline uint32_t resolve_view_extent(IndependentParameters const &param,
                                    uint32_t parent_allotted_extent) {
  return resolve_eqn(parent_allotted_extent, param.scale, param.bias, param.low,
                     param.high, param.clamp, false);
}

// a view's extent is not constrained to the parent's allotted extent
inline uint32_t resolve_view_extent(DependentParameters const &param,
                                    uint32_t child_extent,
                                    uint32_t allotted_extent) {
  auto const &dparam = param.self_allocation;
  return resolve_eqn_dependent(child_extent, allotted_extent, dparam.scale,
                               dparam.bias, dparam.low, dparam.high,
                               dparam.clamp, false);
}

// view's offset is constrained to its extent
inline uint32_t resolve_view_offset(IndependentParameters const &param,
                                    uint32_t extent) {
  return resolve_eqn(extent, param.scale, param.bias, param.low, param.high,
                     param.clamp, true);
}

template <typename T>
inline constexpr bool is_dependent(T const &value) {
  return std::holds_alternative<DependentParameters>(value);
}

}  // namespace ui
}  // namespace vlk
