#pragma once
#include "ashura/std/allocator.h"

namespace ash
{

/// @begin: where the memory block begins
/// @end: one byte past the block
/// @offset: end of the last allocation, must be set to {begin}
struct Arena
{
  void *begin  = nullptr;
  void *end    = nullptr;
  void *offset = nullptr;
};

struct ArenaInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
};

static AllocatorInterface const arena_interface{
    .allocate        = ArenaInterface::allocate,
    .allocate_zeroed = ArenaInterface::allocate_zeroed,
    .reallocate      = ArenaInterface::reallocate,
    .deallocate      = ArenaInterface::deallocate};

}        // namespace ash
