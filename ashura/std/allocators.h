/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"

namespace ash
{

struct ArenaInterface
{
  static bool alloc(Allocator self, usize alignment, usize size, u8 *&mem);
  static bool alloc_zeroed(Allocator self, usize alignment, usize size,
                           u8 *&mem);
  static bool realloc(Allocator self, usize alignment, usize old_size,
                      usize new_size, u8 *&mem);
  static void dealloc(Allocator self, usize alignment, u8 *mem, usize size);
};

static AllocatorInterface const arena_interface{
    .alloc        = ArenaInterface::alloc,
    .alloc_zeroed = ArenaInterface::alloc_zeroed,
    .realloc      = ArenaInterface::realloc,
    .dealloc      = ArenaInterface::dealloc};

/// @param begin where the memory block begins
/// @param end one byte past the block
/// @param offset end of the last allocation, must be set to {begin}
/// @param alignment actual alignment requested from allocator
struct Arena
{
  u8 *begin  = nullptr;
  u8 *end    = nullptr;
  u8 *offset = nullptr;

  [[nodiscard]] constexpr usize size() const
  {
    return end - begin;
  }

  [[nodiscard]] constexpr usize used() const
  {
    return offset - begin;
  }

  [[nodiscard]] constexpr usize available() const
  {
    return end - offset;
  }

  constexpr void reclaim()
  {
    offset = begin;
  }

  [[nodiscard]] bool alloc(usize alignment, usize size, u8 *&mem)
  {
    if (size == 0)
    {
      mem = nullptr;
      return true;
    }

    u8 *aligned    = align_ptr(alignment, offset);
    u8 *new_offset = aligned + size;
    if (new_offset > end)
    {
      mem = nullptr;
      return false;
    }

    offset = new_offset;
    mem    = aligned;
    return true;
  }

  [[nodiscard]] bool alloc_zeroed(usize alignment, usize size, u8 *&mem)
  {
    if (size == 0)
    {
      mem = nullptr;
      return true;
    }

    if (!alloc(alignment, size, mem))
    {
      mem = nullptr;
      return false;
    }

    mem::zero(mem, size);
    return true;
  }

  [[nodiscard]] bool realloc(usize alignment, usize old_size, usize new_size,
                             u8 *&mem)
  {
    // if it is the last allocation, just extend the offset
    if (((mem + old_size) == offset) && ((mem + new_size) <= end))
    {
      offset = mem + new_size;
      return true;
    }

    u8 *new_mem;

    if (!alloc(alignment, new_size, new_mem))
    {
      return false;
    }

    mem::copy(Span{mem, old_size}, new_mem);
    dealloc(alignment, mem, old_size);
    mem = new_mem;
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
  [[nodiscard]] constexpr bool nalloc(usize num, T *&mem)
  {
    return alloc(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc_zeroed(usize num, T *&mem)
  {
    return alloc_zeroed(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num, T *&mem)
  {
    return realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                   (u8 *&) mem);
  }

  template <typename T>
  constexpr void ndealloc(T *mem, usize num)
  {
    dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
  }

  [[nodiscard]] AllocatorImpl to_allocator()
  {
    return AllocatorImpl{.self      = (Allocator) this,
                         .interface = &arena_interface};
  }
};

[[nodiscard]] inline Arena to_arena(Span<u8> buffer)
{
  return Arena{
      .begin = buffer.data(), .end = buffer.end(), .offset = buffer.begin()};
}

struct ArenaPoolInterface
{
  static bool alloc(Allocator self, usize alignment, usize size, u8 *&mem);
  static bool alloc_zeroed(Allocator self, usize alignment, usize size,
                           u8 *&mem);
  static bool realloc(Allocator self, usize alignment, usize old_size,
                      usize new_size, u8 *&mem);
  static void dealloc(Allocator self, usize alignment, u8 *mem, usize size);
};

static AllocatorInterface const arena_sub_interface{
    .alloc        = ArenaPoolInterface::alloc,
    .alloc_zeroed = ArenaPoolInterface::alloc_zeroed,
    .realloc      = ArenaPoolInterface::realloc,
    .dealloc      = ArenaPoolInterface::dealloc};

/// @max_num_arenas: maximum number of arenas that can be allocated
/// @min_arena_size: minimum size of each arena allocation, recommended >= 16KB
/// bytes (approx 1 huge memory page). allocations having sizes higher than that
/// will have a dedicated arena.
/// @max_total_size: total maximum size of all allocations performed.
///
struct ArenaPoolCfg
{
  usize max_num_arenas  = USIZE_MAX;
  usize min_arena_size  = PAGE_SIZE;
  usize max_arena_size  = USIZE_MAX;
  usize max_total_size  = USIZE_MAX;
  usize arena_alignment = MAX_STANDARD_ALIGNMENT;
};

/// An Arena Pool is a collection of arenas. All allocations are reset/free-d at
/// once. Allocation, Reallocation, Deallocation, and Reclamation.
/// Memory can be reclaimed in rare cases. i.e. when `realloc` is called with
/// the last allocated memory on the block and the allocation can easily be
/// extended.
///
/// @source: allocation memory source
struct ArenaPool
{
  AllocatorImpl source        = {};
  Arena        *arenas        = nullptr;
  usize         num_arenas    = 0;
  usize         current_arena = 0;
  ArenaPoolCfg  cfg           = {};

  ArenaPool() = default;

  explicit ArenaPool(AllocatorImpl source, ArenaPoolCfg const &cfg = {}) :
      source{source}, cfg{cfg}
  {
  }

  ArenaPool(ArenaPool const &) = delete;

  ArenaPool &operator=(ArenaPool const &) = delete;

  ArenaPool(ArenaPool &&other) :
      source{other.source},
      arenas{other.arenas},
      num_arenas{other.num_arenas},
      current_arena{other.current_arena},
      cfg{other.cfg}
  {
    other.source        = {};
    other.arenas        = nullptr;
    other.num_arenas    = 0;
    other.current_arena = 0;
  }

  ArenaPool &operator=(ArenaPool &&other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) ArenaPool{(ArenaPool &&) other};
    return *this;
  }

  ~ArenaPool()
  {
    uninit();
  }

  void reclaim()
  {
    for (usize i = 0; i < num_arenas; i++)
    {
      arenas[i].reclaim();
    }

    current_arena = 0;
  }

  [[nodiscard]] usize size() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].size();
    }

    return s;
  }

  [[nodiscard]] usize used() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].used();
    }

    return s;
  }

  [[nodiscard]] usize available() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].available();
    }

    return s;
  }

  void uninit()
  {
    for (usize i = num_arenas; i-- > 0;)
    {
      source.dealloc(cfg.arena_alignment, arenas[i].begin, arenas[i].size());
    }
    source.ndealloc(arenas, num_arenas);
  }

  void reset()
  {
    uninit();
    arenas        = nullptr;
    num_arenas    = 0;
    current_arena = 0;
  }

  [[nodiscard]] bool alloc(usize alignment, usize size, u8 *&mem)
  {
    if (size == 0)
    {
      mem = nullptr;
      return true;
    }

    if (size > cfg.max_arena_size)
    {
      mem = nullptr;
      return false;
    }

    for (usize i = current_arena; i < num_arenas; i++)
    {
      if (arenas[i].alloc(alignment, size, mem))
      {
        return true;
      }
    }

    if (num_arenas == cfg.max_num_arenas)
    {
      mem = nullptr;
      return false;
    }

    usize const arena_size = max(size, cfg.min_arena_size);
    if ((this->size() + arena_size) > cfg.max_total_size)
    {
      mem = nullptr;
      return false;
    }

    u8 *arena_mem;

    if (!source.alloc(cfg.arena_alignment, arena_size, arena_mem))
    {
      mem = nullptr;
      return false;
    }

    if (!source.nrealloc(num_arenas, num_arenas + 1, arenas))
    {
      source.dealloc(cfg.arena_alignment, arena_mem, arena_size);
      mem = nullptr;
      return false;
    }

    Arena *arena = new (arenas + num_arenas) Arena{
        .begin = arena_mem, .end = arena_mem + arena_size, .offset = arena_mem};

    current_arena = num_arenas;

    num_arenas++;

    if (!arena->alloc(alignment, size, mem))
    {
      return false;
    }

    return true;
  }

  [[nodiscard]] bool alloc_zeroed(usize alignment, usize size, u8 *&mem)
  {
    if (!alloc(alignment, size, mem))
    {
      return false;
    }
    mem::zero(Span{mem, size});
    return true;
  }

  [[nodiscard]] bool realloc(usize alignment, usize old_size, usize new_size,
                             u8 *&mem)
  {
    if (new_size > cfg.max_arena_size)
    {
      return false;
    }

    if (num_arenas != 0)
    {
      Arena &arena = arenas[current_arena];
      if (arena.offset == (mem + old_size))
      {
        // try to change the allocation if it was the last allocation
        if ((arena.offset + new_size) <= arena.end)
        {
          arena.offset = mem + new_size;
          return true;
        }

        // if only and first allocation on the arena, realloc arena
        if (arena.begin == mem)
        {
          if (!source.realloc(cfg.arena_alignment, arena.size(), new_size,
                              arena.begin))
          {
            return false;
          }
          arena.end    = arena.begin + new_size;
          arena.offset = arena.begin + new_size;
          return true;
        }
      }
    }

    u8 *new_mem;
    if (!alloc(alignment, new_size, new_mem))
    {
      return false;
    }

    mem::copy(Span{mem, old_size}, new_mem);
    dealloc(alignment, mem, old_size);
    mem = new_mem;
    return true;
  }

  void dealloc(usize alignment, u8 *mem, usize size)
  {
    (void) alignment;
    if (mem == nullptr || size == 0 || num_arenas == 0)
    {
      return;
    }

    // we can try to reclaim some memory, although we'd lose alignment padding.
    // best case: if is at end of arena, shrink arena.
    Arena &arena = arenas[current_arena];
    if (arena.begin == mem && arena.offset == (mem + size))
    {
      arena.reclaim();
      if (current_arena != 0)
      {
        current_arena--;
      }
      return;
    }

    if (arena.offset == (mem + size))
    {
      arena.offset = mem;
      return;
    }
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc(usize num, T *&mem)
  {
    return alloc(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nalloc_zeroed(usize num, T *&mem)
  {
    return alloc_zeroed(alignof(T), sizeof(T) * num, (u8 *&) mem);
  }

  template <typename T>
  [[nodiscard]] constexpr bool nrealloc(usize old_num, usize new_num, T *&mem)
  {
    return realloc(alignof(T), sizeof(T) * old_num, sizeof(T) * new_num,
                   (u8 *&) mem);
  }

  template <typename T>
  constexpr void ndealloc(T *mem, usize num)
  {
    dealloc(alignof(T), (u8 *) mem, sizeof(T) * num);
  }

  [[nodiscard]] AllocatorImpl to_allocator()
  {
    return AllocatorImpl{.self      = (Allocator) this,
                         .interface = &arena_sub_interface};
  }
};

}        // namespace ash
