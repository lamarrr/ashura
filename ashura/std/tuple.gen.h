/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/types.h"
#include "ashura/std/v.h"

namespace ash{

inline constexpr usize MAX_TUPLE_SIZE = 16;


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

};

}

template<typename ... T>
struct Tuple
{
static_assert("Tuple size exceeds MAX_TUPLE_SIZE");
};

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


} // namespace ash
