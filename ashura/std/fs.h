/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

inline bool path_append(Vec<char> &path, Span<char const> tail)
{
  if (!path.is_empty() && path[path.size() - 1] != '/' &&
      path[path.size() - 1] != '\\')
  {
    if (!path.push('/'))
    {
      return false;
    }
  }
  if (!path.extend_copy(tail))
  {
    return false;
  }
  return true;
}

}        // namespace ash