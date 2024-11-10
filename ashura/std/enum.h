/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/enum.gen.h"

namespace ash
{

/*
  // copy
  // move
  // destroy
  // in-place using tags
  //   bool is(usize i);
  // <>
  // bool is();
  // destructor

  template <usize I>
  auto &&operator|(V<I>)
  {
  }

  template <typename... Lambda>
    requires(sizeof...(Lambda) == SIZE)
  void map(Lambda &&...lambda);
template <typename... T, typename... Lambda>
void map(Enum<T...> &&e, Lambda &&...lambda)
{
  // switch on types and invoke lambda based on the type contasined
}
*/

}
