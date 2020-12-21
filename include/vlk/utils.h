#pragma once

#include <iostream>

#include "spdlog/spdlog.h"
#include "stx/panic.h"
#include "vlk/errors.h"

#define VLK_ENSURE(expr, ...)             \
  do {                                    \
    if (!(expr)) stx::panic(__VA_ARGS__); \
  } while (false);

#define VLK_MUST_SUCCEED(expr, message)                                        \
  do {                                                                         \
    auto VLK_VK_GL_Result = (expr);                                            \
    if (VLK_VK_GL_Result != VK_SUCCESS) stx::panic(message, VLK_VK_GL_Result); \
  } while (false);

#define VLK_LOG(fmt, ...)             \
  do {                                \
    spdlog::info(fmt, ##__VA_ARGS__); \
  } while (false);

#define VLK_WARN(fmt, ...)            \
  do {                                \
    spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false);

#define VLK_WARN_IF(expr, fmt, ...)               \
  do {                                            \
    if ((expr)) spdlog::warn(fmt, ##__VA_ARGS__); \
  } while (false);

template <typename Container>
bool any_true(Container const& cont) {
  return std::any_of(cont.begin(), cont.end(),
                     [](auto value) -> bool { return value; });
}
