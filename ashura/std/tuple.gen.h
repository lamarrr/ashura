/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/v.h"
#include "ashura/std/index_pack.h"

namespace ash{

inline constexpr usize MAX_TUPLE_SIZE = 32;


namespace intr
{

template<usize I, typename Tuple>
constexpr decltype(auto) tuple_member(Tuple& t)
{

if constexpr(I == 0)
{
  return (t.v0);
}
else
if constexpr(I == 1)
{
  return (t.v1);
}
else
if constexpr(I == 2)
{
  return (t.v2);
}
else
if constexpr(I == 3)
{
  return (t.v3);
}
else
if constexpr(I == 4)
{
  return (t.v4);
}
else
if constexpr(I == 5)
{
  return (t.v5);
}
else
if constexpr(I == 6)
{
  return (t.v6);
}
else
if constexpr(I == 7)
{
  return (t.v7);
}
else
if constexpr(I == 8)
{
  return (t.v8);
}
else
if constexpr(I == 9)
{
  return (t.v9);
}
else
if constexpr(I == 10)
{
  return (t.v10);
}
else
if constexpr(I == 11)
{
  return (t.v11);
}
else
if constexpr(I == 12)
{
  return (t.v12);
}
else
if constexpr(I == 13)
{
  return (t.v13);
}
else
if constexpr(I == 14)
{
  return (t.v14);
}
else
if constexpr(I == 15)
{
  return (t.v15);
}
else
if constexpr(I == 16)
{
  return (t.v16);
}
else
if constexpr(I == 17)
{
  return (t.v17);
}
else
if constexpr(I == 18)
{
  return (t.v18);
}
else
if constexpr(I == 19)
{
  return (t.v19);
}
else
if constexpr(I == 20)
{
  return (t.v20);
}
else
if constexpr(I == 21)
{
  return (t.v21);
}
else
if constexpr(I == 22)
{
  return (t.v22);
}
else
if constexpr(I == 23)
{
  return (t.v23);
}
else
if constexpr(I == 24)
{
  return (t.v24);
}
else
if constexpr(I == 25)
{
  return (t.v25);
}
else
if constexpr(I == 26)
{
  return (t.v26);
}
else
if constexpr(I == 27)
{
  return (t.v27);
}
else
if constexpr(I == 28)
{
  return (t.v28);
}
else
if constexpr(I == 29)
{
  return (t.v29);
}
else
if constexpr(I == 30)
{
  return (t.v30);
}
else
if constexpr(I == 31)
{
  return (t.v31);
}

};

} // namespace intr

template<typename ... T>
requires(sizeof...(T) <= MAX_TUPLE_SIZE)
struct Tuple;

template<>
struct Tuple<>
{





static constexpr usize SIZE = 0;

static constexpr usize size()
{
  return SIZE;
}



template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0>
struct Tuple<T0>
{

typedef T0 E0;

template<usize I>
using E = index_pack<I, E0>;


static constexpr usize SIZE = 1;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0>
Tuple(T0) -> Tuple<T0>;


template<typename T0, typename T1>
struct Tuple<T0, T1>
{

typedef T0 E0;
typedef T1 E1;

template<usize I>
using E = index_pack<I, E0, E1>;


static constexpr usize SIZE = 2;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1>
Tuple(T0, T1) -> Tuple<T0, T1>;


template<typename T0, typename T1, typename T2>
struct Tuple<T0, T1, T2>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;

template<usize I>
using E = index_pack<I, E0, E1, E2>;


static constexpr usize SIZE = 3;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2>
Tuple(T0, T1, T2) -> Tuple<T0, T1, T2>;


template<typename T0, typename T1, typename T2, typename T3>
struct Tuple<T0, T1, T2, T3>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3>;


static constexpr usize SIZE = 4;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3>
Tuple(T0, T1, T2, T3) -> Tuple<T0, T1, T2, T3>;


template<typename T0, typename T1, typename T2, typename T3, typename T4>
struct Tuple<T0, T1, T2, T3, T4>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4>;


static constexpr usize SIZE = 5;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4>
Tuple(T0, T1, T2, T3, T4) -> Tuple<T0, T1, T2, T3, T4>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct Tuple<T0, T1, T2, T3, T4, T5>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5>;


static constexpr usize SIZE = 6;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
Tuple(T0, T1, T2, T3, T4, T5) -> Tuple<T0, T1, T2, T3, T4, T5>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Tuple<T0, T1, T2, T3, T4, T5, T6>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6>;


static constexpr usize SIZE = 7;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
Tuple(T0, T1, T2, T3, T4, T5, T6) -> Tuple<T0, T1, T2, T3, T4, T5, T6>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7>;


static constexpr usize SIZE = 8;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8>;


static constexpr usize SIZE = 9;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9>;


static constexpr usize SIZE = 10;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10>;


static constexpr usize SIZE = 11;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11>;


static constexpr usize SIZE = 12;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12>;


static constexpr usize SIZE = 13;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13>;


static constexpr usize SIZE = 14;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14>;


static constexpr usize SIZE = 15;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15>;


static constexpr usize SIZE = 16;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16>;


static constexpr usize SIZE = 17;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17>;


static constexpr usize SIZE = 18;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18>;


static constexpr usize SIZE = 19;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19>;


static constexpr usize SIZE = 20;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20>;


static constexpr usize SIZE = 21;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21>;


static constexpr usize SIZE = 22;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22>;


static constexpr usize SIZE = 23;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23>;


static constexpr usize SIZE = 24;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24>;


static constexpr usize SIZE = 25;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25>;


static constexpr usize SIZE = 26;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26>;


static constexpr usize SIZE = 27;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;
typedef T27 E27;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27>;


static constexpr usize SIZE = 28;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;
T27 v27;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;
typedef T27 E27;
typedef T28 E28;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28>;


static constexpr usize SIZE = 29;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;
T27 v27;
T28 v28;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;
typedef T27 E27;
typedef T28 E28;
typedef T29 E29;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29>;


static constexpr usize SIZE = 30;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;
T27 v27;
T28 v28;
T29 v29;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;
typedef T27 E27;
typedef T28 E28;
typedef T29 E29;
typedef T30 E30;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30>;


static constexpr usize SIZE = 31;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;
T27 v27;
T28 v28;
T29 v29;
T30 v30;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>;


template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;
typedef T3 E3;
typedef T4 E4;
typedef T5 E5;
typedef T6 E6;
typedef T7 E7;
typedef T8 E8;
typedef T9 E9;
typedef T10 E10;
typedef T11 E11;
typedef T12 E12;
typedef T13 E13;
typedef T14 E14;
typedef T15 E15;
typedef T16 E16;
typedef T17 E17;
typedef T18 E18;
typedef T19 E19;
typedef T20 E20;
typedef T21 E21;
typedef T22 E22;
typedef T23 E23;
typedef T24 E24;
typedef T25 E25;
typedef T26 E26;
typedef T27 E27;
typedef T28 E28;
typedef T29 E29;
typedef T30 E30;
typedef T31 E31;

template<usize I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31>;


static constexpr usize SIZE = 32;

static constexpr usize size()
{
  return SIZE;
}

T0 v0;
T1 v1;
T2 v2;
T3 v3;
T4 v4;
T5 v5;
T6 v6;
T7 v7;
T8 v8;
T9 v9;
T10 v10;
T11 v11;
T12 v12;
T13 v13;
T14 v14;
T15 v15;
T16 v16;
T17 v17;
T18 v18;
T19 v19;
T20 v20;
T21 v21;
T22 v22;
T23 v23;
T24 v24;
T25 v25;
T26 v26;
T27 v27;
T28 v28;
T29 v29;
T30 v30;
T31 v31;

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
  return intr::tuple_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
  return intr::tuple_member<I>(*this);
}

};

    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>;


} // namespace ash
