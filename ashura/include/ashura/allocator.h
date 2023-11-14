#pragma once

#include "ashura/primitives.h"

namespace ash
{
typedef void *(*PFN_allocate)(void *data, usize size, usize alignment);
typedef void *(*PFN_reallocate)(void *data, void *old, usize size, usize aligment);
typedef void (*PFN_deallocate)(void *data, void *memory);
typedef void (*PFN_release)(void *data);

struct AllocationCallbacks
{
  void          *data       = nullptr;
  PFN_allocate   allocate   = nullptr;
  PFN_reallocate reallocate = nullptr;
  PFN_deallocate deallocate = nullptr;
  PFN_release    release    = nullptr;
};

}        // namespace ash
