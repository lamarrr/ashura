#pragma once

#include "spdlog/spdlog.h"
#include "stx/option.h"
#include "stx/panic.h"

#define ASH_PANIC(...) ::stx::panic(__VA_ARGS__)

#define ASH_UNIMPLEMENTED() \
  ASH_PANIC("Reached unimplemented code path. submit a bug report!")

#define ASH_CHECK(expr, ...)     \
  do                             \
  {                              \
    if (!(expr))                 \
    {                            \
      ::stx::panic(__VA_ARGS__); \
    }                            \
  } while (false)

#define ASH_ERRNUM_CASE(x) \
  case x:                  \
    return #x;

#define ASH_UNREACHABLE() \
  ASH_PANIC("Expected Program Execution Flow To Not Reach This State")

#define AS(type, ...) static_cast<type>(__VA_ARGS__)

namespace ash
{

template <typename Target, typename Source>
STX_FORCE_INLINE stx::Option<Target *> upcast(Source &source)
{
  Target *ptr = dynamic_cast<Target *>(&source);
  if (ptr == nullptr)
  {
    return stx::None;
  }
  else
  {
    return stx::Some(static_cast<Target *>(ptr));
  }
}

}        // namespace ash
