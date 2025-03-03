/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/tuple.gen.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename F, typename Tuple, usize... I>
constexpr decltype(auto) impl_apply(F && f, Tuple && t,
                                    std::index_sequence<I...>)
{
  return static_cast<F &&>(f)((static_cast<Tuple &&>(t).template get<I>())...);
}

template <typename F, typename Tuple>
constexpr decltype(auto) apply(F && f, Tuple && t)
{
  return impl_apply(static_cast<F &&>(f), static_cast<Tuple &&>(t),
                    std::make_index_sequence<decay<Tuple>::SIZE>{});
}

template <usize I, typename Tuple, typename... In>
constexpr decltype(auto) impl_fold_reduce(Tuple & fns, In &&... in)
{
  if constexpr (I == (Tuple::SIZE - 1))
  {
    return fns.template get<I>()(static_cast<In &&>(in)...);
  }
  else
  {
    if constexpr (Same<CallResult<decltype(fns.template get<I>()), In...>,
                       void>)
    {
      fns.template get<I>()(static_cast<In &&>(in)...);
      return impl_fold_reduce<I + 1, Tuple>(fns);
    }
    else
    {
      return impl_fold_reduce<I + 1, Tuple>(
        fns, fns.template get<I>()(static_cast<In &&>(in)...));
    }
  }
}

template <typename Tuple, typename... In>
constexpr decltype(auto) impl_fold(Tuple & fns, In &&... in)
{
  if constexpr (Tuple::SIZE == 0)
  {
    return;
  }
  else
  {
    return impl_fold_reduce<0, Tuple>(fns, static_cast<In &&>(in)...);
  }
}

/// @brief Folds left call a tuple of functions, acting as a single function.
/// i.e. result = ( in... -> fn.0 -> fn.1 -> fn.2 -> return )
/// @return the return value of the last function
template <typename Tuple, typename... In>
constexpr decltype(auto) fold(Tuple & fns, In &&... in)
{
  return impl_fold(fns, static_cast<In &&>(in)...);
}

}    // namespace ash
