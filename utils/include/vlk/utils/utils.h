#pragma once

#include "spdlog/spdlog.h"
#include "stx/panic.h"

#define VLK_ENSURE(expr, ...)               \
  do {                                      \
    if (!(expr)) ::stx::panic(__VA_ARGS__); \
  } while (false);

#define VLK_ENABLE_DEBUG_CHECKS 1

#if VLK_ENABLE_DEBUG_CHECKS
#define VLK_DEBUG_CODE(...) __VA_ARGS__
#else
#define VLK_DEBUG_CODE(...)
#endif

#if VLK_ENABLE_DEBUG_CHECKS
#define VLK_DEBUG_ENSURE(expr, ...) \
  do {                              \
    VLK_ENSURE(expr, __VA_ARGS__)   \
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
  } while (false);

#define VLK_LOG(fmt, ...)               \
  do {                                  \
    ::spdlog::info(fmt, ##__VA_ARGS__); \
  } while (false);

#define VLK_LOG_IF(expr, fmt, ...)                  \
  do {                                              \
    if ((expr)) ::spdlog::info(fmt, ##__VA_ARGS__); \
  } while (false);

#define VLK_WARN(fmt, ...)              \
  do {                                  \
    ::spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false);

#define VLK_WARN_IF(expr, fmt, ...)                 \
  do {                                              \
    if ((expr)) ::spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false);

namespace vlk {

template <typename Container>
bool any_true(Container const& cont) {
  return ::std::any_of(cont.begin(), cont.end(),
                       [](auto value) -> bool { return value; });
}

}  // namespace vlk
