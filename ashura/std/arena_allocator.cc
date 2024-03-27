#include "ashura/std/arena_allocator.h"
#include <string.h>

namespace ash
{

void *ArenaInterface::allocate(Allocator self_, usize alignment, usize size)
{
  Arena *self  = (Arena *) self_;
  usize  skip  = (uintptr_t) self->offset & (alignment - 1);
  void  *alloc = (char *) self->offset + skip;
  void  *end   = (char *) alloc + size;
  if (end > self->end)
  {
    return nullptr;
  }
  self->offset = end;
  return alloc;
}

void *ArenaInterface::allocate_zeroed(Allocator self_, usize alignment,
                                      usize size)
{
  Arena *self  = (Arena *) self_;
  void  *alloc = ArenaInterface::allocate(self_, alignment, size);
  if (alloc != nullptr)
  {
    memset(alloc, 0, size);
  }

  return alloc;
}

void *ArenaInterface::reallocate(Allocator self_, usize alignment, void *memory,
                                 usize old_size, usize new_size)
{
  Arena *self = (Arena *) self_;
  if (new_size <= old_size)
  {
    // shrink if last allocated
    if (self->offset == ((char *) memory + old_size))
    {
      self->offset = (char *) self->offset - (old_size - new_size);
    }
    return memory;
  }

  // check if it is the last allocated element, if so we just need to
  // extend the offset by the extension
  if (self->offset == ((char *) memory + old_size))
  {
    usize const extension = new_size - old_size;
    if (((char *) self->offset + extension) > self->end)
    {
      return nullptr;
    }
    self->offset = (char *) self->offset + extension;
    return memory;
  }

  void *new_memory = ArenaInterface::allocate(self_, alignment, new_size);
  if (new_memory == nullptr)
  {
    return nullptr;
  }

  memcpy(new_memory, memory, old_size);
  ArenaInterface::deallocate(self_, alignment, memory, old_size);

  return new_memory;
}

void ArenaInterface::deallocate(Allocator, usize, void *, usize)
{
}

}        // namespace ash
