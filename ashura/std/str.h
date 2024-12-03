/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

namespace str
{

template <typename C>
Result<> join(Span<Span<C const> const> strings, Span<C const> delimiter,
              Vec<C> & out)
{
  if (strings.size() == 0)
  {
    return Ok{};
  }

  usize initial_size = out.size();

  for (usize i = 0; i < (strings.size() - 1); i++)
  {
    if (!out.extend(strings[i]) || !out.extend(delimiter))
    {
      out.resize_uninit(initial_size).unwrap();
      return Err{};
    }
  }

  if (!out.extend(strings.last()))
  {
    out.resize_uninit(initial_size).unwrap();
    return Err{};
  }

  return Ok{};
}

}        // namespace str

}        // namespace ash
