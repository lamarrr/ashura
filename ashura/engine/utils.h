#pragma once
#include "ashura/std/box.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

namespace bitvec
{

constexpr usize size_u32(usize num_bits)
{
  return (num_bits >> 5) + !!(num_bits & 31);
}

constexpr usize size_u64(usize num_bits)
{
  return (num_bits >> 6) + !!(num_bits & 63);
}

constexpr void relocate(u64 *arr, usize src_index, usize dst_index)
{
  u8 const    src_pack_index = src_index & 63ULL;
  u8 const    dst_pack_index = dst_index & 63ULL;
  usize const src_pack       = src_index >> 6;
  usize const dst_pack       = dst_index >> 6;
  u64 const   src_bit        = (arr[src_pack] >> src_pack_index) & 1;
  u64         dst_pack_bits  = arr[dst_pack];
  dst_pack_bits &= ~(1ULL << dst_pack_index);
  dst_pack_bits |= (src_bit << dst_pack_index);
  arr[dst_pack] = dst_pack_bits;
}

constexpr void relocate(u32 *arr, usize src_index, usize dst_index)
{
  u8 const    src_pack_index = src_index & 31ULL;
  u8 const    dst_pack_index = dst_index & 31ULL;
  usize const src_pack       = src_index >> 5;
  usize const dst_pack       = dst_index >> 5;
  u32 const   src_bit        = (arr[src_pack] >> src_pack_index) & 1;
  u32         dst_pack_bits  = arr[dst_pack];
  dst_pack_bits &= ~(1ULL << dst_pack_index);
  dst_pack_bits |= (src_bit << dst_pack_index);
  arr[dst_pack] = dst_pack_bits;
}

}        // namespace bitvec

template <typename T>
void trivial_relocate(T *arr, usize src_index, usize dst_index)
{
  mem::copy(arr + src_index, arr + dst_index, 1);
}


}        // namespace ash
