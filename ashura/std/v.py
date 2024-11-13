import sys
MAX_SIZE = 16

assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""
/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include <cstddef>

namespace ash{{

template <std::size_t I>
struct V
{{
  static constexpr std::size_t INDEX = I;

  static constexpr std::size_t index()
  {{
    return INDEX;
  }}
}};

""")


value_defs = [f"constexpr V<{i}> v{i};" for i in range(MAX_SIZE)]


out("\n".join(value_defs))

out(f"""

}} // namespace ash

""")


file.close()
