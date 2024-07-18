/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

// TODO(lamarrr): selection for copy and paste, copyable attribute
struct TextBox : public View
{
  TextBlock      block  = {};
  TextBlockStyle style  = {};
  TextLayout     layout = {};
};

}        // namespace ash