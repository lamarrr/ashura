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
  u64 parent       = U64_MAX;
  u64 next_sibling = U64_MAX;
  u64 first_child  = U64_MAX;
  u32 depth        = 0;
};

}        // namespace ash
