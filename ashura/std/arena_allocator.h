#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"

namespace ash
{

// TODO(lamarrr): alloc(0) behaviour for arenas

struct ArenaInterface
{
  static bool alloc(Allocator self, usize alignment, usize size, u8 **mem);
  static bool alloc_zeroed(Allocator self, usize alignment, usize size,
                           u8 **mem);
  static bool realloc(Allocator self, usize alignment, usize old_size,
                      usize new_size, u8 **mem);
  static void dealloc(Allocator self, usize alignment, u8 *mem, usize size);
};

static AllocatorInterface const arena_interface{
    .alloc        = ArenaInterface::alloc,
    .alloc_zeroed = ArenaInterface::alloc_zeroed,
    .realloc      = ArenaInterface::realloc,
    .dealloc      = ArenaInterface::dealloc};

/// @begin: where the memory block begins
/// @end: one byte past the block
/// @offset: end of the last allocation, must be set to {begin}
/// @alignment: actual alignment requested from allocator
struct Arena
{
  u8   *begin     = nullptr;
  u8   *end       = nullptr;
  u8   *offset    = nullptr;
  usize alignment = 1;

  constexpr usize size() const
  {
    return end - begin;
  }

  constexpr usize used() const
  {
    return offset - begin;
  }

  constexpr void reset()
  {
    offset = begin;
  }

  bool alloc(usize alignment, usize size, u8 **mem)
  {
    if (size == 0)
    {
      *mem = nullptr;
      return true;
    }

    u8 *aligned    = (u8 *) mem::align_offset(alignment, (uintptr_t) offset);
    u8 *new_offset = aligned + size;
    if (new_offset > end)
    {
      *mem = nullptr;
      return false;
    }

    offset = new_offset;
    *mem   = aligned;
    return true;
  }

  bool alloc_zeroed(usize alignment, usize size, u8 **mem)
  {
    if (size == 0)
    {
      *mem = nullptr;
      return true;
    }

    if (!alloc(alignment, size, mem))
    {
      *mem = nullptr;
      return false;
    }

    mem::zero(*mem, size);
    return true;
  }

  bool realloc(usize alignment, usize old_size, usize new_size, u8 **mem)
  {
    // if it is the last allocation, just extend the offset
    if (((*mem + old_size) == offset) && ((*mem + new_size) <= end))
    {
      offset = *mem + new_size;
      return true;
    }

    u8 *new_mem;

    if (!alloc(alignment, new_size, &new_mem))
    {
      return false;
    }

    mem::copy(*mem, new_mem, old_size);
    dealloc(alignment, *mem, old_size);
    *mem = new_mem;
    return true;
  }

  void dealloc(usize alignment, u8 *mem, usize size)
  {
    (void) alignment;
    // best-case: stack allocation, we can free memory by adjusting to the
    // beginning of allocation
    if ((mem + size) == offset)
    {
      // we'd lose padding space due to alignment, meaning we wouldn't be able
      // to release allocations if allocations are of different alignments
      offset -= size;
    }
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_alloc(usize num, T **mem)
  {
    return alloc(alignof(T), sizeof(T) * num, (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_alloc_zeroed(usize num, T **mem)
  {
    return alloc_zeroed(alignof(T), sizeof(T) * num, (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_realloc(usize old_num, usize new_num, T **mem)
  {
    return realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                   (u8 **) mem);
  }

  template <typename T>
  constexpr void t_dealloc(T *mem, usize num)
  {
    dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
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
               .alignment = 1};
}

struct ArenaPoolInterface
{
  static bool alloc(Allocator self, usize alignment, usize size, u8 **mem);
  static bool alloc_zeroed(Allocator self, usize alignment, usize size,
                           u8 **mem);
  static bool realloc(Allocator self, usize alignment, usize old_size,
                      usize new_size, u8 **mem);
  static void dealloc(Allocator self, usize alignment, u8 *mem, usize size);
};

static AllocatorInterface const arena_sub_interface{
    .alloc        = ArenaPoolInterface::alloc,
    .alloc_zeroed = ArenaPoolInterface::alloc_zeroed,
    .realloc      = ArenaPoolInterface::realloc,
    .dealloc      = ArenaPoolInterface::dealloc};

/// Forward growing allocator. All allocations are reset/free-d at once.
///
/// @source: allocation memory source
/// @max_num_arenas: maximum number of arenas that can be allocated
/// @min_arena_size: minimum size of each arena allocation, recommended >= 4096
/// bytes (approx 1 memory page). allocations having sizes higher than that will
/// have a dedicated arena.
/// @max_total_size: total maximum size of all allocations performed.
///
struct ArenaPool
{
  AllocatorImpl source              = default_allocator;
  Arena        *arenas              = nullptr;
  usize         num_arenas          = 0;
  usize         max_num_arenas      = USIZE_MAX;
  usize         min_arena_size      = 4096;
  usize         max_arena_size      = USIZE_MAX;
  usize         max_total_size      = USIZE_MAX;
  usize         min_arena_alignment = MAX_STANDARD_ALIGNMENT;

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
      source.dealloc(arenas[i].alignment, arenas[i].begin, arenas[i].size());
    }
    source.t_dealloc(arenas, num_arenas);
    arenas     = nullptr;
    num_arenas = 0;
  }

  bool alloc(usize alignment, usize size, u8 **mem)
  {
    if (size == 0 || size > max_arena_size)
    {
      *mem = nullptr;
      return false;
    }

    if (usize idx = num_arenas; idx-- != 0)
    {
      Arena *arena = arenas + idx;
      if (arena->alloc(alignment, size, mem))
      {
        return true;
      }
    }

    if (num_arenas == max_num_arenas)
    {
      *mem = nullptr;
      return false;
    }

    usize const arena_size      = max(size, min_arena_size);
    usize const arena_alignment = max(min_arena_alignment, alignment);
    if ((this->size() + arena_size) > max_total_size)
    {
      *mem = nullptr;
      return false;
    }

    u8 *arena_mem;

    if (!source.alloc(arena_alignment, arena_size, &arena_mem))
    {
      *mem = nullptr;
      return false;
    }

    if (!source.t_realloc(num_arenas, num_arenas + 1, &arenas))
    {
      source.dealloc(arena_alignment, arena_mem, arena_size);
      *mem = nullptr;
      return false;
    }

    Arena *arena =
        new (arenas + num_arenas) Arena{.begin     = arena_mem,
                                        .end       = arena_mem + arena_size,
                                        .offset    = arena_mem,
                                        .alignment = arena_alignment};
    num_arenas++;

    arena->alloc(alignment, size, mem);
    return true;
  }

  bool alloc_zeroed(usize alignment, usize size, u8 **mem)
  {
    if (!alloc(alignment, size, mem))
    {
      return false;
    }
    mem::zero(*mem, size);
    return true;
  }

  bool realloc(usize alignment, usize old_size, usize new_size, u8 **mem)
  {
    if (new_size > max_arena_size)
    {
      return false;
    }

    if (usize i = num_arenas; i-- != 0)
    {
      Arena *arena = arenas + i;
      if (arena->offset == (*mem + old_size))
      {
        // try to change the allocation if it was the last allocation
        if ((arena->offset + new_size) <= arena->end)
        {
          arena->offset = *mem + new_size;
          return mem;
        }

        // if only and first allocation on the arena, realloc arena
        if (arena->begin == *mem)
        {
          if (!source.realloc(arena->alignment, arena->size(), new_size,
                              &arena->begin))
          {
            return false;
          }
          arena->end    = arena->begin + new_size;
          arena->offset = arena->begin + new_size;
          return true;
        }
      }
    }

    u8 *new_mem;
    if (!alloc(alignment, new_size, &new_mem))
    {
      return false;
    }

    mem::copy(*mem, new_mem, old_size);
    dealloc(alignment, *mem, old_size);
    *mem = new_mem;
    return true;
  }

  void dealloc(usize alignment, u8 *mem, usize size)
  {
    (void) alignment;
    if (mem == nullptr || size == 0)
    {
      return;
    }

    // we can try to reclaim some memory, although we'd lose alignment padding.
    // best case: if is at end of arena, shrink arena.
    if (usize i = num_arenas; i-- != 0)
    {
      Arena *arena = arenas + i;
      if (arena->offset == (mem + size))
      {
        arena->offset = mem;
        return;
      }
    }
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_alloc(usize num, T **mem)
  {
    return alloc(alignof(T), sizeof(T) * num, (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_alloc_zeroed(usize num, T **mem)
  {
    return alloc_zeroed(alignof(T), sizeof(T) * num, (u8 **) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool t_realloc(usize old_num, usize new_num, T **mem)
  {
    return realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                   (u8 **) mem);
  }

  template <typename T>
  constexpr void t_dealloc(T *mem, usize num)
  {
    dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
  }

  AllocatorImpl to_allocator()
  {
    return AllocatorImpl{.self      = (Allocator) this,
                         .interface = &arena_sub_interface};
  }
};

}        // namespace ash
