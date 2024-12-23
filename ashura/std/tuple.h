/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/tuple.gen.h"
#include "ashura/std/types.h"

namespace ash
{

template <usize I, typename Tuple>
constexpr auto && impl_get(Tuple && tuple)
{
  if constexpr (std::is_lvalue_reference_v<Tuple &&>)
  {
    return intr::tuple_member<I>(tuple);
  }
  else
  {
    return std::move(intr::tuple_member<I>(tuple));
  }
}

template <usize I, typename... T>
requires (I < sizeof...(T))
constexpr decltype(auto) get(Tuple<T...> const & tuple)
{
  return impl_get<I>(tuple);
}

template <usize I, typename... T>
requires (I < sizeof...(T))
constexpr decltype(auto) get(Tuple<T...> & tuple)
{
  return impl_get<I>(tuple);
}

template <usize I, typename... T>
requires (I < sizeof...(T))
constexpr decltype(auto) get(Tuple<T...> const && tuple)
{
  return impl_get<I>(static_cast<Tuple<T...> const &&>(tuple));
}

template <usize I, typename... T>
requires (I < sizeof...(T))
constexpr decltype(auto) get(Tuple<T...> && tuple)
{
  return impl_get<I>(static_cast<Tuple<T...> &&>(tuple));
}

template <typename F, typename Tuple, usize... I>
constexpr decltype(auto) impl_apply(F && f, Tuple && t,
                                    std::index_sequence<I...>)
{
  return static_cast<F &&>(f)(get<I>(static_cast<Tuple &&>(t))...);
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
    return get<I>(fns)(static_cast<In &&>(in)...);
  }
  else
  {
    if constexpr (Same<CallResult<decltype(get<I>(fns)), In...>, void>)
    {
      get<I>(fns)(static_cast<In &&>(in)...);
      return impl_fold_reduce<I + 1, Tuple>(fns);
    }
    else
    {
      return impl_fold_reduce<I + 1, Tuple>(
        fns, get<I>(fns)(static_cast<In &&>(in)...));
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
