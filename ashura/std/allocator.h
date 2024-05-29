#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct Allocator_T *Allocator;

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
struct AllocatorInterface
{
  bool (*alloc)(Allocator self, usize alignment, usize size,
                u8 **mem)                   = nullptr;
  bool (*alloc_zeroed)(Allocator self, usize alignment, usize size,
                       u8 **mem)            = nullptr;
  bool (*realloc)(Allocator self, usize alignment, usize old_size,
                  usize new_size, u8 **mem) = nullptr;
  void (*dealloc)(Allocator self, usize alignment, u8 *mem,
                  usize size)               = nullptr;
};

struct AllocatorImpl
{
  Allocator                 self      = nullptr;
  AllocatorInterface const *interface = nullptr;

  [[nodiscard]] constexpr bool alloc(usize alignment, usize size,
                                     u8 **mem) const
  {
    return interface->alloc(self, alignment, size, mem);
  }

  [[nodiscard]] constexpr bool alloc_zeroed(usize alignment, usize size,
                                            u8 **mem) const
  {
    return interface->alloc_zeroed(self, alignment, size, mem);
  }

  [[nodiscard]] constexpr bool realloc(usize alignment, usize old_size,
                                       usize new_size, u8 **mem) const
  {
    return interface->realloc(self, alignment, old_size, new_size, mem);
  }

  constexpr void dealloc(usize alignment, u8 *mem, usize size) const
  {
    interface->dealloc(self, alignment, mem, size);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc(usize num, T **mem) const
  {
    return interface->alloc(self, alignof(T), sizeof(T) * num, (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc_zeroed(usize num, T **mem) const
  {
    return interface->alloc_zeroed(self, alignof(T), sizeof(T) * num,
                                   (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num,
                                        T **mem) const
  {
    return interface->realloc(self, alignof(T), sizeof(T) * old_num,
                              sizeof(T) * new_num, (u8 **) mem);
  }

  template <typename T>
  constexpr void ndealloc(T *mem, usize num) const
  {
    interface->dealloc(self, alignof(T), (u8 *) mem, sizeof(T) * num);
  }
};

/// guarantees at least MAX_STANDARD_ALIGNMENT alignment.
struct HeapInterface
{
  static bool alloc(Allocator self, usize alignment, usize size, u8 **mem);
  static bool alloc_zeroed(Allocator self, usize alignment, usize size,
                           u8 **mem);
  static bool realloc(Allocator self, usize alignment, usize old_size,
                      usize new_size, u8 **mem);
  static void dealloc(Allocator self, usize alignment, u8 *mem, usize size);
};

inline constexpr AllocatorInterface heap_interface{
    .alloc        = HeapInterface::alloc,
    .alloc_zeroed = HeapInterface::alloc_zeroed,
    .realloc      = HeapInterface::realloc,
    .dealloc      = HeapInterface::dealloc};

inline constexpr AllocatorImpl heap_allocator{.self      = nullptr,
                                              .interface = &heap_interface};

inline constexpr ash::AllocatorImpl default_allocator = heap_allocator;

}        // namespace ash
