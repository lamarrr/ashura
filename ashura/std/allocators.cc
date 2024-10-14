/// SPDX-License-Identifier: MIT
#include "ashura/std/allocators.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include <string.h>

namespace ash
{

bool ArenaInterface::alloc(Allocator self_, usize alignment, usize size,
                           u8 *&mem)
{
  Arena *self = (Arena *) self_;
  return self->alloc(alignment, size, mem);
}

bool ArenaInterface::alloc_zeroed(Allocator self_, usize alignment, usize size,
                                  u8 *&mem)
{
  Arena *self = (Arena *) self_;
  return self->alloc_zeroed(alignment, size, mem);
}

bool ArenaInterface::realloc(Allocator self_, usize alignment, usize old_size,
                             usize new_size, u8 *&mem)
{
  Arena *self = (Arena *) self_;
  return self->realloc(alignment, old_size, new_size, mem);
}

void ArenaInterface::dealloc(Allocator self_, usize alignment, u8 *memory,
                             usize size)
{
  Arena *self = (Arena *) self_;
  return self->dealloc(alignment, memory, size);
}

bool ArenaPoolInterface::alloc(Allocator self_, usize alignment, usize size,
                               u8 *&mem)
{
  ArenaPool *self = (ArenaPool *) self_;
  return self->alloc(alignment, size, mem);
}

bool ArenaPoolInterface::alloc_zeroed(Allocator self_, usize alignment,
                                      usize size, u8 *&mem)
{
  ArenaPool *self = (ArenaPool *) self_;
  return self->alloc_zeroed(alignment, size, mem);
}

bool ArenaPoolInterface::realloc(Allocator self_, usize alignment,
                                 usize old_size, usize new_size, u8 *&mem)
{
  ArenaPool *self = (ArenaPool *) self_;
  return self->realloc(alignment, old_size, new_size, mem);
}

void ArenaPoolInterface::dealloc(Allocator self_, usize alignment, u8 *memory,
                                 usize size)
{
  ArenaPool *self = (ArenaPool *) self_;
  return self->dealloc(alignment, memory, size);
}

}        // namespace ash
