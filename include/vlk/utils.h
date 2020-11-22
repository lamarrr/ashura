#pragma once

#include <iostream>

#include "stx/panic.h"

#define VLK_ENSURE(expr, message)     \
  do {                                \
    if (!(expr)) stx::panic(message); \
  } while (false);

#define VLK_LOG(expr)               \
  do {                              \
    std::cout << expr << std::endl; \
  } while (false);

