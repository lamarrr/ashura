#pragma once

#include "ashura/primitives.h"

namespace ash
{
typedef struct Allocator_T       *Allocator;
typedef struct AllocatorInterface AllocatorInterface;
typedef struct AllocatorImpl      AllocatorImpl;

struct AllocatorInterface
{
  void *(*allocate)(Allocator self, usize size, usize alignment)              = nullptr;
  void *(*reallocate)(Allocator self, void *old, usize size, usize alignment) = nullptr;
  void (*deallocate)(Allocator self, void *memory)                            = nullptr;
  void (*release)(Allocator self)                                             = nullptr;
};

struct AllocatorImpl
{
  Allocator                 self      = nullptr;
  AllocatorInterface const *interface = nullptr;

  [[nodiscard]] void *allocate(usize size, usize alignment) const
  {
    return interface->allocate(self, size, alignment);
  }

  [[nodiscard]] void *reallocate(void *old, usize size, usize alignment) const
  {
    return interface->reallocate(self, old, size, alignment);
  }

  void deallocate(void *memory) const
  {
    return interface->deallocate(self, memory);
  }

  void release()
  {
    return interface->release(self);
  }
};

}        // namespace ash
