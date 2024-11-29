/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @param alloc alloc aligned memory. returns false if failed and sets the
/// memory pointer to null.
/// @param alloc_zeroed like alloc but zeroes the allocated memory, this is
/// sometimes performed by the OS and can be faster than calling memset.
/// @param realloc free the previously allocated memory and return a new
/// memory. alignment is not guaranteed to be preserved. if an error occurs, the
/// old memory is unmodified, not free-d and false is returned. alignment must
/// be same as the alignment of the original allocated memory.
/// @param dealloc free the previously allocated memory.
///
/// REQUIREMENTS
/// =============
///
/// @param alignment must be a power of 2. UB if 0 or otherwise.
/// @param size must be 0 or multiple of alignment.
///
struct Allocator
{
  virtual bool alloc(usize alignment, usize size, u8 *& mem) = 0;

  virtual bool alloc_zeroed(usize alignment, usize size, u8 *& mem) = 0;

  virtual bool realloc(usize alignment, usize old_size, usize new_size,
                       u8 *& mem) = 0;

  virtual void dealloc(usize alignment, u8 * mem, usize size) = 0;

  template <typename T>
  [[nodiscard]] constexpr bool nalloc(usize num, T *& mem)
  {
    return alloc(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc_zeroed(usize num, T *& mem)
  {
    return alloc_zeroed(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num, T *& mem)
  {
    return realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                   (u8 *&) mem);
  }

  template <typename T>
  constexpr void ndealloc(T * mem, usize num)
  {
    dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
  }
};

struct NoopAllocator final : Allocator
{
  virtual bool alloc(usize, usize, u8 *&) override
  {
    return false;
  }

  virtual bool alloc_zeroed(usize, usize, u8 *&) override
  {
    return false;
  }

  virtual bool realloc(usize, usize, usize, u8 *&) override
  {
    return false;
  }

  virtual void dealloc(usize, u8 *, usize) override
  {
  }
};

/// @brief General Purpose Heap allocator. guarantees at least
/// MAX_STANDARD_ALIGNMENT alignment, when overaligned memory allocators are
/// available and supported it can allocate over-aligned memory.
struct HeapAllocator final : Allocator
{
  virtual bool alloc(usize alignment, usize size, u8 *& mem) override;

  virtual bool alloc_zeroed(usize alignment, usize size, u8 *& mem) override;

  virtual bool realloc(usize alignment, usize old_size, usize new_size,
                       u8 *& mem) override;

  virtual void dealloc(usize alignment, u8 * mem, usize size) override;
};

extern NoopAllocator noop_allocator_impl;
extern HeapAllocator heap_allocator_impl;

struct AllocatorImpl
{
  Allocator * self = &heap_allocator_impl;

  [[nodiscard]] constexpr bool alloc(usize alignment, usize size,
                                     u8 *& mem) const
  {
    return self->alloc(alignment, size, mem);
  }

  [[nodiscard]] constexpr bool alloc_zeroed(usize alignment, usize size,
                                            u8 *& mem) const
  {
    return self->alloc_zeroed(alignment, size, mem);
  }

  [[nodiscard]] constexpr bool realloc(usize alignment, usize old_size,
                                       usize new_size, u8 *& mem) const
  {
    return self->realloc(alignment, old_size, new_size, mem);
  }

  constexpr void dealloc(usize alignment, u8 * mem, usize size) const
  {
    self->dealloc(alignment, mem, size);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc(usize num, T *& mem) const
  {
    return self->alloc(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc_zeroed(usize num, T *& mem) const
  {
    return self->alloc_zeroed(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num,
                                        T *& mem) const
  {
    return self->realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                         (u8 *&) mem);
  }

  template <typename T>
  constexpr void ndealloc(T * mem, usize num) const
  {
    self->dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
  }
};

inline constexpr AllocatorImpl default_allocator{.self = &heap_allocator_impl};
inline constexpr AllocatorImpl heap_allocator{.self = &heap_allocator_impl};
inline constexpr AllocatorImpl noop_allocator{.self = &noop_allocator_impl};

}        // namespace ash
