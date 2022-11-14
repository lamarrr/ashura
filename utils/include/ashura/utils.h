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

#define ASR_PANIC(...) ::stx::panic(__VA_ARGS__)

#define ASR_ENSURE(expr, ...)               \
  do {                                      \
    if (!(expr)) ::stx::panic(__VA_ARGS__); \
  } while (false)

#define ASR_ENABLE_DEBUG_CHECKS 1

#if ASR_ENABLE_DEBUG_CHECKS
#define ASR_DEBUG_CODE(...) __VA_ARGS__
#else
#define ASR_DEBUG_CODE(...)
#endif

#if ASR_ENABLE_DEBUG_CHECKS
#define ASR_DEBUG_ENSURE(...) \
  do {                        \
    ASR_ENSURE(__VA_ARGS__);  \
  } while (false)
#else
#define ASR_DEBUG_ENSURE(...) \
  do {                        \
  } while (false)
#endif

#define ASR_MUST_SUCCEED(expr, message)        \
  do {                                         \
    auto ASR_VK_GL_Result = (expr);            \
    if (ASR_VK_GL_Result != VK_SUCCESS)        \
      ::stx::panic(message, ASR_VK_GL_Result); \
  } while (false)

#define ASR_LOG(...)             \
  do {                           \
    ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_IF(expr, ...)                \
  do {                                       \
    if ((expr)) ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_WARN(...)        \
  do {                           \
    ::spdlog::warn(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_ERR(...)          \
  do {                            \
    ::spdlog::error(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_WARN_IF(expr, ...)           \
  do {                                       \
    if ((expr)) ::spdlog::warn(__VA_ARGS__); \
  } while (false)

#define ASR_ERRNUM_CASE(x) \
  case x:                  \
    return #x;

#define ASR_UNREACHABLE() \
  ASR_PANIC("Expected program execution to not reach this state")

namespace asr {

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

}  // namespace asr
