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
#include "ashura/std/types.h"

namespace ash
{{

inline constexpr usize MAX_PACK_SIZE = {MAX_SIZE};

namespace intr
{{

template <usize I, typename... T>
requires((I < sizeof...(T)) && (sizeof...(T) <= MAX_PACK_SIZE))
struct index_pack;

""")

for index in range(MAX_SIZE):
  types = [f"E{i}" for i in range(index+1)]
  typenames = [f"typename E{i}" for i in range(index+1)]
  out(f"""template<{", ".join(typenames)}, typename... E>
struct index_pack<{index}, {", ".join(types)}, E...>
{{
  using Type = E{index};
}};

""")


out(f"""
}} // namespace intr
}} // namespace ash

  """)
