/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief A reference-counted resource handle
///
/// Requirements
/// ===========
/// - non-type-centric custom callback for uninitializing resources
/// - non-memory resources, i.e. devices
/// - intrusive and extrusive reference counting
///
/// @tparam H : handle type
template <typename H>
struct Rc
{
  typedef H                                                Handle;
  typedef Fn<void(H, AliasCount &, AllocatorImpl const &)> Uninit;

  struct Inner
  {
    H             handle      = {};
    AliasCount   *alias_count = nullptr;
    AllocatorImpl allocator   = default_allocator;
    Uninit        uninit = fn([](H, AliasCount &, AllocatorImpl const &) {});
  };

  Inner inner{};

  void init(H handle, AliasCount &alias_count, AllocatorImpl const &allocator,
            Uninit uninit)
  {
    inner = Inner{.handle      = handle,
                  .alias_count = &alias_count,
                  .allocator   = allocator,
                  .uninit      = uninit};
  }

  constexpr void uninit() const
  {
    if (inner.alias_count->unalias())
    {
      inner.uninit(inner.handle, *inner.alias_count, inner.allocator);
    }
  }

  constexpr usize num_aliases() const
  {
    return inner.alias_count->count();
  }

  constexpr Rc<H> alias() const
  {
    inner.alias_count->alias();
    return *this;
  }

  constexpr H get() const
  {
    return inner.handle;
  }

  constexpr H &operator*() const
  {
    return *inner.handle;
  }

  constexpr H operator->() const
  {
    return inner.handle;
  }
};

}        // namespace ash
