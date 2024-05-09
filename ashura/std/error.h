#pragma once
#include "ashura/std/log.h"

#define CHECK_EX(description, ...)                                          \
  if (!(__VA_ARGS__))                                                       \
  {                                                                         \
    (default_logger)                                                        \
        ->panic(description, " (expression: " #__VA_ARGS__,                 \
                ") [function: ", ::ash::SourceLocation::current().function, \
                ", file: ", ::ash::SourceLocation::current().file, ":",     \
                ::ash::SourceLocation::current().line, ":",                 \
                ::ash::SourceLocation::current().column, "]");              \
  }

#define CHECK(...) CHECK_EX("", __VA_ARGS__)
#define UNREACHABLE() \
  CHECK_EX("Expected expression to be unreachable, signifies a bug", false)
