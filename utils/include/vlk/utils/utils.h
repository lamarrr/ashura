#pragma once

#include <type_traits>

#include "spdlog/spdlog.h"
#include "stx/panic.h"
#include "vlk/utils/limits.h"

#define VLK_PANIC(...)         \
  do {                         \
    ::stx::panic(__VA_ARGS__); \
  } while (false)

#define VLK_ENSURE(expr, ...)               \
  do {                                      \
    if (!(expr)) ::stx::panic(__VA_ARGS__); \
  } while (false)

#define VLK_ENABLE_DEBUG_CHECKS 1

#if VLK_ENABLE_DEBUG_CHECKS
#define VLK_DEBUG_CODE(...) __VA_ARGS__
#else
#define VLK_DEBUG_CODE(...)
#endif

#if VLK_ENABLE_DEBUG_CHECKS
#define VLK_DEBUG_ENSURE(expr, ...) \
  do {                              \
    VLK_ENSURE(expr, __VA_ARGS__);  \
  } while (false)
#else
#define VLK_DEBUG_ENSURE(expr, ...) \
  do {                              \
  } while (false)
#endif

#define VLK_MUST_SUCCEED(expr, message)        \
  do {                                         \
    auto VLK_VK_GL_Result = (expr);            \
    if (VLK_VK_GL_Result != VK_SUCCESS)        \
      ::stx::panic(message, VLK_VK_GL_Result); \
  } while (false)

#define VLK_LOG(fmt, ...)               \
  do {                                  \
    ::spdlog::info(fmt, ##__VA_ARGS__); \
  } while (false)

#define VLK_LOG_IF(expr, fmt, ...)                  \
  do {                                              \
    if ((expr)) ::spdlog::info(fmt, ##__VA_ARGS__); \
  } while (false)

#define VLK_WARN(fmt, ...)              \
  do {                                  \
    ::spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false)

#define VLK_WARN_IF(expr, fmt, ...)                 \
  do {                                              \
    if ((expr)) ::spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false)

namespace vlk {

template <typename Container>
bool any_true(Container const &cont) {
  return ::std::any_of(cont.begin(), cont.end(),
                       [](auto const &value) -> bool { return value; });
}

template <typename EnumType>
STX_FORCE_INLINE constexpr std::underlying_type_t<EnumType> enum_ut(
    EnumType a) {
  return static_cast<std::underlying_type_t<EnumType>>(a);
}

template <typename EnumType>
STX_FORCE_INLINE constexpr std::underlying_type_t<EnumType> enum_ut_or(
    EnumType a, EnumType b) {
  return enum_ut(a) | enum_ut(b);
}

template <typename EnumType>
STX_FORCE_INLINE constexpr EnumType enum_or(EnumType a, EnumType b) {
  return static_cast<EnumType>(enum_ut_or(a, b));
}

template <typename EnumType>
STX_FORCE_INLINE constexpr std::underlying_type_t<EnumType> enum_ut_and(
    EnumType a, EnumType b) {
  return enum_ut(a) & enum_ut(b);
}

template <typename EnumType>
STX_FORCE_INLINE constexpr EnumType enum_and(EnumType a, EnumType b) {
  return static_cast<EnumType>(
      static_cast<std::underlying_type_t<EnumType>>(a) &
      static_cast<std::underlying_type_t<EnumType>>(b));
}

STX_FORCE_INLINE constexpr bool f32_eq(float a, float b) {
  return std::abs(a - b) < f32_epsilon;
}

}  // namespace vlk
