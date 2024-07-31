/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/light.h"
#include "ashura/std/math.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/types.h"

namespace ash
{
/// linearly-tilted tree node
/// @param depth depth of the tree this node belongs to. there's ever only one
/// root node at depth 0
struct SceneNode
{
  uid parent       = UID_MAX;
  uid next_sibling = UID_MAX;
  uid first_child  = UID_MAX;
  u32 depth        = 0;
};

}        // namespace ash
