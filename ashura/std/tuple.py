import sys
MAX_TUPLE_SIZE = 16

assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/v.h"

namespace ash{{

inline constexpr usize MAX_TUPLE_SIZE = {MAX_TUPLE_SIZE};

""")


tuple_cases = [
    f"""
if constexpr(I == {i})
{{
    return (t.v{i});
}}
""" for i in range(MAX_TUPLE_SIZE)]

out(f"""
namespace intr
{{

template<usize I, typename Tuple>
constexpr decltype(auto) tuple_member(Tuple& t)
{{
{"else".join(tuple_cases)}
}};

}}
""")


out(
    f"""
template<typename ... T>
struct Tuple
{{
static_assert("Tuple size exceeds MAX_TUPLE_SIZE");
}};
"""
)


for size in range(0,  MAX_TUPLE_SIZE + 1):

    types = [f"T{i}" for i in range(size)]
    aliases = [f"E{i}" for i in range(size)]
    values = [f"v{i}" for i in range(size)]
    typename_decls = [f"typename {t}" for t in types]
    alias_decls = [f"typedef {t} {a};" for t, a in zip(types, aliases)]
    value_decls = [f"{t} {v};" for t, v in zip(types, values)]

    tuple_def = f"""
template<{", ".join(typename_decls)}>
struct Tuple<{", ".join(types)}>
{{

{"\n".join(alias_decls)}

static constexpr usize SIZE = {size};

static constexpr usize size()
{{
    return SIZE;
}}

{"\n".join(value_decls)}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{{
    return intr::tuple_member<I>(*this);
}}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{{
    return intr::tuple_member<I>(*this);
}}

}};

    """

    out(tuple_def)

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
