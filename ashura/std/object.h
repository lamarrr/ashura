#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct Object_T *Object;

/// @param brief Type-erased object
/// @param tag used to recognize the object and whether to touch it or not. the
/// tag must exist for the lifetime of the program. recommended to be made as
/// differentiablen and unique as possible.
/// @param object the wrapped object. its address is always stable.
struct ObjectImpl
{
  Span<char const> tag       = {};
  Object           object    = nullptr;
  void (*uninit)(Object obj) = nullptr;
};

}        // namespace ash