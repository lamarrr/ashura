#pragma once

#include "spdlog/spdlog.h"
#include "stx/option.h"
#include "stx/panic.h"

#define ASH_PANIC(...) ::stx::panic(__VA_ARGS__)

#define ASH_CHECK(expr, ...)     \
  do                             \
  {                              \
    if (!(expr))                 \
      ::stx::panic(__VA_ARGS__); \
  } while (false)

#define ASH_ERRNUM_CASE(x) \
  case x:                  \
    return #x;

#define ASH_UNREACHABLE() ASH_PANIC("Expected program execution to not reach this state")

#define AS(type, ...) static_cast<type>(__VA_ARGS__)

#define AS_U8(...) AS(::ash::u8, __VA_ARGS__)
#define AS_U16(...) AS(::ash::u16, __VA_ARGS__)
#define AS_U32(...) AS(::ash::u32, __VA_ARGS__)
#define AS_U64(...) AS(::ash::u64, __VA_ARGS__)

#define AS_I8(...) AS(::ash::i8, __VA_ARGS__)
#define AS_I16(...) AS(::ash::i16, __VA_ARGS__)
#define AS_I32(...) AS(::ash::i32, __VA_ARGS__)
#define AS_I64(...) AS(::ash::i64, __VA_ARGS__)

#define AS_F32(...) AS(::ash::f32, __VA_ARGS__)
#define AS_F64(...) AS(::ash::f64, __VA_ARGS__)

#define ASH_U8_CLAMP(...)              \
  ((__VA_ARGS__) < 0 ? (::ash::u8) 0 : \
                       ((__VA_ARGS__) > 255 ? (::ash::u8) 255 : (::ash::u8)(__VA_ARGS__)))

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
