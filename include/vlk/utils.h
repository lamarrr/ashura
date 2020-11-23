#pragma once

#include <iostream>

#include "stx/panic.h"

#define IMPL_VLK_ENSURE(expr, ...)        \
  do {                                    \
    if (!(expr)) stx::panic(__VA_ARGS__); \
  } while (false);

#define VLK_ENSURE(expr, ...) IMPL_VLK_ENSURE(expr, __VA_ARGS__)

#define VLK_LOG(expr)               \
  do {                              \
    std::cout << expr << std::endl; \
  } while (false);
