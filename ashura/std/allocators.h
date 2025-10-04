/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"

namespace ash
{

struct Arena final : IAllocator
{
  /// @brief where the memory block begins
  u8 * begin;

  /// @brief one byte past the block
  u8 * end;

  /// @brief end of the last allocation, must be set to {begin}
  u8 * offset;

  /// @brief actual alignment requested from allocator
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

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  constexpr Arena(Span<u8> memory) : Arena{memory.pbegin(), memory.pend()}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  constexpr Arena(Span<i8> memory) : Arena{memory.pbegin(), memory.pend()}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  template <usize N>
  constexpr Arena(u8 (&memory)[N]) : Arena{memory, memory + N}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  template <usize N>
  constexpr Arena(i8 (&memory)[N]) : Arena{memory, memory + N}
  {
  }

  constexpr Arena(Arena const &)             = delete;
  constexpr Arena(Arena &&)                  = delete;
  constexpr Arena & operator=(Arena const &) = delete;
  constexpr Arena & operator=(Arena &&)      = delete;
  constexpr ~Arena()                         = default;

  /// @brief Total capacity of the arena in bytes
  [[nodiscard]] constexpr usize capacity() const
  {
    return end - begin;
  }

  /// @brief total bytes used in the arena
  [[nodiscard]] constexpr usize used() const
  {
    return offset - begin;
  }

  /// @brief total bytes available in the arena
  [[nodiscard]] constexpr usize available() const
  {
    return end - offset;
  }

  /// @brief force reclaim all allocated memory
  constexpr void reclaim()
  {
    offset    = begin;
    allocated = 0;
  }

  /// @brief try to reclaim all allocated memory if there's no active allocation
  /// @returns true if successful
  constexpr bool try_reclaim()
  {
    if (allocated != 0)
    {
      return false;
    }

    reclaim();
    return true;
  }

  /// @brief check if the arena contains a memory region
  constexpr bool contains(Layout layout, u8 * mem) const
  {
    return (begin <= mem) && (end >= (mem + layout.size));
  }

  /// @copydoc IAllocator::alloc
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

  /// @copydoc IAllocator::zalloc
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

  /// @copydoc IAllocator::realloc
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

  /// @copydoc IAllocator::dealloc
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

struct ArenaPoolCfg
{
  /// @brief maximum number of arenas that can be allocated
  usize max_num_arenas = USIZE_MAX;

  /// @brief minimum size of each arena allocation
  usize min_arena_size = PAGE_SIZE;

  /// @brief minimum size of each arena allocation, recommended >= 16KB
  /// bytes (approx 1 huge memory page). allocations having sizes higher than that
  /// will have a dedicated arena.
  usize max_arena_size = USIZE_MAX;

  /// @brief total maximum size of all allocations performed.
  usize max_total_size = USIZE_MAX;

  /// @brief alignment of each arena allocation
  usize arena_alignment = MAX_STANDARD_ALIGNMENT;
};

/// @brief An Arena Pool is a collection of arenas. All allocations are reset/free-d at
/// once.
/// Memory can be reclaimed in rare cases. i.e. when `realloc` is called with
/// the last allocated memory on the block and the allocation can easily be
/// extended.
///
struct ArenaPool final : IAllocator
{
  /// @brief allocation memory source
  Allocator source = {};

  /// @brief allocated arenas
  Arena * arenas     = nullptr;
  usize   num_arenas = 0;

  /// @brief currently active arena
  usize current_arena = 0;

  /// @brief configuration of the arena
  ArenaPoolCfg cfg = {};

  /// @brief Create an arena pool from an upstream memory source and its configuration
  /// @param source upstream memory allocator
  /// @param cfg the pool memory configuration
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

  /// @brief force-reclaim all allocated memory on the pool
  void reclaim()
  {
    for (usize i = 0; i < num_arenas; i++)
    {
      arenas[i].reclaim();
    }

    current_arena = 0;
  }

  /// @brief get the total capacity of the pool
  [[nodiscard]] usize capacity() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].capacity();
    }

    return s;
  }

  /// @brief get the total memory usage of the pool out of its capacity
  [[nodiscard]] usize used() const
  {
    usize s = 0;
    for (usize i = 0; i < num_arenas; i++)
    {
      s += arenas[i].used();
    }

    return s;
  }

  /// @brief get the available capacity of the pool
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

  /// @brief reset all allocations and free all memory
  void reset()
  {
    uninit();
    arenas        = nullptr;
    num_arenas    = 0;
    current_arena = 0;
  }

  /// @copydoc IAllocator::alloc
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

  /// @copydoc IAllocator::zalloc
  [[nodiscard]] virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!alloc(layout, mem))
    {
      return false;
    }
    mem::zero(Span{mem, layout.size});
    return true;
  }

  /// @copydoc IAllocator::realloc
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

  /// @copydoc IAllocator::dealloc
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

/// @brief An allocator that attempts to use a fast-path allocator if possible,
/// but falls back to an upstream and possibly slow-path allocator otherwise.
struct FallbackAllocator : IAllocator
{
  /// @brief pre-allocated arena to allocate on the fast path for
  Arena     arena;
  /// @brief the fallback upstream allocator
  Allocator fallback;

  /// @brief Construct a `FallbackAllocator` from a preallocated memory block
  /// and a fallback allocator
  /// @param arena pre-allocated arena to allocate on the fast path for
  /// @param fallback the fallback upstream allocator
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

  /// @copydoc IAllocator::alloc
  virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (arena.alloc(layout, mem))
    {
      return true;
    }

    return fallback->alloc(layout, mem);
  }

  /// @copydoc IAllocator::zalloc
  virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!arena.zalloc(layout, mem))
    {
      return false;
    }

    return fallback->zalloc(layout, mem);
  }

  /// @copydoc IAllocator::realloc
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

  /// @copydoc IAllocator::dealloc
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
