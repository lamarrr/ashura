/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

template <usize I, typename Tuple, typename... In>
constexpr decltype(auto) fold_impl_reduce(Tuple &fns, In &&...in)
{
  if constexpr (I == (Tuple::SIZE - 1))
  {
    return get<I>(fns)(((In &&) in)...);
  }
  else
  {
    if constexpr (Same<CallResult<decltype(get<I>(fns)), In...>, void>)
    {
      get<I>(fns)(((In &&) in)...);
      return fold_impl_reduce<I + 1, Tuple>(fns);
    }
    else
    {
      return fold_impl_reduce<I + 1, Tuple>(fns, get<I>(fns)(((In &&) in)...));
    }
  }
}

template <class Tuple, typename... In>
constexpr decltype(auto) fold_impl(Tuple &fns, In &&...in)
{
  if constexpr (Tuple::SIZE == 0)
  {
    return;
  }
  else
  {
    return fold_impl_reduce<0, Tuple>(fns, ((In &&) in)...);
  }
}

/// @brief Folds left call a tuple of functions, acting as a single function.
/// i.e. result = ( in... -> fn.0 -> fn.1 -> fn.2 -> return )
/// @return the return value of the last function
template <class Tuple, typename... In>
constexpr decltype(auto) fold(Tuple &fns, In &&...in)
{
  return fold_impl(fns, ((In &&) in)...);
}

}        // namespace ash
