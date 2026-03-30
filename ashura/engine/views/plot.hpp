/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.hpp"
#include "ashura/std/types.hpp"

namespace ash
{
namespace ui
{

/// REQUIREMENTS:
/// - plot modes: histogram, lines, scale, log
/// - plot from user buffer: can be at specific index and will plot rest from
/// head.
/// - buffer size
/// - line size, color, thickness
/// - background size color thickness
/// - show dims on hover (if enabled)
struct Plot : View
{
};

}    // namespace ui

}    // namespace ash
