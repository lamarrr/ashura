#pragma once

#include <type_traits>

#include "spdlog/spdlog.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/option.h"
#include "stx/panic.h"
#include "stx/struct.h"

// TODO(lamarrr): separate into log utils, error utils, iter utils, and enum
// utils, object utils

#define VLK_PANIC(...) ::stx::panic(__VA_ARGS__)

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

#define VLK_ERR(...)              \
  do {                            \
    ::spdlog::error(__VA_ARGS__); \
  } while (false)

#define VLK_WARN_IF(expr, ...)               \
  do {                                       \
    if ((expr)) ::spdlog::warn(__VA_ARGS__); \
  } while (false)

// TODO(lamarrr): rename
#define VLK_ERRNUM_CASE(x) \
  case x:                  \
    return #x;

/// used for making handle types. they typically store pointers which could be
/// dangerous when copied across structs as this would eventually lead to a
/// double or multiple destruction of the same resource the handle points to.
/// the `target_type` is usually used in conjuction with smart pointers: i.e.
/// shared_ptr, intrusive_ptr, and unique_ptr.
///
/// handles point to resources and are default-able and nullable.
/// their wrapping smart pointers are also inherently nulled by default.
#define VLK_MAKE_HANDLE(target_type)   \
  STX_DEFAULT_CONSTRUCTOR(target_type) \
  STX_DISABLE_COPY(target_type)        \
  STX_DISABLE_MOVE(target_type)

namespace vlk {

template <typename Container>
constexpr bool any_true(Container const &cont) {
  return ::std::any_of(cont.begin(), cont.end(),
                       [](auto const &value) -> bool { return value; });
}

template <typename Container, typename Value>
constexpr bool any_eq(Container const &cont, Value &&value) {
  return ::std::find(cont.begin(), cont.end(), static_cast<Value &&>(value)) !=
         cont.end();
}

template <typename Container, typename UnaryPredicate>
constexpr bool any(Container const &cont, UnaryPredicate &&predicate) {
  return ::std::any_of(cont.begin(), cont.end(),
                       static_cast<UnaryPredicate &&>(predicate));
}

constexpr bool f32_approx_eq(float a, float b) {
  return std::abs(a - b) < stx::f32_epsilon;
}

template <typename Target, typename Source>
STX_FORCE_INLINE stx::Option<stx::Ref<Target>> upcast(Source &source) {
  auto *dyn_ptr = dynamic_cast<Target *>(&source);
  if (dyn_ptr == nullptr) {
    return stx::None;
  } else {
    return stx::Some(stx::Ref<Target>(*dyn_ptr));
  }
}

}  // namespace vlk
