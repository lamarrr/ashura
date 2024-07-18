
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct CollapsableHeader : public View
{
};

// TODO(lamarrr)
struct Collapsable : public View
{
  CollapsableHeader header;
  // body
};

}        // namespace ash