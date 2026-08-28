/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/log.hpp"
#include "ashura/std/source_location.hpp"

#define ASH_CHECK_EX(logger_expr, src_loc_expr, cond_expr, description_fstr, ...)    \
    do                                                                               \
    {                                                                                \
        if (!(cond_expr)) [[unlikely]]                                               \
        {                                                                            \
            auto __src_loc = (src_loc_expr);                                         \
            (logger_expr)                                                            \
              .panic(::ash::cstr("panic in function: "                               \
                                 "{}\n{}:{}:{}: " description_fstr "\ntriggered by " \
                                 "assertion: \n\t{}\t|\t... {} ..."),                \
                     __src_loc.function, __src_loc.file, __src_loc.line,             \
                     __src_loc.column __VA_OPT__(, ) __VA_ARGS__, __src_loc.line,    \
                     #cond_expr);                                                    \
        }                                                                            \
    } while (false)

#define ASH_CHECK_SLOC(src_loc_expr, cond_expr, description_fstr, ...) \
    ASH_CHECK_EX(::ash::logger(), src_loc_expr, cond_expr,             \
                 description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define ASH_CHECK(cond_expr, description_fstr, ...)                            \
    ASH_CHECK_EX(::ash::logger(), ::ash::SourceLocation::current(), cond_expr, \
                 description_fstr __VA_OPT__(, ) __VA_ARGS__)

#define ASH_CHECK_UNREACHABLE() \
    ASH_CHECK(false, "Expected code section to be unreachable")

#define ASH_TRY(value_id, ...)                                                      \
    auto __##value_id##_result = (__VA_ARGS__);                                     \
    if (!__##value_id##_result.is_ok_)                                              \
    {                                                                               \
        return ::ash::Err{static_cast<decltype(__##value_id##_result)::ErrType &&>( \
          __##value_id##_result.v1_)};                                              \
    }                                                                               \
    auto value_id = static_cast<decltype(__##value_id##_result)::Type &&>(          \
      __##value_id##_result.v0_);

#define ASH_BOUNDS_CHECK(index, size)                                             \
    do                                                                            \
    {                                                                             \
        auto __index = (index);                                                   \
        auto __size  = (size);                                                    \
        ASH_CHECK(__index < __size, "Index out of bounds: index = {}, size = {}", \
                  __index, __size);                                               \
    } while (false)
