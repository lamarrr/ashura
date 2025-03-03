# SPDX-License-Identifier: MIT

import sys
MAX_TUPLE_SIZE = 32

assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/v.h"
#include "ashura/std/index_pack.h"

namespace ash{{

inline constexpr unsigned int MAX_TUPLE_SIZE = {MAX_TUPLE_SIZE};

""")

tuple_cases = [
  f"""
if constexpr(I == {i})
{{
  return t.v{i};
}}
""" for i in range(MAX_TUPLE_SIZE)]

out(f"""
namespace intr
{{

template<usize I, typename Tuple>
constexpr auto& tuple_member(Tuple& t)
{{
{"else".join(tuple_cases)}
}};

}} // namespace intr
""")


out(
  f"""
template<typename ... T>
requires(sizeof...(T) <= MAX_TUPLE_SIZE)
struct Tuple;
"""
)


for size in range(0,  MAX_TUPLE_SIZE + 1):

  types = [f"T{i}" for i in range(size)]
  aliases = [f"E{i}" for i in range(size)]
  values = [f"v{i}" for i in range(size)]
  typename_decls = [f"typename {t}" for t in types]
  alias_decls = [f"typedef {t} {a};" for t, a in zip(types, aliases)]
  value_decls = [f"{t} {v};" for t, v in zip(types, values)]

  out(f"""
template<{", ".join(typename_decls)}>
struct Tuple<{", ".join(types)}>
{{

{"\t".join(alias_decls)}

{"" if size == 0 else 
f"""template<unsigned int I>
using E = index_pack<I, {", ".join(aliases)}>;
"""}

static constexpr unsigned int SIZE = {size};

static constexpr unsigned int size()
{{
  return SIZE;
}}

{"\t".join(value_decls)}

template<unsigned int I> requires(I < SIZE)
constexpr auto & get() & {{
 return intr::tuple_member<I>(*this);
}}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get() const & {{
 return intr::tuple_member<I>(*this);
}}


template<usize I>
constexpr auto& operator[](V<I>) &
{{
  return get<I>(*this);
}}

template<usize I>
constexpr auto const& operator[](V<I>) const &
{{
  return get<I>(*this);
}}

}};

    """)


  deduction_guide = ""

  if size != 0:
        deduction_guide = f"""
template<{", ".join(typename_decls)}>
Tuple({", ".join(types)}) -> Tuple<{", ".join(types)}>;

"""

  out(deduction_guide)


out(f"""
}} // namespace ash
""")
