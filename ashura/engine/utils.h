#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

constexpr void bit_relocate(u64 *arr, usize src_index, usize dst_index)
{
  u16 const   src_pack_index = src_index & 63ULL;
  u16 const   dst_pack_index = dst_index & 63ULL;
  usize const src_pack       = src_index >> 6;
  usize const dst_pack       = dst_index >> 6;
  u64 const   src_bit        = (arr[src_pack] >> src_pack_index) & 1;
  u64         dst_pack_bits  = arr[dst_pack];
  dst_pack_bits &= ~(1ULL << dst_pack_index);
  dst_pack_bits |= (src_bit << dst_pack_index);
  arr[dst_pack] = dst_pack_bits;
}

constexpr void bit_relocate(u32 *arr, usize src_index, usize dst_index)
{
  u16 const   src_pack_index = src_index & 31ULL;
  u16 const   dst_pack_index = dst_index & 31ULL;
  usize const src_pack       = src_index >> 5;
  usize const dst_pack       = dst_index >> 5;
  u32 const   src_bit        = (arr[src_pack] >> src_pack_index) & 1;
  u32         dst_pack_bits  = arr[dst_pack];
  dst_pack_bits &= ~(1ULL << dst_pack_index);
  dst_pack_bits |= (src_bit << dst_pack_index);
  arr[dst_pack] = dst_pack_bits;
}

template <typename T>
void trivial_relocate(T *arr, usize src_index, usize dst_index)
{
  mem::copy(arr + src_index, arr + dst_index, 1);
}

}        // namespace ash
