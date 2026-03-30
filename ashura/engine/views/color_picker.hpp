/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

namespace ui
{

/// REQUIREMENTS:
/// - Linear and Non-Linear Color Space Independence
/// - Rectangular Box with visualizations
/// - Text-based manual input
/// - RGB, SRGB, HSV, HEX, Linear, Hue, YUV
/// - color space, pixel info for color pickers
struct ColorPicker : View
{
};

}    // namespace ui

}    // namespace ash
