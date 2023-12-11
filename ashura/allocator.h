#pragma once

#include "ashura/types.h"

// allocate using standard-based object alignment
// TODO(lamarrr): cast pointer to given type so we can check if the pointer type
// is correct

// REQUIREMENTS: basic typing and type cast checks
// type-erased allocations

namespace ash
{

typedef struct Allocator_T       *Allocator;
typedef struct AllocatorInterface AllocatorInterface;
typedef struct AllocatorImpl      AllocatorImpl;

///
/// NOTE: it is undefined behaviour to pass over-aligned memory to non
/// over-aligned functions and vice-versa.
///
/// @allocate: allocate memory sized to size. the alignment is guaranteed to be
/// at least standard alignment => MAX_STANDARD_ALIGNMENT. returns null if
/// failed.
/// @allocate_zeroed: like allocate but zeroes the allocated memory, this is
/// performed by the OS and can be faster. returns null if failed.
/// @reallocate: free the previously allocated memory and return a new memory,
/// alignment is not guaranteed to be preserved. if an error occurs, the old
/// memory is not freed and null is returned.
/// @deallocate: free the previously allocated memory.
///
/// @over_aligned_allocate: same purpose as @allocate but for alignment greater
/// than MAX_STANDARD_ALIGNMENT.
/// @over_aligned_allocate_zeroed: same purpose as @allocate_zeroed but for
/// alignment greater than MAX_STANDARD_ALIGNMENT.
/// @over_aligned_reallocate: same purpose as @reallocate but for alignment
/// greater than MAX_STANDARD_ALIGNMENT. if the platform does not provide an
/// implementation, this will be implemented with an overaligned alloc + memcpy
/// + overaligned dealloc.
/// @over_aligned_deallocate: same purpose as @deallocate but for alignment
/// greater than MAX_STANDARD_ALIGNMENT.
///
/// @release: releases all allocated memory on the allocator.
///
struct AllocatorInterface
{
  void *(*allocate)(Allocator self, usize alignment, usize size) = nullptr;
  void *(*allocate_zeroed)(Allocator self, usize alignment,
                           usize size)                           = nullptr;
  void *(*reallocate)(Allocator self, usize alignment, void *old_memory,
                      usize old_size, usize new_size)            = nullptr;
  void (*deallocate)(Allocator self, usize alignment, void *memory,
                     usize size)                                 = nullptr;
  void *(*over_aligned_allocate)(Allocator self, usize alignment,
                                 usize size)                     = nullptr;
  void *(*over_aligned_allocate_zeroed)(Allocator self, usize alignment,
                                        usize size)              = nullptr;
  void *(*over_aligned_reallocate)(Allocator self, usize alignment,
                                   void *old_memory, usize old_size,
                                   usize new_size)               = nullptr;
  void (*over_aligned_deallocate)(Allocator self, usize alignment, void *memory,
                                  usize size)                    = nullptr;
  void (*release)(Allocator self)                                = nullptr;
};

struct AllocatorImpl
{
  Allocator                 self      = nullptr;
  AllocatorInterface const *interface = nullptr;

  void allocate(...);
  void allocate_t(...);
  void allocate_zeroed(...);
  void allocate_zeroed_t(...);
  void reallocate(...);
  void realloce_t(...);
  void grow_allocate(...);
  void grow_allocate_t(...);
  void de_allocate(...);
  void de_allocate_t(...);
  void over_aligned_allocate(...);
  void over_aligned_allocate_t(...);
  void over_aligned_allocate_zeroed(...);
  void over_aligned_allocate_zeroed_t(...);
  void over_aligned_reallocate(...);
  void over_aligned_reallocate_t(...);
  void over_aligned_grow_allocate(...);
  void over_aligned_grow_allocate_t(...);
  void over_aligned_de_alloc(...);
  void over_aligned_de_alloc_T(...);
};

struct DefaultHeapInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *old_memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
  static void *over_aligned_allocate(Allocator self, usize alignment,
                                     usize size);
  static void *over_aligned_allocate_zeroed(Allocator self, usize alignment,
                                            usize size);
  static void *over_aligned_reallocate(Allocator self, usize alignment,
                                       void *old_memory, usize old_size,
                                       usize new_size);
  static void  over_aligned_deallocate(Allocator self, usize alignment,
                                       void *memory, usize size);
  static void  release(Allocator self);
};

struct DefaultHeap
{
};

extern DefaultHeap default_heap;

static AllocatorInterface default_heap_allocator_interface{
    .allocate              = DefaultHeapInterface::allocate,
    .allocate_zeroed       = DefaultHeapInterface::allocate_zeroed,
    .reallocate            = DefaultHeapInterface::reallocate,
    .deallocate            = DefaultHeapInterface::deallocate,
    .over_aligned_allocate = DefaultHeapInterface::over_aligned_allocate,
    .over_aligned_allocate_zeroed =
        DefaultHeapInterface::over_aligned_allocate_zeroed,
    .over_aligned_reallocate = DefaultHeapInterface::over_aligned_reallocate,
    .over_aligned_deallocate = DefaultHeapInterface::over_aligned_deallocate,
    .release                 = DefaultHeapInterface::release};

static AllocatorImpl default_heap_allocator{
    .self      = (Allocator) &default_heap,
    .interface = &default_heap_allocator_interface};

}        // namespace ash
