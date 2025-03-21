# SPDX-License-Identifier: MIT

import sys
MAX_SIZE = 32

assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once

namespace ash
{{

template <unsigned int I>
struct V
{{
  static constexpr unsigned int INDEX = I;

  static constexpr unsigned int index()
  {{
    return INDEX;
  }}
}};

""")


value_defs = [f"inline constexpr V<{i}> v{i};" for i in range(MAX_SIZE)]


out("\n".join(value_defs))

out(f"""

}} // namespace ash

""")


file.close()
