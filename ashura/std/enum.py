import sys
MAX_ENUM_SIZE = 16


assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""
/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include <cstddef>
#include "ashura/std/v.h"

namespace ash{{

static constexpr std::size_t MAX_ENUM_SIZE = {MAX_ENUM_SIZE};

template<typename ... T>
struct Enum{{
static_assert("Enum size exceeds MAX_ENUM_SIZE");
}};
"""
)

member_cases = [
    f"""
if constexpr(I == {i})
{{
    return (e.v{i}_);
}}
""" for i in range(MAX_ENUM_SIZE)]

out(f"""
template<std::size_t I, typename Enum>
constexpr decltype(auto) enum_member(Enum& e){{
{"else".join(member_cases)}
}};
""")

move_constructor_cases = [f"""
if constexpr(Dst::SIZE > {i})
{{
if(src->index_ == {i})
{{
    enum_member_construct(&dst->v{i}_, (typename Src::E{i} &&) src->v{i}_);
    return;
}}
}}""" for i in range(0, MAX_ENUM_SIZE)]

copy_constructor_cases = [f"""
if constexpr(Dst::SIZE > {i})
{{
if(src->index_ == {i})
{{
    enum_member_construct(&dst->v{i}_, src->v{i}_);
    return;
}}
}}""" for i in range(0, MAX_ENUM_SIZE)]

destructor_cases = [f"""
if constexpr(Enum::SIZE > {i}){{
if(e->index_ == {i})
{{
    enum_member_destruct(&e->v{i}_);
    return;
}}
}}
""" for i in range(0, MAX_ENUM_SIZE)]


out(f"""
namespace intr{{

template<typename E, typename... Args>
constexpr void enum_member_construct(E * e, Args&&... args )
{{
    new (e) E {{ ((Args&&)args)...  }};
}}

template<typename E>
constexpr void enum_member_destruct(E * e){{
   e->~E();
}}

template<typename Enum>
constexpr void enum_destruct(Enum * e){{
{"\n".join(destructor_cases)}
}}

template<typename Src, typename Dst>
constexpr void enum_move_construct(Src * src, Dst * dst){{
if constexpr(Dst::SIZE > 0)
{{
    dst->index_ = src->index_;
}}

{"\n".join(move_constructor_cases)}
}}

template<typename Src, typename Dst>
constexpr void enum_copy_construct(Src * src, Dst * dst){{
if constexpr(Dst::SIZE > 0)
{{
    dst->index_ = src->index_;
}}

{"\n".join(copy_constructor_cases)}
}}

template<typename Src, typename Dst>
constexpr void enum_move_assign(Src * src, Dst * dst){{
if(src == dst)
{{
    return;
}}
enum_destruct(dst);
enum_move_construct(src, dst);
}}

template<typename Src, typename Dst>
constexpr void enum_copy_assign(Src * src, Dst * dst)
{{
if(src == dst)
{{
    return;
}}
enum_destruct(dst);
enum_copy_construct(src, dst);
}}
}}
""")


for size in range(0,  MAX_ENUM_SIZE + 1):

    types = [f"T{i}" for i in range(size)]
    aliases = [f"E{i}" for i in range(size)]
    values = [f"v{i}_" for i in range(size)]
    typename_decls = [f"typename {t}" for t in types]
    alias_decls = [f"typedef {t} {a};" for t, a in zip(types, aliases)]
    value_decls = [f"{t} {v};" for t, v in zip(types, values)]

    index_def = ""
    if size == 0:
        pass
    else:
        index_def = f"""
std::size_t index_;

constexpr std::size_t index() const {{
return index_;
}}
"""

    storage_def = ""

    if size == 0:
        pass
    else:
        members_def = [f"T{i} v{i}_;" for i in range(0, size)]
        storage_def = f"""
union {{
{"\n".join(members_def)}
}};
"""

    value_constructors = [f"""
constexpr Enum(T{i} v) :
index_{{{i}}},
v{i}_{{(T{i} &&) v}}
{{ }}
""" for i in range(0, size)]

    base_constructors = f"""

constexpr Enum(Enum const& other)
{{
    intr::enum_copy_construct(&other, this);
}}

constexpr Enum(Enum && other)
{{
    intr::enum_move_construct(&other, this);
}}

constexpr Enum& operator=(Enum const& other)
{{
    intr::enum_copy_assign(&other, this);
    return *this;
}}

constexpr Enum& operator=(Enum && other)
{{
    intr::enum_move_assign(&other, this);
    return *this;
}}

constexpr ~Enum()
{{
    intr::enum_destruct(this);
}}

"""

    out(f"""
template<{", ".join(typename_decls)}>
struct Enum<{", ".join(types)}>
{{

{"\n".join(alias_decls)}

static constexpr std::size_t SIZE = {size};

static constexpr std::size_t size(){{
    return SIZE;
}}

{index_def}
{storage_def}
{base_constructors if size != 0 else ""}

{"\n".join(value_constructors)}
}};

    """)

    # deduction guides


out(f"""
}} // namespace ash
""")


file.close()
