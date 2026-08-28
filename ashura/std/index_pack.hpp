/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/index_pack.gen.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

template <usize I, typename... E>
using index_pack = intr::index_pack<I, E...>::Type;

namespace intr
{

template <typename F, usize... I>
constexpr decltype(auto) index_apply(F && f, std::index_sequence<I...>)
{
    return static_cast<F &&>(f).template operator()<I...>();
}

}    // namespace intr

template <usize N, typename F>
constexpr decltype(auto) index_apply(F && f)
{
    return intr::index_apply(static_cast<F &&>(f), std::make_index_sequence<N>{});
}

}    // namespace ash
