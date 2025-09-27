/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"

namespace ash
{

/// @param begin where the memory block begins
/// @param end one byte past the block
/// @param offset end of the last allocation, must be set to {begin}
/// @param alignment actual alignment requested from allocator
struct Arena final : IAllocator
{
  u8 *  begin;
  u8 *  end;
  u8 *  offset;
  usize allocated;

  constexpr Arena() :
    IAllocator{},
    begin{nullptr},
    end{nullptr},
    offset{nullptr},
    allocated{0}
  {
  }

  constexpr Arena(u8 * begin, u8 * end) :
    IAllocator{},
    begin{begin},
    end{end},
    offset{begin},
    allocated{0}
  {
  }

  constexpr Arena(i8 * begin, i8 * end) :
    Arena{reinterpret_cast<u8 *>(begin), reinterpret_cast<u8 *>(end)}
  {
  }

  constexpr Arena(void * begin, void * end) :
    Arena{static_cast<u8 *>(begin), static_cast<u8 *>(end)}
  {
  }

  constexpr Arena(Span<u8> buffer) : Arena{buffer.pbegin(), buffer.pend()}
  {
  }

  constexpr Arena(Span<i8> buffer) : Arena{buffer.pbegin(), buffer.pend()}
  {
  }

  template <usize N>
  constexpr Arena(u8 (&buffer)[N]) : Arena{buffer, buffer + N}
  {
  }

  template <usize N>
  constexpr Arena(i8 (&buffer)[N]) : Arena{buffer, buffer + N}
  {
  }

  constexpr Arena(Arena const &)             = delete;
  constexpr Arena(Arena &&)                  = delete;
  constexpr Arena & operator=(Arena const &) = delete;
  constexpr Arena & operator=(Arena &&)      = delete;
  constexpr ~Arena()                         = default;

  [[nodiscard]] constexpr usize capacity() const
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
    offset    = begin;
    allocated = 0;
  }

  constexpr void try_reclaim()
  {
    if (allocated == 0)
    {
      reclaim();
    }
  }

  constexpr bool contains(Layout layout, u8 * mem) const
  {
    return (begin <= mem) && (end >= (mem + layout.size));
  }

  [[nodiscard]] virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (layout.size == 0)
    {
      mem = nullptr;
      return true;
    }

    u8 * aligned    = align_up(layout.alignment, offset);
    u8 * new_offset = aligned + layout.size;
    if (new_offset > end)
    {
      mem = nullptr;
      return false;
    }

    offset = new_offset;
    mem    = aligned;
    allocated += layout.size;
    return true;
  }

  [[nodiscard]] virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!alloc(layout, mem))
    {
      mem = nullptr;
      return false;
    }

    mem::zero(mem, layout.size);

    return true;
  }

  [[nodiscard]] virtual bool realloc(Layout layout, usize new_size,
                                     u8 *& mem) override
  {
    // if it is the last allocation and within capacity, just extend the offset
    if (((mem + layout.size) == offset) && ((mem + new_size) <= end))
    {
      offset = mem + new_size;
      allocated -= layout.size;
      try_reclaim();
      allocated += new_size;
      return true;
    }

    u8 * new_mem;

    if (!alloc(layout.with_size(new_size), new_mem))
    {
      return false;
    }

    mem::copy(Span{mem, layout.size}, new_mem);
    dealloc(layout, mem);
    mem = new_mem;
    return true;
  }

  virtual void dealloc(Layout layout, u8 * mem) override
  {
    // best-case: stack allocation, we can free memory by adjusting to the
    // beginning of allocation
    if ((mem + layout.size) == offset)
    {
      offset -= layout.size;
    }

    allocated -= layout.size;
    try_reclaim();
  }

  constexpr Allocator ref()
  {
    return Allocator{*this};
  }
};

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
struct ArenaPool final : IAllocator
{
  Allocator    source        = {};
  Arena *      arenas        = nullptr;
  usize        num_arenas    = 0;
  usize        current_arena = 0;
  ArenaPoolCfg cfg           = {};

  explicit ArenaPool(Allocator source = {}, ArenaPoolCfg const & cfg = {}) :
    IAllocator{},
    source{source},
    cfg{cfg}
  {
  }

  ArenaPool(ArenaPool const &) = delete;

  ArenaPool & operator=(ArenaPool const &) = delete;

  ArenaPool(ArenaPool && other) = delete;

  ArenaPool & operator=(ArenaPool && other) = delete;

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

  [[nodiscard]] usize capacity() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].capacity();
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
      source->dealloc(
        Layout{.alignment = cfg.arena_alignment, .size = arenas[i].capacity()},
        arenas[i].begin);
    }
    source->ndealloc(num_arenas, arenas);
  }

  void reset()
  {
    uninit();
    arenas        = nullptr;
    num_arenas    = 0;
    current_arena = 0;
  }

  [[nodiscard]] virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (layout.size == 0)
    {
      mem = nullptr;
      return true;
    }

    if (layout.size > cfg.max_arena_size)
    {
      mem = nullptr;
      return false;
    }

    for (usize i = current_arena; i < num_arenas; i++)
    {
      if (arenas[i].alloc(layout, mem))
      {
        return true;
      }
    }

    if (num_arenas == cfg.max_num_arenas)
    {
      mem = nullptr;
      return false;
    }

    Layout arena_layout{cfg.arena_alignment,
                        max(layout.size, cfg.min_arena_size)};

    if ((capacity() + arena_layout.size) > cfg.max_total_size)
    {
      mem = nullptr;
      return false;
    }

    u8 * arena_mem;

    if (!source->alloc(arena_layout, arena_mem))
    {
      mem = nullptr;
      return false;
    }

    if (!source->nrealloc(num_arenas, num_arenas + 1, arenas))
    {
      source->dealloc(arena_layout, arena_mem);
      mem = nullptr;
      return false;
    }

    Arena * arena =
      new (arenas + num_arenas) Arena{arena_mem, arena_mem + arena_layout.size};

    current_arena = num_arenas;

    num_arenas++;

    if (!arena->alloc(layout, mem))
    {
      return false;
    }

    return true;
  }

  [[nodiscard]] virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!alloc(layout, mem))
    {
      return false;
    }
    mem::zero(Span{mem, layout.size});
    return true;
  }

  [[nodiscard]] virtual bool realloc(Layout layout, usize new_size,
                                     u8 *& mem) override
  {
    if (new_size > cfg.max_arena_size)
    {
      return false;
    }

    if (num_arenas != 0)
    {
      Arena & arena = arenas[current_arena];
      if (arena.offset == (mem + layout.size))
      {
        // extend the arena offset if the allocation was the last one and it is within capacity
        if ((arena.offset + new_size) <= arena.end)
        {
          arena.offset = mem + new_size;
          return true;
        }

        // if only and first allocation on the arena, realloc arena
        if (arena.begin == mem)
        {
          if (!source->realloc(Layout{.alignment = cfg.arena_alignment,
                                      .size      = arena.capacity()},
                               new_size, arena.begin))
          {
            return false;
          }
          arena.end    = arena.begin + new_size;
          arena.offset = arena.begin + new_size;
          return true;
        }
      }
    }

    u8 * new_mem;
    if (!alloc(layout.with_size(new_size), new_mem))
    {
      return false;
    }

    mem::copy(Span{mem, layout.size}, new_mem);
    dealloc(layout, mem);
    mem = new_mem;
    return true;
  }

  virtual void dealloc(Layout layout, u8 * mem) override
  {
    if (mem == nullptr || layout.size == 0 || num_arenas == 0)
    {
      return;
    }

    // we can try to reclaim some memory.
    // best case: stack allocation, if it is at end of arena, adjust arena offset
    Arena & arena = arenas[current_arena];
    if (arena.begin == mem && arena.offset == (mem + layout.size))
    {
      arena.reclaim();
      if (current_arena != 0)
      {
        current_arena--;
      }
      return;
    }

    if (arena.offset == (mem + layout.size))
    {
      arena.offset = mem;
      return;
    }
  }

  // [ ] implement
  void shrink();

  constexpr Allocator ref()
  {
    return Allocator{*this};
  }
};

struct FallbackAllocator : IAllocator
{
  Arena     arena;
  Allocator fallback;

  constexpr FallbackAllocator(Span<u8> arena, Allocator fallback) :
    IAllocator{},
    arena{arena},
    fallback{fallback}
  {
  }

  constexpr FallbackAllocator(FallbackAllocator const &)             = delete;
  constexpr FallbackAllocator(FallbackAllocator &&)                  = delete;
  constexpr FallbackAllocator & operator=(FallbackAllocator const &) = delete;
  constexpr FallbackAllocator & operator=(FallbackAllocator &&)      = delete;
  constexpr ~FallbackAllocator()                                     = default;

  virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (arena.alloc(layout, mem))
    {
      return true;
    }

    return fallback->alloc(layout, mem);
  }

  virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!arena.zalloc(layout, mem))
    {
      return false;
    }

    return fallback->zalloc(layout, mem);
  }

  virtual bool realloc(Layout layout, usize new_size, u8 *& mem) override
  {
    if (mem == nullptr)
    {
      if (arena.alloc(layout.with_size(new_size), mem))
      {
        return true;
      }

      return fallback->alloc(layout.with_size(new_size), mem);
    }

    if (arena.contains(layout, mem))
    {
      if (arena.realloc(layout, new_size, mem))
      {
        return true;
      }

      u8 * new_mem;

      if (!fallback->alloc(layout.with_size(new_size), new_mem))
      {
        return false;
      }

      mem::copy(Span{mem, layout.size}, new_mem);
      arena.dealloc(layout, mem);

      mem = new_mem;

      return true;
    }
    else
    {
      return fallback->realloc(layout, new_size, mem);
    }
  }

  virtual void dealloc(Layout layout, u8 * mem) override
  {
    if (mem == nullptr || layout.size == 0)
    {
      return;
    }

    if (arena.contains(layout, mem))
    {
      arena.dealloc(layout, mem);
      return;
    }

    return fallback->dealloc(layout, mem);
  }

  constexpr Allocator ref()
  {
    return Allocator{*this};
  }
};

}    // namespace ash
