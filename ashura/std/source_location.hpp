/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/span.hpp"

namespace ash
{

/// @brief The `SourceLocation`  class represents certain information about the
/// source code, such as file names, line numbers, and function names.
/// Previously, functions that desire to obtain this information about the call
/// site (for logging, testing, or debugging purposes) must use macros so that
/// predefined macros like `__LINE__` and `__FILE__` are expanded in the context
/// of the caller. The `SourceLocation` class provides a better alternative.
///
/// based on: https://en.cppreference.com/w/cpp/utility/source_location
///
struct SourceLocation
{
    static constexpr SourceLocation current(
#if ASH_HAS_BUILTIN(FILE) || \
  (defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201'907L)
      Str file = cstr(__builtin_FILE()),
#elif defined(__FILE__)
      Str file = cstr(__FILE__),
#else
      Str file = cstr("unknown"),
#endif

#if ASH_HAS_BUILTIN(FUNCTION) || \
  (defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201'907L)
      Str function = cstr(__builtin_FUNCTION()),
#else
      Str function = cstr("unknown"),
#endif

#if ASH_HAS_BUILTIN(LINE) || \
  (defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201'907L)
      u32 line = __builtin_LINE(),
#elif defined(__LINE__)
      u32 line = __LINE__,
#else
      u32 line = 0,
#endif

#if ASH_HAS_BUILTIN(COLUMN) || \
  (defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201'907L)
      u32 column = __builtin_COLUMN()
#else
      u32 column = 0
#endif
    )
    {
        return SourceLocation{file, function, line, column};
    }

    Str file     = ""_s;
    Str function = ""_s;
    u32 line     = 0;
    u32 column   = 0;
};

}    // namespace ash
