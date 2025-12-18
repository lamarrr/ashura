/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/list.h"
#include "ashura/std/mem.h"

namespace ash
{

typedef struct IArena * Arena;

struct IArena final : IAllocator
{
  /// @brief where the memory block begins
  u8 * begin;

  /// @brief one byte past the block
  u8 * end;

  /// @brief end of the last allocation
  u8 * offset;

  /// @brief total allocated bytes
  usize allocated;

  constexpr IArena() :
    IAllocator{},
    begin{nullptr},
    end{nullptr},
    offset{nullptr},
    allocated{0}
  {
  }

  constexpr IArena(u8 * begin, u8 * end, u8 * offset, usize allocated) :
    IAllocator{},
    begin{begin},
    end{end},
    offset{offset},
    allocated{allocated}
  {
  }

  constexpr IArena(u8 * begin, u8 * end) :
    IAllocator{},
    begin{begin},
    end{end},
    offset{begin},
    allocated{0}
  {
  }

  constexpr IArena(i8 * begin, i8 * end) :
    IArena{reinterpret_cast<u8 *>(begin), reinterpret_cast<u8 *>(end)}
  {
  }

  constexpr IArena(void * begin, void * end) :
    IArena{static_cast<u8 *>(begin), static_cast<u8 *>(end)}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  constexpr IArena(Span<u8> memory) : IArena{memory.pbegin(), memory.pend()}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  constexpr IArena(Span<i8> memory) : IArena{memory.pbegin(), memory.pend()}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  template <usize N>
  constexpr IArena(u8 (&memory)[N]) : IArena{memory, memory + N}
  {
  }

  /// @brief Create arena using pre-allocated memory block
  /// @param buffer memory to block to use
  template <usize N>
  constexpr IArena(i8 (&memory)[N]) : IArena{memory, memory + N}
  {
  }

  constexpr IArena(IArena const &)              = delete;
  constexpr IArena & operator=(IArena const &)  = delete;
  constexpr IArena(IArena && other)             = delete;
  constexpr IArena & operator=(IArena && other) = delete;
  constexpr ~IArena()                           = default;

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

typedef struct IArenaPool * ArenaPool;

/// @brief An IArena Pool is a collection of arenas. All allocations are reset/free-d at
/// once.
/// Memory can be reclaimed in rare cases. i.e. when `realloc` is called with
/// the last allocated memory on the block and the allocation can easily be
/// extended.
///
struct IArenaPool final : IAllocator
{
  /// @brief allocation memory source
  Allocator source_;

  struct ArenaNode
  {
    ArenaNode * prev = nullptr;
    ArenaNode * next = nullptr;
    IArena      v;
  };

  /// @brief list of arenas
  List<ArenaNode> arenas_;

  /// @brief configuration of the arena
  ArenaPoolCfg cfg_ = {};

  /// @brief Create an arena pool from an upstream memory source and its configuration
  /// @param source upstream memory allocator
  /// @param cfg the pool memory configuration
  explicit IArenaPool(Allocator source, ArenaPoolCfg const & cfg) :
    IAllocator{},
    source_{source},
    cfg_{cfg}
  {
  }

  IArenaPool(IArenaPool const &) = delete;

  IArenaPool & operator=(IArenaPool const &) = delete;

  IArenaPool(IArenaPool && other) = delete;

  IArenaPool & operator=(IArenaPool && other) = delete;

  ~IArenaPool()
  {
    uninit();
  }

  // [ ] implement
  void shrink()
  {
  }

  /// @brief force-reclaim all allocated memory on the pool
  void reclaim()
  {
    for (auto & arena : arenas_)
    {
      arena.v.reclaim();
    }
  }

  /// @brief get the total capacity of the pool
  [[nodiscard]] usize capacity() const
  {
    usize s = 0;
    for (auto & arena : arenas_)
    {
      s += arena.v.capacity();
    }

    return s;
  }

  /// @brief get the total memory usage of the pool out of its capacity
  [[nodiscard]] usize used() const
  {
    usize s = 0;
    for (auto & arena : arenas_)
    {
      s += arena.v.used();
    }

    return s;
  }

  /// @brief get the available capacity of the pool
  [[nodiscard]] usize available() const
  {
    usize s = 0;
    for (auto & arena : arenas_)
    {
      s += arena.v.available();
    }

    return s;
  }

  void uninit()
  {
    auto it = arenas_.pop_front();

    while (it != nullptr)
    {
      auto layout =
        Layout{.alignment = cfg_.arena_alignment, .size = it->v.capacity()};
      source_->dealloc(layout, it->v.begin);
      source_->ndealloc(1, it);
      it = arenas_.pop_front();
    };
  }

  /// @brief reset all allocations and free all memory
  void reset()
  {
    uninit();
    arenas_ = {};
  }

  /// @copydoc IAllocator::alloc
  [[nodiscard]] virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (layout.size == 0)
    {
      mem = nullptr;
      return true;
    }

    if (layout.size > cfg_.max_arena_size)
    {
      mem = nullptr;
      return false;
    }

    for (auto & arena : arenas_)
    {
      if (arena.v.alloc(layout, mem))
      {
        return true;
      }
    }

    usize num_arenas = 0;

    for (auto & _ : arenas_)
    {
      num_arenas++;
    }

    if (num_arenas == cfg_.max_num_arenas)
    {
      mem = nullptr;
      return false;
    }

    ArenaNode * arena;

    if (!source_->nalloc(1, arena))
    {
      mem = nullptr;
      return false;
    }

    Layout arena_layout{cfg_.arena_alignment,
                        max(layout.size, cfg_.min_arena_size)};

    if ((capacity() + arena_layout.size) > cfg_.max_total_size)
    {
      mem = nullptr;
      source_->ndealloc(1, arena);
      return false;
    }

    u8 * arena_mem;

    if (!source_->alloc(arena_layout, arena_mem))
    {
      mem = nullptr;
      source_->ndealloc(1, arena);
      return false;
    }

    new (arena) ArenaNode{
      .v{arena_mem, arena_mem + arena_layout.size}
    };

    arenas_.push_back(arena);

    if (!arena->v.alloc(layout, mem)) [[unlikely]]
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
    if (new_size > cfg_.max_arena_size)
    {
      return false;
    }

    for (auto & arena : arenas_)
    {
      if (arena.v.contains(layout, mem))
      {
        // extend the arena offset if the allocation was the last one and it is within capacity
        if (arena.v.realloc(layout, new_size, mem))
        {
          return true;
        }

        // if only and first allocation on the arena, realloc arena
        if (arena.v.begin == mem && arena.v.offset == (mem + layout.size))
        {
          if (!source_->realloc(Layout{.alignment = cfg_.arena_alignment,
                                       .size      = arena.v.capacity()},
                                new_size, arena.v.begin))
          {
            return false;
          }

          arena.v.end    = arena.v.begin + new_size;
          arena.v.offset = arena.v.begin + new_size;
          return true;
        }

        break;
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
    if (mem == nullptr || layout.size == 0 || arenas_.is_empty())
    {
      return;
    }

    // we can try to reclaim some memory.
    // best case: stack allocation, if it is at end of arena, adjust arena offset
    // [ ] iterate from back to front
    for (auto & arena : arenas_)
    {
      if (arena.v.contains(layout, mem))
      {
        arena.v.dealloc(layout, mem);
        break;
      }
    }
  }

  constexpr Allocator ref()
  {
    return Allocator{*this};
  }
};

/// @brief An allocator that attempts to use a fast-path allocator if possible,
/// but falls back to an upstream and possibly slow-path allocator otherwise.
struct IFallbackAllocator : IAllocator
{
  Arena     arena;
  /// @brief the fallback upstream allocator
  Allocator fallback;

  /// @brief Construct a `IFallbackAllocator` from a preallocated memory block
  /// and a fallback allocator
  /// @param arena pre-allocated arena to allocate on the fast path for
  /// @param fallback the fallback upstream allocator
  constexpr IFallbackAllocator(Arena arena, Allocator fallback) :
    IAllocator{},
    arena{arena},
    fallback{fallback}
  {
  }

  constexpr IFallbackAllocator(IFallbackAllocator const &) = delete;
  constexpr IFallbackAllocator(IFallbackAllocator &&)      = default;
  constexpr IFallbackAllocator & operator=(IFallbackAllocator const &) = delete;
  constexpr IFallbackAllocator & operator=(IFallbackAllocator &&) = default;
  constexpr ~IFallbackAllocator()                                 = default;

  /// @copydoc IAllocator::alloc
  virtual bool alloc(Layout layout, u8 *& mem) override
  {
    if (arena->alloc(layout, mem))
    {
      return true;
    }

    return fallback->alloc(layout, mem);
  }

  /// @copydoc IAllocator::zalloc
  virtual bool zalloc(Layout layout, u8 *& mem) override
  {
    if (!arena->zalloc(layout, mem))
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
      if (arena->alloc(layout.with_size(new_size), mem))
      {
        return true;
      }

      return fallback->alloc(layout.with_size(new_size), mem);
    }

    if (arena->contains(layout, mem))
    {
      if (arena->realloc(layout, new_size, mem))
      {
        return true;
      }

      u8 * new_mem;

      if (!fallback->alloc(layout.with_size(new_size), new_mem))
      {
        return false;
      }

      mem::copy(Span{mem, layout.size}, new_mem);
      arena->dealloc(layout, mem);

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

    if (arena->contains(layout, mem))
    {
      arena->dealloc(layout, mem);
      return;
    }

    return fallback->dealloc(layout, mem);
  }

  constexpr Allocator ref()
  {
    return Allocator{*this};
  }
};

Allocator get_thread_arena_upstream();

Layout get_thread_arena_layout();

Arena get_thread_arena();

}    // namespace ash
