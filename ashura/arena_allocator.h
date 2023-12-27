#pragma once
#include "ashura/allocator.h"

namespace ash
{

/// @memory_begin: where the memory block begins
/// @memory_end: one byte past the block
/// @offset: end of the last allocation, must be set to {memory_begin}
struct Arena
{
  void *memory_begin = nullptr;
  void *memory_end   = nullptr;
  void *offset       = nullptr;
};

struct ArenaInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
  static void  release(Allocator self);
};

static AllocatorInterface const arena_interface{
    .allocate        = ArenaInterface::allocate,
    .allocate_zeroed = ArenaInterface::allocate_zeroed,
    .reallocate      = ArenaInterface::reallocate,
    .deallocate      = ArenaInterface::deallocate,
    .release         = ArenaInterface::release};

}        // namespace ash
