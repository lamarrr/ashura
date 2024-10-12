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

/// @brief flattened hierarchical tree node, all siblings are packed
/// sequentially.
/// This only represents the parent node.
/// Since the tree is rebuilt from scratch every time, the order is preserved in
/// that parents always come before children.
/// @param depth depth of the tree this node belongs to. there's ever only one
/// node at depth 0: the root node.
struct SceneNode
{
  u32 depth        = 0;
  u32 parent       = U32_MAX;
  u32 first_child  = U32_MAX;
  u32 num_children = 0;
};

}        // namespace ash
