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
#define VLK_DEBUG_ENSURE(...) \
  do {                        \
    VLK_ENSURE(__VA_ARGS__);  \
  } while (false)
#else
#define VLK_DEBUG_ENSURE(...) \
  do {                        \
  } while (false)
#endif

#define VLK_MUST_SUCCEED(expr, message)        \
  do {                                         \
    auto VLK_VK_GL_Result = (expr);            \
    if (VLK_VK_GL_Result != VK_SUCCESS)        \
      ::stx::panic(message, VLK_VK_GL_Result); \
  } while (false)

#define VLK_LOG(...)             \
  do {                           \
    ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define VLK_LOG_IF(expr, ...)                \
  do {                                       \
    if ((expr)) ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define VLK_WARN(...)            \
  do {                           \
    ::spdlog::warn(__VA_ARGS__); \
  } while (false)

#define VLK_WARN_IF(expr, ...)               \
  do {                                       \
    if ((expr)) ::spdlog::warn(__VA_ARGS__); \
  } while (false)

namespace vlk {

template <typename Container>
constexpr bool any_true(Container const &cont) {
  return ::std::any_of(cont.begin(), cont.end(),
                       [](auto const &value) -> bool { return value; });
}

template <typename Container, typename UnaryPredicate>
constexpr bool any(Container const &cont, UnaryPredicate &&predicate) {
  return ::std::any_of(cont.begin(), cont.end(),
                       std::forward<UnaryPredicate &&>(predicate));
}

template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_ut(EnumType a) {
  return static_cast<std::underlying_type_t<EnumType>>(a);
}

template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_ut_or(EnumType a, EnumType b) {
  return enum_ut(a) | enum_ut(b);
}

template <typename EnumType>
constexpr EnumType enum_or(EnumType a, EnumType b) {
  return static_cast<EnumType>(enum_ut_or(a, b));
}

template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_ut_and(EnumType a, EnumType b) {
  return enum_ut(a) & enum_ut(b);
}

template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_ut_toggle(EnumType a) {
  return ~enum_ut(a);
}

template <typename EnumType>
constexpr EnumType enum_toggle(EnumType a) {
  return static_cast<EnumType>(enum_ut_toggle(a));
}

template <typename EnumType>
constexpr EnumType enum_and(EnumType a, EnumType b) {
  return static_cast<EnumType>(enum_ut_and(a, b));
}

#define VLK_DEFINE_ENUM_BIT_OPS(enum_identifier)                              \
  constexpr enum_identifier operator|(enum_identifier a, enum_identifier b) { \
    return vlk::enum_or(a, b);                                                \
  }                                                                           \
  constexpr enum_identifier operator~(enum_identifier a) {                    \
    return vlk::enum_toggle(a);                                               \
  }                                                                           \
                                                                              \
  constexpr enum_identifier &operator|=(enum_identifier &a,                   \
                                        enum_identifier b) {                  \
    a = a | b;                                                                \
    return a;                                                                 \
  }                                                                           \
                                                                              \
  constexpr enum_identifier operator&(enum_identifier a, enum_identifier b) { \
    return vlk::enum_and(a, b);                                               \
  }                                                                           \
                                                                              \
  constexpr enum_identifier &operator&=(enum_identifier &a,                   \
                                        enum_identifier b) {                  \
    a = a & b;                                                                \
    return a;                                                                 \
  }

constexpr bool f32_eq(float a, float b) {
  return std::abs(a - b) < f32_epsilon;
}

template <typename Target, typename Source>
STX_FORCE_INLINE Target upcast(Source &source) {
  static_assert(std::is_lvalue_reference_v<Target>);
  auto *const dyn_ptr =
      dynamic_cast<std::remove_reference_t<Target> *>(&source);
  VLK_ENSURE(dyn_ptr != nullptr, "Dynamic upcast failed");
  return *dyn_ptr;
}

}  // namespace vlk
