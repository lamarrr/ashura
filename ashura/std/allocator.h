/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

/// @param alloc alloc aligned memory. returns false if failed and sets the
/// memory pointer to null.
/// @param zalloc like alloc but zeroes the allocated memory, this is
/// sometimes performed by the OS and can be faster than calling memset.
/// @param realloc free the previously allocated memory and return a new
/// memory. alignment is guaranteed to be preserved. if an error occurs, the
/// old memory is unmodified, not free-d and false is returned. alignment must
/// be same as the alignment of the original allocated memory.
/// @param dealloc free the previously allocated memory.
///
/// REQUIREMENTS
/// =============
///
/// @param alignment must be a power of 2. UB if 0 or otherwise.
///
struct Allocator
{
  [[nodiscard]] constexpr virtual bool alloc(Layout layout, u8 *& mem) = 0;

  [[nodiscard]] constexpr virtual bool zalloc(Layout layout, u8 *& mem) = 0;

  [[nodiscard]] constexpr virtual bool realloc(Layout layout, usize new_size,
                                               u8 *& mem) = 0;

  constexpr virtual void dealloc(Layout layout, u8 * mem) = 0;

  template <typename T>
  [[nodiscard]] constexpr bool nalloc(usize num, T *& mem)
  {
    return alloc(layout_of<T>.array(num), (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nzalloc(usize num, T *& mem)
  {
    return zalloc(layout_of<T>.array(num), (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num, T *& mem)
  {
    return realloc(layout_of<T>.array(old_num), sizeof(T) * new_num,
                   (u8 *&) mem);
  }

  template <typename T>
  constexpr void ndealloc(usize num, T * mem)
  {
    dealloc(layout_of<T>.array(num), (u8 *) mem);
  }

  /// @brief alignment-padded allocation
  template <typename T>
  [[nodiscard]] constexpr bool pnalloc(usize alignment, usize num, T *& mem)
  {
    return alloc(layout_of<T>.array(num).align_to(alignment).aligned(),
                 (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool pnzalloc(usize alignment, usize num, T *& mem)
  {
    return zalloc(layout_of<T>.array(num).align_to(alignment).aligned(),
                  (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool pnrealloc(usize alignment, usize old_num,
                                         usize new_num, T *& mem)
  {
    return realloc(
      layout_of<T>.array(old_num).align_to(alignment).aligned(),
      layout_of<T>.array(new_num).align_to(alignment).aligned().size,
      (u8 *&) mem);
  }

  template <typename T>
  constexpr void pndealloc(usize alignment, usize num, T * mem)
  {
    dealloc(layout_of<T>.array(num).align_to(alignment).aligned(), (u8 *) mem);
  }
};

struct NoopAllocator final : Allocator
{
  virtual bool alloc(Layout, u8 *&) override
  {
    return false;
  }

  virtual bool zalloc(Layout, u8 *&) override
  {
    return false;
  }

  virtual bool realloc(Layout, usize new_size, u8 *&) override
  {
    return new_size == 0;
  }

  virtual void dealloc(Layout, u8 *) override
  {
  }
};

/// @brief General Purpose Heap allocator. guarantees at least
/// MAX_STANDARD_ALIGNMENT alignment, when overaligned memory allocators are
/// available and supported it can allocate over-aligned memory.
struct HeapAllocator final : Allocator
{
  virtual bool alloc(Layout, u8 *& mem) override;

  virtual bool zalloc(Layout, u8 *& mem) override;

  virtual bool realloc(Layout, usize new_size, u8 *& mem) override;

  virtual void dealloc(Layout, u8 * mem) override;
};

extern NoopAllocator noop_allocator_impl;

extern HeapAllocator heap_allocator_impl;

struct [[nodiscard]] AllocatorRef
{
  Allocator * self;

  constexpr AllocatorRef(Allocator & allocator = heap_allocator_impl) :
    self{&allocator}
  {
  }

  constexpr AllocatorRef(AllocatorRef const &)             = default;
  constexpr AllocatorRef(AllocatorRef &&)                  = default;
  constexpr AllocatorRef & operator=(AllocatorRef const &) = default;
  constexpr AllocatorRef & operator=(AllocatorRef &&)      = default;
  constexpr ~AllocatorRef()                                = default;

  constexpr Allocator * operator->() const
  {
    return self;
  }

  constexpr Allocator & operator*() const
  {
    return *self;
  }

  constexpr Allocator * ptr() const
  {
    return self;
  }

  constexpr Allocator & unref() const
  {
    return *self;
  }
};

inline constexpr AllocatorRef heap_allocator{heap_allocator_impl};

inline constexpr AllocatorRef noop_allocator{noop_allocator_impl};

inline constexpr AllocatorRef default_allocator{};

}    // namespace ash
