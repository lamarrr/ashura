#pragma once

#include <iostream>

#include "stx/panic.h"

#define IMPL_VLK_ENSURE(expr, ...)        \
  do {                                    \
    if (!(expr)) stx::panic(__VA_ARGS__); \
  } while (false);

#define VLK_ENSURE(expr, ...) IMPL_VLK_ENSURE(expr, __VA_ARGS__)

#define VLK_MUST_SUCCEED(expr, message)                      \
  do {                                                       \
    auto result = (expr);                                    \
    if (result != VK_SUCCESS) stx::panic(message, result); \
  } while (false);

#define VLK_LOG(expr)               \
  do {                              \
    std::cout << expr << std::endl; \
  } while (false);

#define IMPL_VLK_WARN_IF(expr, ...)                                     \
  do {                                                                  \
    if (!(expr)) std::cout << "[WARNING] " << __VA_ARGS__ << std::endl; \
  } while (false);

#define VLK_WARN_IF(expr, ...) IMPL_VLK_WARN_IF(expr, __VA_ARGS__)

#define IMPL_VLK_WARN(...)                                 \
  do {                                                     \
    std::cout << "[WARNING] " << __VA_ARGS__ << std::endl; \
  } while (false);

#define VLK_WARN(...) IMPL_VLK_WARN(__VA_ARGS__)
