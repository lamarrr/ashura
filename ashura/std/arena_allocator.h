#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"

namespace ash
{

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

/// @begin: where the memory block begins
/// @end: one byte past the block
/// @offset: end of the last allocation, must be set to {begin}
/// @alignment: actual alignment requested from allocator
struct Arena
{
  void *begin     = nullptr;
  void *end       = nullptr;
  void *offset    = nullptr;
  usize alignment = 1;

  constexpr usize size() const
  {
    return (char *) end - (char *) begin;
  }

  constexpr usize used() const
  {
    return (char *) offset - (char *) begin;
  }

  constexpr void reset()
  {
    offset = begin;
  }

  void *allocate(usize alignment, usize size)
  {
    if (size == 0)
    {
      return nullptr;
    }

    void *aligned = (void *) mem::align_offset((uintptr_t) offset, alignment);
    void *new_offset = (char *) aligned + size;
    if (new_offset > end)
    {
      return nullptr;
    }
    offset = new_offset;
    return aligned;
  }

  void *allocate_zeroed(usize alignment, usize size)
  {
    void *memory = allocate(alignment, size);
    if (memory != nullptr)
    {
      memset(memory, 0, size);
    }
    return memory;
  }

  void *reallocate(usize alignment, void *memory, usize old_size,
                   usize new_size)
  {
    // check if it is the last allocation, if so we just need to  extend the
    // offset
    if ((((char *) memory + old_size) == offset) &&
        (((char *) memory + new_size) <= end))
    {
      offset = (char *) memory + new_size;
      return memory;
    }

    void *new_memory = allocate(alignment, new_size);
    if (new_memory == nullptr)
    {
      return nullptr;
    }

    memcpy(new_memory, memory, old_size);
    deallocate(alignment, memory, old_size);

    return new_memory;
  }

  void deallocate(usize alignment, void *memory, usize size)
  {
    (void) alignment;
    // best-case: stack allocation, we can free memory by adjusting to the
    // beginning of allocation
    if (((char *) memory + size) == offset)
    {
      // we'd lose padding space due to alignment, meaning we wouldn't be able
      // to release allocations if allocations are of different alignments
      offset = (char *) offset - size;
    }
  }

  AllocatorImpl to_allocator()
  {
    return AllocatorImpl{.self      = (Allocator) this,
                         .interface = &arena_interface};
  }
};

inline Arena to_arena(Span<u8> buffer, usize alignment)
{
  return Arena{.begin     = buffer.data(),
               .end       = buffer.data() + buffer.size(),
               .offset    = buffer.begin(),
               .alignment = alignment};
}

struct ArenaBatchInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
};

static AllocatorInterface const arena_sub_interface{
    .allocate        = ArenaBatchInterface::allocate,
    .allocate_zeroed = ArenaBatchInterface::allocate_zeroed,
    .reallocate      = ArenaBatchInterface::reallocate,
    .deallocate      = ArenaBatchInterface::deallocate};

/// Forward growing allocator. All allocations are reset/free-d at once.
///
/// @source: allocation memory source
/// @max_num_arenas: maximum number of arenas that can be allocated
/// @min_arena_size: minimum size of each arena allocation, recommended >= 4096
/// bytes (approx 1 memory page). allocations having sizes higher than that will
/// have a dedicated arena.
/// @max_alloc_size: total maximum size of all allocations performed.
///
struct ArenaBatch
{
  AllocatorImpl source         = default_allocator;
  Arena        *arenas         = nullptr;
  usize         num_arenas     = 0;
  usize         max_num_arenas = USIZE_MAX;
  usize         min_arena_size = 4096;
  usize         max_arena_size = USIZE_MAX;
  usize         max_alloc_size = USIZE_MAX;

  void reset()
  {
    for (usize i = num_arenas; i-- > 0;)
    {
      arenas[i].reset();
    }
  }

  usize size() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].size();
    }
    return s;
  }

  void release()
  {
    for (usize i = num_arenas; i-- > 0;)
    {
      source.deallocate(arenas[i].alignment, arenas[i].begin,
                        (char *) arenas[i].end - (char *) arenas[i].begin);
    }
    source.deallocate_typed(arenas, num_arenas);
    arenas     = nullptr;
    num_arenas = 0;
  }

  void *allocate(usize alignment, usize size)
  {
    if (size == 0 || size > max_arena_size)
    {
      return nullptr;
    }

    if (usize idx = num_arenas; idx-- != 0)
    {
      Arena *arena  = arenas + idx;
      void  *memory = arena->allocate(alignment, size);
      if (memory != nullptr)
      {
        return memory;
      }
    }

    if (num_arenas == max_num_arenas)
    {
      return nullptr;
    }

    usize const arena_size = max(size, min_arena_size);
    if ((this->size() + arena_size) > max_alloc_size)
    {
      return nullptr;
    }

    void *arena_memory = source.allocate(alignment, arena_size);
    if (arena_memory == nullptr)
    {
      return nullptr;
    }

    Arena *new_arenas =
        source.reallocate_typed(arenas, num_arenas, num_arenas + 1);

    if (new_arenas == nullptr)
    {
      source.deallocate(alignment, arena_memory, arena_size);
      return nullptr;
    }

    arenas       = new_arenas;
    Arena *arena = new (arenas + num_arenas)
        Arena{.begin     = arena_memory,
              .end       = (char *) arena_memory + arena_size,
              .offset    = arena_memory,
              .alignment = alignment};
    num_arenas++;
    return arena->allocate(alignment, size);
  }

  void *allocate_zeroed(usize alignment, usize size)
  {
    void *memory = allocate(alignment, size);
    if (memory == nullptr)
    {
      return nullptr;
    }
    memset(memory, 0, size);
    return memory;
  }

  void *reallocate(usize alignment, void *memory, usize old_size,
                   usize new_size)
  {
    if (new_size > max_arena_size)
    {
      return nullptr;
    }

    if (usize i = num_arenas; i-- != 0)
    {
      Arena *arena = arenas + i;
      if (arena->offset == ((char *) memory + old_size))
      {
        // try to extend the allocation if it was the last allocation
        if (((char *) arena->offset + new_size) <= arena->end)
        {
          arena->offset = (char *) memory + new_size;
          return memory;
        }

        // if only allocation on the arena, reallocate arena
        if (arena->begin == memory)
        {
          void *new_memory = source.reallocate(arena->alignment, arena->begin,
                                               arena->size(), new_size);
          if (new_memory == nullptr)
          {
            return nullptr;
          }
          *arena = Arena{.begin     = new_memory,
                         .end       = (char *) new_memory + new_size,
                         .offset    = (char *) new_memory + new_size,
                         .alignment = arena->alignment};
          return new_memory;
        }
      }
    }

    void *new_memory = allocate(alignment, new_size);
    if (new_memory == nullptr)
    {
      return nullptr;
    }
    memcpy(new_memory, memory, old_size);
    deallocate(alignment, memory, old_size);
    return new_memory;
  }

  void deallocate(usize alignment, void *memory, usize size)
  {
    (void) alignment;
    if (memory == nullptr)
    {
      return;
    }

    // we can try to reclaim some memory, although we'd lose alignment padding.
    // best case: if is at end of arena, shrink arena.
    if (usize i = num_arenas; i-- != 0)
    {
      Arena *arena = arenas + i;
      if (arena->offset == ((char *) memory + size))
      {
        arena->offset = memory;
        return;
      }
    }
  }

  AllocatorImpl to_allocator()
  {
    return AllocatorImpl{.self      = (Allocator) this,
                         .interface = &arena_sub_interface};
  }
};

}        // namespace ash
