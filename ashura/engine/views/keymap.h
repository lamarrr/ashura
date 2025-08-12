/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

struct KeyMap
{
  struct Entry
  {
    Slice32 and_events = {};
    Slice32 or_events  = {};
    u64     action     = 0;
  };

  Vec<u32> events_;
};

}    // namespace ash
