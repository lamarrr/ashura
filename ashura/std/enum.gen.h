/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/v.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/types.h"

namespace ash
{

inline constexpr usize MAX_ENUM_SIZE = 16;

template<typename ... T>
struct Enum
{
static_assert("Enum size exceeds MAX_ENUM_SIZE");
};

namespace intr{

template<usize I, typename Enum>
constexpr decltype(auto) enum_member(Enum& e)
{

if constexpr(I == 0)
{
    return (e.v0_);
}
else
if constexpr(I == 1)
{
    return (e.v1_);
}
else
if constexpr(I == 2)
{
    return (e.v2_);
}
else
if constexpr(I == 3)
{
    return (e.v3_);
}
else
if constexpr(I == 4)
{
    return (e.v4_);
}
else
if constexpr(I == 5)
{
    return (e.v5_);
}
else
if constexpr(I == 6)
{
    return (e.v6_);
}
else
if constexpr(I == 7)
{
    return (e.v7_);
}
else
if constexpr(I == 8)
{
    return (e.v8_);
}
else
if constexpr(I == 9)
{
    return (e.v9_);
}
else
if constexpr(I == 10)
{
    return (e.v10_);
}
else
if constexpr(I == 11)
{
    return (e.v11_);
}
else
if constexpr(I == 12)
{
    return (e.v12_);
}
else
if constexpr(I == 13)
{
    return (e.v13_);
}
else
if constexpr(I == 14)
{
    return (e.v14_);
}
else
if constexpr(I == 15)
{
    return (e.v15_);
}

};

template<typename E, typename... Args>
constexpr void enum_member_construct(E * e, Args&&... args )
{
    new (e) E { static_cast<Args &&>(args)...  };
}

template<typename E>
constexpr void enum_member_destruct(E * e){
   e->~E();
}

template<typename Enum>
constexpr void enum_destruct(Enum * e){

if constexpr(Enum::SIZE > 0){
if(e->index_ == 0)
{
    enum_member_destruct(&e->v0_);
    return;
}
}


if constexpr(Enum::SIZE > 1){
if(e->index_ == 1)
{
    enum_member_destruct(&e->v1_);
    return;
}
}


if constexpr(Enum::SIZE > 2){
if(e->index_ == 2)
{
    enum_member_destruct(&e->v2_);
    return;
}
}


if constexpr(Enum::SIZE > 3){
if(e->index_ == 3)
{
    enum_member_destruct(&e->v3_);
    return;
}
}


if constexpr(Enum::SIZE > 4){
if(e->index_ == 4)
{
    enum_member_destruct(&e->v4_);
    return;
}
}


if constexpr(Enum::SIZE > 5){
if(e->index_ == 5)
{
    enum_member_destruct(&e->v5_);
    return;
}
}


if constexpr(Enum::SIZE > 6){
if(e->index_ == 6)
{
    enum_member_destruct(&e->v6_);
    return;
}
}


if constexpr(Enum::SIZE > 7){
if(e->index_ == 7)
{
    enum_member_destruct(&e->v7_);
    return;
}
}


if constexpr(Enum::SIZE > 8){
if(e->index_ == 8)
{
    enum_member_destruct(&e->v8_);
    return;
}
}


if constexpr(Enum::SIZE > 9){
if(e->index_ == 9)
{
    enum_member_destruct(&e->v9_);
    return;
}
}


if constexpr(Enum::SIZE > 10){
if(e->index_ == 10)
{
    enum_member_destruct(&e->v10_);
    return;
}
}


if constexpr(Enum::SIZE > 11){
if(e->index_ == 11)
{
    enum_member_destruct(&e->v11_);
    return;
}
}


if constexpr(Enum::SIZE > 12){
if(e->index_ == 12)
{
    enum_member_destruct(&e->v12_);
    return;
}
}


if constexpr(Enum::SIZE > 13){
if(e->index_ == 13)
{
    enum_member_destruct(&e->v13_);
    return;
}
}


if constexpr(Enum::SIZE > 14){
if(e->index_ == 14)
{
    enum_member_destruct(&e->v14_);
    return;
}
}


if constexpr(Enum::SIZE > 15){
if(e->index_ == 15)
{
    enum_member_destruct(&e->v15_);
    return;
}
}

}

template<typename Src, typename Dst>
constexpr void enum_move_construct(Src * src, Dst * dst){
if constexpr(Dst::SIZE > 0)
{
    dst->index_ = src->index_;
}


if constexpr(Dst::SIZE > 0)
{
if(src->index_ == 0)
{
    enum_member_construct(&dst->v0_, static_cast<typename Src::E0 &&>(src->v0_));
    return;
}
}

if constexpr(Dst::SIZE > 1)
{
if(src->index_ == 1)
{
    enum_member_construct(&dst->v1_, static_cast<typename Src::E1 &&>(src->v1_));
    return;
}
}

if constexpr(Dst::SIZE > 2)
{
if(src->index_ == 2)
{
    enum_member_construct(&dst->v2_, static_cast<typename Src::E2 &&>(src->v2_));
    return;
}
}

if constexpr(Dst::SIZE > 3)
{
if(src->index_ == 3)
{
    enum_member_construct(&dst->v3_, static_cast<typename Src::E3 &&>(src->v3_));
    return;
}
}

if constexpr(Dst::SIZE > 4)
{
if(src->index_ == 4)
{
    enum_member_construct(&dst->v4_, static_cast<typename Src::E4 &&>(src->v4_));
    return;
}
}

if constexpr(Dst::SIZE > 5)
{
if(src->index_ == 5)
{
    enum_member_construct(&dst->v5_, static_cast<typename Src::E5 &&>(src->v5_));
    return;
}
}

if constexpr(Dst::SIZE > 6)
{
if(src->index_ == 6)
{
    enum_member_construct(&dst->v6_, static_cast<typename Src::E6 &&>(src->v6_));
    return;
}
}

if constexpr(Dst::SIZE > 7)
{
if(src->index_ == 7)
{
    enum_member_construct(&dst->v7_, static_cast<typename Src::E7 &&>(src->v7_));
    return;
}
}

if constexpr(Dst::SIZE > 8)
{
if(src->index_ == 8)
{
    enum_member_construct(&dst->v8_, static_cast<typename Src::E8 &&>(src->v8_));
    return;
}
}

if constexpr(Dst::SIZE > 9)
{
if(src->index_ == 9)
{
    enum_member_construct(&dst->v9_, static_cast<typename Src::E9 &&>(src->v9_));
    return;
}
}

if constexpr(Dst::SIZE > 10)
{
if(src->index_ == 10)
{
    enum_member_construct(&dst->v10_, static_cast<typename Src::E10 &&>(src->v10_));
    return;
}
}

if constexpr(Dst::SIZE > 11)
{
if(src->index_ == 11)
{
    enum_member_construct(&dst->v11_, static_cast<typename Src::E11 &&>(src->v11_));
    return;
}
}

if constexpr(Dst::SIZE > 12)
{
if(src->index_ == 12)
{
    enum_member_construct(&dst->v12_, static_cast<typename Src::E12 &&>(src->v12_));
    return;
}
}

if constexpr(Dst::SIZE > 13)
{
if(src->index_ == 13)
{
    enum_member_construct(&dst->v13_, static_cast<typename Src::E13 &&>(src->v13_));
    return;
}
}

if constexpr(Dst::SIZE > 14)
{
if(src->index_ == 14)
{
    enum_member_construct(&dst->v14_, static_cast<typename Src::E14 &&>(src->v14_));
    return;
}
}

if constexpr(Dst::SIZE > 15)
{
if(src->index_ == 15)
{
    enum_member_construct(&dst->v15_, static_cast<typename Src::E15 &&>(src->v15_));
    return;
}
}
}

template<typename Src, typename Dst>
constexpr void enum_copy_construct(Src * src, Dst * dst){
if constexpr(Dst::SIZE > 0)
{
    dst->index_ = src->index_;
}


if constexpr(Dst::SIZE > 0)
{
if(src->index_ == 0)
{
    enum_member_construct(&dst->v0_, src->v0_);
    return;
}
}

if constexpr(Dst::SIZE > 1)
{
if(src->index_ == 1)
{
    enum_member_construct(&dst->v1_, src->v1_);
    return;
}
}

if constexpr(Dst::SIZE > 2)
{
if(src->index_ == 2)
{
    enum_member_construct(&dst->v2_, src->v2_);
    return;
}
}

if constexpr(Dst::SIZE > 3)
{
if(src->index_ == 3)
{
    enum_member_construct(&dst->v3_, src->v3_);
    return;
}
}

if constexpr(Dst::SIZE > 4)
{
if(src->index_ == 4)
{
    enum_member_construct(&dst->v4_, src->v4_);
    return;
}
}

if constexpr(Dst::SIZE > 5)
{
if(src->index_ == 5)
{
    enum_member_construct(&dst->v5_, src->v5_);
    return;
}
}

if constexpr(Dst::SIZE > 6)
{
if(src->index_ == 6)
{
    enum_member_construct(&dst->v6_, src->v6_);
    return;
}
}

if constexpr(Dst::SIZE > 7)
{
if(src->index_ == 7)
{
    enum_member_construct(&dst->v7_, src->v7_);
    return;
}
}

if constexpr(Dst::SIZE > 8)
{
if(src->index_ == 8)
{
    enum_member_construct(&dst->v8_, src->v8_);
    return;
}
}

if constexpr(Dst::SIZE > 9)
{
if(src->index_ == 9)
{
    enum_member_construct(&dst->v9_, src->v9_);
    return;
}
}

if constexpr(Dst::SIZE > 10)
{
if(src->index_ == 10)
{
    enum_member_construct(&dst->v10_, src->v10_);
    return;
}
}

if constexpr(Dst::SIZE > 11)
{
if(src->index_ == 11)
{
    enum_member_construct(&dst->v11_, src->v11_);
    return;
}
}

if constexpr(Dst::SIZE > 12)
{
if(src->index_ == 12)
{
    enum_member_construct(&dst->v12_, src->v12_);
    return;
}
}

if constexpr(Dst::SIZE > 13)
{
if(src->index_ == 13)
{
    enum_member_construct(&dst->v13_, src->v13_);
    return;
}
}

if constexpr(Dst::SIZE > 14)
{
if(src->index_ == 14)
{
    enum_member_construct(&dst->v14_, src->v14_);
    return;
}
}

if constexpr(Dst::SIZE > 15)
{
if(src->index_ == 15)
{
    enum_member_construct(&dst->v15_, src->v15_);
    return;
}
}
}

template<typename Src, typename Dst>
constexpr void enum_move_assign(Src * src, Dst * dst){
if(src == dst)
{
    return;
}
enum_destruct(dst);
enum_move_construct(src, dst);
}

template<typename Src, typename Dst>
constexpr void enum_copy_assign(Src * src, Dst * dst)
{
if(src == dst)
{
    return;
}
enum_destruct(dst);
enum_copy_construct(src, dst);
}
}

template<>
struct Enum<>
{



static constexpr usize SIZE = 0;

static constexpr usize size()
{
    return SIZE;
}



};
    
template<typename T0>
struct Enum<T0>
{

typedef T0 E0;

static constexpr usize SIZE = 1;

static constexpr usize size()
{
    return SIZE;
}


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1>
struct Enum<T0, T1>
{

typedef T0 E0;
typedef T1 E1;

static constexpr usize SIZE = 2;

static constexpr usize size()
{
    return SIZE;
}


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2>
struct Enum<T0, T1, T2>
{

typedef T0 E0;
typedef T1 E1;
typedef T2 E2;

static constexpr usize SIZE = 3;

static constexpr usize size()
{
    return SIZE;
}


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3>
struct Enum<T0, T1, T2, T3>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4>
struct Enum<T0, T1, T2, T3, T4>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct Enum<T0, T1, T2, T3, T4, T5>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Enum<T0, T1, T2, T3, T4, T5, T6>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
T11 v11_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


constexpr Enum(T11 v) :
index_{11},
v11_{static_cast<T11 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
T11 v11_;
T12 v12_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


constexpr Enum(T11 v) :
index_{11},
v11_{static_cast<T11 &&>(v)}
{ }


constexpr Enum(T12 v) :
index_{12},
v12_{static_cast<T12 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
T11 v11_;
T12 v12_;
T13 v13_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


constexpr Enum(T11 v) :
index_{11},
v11_{static_cast<T11 &&>(v)}
{ }


constexpr Enum(T12 v) :
index_{12},
v12_{static_cast<T12 &&>(v)}
{ }


constexpr Enum(T13 v) :
index_{13},
v13_{static_cast<T13 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
T11 v11_;
T12 v12_;
T13 v13_;
T14 v14_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


constexpr Enum(T11 v) :
index_{11},
v11_{static_cast<T11 &&>(v)}
{ }


constexpr Enum(T12 v) :
index_{12},
v12_{static_cast<T12 &&>(v)}
{ }


constexpr Enum(T13 v) :
index_{13},
v13_{static_cast<T13 &&>(v)}
{ }


constexpr Enum(T14 v) :
index_{14},
v14_{static_cast<T14 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>
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


usize index_;

constexpr usize index() const
{
    return index_;
}

union {
T0 v0_;
T1 v1_;
T2 v2_;
T3 v3_;
T4 v4_;
T5 v5_;
T6 v6_;
T7 v7_;
T8 v8_;
T9 v9_;
T10 v10_;
T11 v11_;
T12 v12_;
T13 v13_;
T14 v14_;
T15 v15_;
};

constexpr Enum(Enum const& other)
{
    intr::enum_copy_construct(&other, this);
}

constexpr Enum(Enum && other)
{
    intr::enum_move_construct(&other, this);
}

constexpr Enum& operator=(Enum const& other)
{
    intr::enum_copy_assign(&other, this);
    return *this;
}

constexpr Enum& operator=(Enum && other)
{
    intr::enum_move_assign(&other, this);
    return *this;
}

constexpr ~Enum()
{
    intr::enum_destruct(this);
}

template<usize I, typename ...Args>
constexpr Enum(V<I>, Args &&... args)
{
    intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}


constexpr Enum(T0 v) :
index_{0},
v0_{static_cast<T0 &&>(v)}
{ }


constexpr Enum(T1 v) :
index_{1},
v1_{static_cast<T1 &&>(v)}
{ }


constexpr Enum(T2 v) :
index_{2},
v2_{static_cast<T2 &&>(v)}
{ }


constexpr Enum(T3 v) :
index_{3},
v3_{static_cast<T3 &&>(v)}
{ }


constexpr Enum(T4 v) :
index_{4},
v4_{static_cast<T4 &&>(v)}
{ }


constexpr Enum(T5 v) :
index_{5},
v5_{static_cast<T5 &&>(v)}
{ }


constexpr Enum(T6 v) :
index_{6},
v6_{static_cast<T6 &&>(v)}
{ }


constexpr Enum(T7 v) :
index_{7},
v7_{static_cast<T7 &&>(v)}
{ }


constexpr Enum(T8 v) :
index_{8},
v8_{static_cast<T8 &&>(v)}
{ }


constexpr Enum(T9 v) :
index_{9},
v9_{static_cast<T9 &&>(v)}
{ }


constexpr Enum(T10 v) :
index_{10},
v10_{static_cast<T10 &&>(v)}
{ }


constexpr Enum(T11 v) :
index_{11},
v11_{static_cast<T11 &&>(v)}
{ }


constexpr Enum(T12 v) :
index_{12},
v12_{static_cast<T12 &&>(v)}
{ }


constexpr Enum(T13 v) :
index_{13},
v13_{static_cast<T13 &&>(v)}
{ }


constexpr Enum(T14 v) :
index_{14},
v14_{static_cast<T14 &&>(v)}
{ }


constexpr Enum(T15 v) :
index_{15},
v15_{static_cast<T15 &&>(v)}
{ }


template<usize I> requires(I < SIZE)
constexpr bool is()
{
    return index_ == I;
}

template<usize I> requires(I < SIZE)
constexpr auto& operator[](V<I>)
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<usize I> requires(I < SIZE)
constexpr auto const& operator[](V<I>) const
{
    CHECK_DESC(index_ == I, "Accessed Enum value at index: ", I, " but index is: ", index_);
    return intr::enum_member<I>(*this);
}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas)
{

}

template<typename... Lambdas> requires(sizeof...(Lambdas) == SIZE)
constexpr decltype(auto) match(Lambdas && ... lambdas) const
{

}

template<typename ... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors);

template<typename... Visitors> 
constexpr decltype(auto) visit(Visitors && ... visitors) const;


};
    
} // namespace ash
