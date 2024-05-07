#include "ashura/std/arena_allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include <string.h>

namespace ash
{

void *ArenaInterface::allocate(Allocator self_, usize alignment, usize size)
{
  Arena *self = (Arena *) self_;
  return self->allocate(alignment, size);
}

void *ArenaInterface::allocate_zeroed(Allocator self_, usize alignment,
                                      usize size)
{
  Arena *self = (Arena *) self_;
  return self->allocate_zeroed(alignment, size);
}

void *ArenaInterface::reallocate(Allocator self_, usize alignment, void *memory,
                                 usize old_size, usize new_size)
{
  Arena *self = (Arena *) self_;
  return self->reallocate(alignment, memory, old_size, new_size);
}

void ArenaInterface::deallocate(Allocator self_, usize alignment, void *memory,
                                usize size)
{
  Arena *self = (Arena *) self_;
  return self->deallocate(alignment, memory, size);
}

void *ArenaBatchInterface::allocate(Allocator self_, usize alignment,
                                    usize size)
{
  ArenaBatch *self = (ArenaBatch *) self_;
  return self->allocate(alignment, size);
}

void *ArenaBatchInterface::allocate_zeroed(Allocator self_, usize alignment,
                                           usize size)
{
  ArenaBatch *self = (ArenaBatch *) self_;
  return self->allocate_zeroed(alignment, size);
}

void *ArenaBatchInterface::reallocate(Allocator self_, usize alignment,
                                      void *memory, usize old_size,
                                      usize new_size)
{
  ArenaBatch *self = (ArenaBatch *) self_;
  return self->reallocate(alignment, memory, old_size, new_size);
}

void ArenaBatchInterface::deallocate(Allocator self_, usize alignment,
                                     void *memory, usize size)
{
  ArenaBatch *self = (ArenaBatch *) self_;
  return self->deallocate(alignment, memory, size);
}

}        // namespace ash
