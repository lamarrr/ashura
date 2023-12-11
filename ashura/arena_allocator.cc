#include "ashura/arena_allocator.h"

namespace ash
{

void *ArenaInterface::allocate(Allocator self, usize alignment, usize size)
{
  return nullptr;
}

void *ArenaInterface::allocate_zeroed(Allocator self, usize alignment,
                                      usize size)
{
  return nullptr;
}

void *ArenaInterface::reallocate(Allocator self, usize alignment, void *memory,
                                 usize old_size, usize new_size)
{
  return nullptr;
}

void ArenaInterface::deallocate(Allocator self, usize alignment, void *memory,
                                usize size)
{
}

void ArenaInterface::release(Allocator self)
{
}

}        // namespace ash
