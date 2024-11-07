/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

template <usize I, typename... Funcs, typename... Inputs>
  requires(sizeof...(Funcs) > 0)
constexpr auto fold_impl(Tuple<Funcs...> &funcs, Inputs &&...inputs)
{
  if constexpr (I < (sizeof...(Funcs) - 1))
  {
    if constexpr (Same<CallResult<decltype(get<I>(funcs)), Inputs...>, void>)
    {
      get<I>(funcs)(((Inputs &&) inputs)...);
      return fold_impl<I + 1>(funcs);
    }
    else
    {
      return fold_impl<I + 1>(funcs, get<I>(funcs)(((Inputs &&) inputs)...));
    }
  }
  else
  {
    return get<I>(funcs)(((Inputs &&) inputs)...);
  }
}

/// @brief Fold-call a tuple of functions, passing the return type of func_0 as
/// argument to func_1
/// @return the return value of the last function
template <typename... Funcs>
  requires(sizeof...(Funcs) > 0)
auto fold(Tuple<Funcs...> &funcs)
{
  return fold_impl<0>(funcs);
}

}        // namespace ash
