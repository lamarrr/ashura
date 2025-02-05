/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once
#include "ashura/std/v.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/tuple.h"

namespace ash
{

inline constexpr unsigned int MAX_ENUM_SIZE = 32;

template<typename ... T>
requires(sizeof...(T) <= MAX_ENUM_SIZE)
struct Enum;

namespace intr{

template<unsigned int I, typename Enum>
constexpr auto& enum_member(Enum& e)
{

if constexpr(I == 0)
{
  return e.v0_;
}
else
if constexpr(I == 1)
{
  return e.v1_;
}
else
if constexpr(I == 2)
{
  return e.v2_;
}
else
if constexpr(I == 3)
{
  return e.v3_;
}
else
if constexpr(I == 4)
{
  return e.v4_;
}
else
if constexpr(I == 5)
{
  return e.v5_;
}
else
if constexpr(I == 6)
{
  return e.v6_;
}
else
if constexpr(I == 7)
{
  return e.v7_;
}
else
if constexpr(I == 8)
{
  return e.v8_;
}
else
if constexpr(I == 9)
{
  return e.v9_;
}
else
if constexpr(I == 10)
{
  return e.v10_;
}
else
if constexpr(I == 11)
{
  return e.v11_;
}
else
if constexpr(I == 12)
{
  return e.v12_;
}
else
if constexpr(I == 13)
{
  return e.v13_;
}
else
if constexpr(I == 14)
{
  return e.v14_;
}
else
if constexpr(I == 15)
{
  return e.v15_;
}
else
if constexpr(I == 16)
{
  return e.v16_;
}
else
if constexpr(I == 17)
{
  return e.v17_;
}
else
if constexpr(I == 18)
{
  return e.v18_;
}
else
if constexpr(I == 19)
{
  return e.v19_;
}
else
if constexpr(I == 20)
{
  return e.v20_;
}
else
if constexpr(I == 21)
{
  return e.v21_;
}
else
if constexpr(I == 22)
{
  return e.v22_;
}
else
if constexpr(I == 23)
{
  return e.v23_;
}
else
if constexpr(I == 24)
{
  return e.v24_;
}
else
if constexpr(I == 25)
{
  return e.v25_;
}
else
if constexpr(I == 26)
{
  return e.v26_;
}
else
if constexpr(I == 27)
{
  return e.v27_;
}
else
if constexpr(I == 28)
{
  return e.v28_;
}
else
if constexpr(I == 29)
{
  return e.v29_;
}
else
if constexpr(I == 30)
{
  return e.v30_;
}
else
if constexpr(I == 31)
{
  return e.v31_;
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
if constexpr(Enum::SIZE > 0)
{
if(e->index_ == 0)
{
  enum_member_destruct(&e->v0_);
  return;
}
}

if constexpr(Enum::SIZE > 1)
{
if(e->index_ == 1)
{
  enum_member_destruct(&e->v1_);
  return;
}
}

if constexpr(Enum::SIZE > 2)
{
if(e->index_ == 2)
{
  enum_member_destruct(&e->v2_);
  return;
}
}

if constexpr(Enum::SIZE > 3)
{
if(e->index_ == 3)
{
  enum_member_destruct(&e->v3_);
  return;
}
}

if constexpr(Enum::SIZE > 4)
{
if(e->index_ == 4)
{
  enum_member_destruct(&e->v4_);
  return;
}
}

if constexpr(Enum::SIZE > 5)
{
if(e->index_ == 5)
{
  enum_member_destruct(&e->v5_);
  return;
}
}

if constexpr(Enum::SIZE > 6)
{
if(e->index_ == 6)
{
  enum_member_destruct(&e->v6_);
  return;
}
}

if constexpr(Enum::SIZE > 7)
{
if(e->index_ == 7)
{
  enum_member_destruct(&e->v7_);
  return;
}
}

if constexpr(Enum::SIZE > 8)
{
if(e->index_ == 8)
{
  enum_member_destruct(&e->v8_);
  return;
}
}

if constexpr(Enum::SIZE > 9)
{
if(e->index_ == 9)
{
  enum_member_destruct(&e->v9_);
  return;
}
}

if constexpr(Enum::SIZE > 10)
{
if(e->index_ == 10)
{
  enum_member_destruct(&e->v10_);
  return;
}
}

if constexpr(Enum::SIZE > 11)
{
if(e->index_ == 11)
{
  enum_member_destruct(&e->v11_);
  return;
}
}

if constexpr(Enum::SIZE > 12)
{
if(e->index_ == 12)
{
  enum_member_destruct(&e->v12_);
  return;
}
}

if constexpr(Enum::SIZE > 13)
{
if(e->index_ == 13)
{
  enum_member_destruct(&e->v13_);
  return;
}
}

if constexpr(Enum::SIZE > 14)
{
if(e->index_ == 14)
{
  enum_member_destruct(&e->v14_);
  return;
}
}

if constexpr(Enum::SIZE > 15)
{
if(e->index_ == 15)
{
  enum_member_destruct(&e->v15_);
  return;
}
}

if constexpr(Enum::SIZE > 16)
{
if(e->index_ == 16)
{
  enum_member_destruct(&e->v16_);
  return;
}
}

if constexpr(Enum::SIZE > 17)
{
if(e->index_ == 17)
{
  enum_member_destruct(&e->v17_);
  return;
}
}

if constexpr(Enum::SIZE > 18)
{
if(e->index_ == 18)
{
  enum_member_destruct(&e->v18_);
  return;
}
}

if constexpr(Enum::SIZE > 19)
{
if(e->index_ == 19)
{
  enum_member_destruct(&e->v19_);
  return;
}
}

if constexpr(Enum::SIZE > 20)
{
if(e->index_ == 20)
{
  enum_member_destruct(&e->v20_);
  return;
}
}

if constexpr(Enum::SIZE > 21)
{
if(e->index_ == 21)
{
  enum_member_destruct(&e->v21_);
  return;
}
}

if constexpr(Enum::SIZE > 22)
{
if(e->index_ == 22)
{
  enum_member_destruct(&e->v22_);
  return;
}
}

if constexpr(Enum::SIZE > 23)
{
if(e->index_ == 23)
{
  enum_member_destruct(&e->v23_);
  return;
}
}

if constexpr(Enum::SIZE > 24)
{
if(e->index_ == 24)
{
  enum_member_destruct(&e->v24_);
  return;
}
}

if constexpr(Enum::SIZE > 25)
{
if(e->index_ == 25)
{
  enum_member_destruct(&e->v25_);
  return;
}
}

if constexpr(Enum::SIZE > 26)
{
if(e->index_ == 26)
{
  enum_member_destruct(&e->v26_);
  return;
}
}

if constexpr(Enum::SIZE > 27)
{
if(e->index_ == 27)
{
  enum_member_destruct(&e->v27_);
  return;
}
}

if constexpr(Enum::SIZE > 28)
{
if(e->index_ == 28)
{
  enum_member_destruct(&e->v28_);
  return;
}
}

if constexpr(Enum::SIZE > 29)
{
if(e->index_ == 29)
{
  enum_member_destruct(&e->v29_);
  return;
}
}

if constexpr(Enum::SIZE > 30)
{
if(e->index_ == 30)
{
  enum_member_destruct(&e->v30_);
  return;
}
}

if constexpr(Enum::SIZE > 31)
{
if(e->index_ == 31)
{
  enum_member_destruct(&e->v31_);
  return;
}
}

}

template<typename Src, typename Dst>
constexpr void enum_move_construct(Src * src, Dst * dst){
if constexpr(Dst::SIZE > 1)
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

if constexpr(Dst::SIZE > 16)
{
if(src->index_ == 16)
{
  enum_member_construct(&dst->v16_, static_cast<typename Src::E16 &&>(src->v16_));
  return;
}
}

if constexpr(Dst::SIZE > 17)
{
if(src->index_ == 17)
{
  enum_member_construct(&dst->v17_, static_cast<typename Src::E17 &&>(src->v17_));
  return;
}
}

if constexpr(Dst::SIZE > 18)
{
if(src->index_ == 18)
{
  enum_member_construct(&dst->v18_, static_cast<typename Src::E18 &&>(src->v18_));
  return;
}
}

if constexpr(Dst::SIZE > 19)
{
if(src->index_ == 19)
{
  enum_member_construct(&dst->v19_, static_cast<typename Src::E19 &&>(src->v19_));
  return;
}
}

if constexpr(Dst::SIZE > 20)
{
if(src->index_ == 20)
{
  enum_member_construct(&dst->v20_, static_cast<typename Src::E20 &&>(src->v20_));
  return;
}
}

if constexpr(Dst::SIZE > 21)
{
if(src->index_ == 21)
{
  enum_member_construct(&dst->v21_, static_cast<typename Src::E21 &&>(src->v21_));
  return;
}
}

if constexpr(Dst::SIZE > 22)
{
if(src->index_ == 22)
{
  enum_member_construct(&dst->v22_, static_cast<typename Src::E22 &&>(src->v22_));
  return;
}
}

if constexpr(Dst::SIZE > 23)
{
if(src->index_ == 23)
{
  enum_member_construct(&dst->v23_, static_cast<typename Src::E23 &&>(src->v23_));
  return;
}
}

if constexpr(Dst::SIZE > 24)
{
if(src->index_ == 24)
{
  enum_member_construct(&dst->v24_, static_cast<typename Src::E24 &&>(src->v24_));
  return;
}
}

if constexpr(Dst::SIZE > 25)
{
if(src->index_ == 25)
{
  enum_member_construct(&dst->v25_, static_cast<typename Src::E25 &&>(src->v25_));
  return;
}
}

if constexpr(Dst::SIZE > 26)
{
if(src->index_ == 26)
{
  enum_member_construct(&dst->v26_, static_cast<typename Src::E26 &&>(src->v26_));
  return;
}
}

if constexpr(Dst::SIZE > 27)
{
if(src->index_ == 27)
{
  enum_member_construct(&dst->v27_, static_cast<typename Src::E27 &&>(src->v27_));
  return;
}
}

if constexpr(Dst::SIZE > 28)
{
if(src->index_ == 28)
{
  enum_member_construct(&dst->v28_, static_cast<typename Src::E28 &&>(src->v28_));
  return;
}
}

if constexpr(Dst::SIZE > 29)
{
if(src->index_ == 29)
{
  enum_member_construct(&dst->v29_, static_cast<typename Src::E29 &&>(src->v29_));
  return;
}
}

if constexpr(Dst::SIZE > 30)
{
if(src->index_ == 30)
{
  enum_member_construct(&dst->v30_, static_cast<typename Src::E30 &&>(src->v30_));
  return;
}
}

if constexpr(Dst::SIZE > 31)
{
if(src->index_ == 31)
{
  enum_member_construct(&dst->v31_, static_cast<typename Src::E31 &&>(src->v31_));
  return;
}
}

}

template<typename Src, typename Dst>
constexpr void enum_copy_construct(Src * src, Dst * dst){
if constexpr(Dst::SIZE > 1)
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

if constexpr(Dst::SIZE > 16)
{
if(src->index_ == 16)
{
  enum_member_construct(&dst->v16_, src->v16_);
  return;
}
}

if constexpr(Dst::SIZE > 17)
{
if(src->index_ == 17)
{
  enum_member_construct(&dst->v17_, src->v17_);
  return;
}
}

if constexpr(Dst::SIZE > 18)
{
if(src->index_ == 18)
{
  enum_member_construct(&dst->v18_, src->v18_);
  return;
}
}

if constexpr(Dst::SIZE > 19)
{
if(src->index_ == 19)
{
  enum_member_construct(&dst->v19_, src->v19_);
  return;
}
}

if constexpr(Dst::SIZE > 20)
{
if(src->index_ == 20)
{
  enum_member_construct(&dst->v20_, src->v20_);
  return;
}
}

if constexpr(Dst::SIZE > 21)
{
if(src->index_ == 21)
{
  enum_member_construct(&dst->v21_, src->v21_);
  return;
}
}

if constexpr(Dst::SIZE > 22)
{
if(src->index_ == 22)
{
  enum_member_construct(&dst->v22_, src->v22_);
  return;
}
}

if constexpr(Dst::SIZE > 23)
{
if(src->index_ == 23)
{
  enum_member_construct(&dst->v23_, src->v23_);
  return;
}
}

if constexpr(Dst::SIZE > 24)
{
if(src->index_ == 24)
{
  enum_member_construct(&dst->v24_, src->v24_);
  return;
}
}

if constexpr(Dst::SIZE > 25)
{
if(src->index_ == 25)
{
  enum_member_construct(&dst->v25_, src->v25_);
  return;
}
}

if constexpr(Dst::SIZE > 26)
{
if(src->index_ == 26)
{
  enum_member_construct(&dst->v26_, src->v26_);
  return;
}
}

if constexpr(Dst::SIZE > 27)
{
if(src->index_ == 27)
{
  enum_member_construct(&dst->v27_, src->v27_);
  return;
}
}

if constexpr(Dst::SIZE > 28)
{
if(src->index_ == 28)
{
  enum_member_construct(&dst->v28_, src->v28_);
  return;
}
}

if constexpr(Dst::SIZE > 29)
{
if(src->index_ == 29)
{
  enum_member_construct(&dst->v29_, src->v29_);
  return;
}
}

if constexpr(Dst::SIZE > 30)
{
if(src->index_ == 30)
{
  enum_member_construct(&dst->v30_, src->v30_);
  return;
}
}

if constexpr(Dst::SIZE > 31)
{
if(src->index_ == 31)
{
  enum_member_construct(&dst->v31_, src->v31_);
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


template<unsigned int SIZE, typename Enum, typename ... Fns>
constexpr decltype(auto) match(Enum && e, Fns && ... fns)
{
  Tuple<Fns && ...> fns_ref{ static_cast<Fns && >(fns)... };

 if constexpr(SIZE > 0)
{
  if(e.index_ == 0)
  {
    return fns_ref.v0(e.v0_);
  }
}
 if constexpr(SIZE > 1)
{
  if(e.index_ == 1)
  {
    return fns_ref.v1(e.v1_);
  }
}
 if constexpr(SIZE > 2)
{
  if(e.index_ == 2)
  {
    return fns_ref.v2(e.v2_);
  }
}
 if constexpr(SIZE > 3)
{
  if(e.index_ == 3)
  {
    return fns_ref.v3(e.v3_);
  }
}
 if constexpr(SIZE > 4)
{
  if(e.index_ == 4)
  {
    return fns_ref.v4(e.v4_);
  }
}
 if constexpr(SIZE > 5)
{
  if(e.index_ == 5)
  {
    return fns_ref.v5(e.v5_);
  }
}
 if constexpr(SIZE > 6)
{
  if(e.index_ == 6)
  {
    return fns_ref.v6(e.v6_);
  }
}
 if constexpr(SIZE > 7)
{
  if(e.index_ == 7)
  {
    return fns_ref.v7(e.v7_);
  }
}
 if constexpr(SIZE > 8)
{
  if(e.index_ == 8)
  {
    return fns_ref.v8(e.v8_);
  }
}
 if constexpr(SIZE > 9)
{
  if(e.index_ == 9)
  {
    return fns_ref.v9(e.v9_);
  }
}
 if constexpr(SIZE > 10)
{
  if(e.index_ == 10)
  {
    return fns_ref.v10(e.v10_);
  }
}
 if constexpr(SIZE > 11)
{
  if(e.index_ == 11)
  {
    return fns_ref.v11(e.v11_);
  }
}
 if constexpr(SIZE > 12)
{
  if(e.index_ == 12)
  {
    return fns_ref.v12(e.v12_);
  }
}
 if constexpr(SIZE > 13)
{
  if(e.index_ == 13)
  {
    return fns_ref.v13(e.v13_);
  }
}
 if constexpr(SIZE > 14)
{
  if(e.index_ == 14)
  {
    return fns_ref.v14(e.v14_);
  }
}
 if constexpr(SIZE > 15)
{
  if(e.index_ == 15)
  {
    return fns_ref.v15(e.v15_);
  }
}
 if constexpr(SIZE > 16)
{
  if(e.index_ == 16)
  {
    return fns_ref.v16(e.v16_);
  }
}
 if constexpr(SIZE > 17)
{
  if(e.index_ == 17)
  {
    return fns_ref.v17(e.v17_);
  }
}
 if constexpr(SIZE > 18)
{
  if(e.index_ == 18)
  {
    return fns_ref.v18(e.v18_);
  }
}
 if constexpr(SIZE > 19)
{
  if(e.index_ == 19)
  {
    return fns_ref.v19(e.v19_);
  }
}
 if constexpr(SIZE > 20)
{
  if(e.index_ == 20)
  {
    return fns_ref.v20(e.v20_);
  }
}
 if constexpr(SIZE > 21)
{
  if(e.index_ == 21)
  {
    return fns_ref.v21(e.v21_);
  }
}
 if constexpr(SIZE > 22)
{
  if(e.index_ == 22)
  {
    return fns_ref.v22(e.v22_);
  }
}
 if constexpr(SIZE > 23)
{
  if(e.index_ == 23)
  {
    return fns_ref.v23(e.v23_);
  }
}
 if constexpr(SIZE > 24)
{
  if(e.index_ == 24)
  {
    return fns_ref.v24(e.v24_);
  }
}
 if constexpr(SIZE > 25)
{
  if(e.index_ == 25)
  {
    return fns_ref.v25(e.v25_);
  }
}
 if constexpr(SIZE > 26)
{
  if(e.index_ == 26)
  {
    return fns_ref.v26(e.v26_);
  }
}
 if constexpr(SIZE > 27)
{
  if(e.index_ == 27)
  {
    return fns_ref.v27(e.v27_);
  }
}
 if constexpr(SIZE > 28)
{
  if(e.index_ == 28)
  {
    return fns_ref.v28(e.v28_);
  }
}
 if constexpr(SIZE > 29)
{
  if(e.index_ == 29)
  {
    return fns_ref.v29(e.v29_);
  }
}
 if constexpr(SIZE > 30)
{
  if(e.index_ == 30)
  {
    return fns_ref.v30(e.v30_);
  }
}
 if constexpr(SIZE > 31)
{
  if(e.index_ == 31)
  {
    return fns_ref.v31(e.v31_);
  }
}

  ASH_UNREACHABLE;
}

} // namespace intr


template<>
struct Enum<>
{





static constexpr unsigned int SIZE = 0;

static constexpr unsigned int size()
{
  return SIZE;
}


constexpr bool is(unsigned int) const
{
  return false;
}

constexpr void match()
{
}

constexpr void match() const
{
}

};
    
template<typename T0>
struct Enum<T0>
{

typedef T0 E0;

template<unsigned int I>
using E = index_pack<I, E0>;


static constexpr unsigned int SIZE = 1;

static constexpr unsigned int size()
{
  return SIZE;
}

static constexpr unsigned int index_ = 0;

static constexpr unsigned int index()
{
  return 0;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
{
  intr::enum_member_construct(&intr::enum_member<I>(*this), static_cast<Args &&>(args)...);
}

constexpr Enum(T0 v) :
v0_{static_cast<T0 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1>
struct Enum<T0, T1>
{

typedef T0 E0;	typedef T1 E1;

template<unsigned int I>
using E = index_pack<I, E0, E1>;


static constexpr unsigned int SIZE = 2;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2>
struct Enum<T0, T1, T2>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2>;


static constexpr unsigned int SIZE = 3;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3>
struct Enum<T0, T1, T2, T3>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3>;


static constexpr unsigned int SIZE = 4;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4>
struct Enum<T0, T1, T2, T3, T4>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4>;


static constexpr unsigned int SIZE = 5;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct Enum<T0, T1, T2, T3, T4, T5>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5>;


static constexpr unsigned int SIZE = 6;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Enum<T0, T1, T2, T3, T4, T5, T6>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6>;


static constexpr unsigned int SIZE = 7;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7>;


static constexpr unsigned int SIZE = 8;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8>;


static constexpr unsigned int SIZE = 9;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9>;


static constexpr unsigned int SIZE = 10;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10>;


static constexpr unsigned int SIZE = 11;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11>;


static constexpr unsigned int SIZE = 12;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12>;


static constexpr unsigned int SIZE = 13;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13>;


static constexpr unsigned int SIZE = 14;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14>;


static constexpr unsigned int SIZE = 15;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15>;


static constexpr unsigned int SIZE = 16;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16>;


static constexpr unsigned int SIZE = 17;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17>;


static constexpr unsigned int SIZE = 18;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18>;


static constexpr unsigned int SIZE = 19;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19>;


static constexpr unsigned int SIZE = 20;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20>;


static constexpr unsigned int SIZE = 21;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21>;


static constexpr unsigned int SIZE = 22;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22>;


static constexpr unsigned int SIZE = 23;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23>;


static constexpr unsigned int SIZE = 24;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24>;


static constexpr unsigned int SIZE = 25;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25>;


static constexpr unsigned int SIZE = 26;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26>;


static constexpr unsigned int SIZE = 27;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;	typedef T27 E27;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27>;


static constexpr unsigned int SIZE = 28;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;	T27 v27_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }

constexpr Enum(T27 v) :
index_{27},
v27_{static_cast<T27 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;	typedef T27 E27;	typedef T28 E28;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28>;


static constexpr unsigned int SIZE = 29;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;	T27 v27_;	T28 v28_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }

constexpr Enum(T27 v) :
index_{27},
v27_{static_cast<T27 &&>(v)}
{ }

constexpr Enum(T28 v) :
index_{28},
v28_{static_cast<T28 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;	typedef T27 E27;	typedef T28 E28;	typedef T29 E29;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29>;


static constexpr unsigned int SIZE = 30;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;	T27 v27_;	T28 v28_;	T29 v29_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }

constexpr Enum(T27 v) :
index_{27},
v27_{static_cast<T27 &&>(v)}
{ }

constexpr Enum(T28 v) :
index_{28},
v28_{static_cast<T28 &&>(v)}
{ }

constexpr Enum(T29 v) :
index_{29},
v29_{static_cast<T29 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;	typedef T27 E27;	typedef T28 E28;	typedef T29 E29;	typedef T30 E30;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30>;


static constexpr unsigned int SIZE = 31;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;	T27 v27_;	T28 v28_;	T29 v29_;	T30 v30_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }

constexpr Enum(T27 v) :
index_{27},
v27_{static_cast<T27 &&>(v)}
{ }

constexpr Enum(T28 v) :
index_{28},
v28_{static_cast<T28 &&>(v)}
{ }

constexpr Enum(T29 v) :
index_{29},
v29_{static_cast<T29 &&>(v)}
{ }

constexpr Enum(T30 v) :
index_{30},
v30_{static_cast<T30 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
struct Enum<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>
{

typedef T0 E0;	typedef T1 E1;	typedef T2 E2;	typedef T3 E3;	typedef T4 E4;	typedef T5 E5;	typedef T6 E6;	typedef T7 E7;	typedef T8 E8;	typedef T9 E9;	typedef T10 E10;	typedef T11 E11;	typedef T12 E12;	typedef T13 E13;	typedef T14 E14;	typedef T15 E15;	typedef T16 E16;	typedef T17 E17;	typedef T18 E18;	typedef T19 E19;	typedef T20 E20;	typedef T21 E21;	typedef T22 E22;	typedef T23 E23;	typedef T24 E24;	typedef T25 E25;	typedef T26 E26;	typedef T27 E27;	typedef T28 E28;	typedef T29 E29;	typedef T30 E30;	typedef T31 E31;

template<unsigned int I>
using E = index_pack<I, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31>;


static constexpr unsigned int SIZE = 32;

static constexpr unsigned int size()
{
  return SIZE;
}

unsigned int index_;

constexpr unsigned int index() const
{
  return index_;
}

union {
T0 v0_;	T1 v1_;	T2 v2_;	T3 v3_;	T4 v4_;	T5 v5_;	T6 v6_;	T7 v7_;	T8 v8_;	T9 v9_;	T10 v10_;	T11 v11_;	T12 v12_;	T13 v13_;	T14 v14_;	T15 v15_;	T16 v16_;	T17 v17_;	T18 v18_;	T19 v19_;	T20 v20_;	T21 v21_;	T22 v22_;	T23 v23_;	T24 v24_;	T25 v25_;	T26 v26_;	T27 v27_;	T28 v28_;	T29 v29_;	T30 v30_;	T31 v31_;
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

template<unsigned int I, typename ...Args>
constexpr Enum(V<I>, Args &&... args) requires(I < SIZE)
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

constexpr Enum(T16 v) :
index_{16},
v16_{static_cast<T16 &&>(v)}
{ }

constexpr Enum(T17 v) :
index_{17},
v17_{static_cast<T17 &&>(v)}
{ }

constexpr Enum(T18 v) :
index_{18},
v18_{static_cast<T18 &&>(v)}
{ }

constexpr Enum(T19 v) :
index_{19},
v19_{static_cast<T19 &&>(v)}
{ }

constexpr Enum(T20 v) :
index_{20},
v20_{static_cast<T20 &&>(v)}
{ }

constexpr Enum(T21 v) :
index_{21},
v21_{static_cast<T21 &&>(v)}
{ }

constexpr Enum(T22 v) :
index_{22},
v22_{static_cast<T22 &&>(v)}
{ }

constexpr Enum(T23 v) :
index_{23},
v23_{static_cast<T23 &&>(v)}
{ }

constexpr Enum(T24 v) :
index_{24},
v24_{static_cast<T24 &&>(v)}
{ }

constexpr Enum(T25 v) :
index_{25},
v25_{static_cast<T25 &&>(v)}
{ }

constexpr Enum(T26 v) :
index_{26},
v26_{static_cast<T26 &&>(v)}
{ }

constexpr Enum(T27 v) :
index_{27},
v27_{static_cast<T27 &&>(v)}
{ }

constexpr Enum(T28 v) :
index_{28},
v28_{static_cast<T28 &&>(v)}
{ }

constexpr Enum(T29 v) :
index_{29},
v29_{static_cast<T29 &&>(v)}
{ }

constexpr Enum(T30 v) :
index_{30},
v30_{static_cast<T30 &&>(v)}
{ }

constexpr Enum(T31 v) :
index_{31},
v31_{static_cast<T31 &&>(v)}
{ }


constexpr bool is(unsigned int i) const
{
  return index_ == i;
}

template<unsigned int I> requires(I < SIZE)
constexpr auto& get(V<I>) &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const& get(V<I>) const &
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto&& get(V<I>) &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I> requires(I < SIZE)
constexpr auto const&& get(V<I>) const &&
{
  CHECK(index_ == I, "Accessed Enum type: {} but type is: {}", I, index_);
  return intr::enum_member<I>(*this);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &
{
  return get(v);
}

template<unsigned int I>
constexpr auto const& operator[](V<I> v) const &
{
  return get(v);
}

template<unsigned int I>
constexpr auto& operator[](V<I> v) &&
{
  return get(v);
}

template<unsigned int I>
constexpr auto const&& operator[](V<I> v) const &&
{
  return get(v);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns)
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}

template<typename... Fns>
requires(sizeof...(Fns) == SIZE)
constexpr decltype(auto) match(Fns && ... fns) const
{
  return intr::match<SIZE>(*this, static_cast<Fns &&>(fns)...);
}


};
    
} // namespace ash
