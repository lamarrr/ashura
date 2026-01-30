/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/log.h"

#define ASH_CHECK_EX(logger_expr, src_loc_expr, cond_expr, description_fstr, ...)    \
    do                                                                               \
    {                                                                                \
        if (!(cond_expr)) [[unlikely]]                                               \
        {                                                                            \
            ::ash::SourceLocation const src_loc_ = (src_loc_expr);                   \
            (logger_expr)                                                            \
              .panic(::ash::cstr("panic in function: "                               \
                                 "{}\n{}:{}:{}: " description_fstr "\ntriggered by " \
                                 "expression: \n\t{}\t|\t... {} ..."),               \
                     src_loc_.function, src_loc_.file, src_loc_.line,                \
                     src_loc_.column __VA_OPT__(, ) __VA_ARGS__, src_loc_.line,      \
                     #cond_expr);                                                    \
        }                                                                            \
    } while (false)

#define ASH_CHECK_SLOC(src_loc_expr, cond_expr, description_fstr, ...) \
    ASH_CHECK_EX(*::ash::logger, src_loc_expr, cond_expr,              \
                 description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define ASH_CHECK(cond_expr, description_fstr, ...)                           \
    ASH_CHECK_EX(*::ash::logger, ::ash::SourceLocation::current(), cond_expr, \
                 description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define ASH_CHECK_UNREACHABLE() \
    ASH_CHECK(false, "Expected code section to be unreachable")

#define ASH_TRY(var_identifier, ...)                                        \
    auto __result_for_##var_identifier = (__VA_ARGS__);                     \
    if (!__result_for_##var_identifier.is_ok_)                              \
    {                                                                       \
        return ::ash::Err{                                                  \
          static_cast<decltype(__result_for_##var_identifier)::ErrType &&>( \
            __result_for_##var_identifier.v1_)};                            \
    }                                                                       \
    auto var_identifier =                                                   \
      static_cast<decltype(__result_for_##var_identifier)::Type &&>(        \
        __result_for_##var_identifier.v0_);

#define ASH_BOUNDS_CHECK(index, size)                                               \
    do                                                                              \
    {                                                                               \
        auto index___ = (index);                                                    \
        auto size___  = (size);                                                     \
        ASH_CHECK(index___ < size___, "Index out of bounds: index = {}, size = {}", \
                  index___, size___);                                               \
    } while (false)
