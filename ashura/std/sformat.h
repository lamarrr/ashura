/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/format.h"
#include "ashura/std/result.h"
#include "ashura/std/vec.h"

namespace ash
{

namespace impl
{

template <typename Vec, typename... Args>
constexpr Result<Void, fmt::Result> sformat_to(Vec & out, Span<char const> fstr,
                                               Args const &... args)
{
  bool oom = false;

  auto sink = [&](Str str) {
    if (!oom)
    {
      if (!out.extend(str))
      {
        oom = true;
      }
    }
  };

  fmt::Op ops[fmt::MAX_ARGS];

  fmt::Context ctx{&sink, Buffer{ops}};

  if (auto r = ctx.format(fstr, args...); r.error != fmt::Error::None)
  {
    return Err{r};
  }

  if (oom)
  {
    return Err{fmt::Result{.error = fmt::Error::OutOfMemory}};
  }

  return Ok{};
}

}    // namespace impl

/// @brief Format to a dynamic string using custom allocator
template <typename... Args>
constexpr Result<Vec<char>, fmt::Result>
  sformat(Allocator allocator, Span<char const> fstr, Args const &... args)
{
  Vec<char> out{allocator};

  return impl::sformat_to(out, fstr, args...).map([&](auto &) {
    return std::move(out);
  });
}

/// @brief Format to a static capacity string
template <usize Capacity, typename... Args>
constexpr Result<InplaceVec<char, Capacity>, fmt::Result>
  snformat(Span<char const> fstr, Args const &... args)
{
  InplaceVec<char, Capacity> out;

  return impl::sformat_to(out, fstr, args...).map([&](auto &) {
    return std::move(out);
  });
}

/// @brief Small-string format
template <usize InlineCapacity, typename... Args>
constexpr Result<SmallVec<char, InlineCapacity>, fmt::Result>
  ssformat(Allocator allocator, Span<char const> fstr, Args const &... args)
{
  SmallVec<char, InlineCapacity> out{allocator};

  return impl::sformat_to(out, fstr, args...).map([&](auto &) {
    return std::move(out);
  });
}

}    // namespace ash
