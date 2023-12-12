#pragma once

#include "ashura/types.h"

namespace ash
{

typedef struct Allocator_T             *Allocator;
typedef struct AllocatorInterface       AllocatorInterface;
typedef struct AllocatorImpl            AllocatorImpl;
typedef struct Heap                     Heap;
typedef struct HeapInterface            HeapInterface;
typedef struct OverAlignedHeap          OverAlignedHeap;
typedef struct OverAlignedHeapInterface OverAlignedHeapInterface;

/// @allocate: allocate aligned memory. returns null if
/// failed.
/// @allocate_zeroed: like allocate but zeroes the allocated memory, this is
/// performed by the OS and can be faster. returns null if allocation failed.
/// @reallocate: free the previously allocated memory and return a new memory,
/// alignment is not guaranteed to be preserved. if an error occurs, the old
/// memory is not freed and null is returned. alignment must be same as the
/// alignment of the original allocated memory.
/// @deallocate: free the previously allocated memory.
///
/// @release: releases all allocated memory on the allocator.
///
///
/// REQUIREMENTS
/// =============
///
/// @alignment: must be a power of 2
///
struct AllocatorInterface
{
  void *(*allocate)(Allocator self, usize alignment, usize size) = nullptr;
  void *(*allocate_zeroed)(Allocator self, usize alignment,
                           usize size)                           = nullptr;
  void *(*reallocate)(Allocator self, usize alignment, void *memory,
                      usize old_size, usize new_size)            = nullptr;
  void (*deallocate)(Allocator self, usize alignment, void *memory,
                     usize size)                                 = nullptr;
  void (*release)(Allocator self)                                = nullptr;
};

struct AllocatorImpl
{
  Allocator                 self      = nullptr;
  AllocatorInterface const *interface = nullptr;

  [[nodiscard]] void *allocate(usize alignment, usize size) const
  {
    return interface->allocate(self, alignment, size);
  }

  template <typename T>
  [[nodiscard]] T *allocate_typed(usize num) const
  {
    return (T *) interface->allocate(self, alignof(T), sizeof(T) * num);
  }

  [[nodiscard]] void *allocate_zeroed(usize alignment, usize size) const
  {
    return interface->allocate_zeroed(self, alignment, size);
  }

  template <typename T>
  [[nodiscard]] T *allocate_zeroed_typed(usize num) const
  {
    return (T *) interface->allocate_zeroed(self, alignof(T), sizeof(T) * num);
  }

  [[nodiscard]] void *reallocate(usize alignment, void *memory, usize old_size,
                                 usize new_size) const
  {
    return interface->reallocate(self, alignment, memory, old_size, new_size);
  }

  template <typename T>
  [[nodiscard]] T *reallocate_typed(T *memory, usize old_num,
                                    usize new_num) const
  {
    return (T *) interface->reallocate(
        self, alignof(T), memory, sizeof(T) * old_num, sizeof(T) * new_num);
  }

  [[nodiscard]] void *grow(usize alignment, void *memory, usize old_size,
                           usize growth)
  {
    return interface->reallocate(self, alignment, memory, old_size,
                                 old_size + growth);
  }

  template <typename T>
  [[nodiscard]] T *grow_typed(T *memory, usize old_num, usize growth) const
  {
    return (T *) interface->reallocate(self, alignof(T), memory,
                                       sizeof(T) * old_num,
                                       sizeof(T) * (old_num + growth));
  }

  void deallocate(usize alignment, void *memory, usize size) const
  {
    interface->deallocate(self, alignment, memory, size);
  }

  template <typename T>
  void deallocate_typed(T *memory, usize num) const
  {
    interface->deallocate(self, alignof(T), memory, sizeof(T) * num);
  }

  void release() const
  {
    interface->release(self);
  }
};

struct Heap
{
};

extern Heap const heap;

struct HeapInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
  static void  release(Allocator self);
};

static AllocatorInterface heap_interface{
    .allocate        = HeapInterface::allocate,
    .allocate_zeroed = HeapInterface::allocate_zeroed,
    .reallocate      = HeapInterface::reallocate,
    .deallocate      = HeapInterface::deallocate,
    .release         = HeapInterface::release};

/// allocator for standard-aligned allocations, guarantees at most
/// MAX_STANDARD_ALIGNMENT.
static AllocatorImpl const heap_allocator{.self      = (Allocator) &heap,
                                          .interface = &heap_interface};

struct OverAlignedHeap
{
};

extern OverAlignedHeap const over_aligned_heap;

struct OverAlignedHeapInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
  static void  release(Allocator self);
};

static AllocatorInterface const over_aligned_heap_interface{
    .allocate        = OverAlignedHeapInterface::allocate,
    .allocate_zeroed = OverAlignedHeapInterface::allocate_zeroed,
    .reallocate      = OverAlignedHeapInterface::reallocate,
    .deallocate      = OverAlignedHeapInterface::deallocate,
    .release         = OverAlignedHeapInterface::release};

/// allocator for over-aligned allocations, i.e. allocations aligned beyond
/// standard alignment requirement. guarantees more than MAX_STANDARD_ALIGNMENT
static AllocatorImpl const over_aligned_heap_allocator{
    .self      = (Allocator) &over_aligned_heap,
    .interface = &over_aligned_heap_interface};

}        // namespace ash
