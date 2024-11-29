import sys
MAX_ENUM_SIZE = 16


assert len(sys.argv) > 1

file = open(sys.argv[1], "w", encoding="ascii")


def out(code): return file.write(code)


out(f"""/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/v.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/types.h"

namespace ash
{{

static constexpr usize MAX_ENUM_SIZE = {MAX_ENUM_SIZE};

template<typename ... T>
struct Enum
{{
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

 

move_constructor_cases = [f"""
if constexpr(Dst::SIZE > {i})
{{
if(src->index_ == {i})
{{
    enum_member_construct(&dst->v{i}_, static_cast<typename Src::E{i} &&>(src->v{i}_));
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

template<usize I, typename Enum>
constexpr decltype(auto) enum_member(Enum& e)
{{
{"else".join(member_cases)}
}};

template<typename E, typename... Args>
constexpr void enum_member_construct(E * e, Args&&... args )
{{
    new (e) E {{ static_cast<Args &&>(args)...  }};
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

     
    members_def = [f"T{i} v{i}_;" for i in range(0, size)]

    value_constructors = [f"""
constexpr Enum(T{i} v) :
index_{{{i}}},
v{i}_{{static_cast<T{i} &&>(v)}}
{{ }}
""" for i in range(0, size)]
 

    out(f"""
template<{", ".join(typename_decls)}>
struct Enum<{", ".join(types)}>
{{

{"\n".join(alias_decls)}

static constexpr usize SIZE = {size};

static constexpr usize size()
{{
    return SIZE;
}}

{
    """
"""
if size == 0 else
f"""
usize index_;

constexpr usize index() const
{{
    return index_;
}}

union {{
{"\n".join(members_def)}
}};

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

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}}

{"\n".join(value_constructors)}

template<usize I> requires(I < SIZE)
constexpr bool is()
{{
    return index_ == I;
}}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{{

}}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{{

}}
"""
}
}};
    """)

    # deduction guides


out(f"""
}} // namespace ash
""")


file.close()
