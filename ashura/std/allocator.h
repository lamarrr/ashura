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
struct IAllocator
{
  constexpr IAllocator()                               = default;
  constexpr IAllocator(IAllocator const &)             = delete;
  constexpr IAllocator(IAllocator &&)                  = delete;
  constexpr IAllocator & operator=(IAllocator const &) = delete;
  constexpr IAllocator & operator=(IAllocator &&)      = delete;
  constexpr ~IAllocator()                              = default;

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

  /// @brief Alignment-padded allocation
  template <typename T>
  [[nodiscard]] constexpr bool pnalloc(usize alignment, usize num, T *& mem)
  {
    return alloc(layout_of<T>.array(num).align_to(alignment), (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool pnzalloc(usize alignment, usize num, T *& mem)
  {
    return zalloc(layout_of<T>.array(num).align_to(alignment), (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool pnrealloc(usize alignment, usize old_num,
                                         usize new_num, T *& mem)
  {
    return realloc(layout_of<T>.array(old_num).align_to(alignment),
                   layout_of<T>.array(new_num).align_to(alignment).size,
                   (u8 *&) mem);
  }

  template <typename T>
  constexpr void pndealloc(usize alignment, usize num, T * mem)
  {
    dealloc(layout_of<T>.array(num).align_to(alignment), (u8 *) mem);
  }
};

struct NoopAllocator final : IAllocator
{
  constexpr NoopAllocator()                                  = default;
  constexpr NoopAllocator(NoopAllocator const &)             = delete;
  constexpr NoopAllocator(NoopAllocator &&)                  = delete;
  constexpr NoopAllocator & operator=(NoopAllocator const &) = delete;
  constexpr NoopAllocator & operator=(NoopAllocator &&)      = delete;
  constexpr ~NoopAllocator()                                 = default;

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
struct HeapAllocator final : IAllocator
{
  constexpr HeapAllocator()                                  = default;
  constexpr HeapAllocator(HeapAllocator const &)             = delete;
  constexpr HeapAllocator(HeapAllocator &&)                  = delete;
  constexpr HeapAllocator & operator=(HeapAllocator const &) = delete;
  constexpr HeapAllocator & operator=(HeapAllocator &&)      = delete;
  constexpr ~HeapAllocator()                                 = default;

  virtual bool alloc(Layout, u8 *& mem) override;

  virtual bool zalloc(Layout, u8 *& mem) override;

  virtual bool realloc(Layout, usize new_size, u8 *& mem) override;

  virtual void dealloc(Layout, u8 * mem) override;
};

extern NoopAllocator noop_allocator_impl;

extern HeapAllocator heap_allocator_impl;

struct [[nodiscard]] Allocator
{
  IAllocator * self;

  constexpr Allocator(IAllocator & allocator = heap_allocator_impl) :
    self{&allocator}
  {
  }

  constexpr Allocator(Allocator const &)             = default;
  constexpr Allocator(Allocator &&)                  = default;
  constexpr Allocator & operator=(Allocator const &) = default;
  constexpr Allocator & operator=(Allocator &&)      = default;
  constexpr ~Allocator()                             = default;

  constexpr IAllocator * operator->() const
  {
    return self;
  }

  constexpr IAllocator & operator*() const
  {
    return *self;
  }

  constexpr IAllocator * ptr() const
  {
    return self;
  }

  constexpr IAllocator & unref() const
  {
    return *self;
  }
};

inline constexpr Allocator heap_allocator{heap_allocator_impl};

inline constexpr Allocator noop_allocator{noop_allocator_impl};

inline constexpr Allocator default_allocator{};

}    // namespace ash
