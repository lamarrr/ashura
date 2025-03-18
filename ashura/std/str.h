/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{
namespace str
{

namespace impl
{
template <typename Vec, typename C>
Result<> join(Vec & out, Span<C const> delimiter,
              Span<Span<C const> const> strings)
{
  if (strings.size() == 0)
  {
    return Ok{};
  }

  usize const initial_size = out.size();

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
}    // namespace impl

template <typename Char, typename DelimChar, typename Str0Char,
          typename... StrChars>
Result<> join(Vec<Char> & out, Span<DelimChar> delimiter, Span<Str0Char> str0,
              Span<StrChars>... strs)
{
  Str strings[] = {str0, strs...};
  return impl::join(out, delimiter, strings);
}

template <typename Char, usize Capacity, typename DelimChar, typename Str0Char,
          typename... StrChars>
Result<> join(InplaceVec<Char, Capacity> & out, Span<DelimChar> delimiter,
              Span<Str0Char> str0, Span<StrChars>... strs)
{
  Str strings[] = {str0, strs...};
  return impl::join(out, delimiter, strings);
}

}    // namespace str
}    // namespace ash
