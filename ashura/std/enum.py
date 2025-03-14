# SPDX-License-Identifier: MIT

import sys
MAX_ENUM_SIZE = 32


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
#include "ashura/std/tuple.h"

namespace ash
{{

inline constexpr unsigned int MAX_ENUM_SIZE = {MAX_ENUM_SIZE};

template<typename ... T>
requires(sizeof...(T) <= MAX_ENUM_SIZE)
struct Enum;
"""
)

member_cases = [
  f"""
if constexpr(I == {i})
{{
  return e.v{i}_;
}}
""" for i in range(MAX_ENUM_SIZE)]

move_constructor_cases = [f"""if constexpr(Dst::SIZE > {i})
{{
if(src->index_ == {i})
{{
  enum_member_construct(&dst->v{i}_, static_cast<typename Src::E{i} &&>(src->v{i}_));
  return;
}}
}}
""" for i in range(0, MAX_ENUM_SIZE)]

copy_constructor_cases = [f"""if constexpr(Dst::SIZE > {i})
{{
if(src->index_ == {i})
{{
  enum_member_construct(&dst->v{i}_, src->v{i}_);
  return;
}}
}}
""" for i in range(0, MAX_ENUM_SIZE)]

destructor_cases = [f"""if constexpr(Enum::SIZE > {i})
{{
if(e->index_ == {i})
{{
  enum_member_destruct(&e->v{i}_);
  return;
}}
}}
""" for i in range(0, MAX_ENUM_SIZE)]


out(f"""
namespace intr{{

template<unsigned int I, typename Enum>
constexpr auto& enum_member(Enum& e)
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
if constexpr(Dst::SIZE > 1)
{{
  dst->index_ = src->index_;
}}

{"\n".join(move_constructor_cases)}
}}

template<typename Src, typename Dst>
constexpr void enum_copy_construct(Src * src, Dst * dst){{
if constexpr(Dst::SIZE > 1)
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

""")

match_cases = [
f""" if constexpr(SIZE > {i})
{{
  if(e.index_ == {i})
  {{
    return fns_ref.v{i}(e.v{i}_);
  }}
}}""" for i in range(MAX_ENUM_SIZE)]

out(
f"""
template<unsigned int SIZE, typename Enum, typename ... Fns>
constexpr decltype(auto) match(Enum && e, Fns && ... fns)
{{
  Tuple<Fns && ...> fns_ref{{ static_cast<Fns && >(fns)... }};

{"\n".join(match_cases)}

  ASH_UNREACHABLE;
}}
""")

out(f"""
}} // namespace intr

""")

for size in range(0,  MAX_ENUM_SIZE + 1):

  types = [f"T{i}" for i in range(size)]
  aliases = [f"E{i}" for i in range(size)]
  values = [f"v{i}_" for i in range(size)]
  typename_decls = [f"typename {t}" for t in types]
  alias_decls = [f"typedef {t} {a};" for t, a in zip(types, aliases)]
  value_decls = [f"{t} {v};" for t, v in zip(types, values)]

  members_def = [f"T{i} v{i}_;" for i in range(0, size)]

  index_def = ""

  if size == 0:
        index_def = ""
  elif size == 1:
        index_def = """static constexpr unsigned int index_ = 0;

static constexpr unsigned int index()
{
  return 0;
}"""
  else:
        index_def = """unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}"""

  value_constructors = []

  if size == 0:
        value_constructors = []
  elif size == 1:
        value_constructors = [f"""constexpr Enum(T0 v) :
v0_{{static_cast<T0 &&>(v)}}
{{ }}
"""]
  else:
        value_constructors = [f"""constexpr Enum(T{i} v) :
index_{{{i}}},
v{i}_{{static_cast<T{i} &&>(v)}}
{{ }}
""" for i in range(0, size)]

  out(f"""
template<{", ".join(typename_decls)}>
struct Enum<{", ".join(types)}>
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

{
f"""
constexpr bool is(unsigned int) const
{{
  return false;
}}

constexpr void match()
{{
}}

constexpr void match() const
{{
}}
"""

if size == 0 else f"""{index_def}

union {{
{"\t".join(members_def)}
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
{{
  intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}}

{"\n".join(value_constructors)}

constexpr bool is(unsigned int i) const
{{
  return index_ == i;
}}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{{
  CHECK(index_ == I, "Accessed Enum type: {{}} but type is: {{}}", I, index_);
  return intr::enum_member<I>(*this);
}}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{{
  CHECK(index_ == I, "Accessed Enum type: {{}} but type is: {{}}", I, index_);
  return intr::enum_member<I>(*this);
}}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{{
  CHECK(index_ == I, "Accessed Enum type: {{}} but type is: {{}}", I, index_);
  return intr::enum_member<I>(*this);
}}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{{
  CHECK(index_ == I, "Accessed Enum type: {{}} but type is: {{}}", I, index_);
  return intr::enum_member<I>(*this);
}}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{{
  return get(v);
}}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{{
  return get(v);
}}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{{
  return get(v);
}}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{{
  return get(v);
}}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}}

"""
}
}};
    """)


out(f"""
}} // namespace ash
""")


file.close()
