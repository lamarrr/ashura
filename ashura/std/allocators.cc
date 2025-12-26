/// SPDX-License-Identifier: MIT
#include "ashura/std/allocators.h"
#include "ashura/std/async.h"
#include "ashura/std/list.h"
#include "ashura/std/mem.h"

namespace ash
{

struct ThreadArena
{
  Layout    arena_layout;
  IArena    arena;
  Allocator upstream;

  ThreadArena(u8 * arena_mem, Layout arena_layout, Allocator upstream) :
    arena_layout{arena_layout},
    arena{arena_mem, arena_mem + arena_layout.size},
    upstream{upstream}
  {
  }

  ThreadArena(Tuple<u8 *, Layout, Allocator> args) :
    ThreadArena{args.v0, args.v1, args.v2}
  {
  }

  ThreadArena(ThreadArena const &)             = delete;
  ThreadArena(ThreadArena &&)                  = delete;
  ThreadArena & operator=(ThreadArena const &) = delete;
  ThreadArena & operator=(ThreadArena &&)      = delete;

  ~ThreadArena()
  {
    upstream->dealloc(arena_layout, arena.begin);
  }
};

struct ThreadArenaHook
{
  ThreadArenaHook * next;
  ThreadArenaHook * prev;
  ThreadArena       v;

  static void push(ThreadArenaHook * hook);

  static void pop(ThreadArenaHook * hook);

  template <typename... Args>
  ThreadArenaHook(Args &&... args) :
    next{nullptr},
    prev{nullptr},
    v{static_cast<Args &&>(args)...}
  {
    push(this);
  }

  ThreadArenaHook(ThreadArenaHook const &)             = delete;
  ThreadArenaHook(ThreadArenaHook &&)                  = delete;
  ThreadArenaHook & operator=(ThreadArenaHook const &) = delete;
  ThreadArenaHook & operator=(ThreadArenaHook &&)      = delete;

  ~ThreadArenaHook()
  {
    pop(this);
  }
};

struct ThreadArenaSinks
{
  ISpinLock             lock;
  List<ThreadArenaHook> sinks{};
};

static ThreadArenaSinks thread_arena_sinks{};

void ThreadArenaHook::push(ThreadArenaHook * hook)
{
  LockGuard guard{thread_arena_sinks.lock};
  thread_arena_sinks.sinks.push_back(hook);
}

void ThreadArenaHook::pop(ThreadArenaHook * hook)
{
  LockGuard guard{thread_arena_sinks.lock};
  thread_arena_sinks.sinks.pop_at(hook);
}

Allocator get_thread_arena_upstream()
{
  return heap_allocator;
}

Layout get_thread_arena_layout()
{
  static constexpr usize CFG_THREAD_ARENA_SIZE = 8_MB;
  return Layout{alignof(max_align_t), CFG_THREAD_ARENA_SIZE};
}

Arena get_thread_arena()
{
  static thread_local ThreadArenaHook thread_arena{
    [arena_allocator = get_thread_arena_upstream(),
     arena_layout    = get_thread_arena_layout()] {
      u8 * arena_mem;
      CHECK(arena_allocator->alloc(arena_layout, arena_mem), "");
      IArena arena{arena_mem, arena_mem + arena_layout.size};
      return Tuple{arena_mem, arena_layout, arena_allocator};
    }()};

  return &thread_arena.v.arena;
}

}    // namespace ash
