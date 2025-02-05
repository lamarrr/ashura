/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/log.h"

#define CHECK_EX(logger_expr, src_loc_expr, cond_expr, description_fstr, ...) \
  do                                                                          \
  {                                                                           \
    if (!(cond_expr)) [[unlikely]]                                            \
    {                                                                         \
      ::ash::SourceLocation const src_loc_ = (src_loc_expr);                  \
      (logger_expr)                                                           \
        .panic(::ash::cstr_span(                                              \
                 "panic in function: {}\n{}:{}:{}: " description_fstr         \
                 "\ntriggered by "                                            \
                 "expression: \n\t{}\t|\t... {} ..."),                        \
               src_loc_.function, src_loc_.file, src_loc_.line,               \
               src_loc_.column __VA_OPT__(, ) __VA_ARGS__, src_loc_.line,     \
               #cond_expr);                                                   \
    }                                                                         \
  } while (false)

#define CHECK_SLOC(src_loc_expr, cond_expr, description_fstr, ...) \
  CHECK_EX(*::ash::logger, src_loc_expr, cond_expr,                \
           description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define CHECK(cond_expr, description_fstr, ...)                         \
  CHECK_EX(*::ash::logger, ::ash::SourceLocation::current(), cond_expr, \
           description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define CHECK_UNREACHABLE() \
  CHECK(false, "Expected code section to be unreachable")
