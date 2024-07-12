
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

struct CollapsableHeader: public Widget{};

// TODO(lamarrr)
struct Collapsable : public Widget
{
  CollapsableHeader header;
  // body
};

}        // namespace ash