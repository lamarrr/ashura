#pragma once

#include "ashura/std/log.h"

#define CHECK_EX(logger_expr, src_loc_expr, cond_expr, description_log, ...)   \
  do                                                                           \
  {                                                                            \
    if (!(cond_expr)) [[unlikely]]                                             \
    {                                                                          \
      ::ash::SourceLocation const CHECK_EX_src_loc_ = (src_loc_expr);          \
      (logger_expr)                                                            \
          .panic("panic in function: '", CHECK_EX_src_loc_.function, "'\n",    \
                 CHECK_EX_src_loc_.file, ":", CHECK_EX_src_loc_.line, ":",     \
                 CHECK_EX_src_loc_.column, ": ",                               \
                 description_log __VA_OPT__(, ) __VA_ARGS__,                   \
                 ", triggered by expression:\n", "\t", CHECK_EX_src_loc_.line, \
                 "\t|\t ... ", #cond_expr, " ...");                            \
    }                                                                          \
  } while (false)

#define CHECK_DESC_SRC(src_loc_expr, cond_expr, description_expr, ...)        \
  CHECK_EX(*::ash::default_logger, src_loc_expr, cond_expr, description_expr, \
           __VA_ARGS__)

#define CHECK_DESC(cond_expr, description_expr, ...)                 \
  CHECK_EX(*::ash::default_logger, ::ash::SourceLocation::current(), \
           cond_expr, description_expr, __VA_ARGS__)

#define CHECK(cond_expr)                                             \
  CHECK_EX(*::ash::default_logger, ::ash::SourceLocation::current(), \
           cond_expr, "[no description provided]")

#define UNREACHABLE() \
  CHECK_DESC(false, "Expected code section to be unreachable")
