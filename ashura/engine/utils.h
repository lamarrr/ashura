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

constexpr bool get(u64 const *arr, usize index)
{
  u8 const    pack_index = index & 63ULL;
  usize const pack       = index >> 6;
  return (arr[pack] & (1ULL << pack_index)) != 0;
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

constexpr void set(u64 *arr, usize index, bool bit)
{
  u64 const   bit_u64    = bit;
  u8 const    pack_index = index & 63ULL;
  usize const pack       = index >> 6;
  u64         src_bit    = arr[pack];
  src_bit &= ~(1ULL << pack_index);
  src_bit |= (bit_u64 << pack_index);
  arr[pack] = src_bit;
}

constexpr void or_bit(u64 *arr, usize index, bool bit)
{
  u64 const   bit_u64    = bit;
  u8 const    pack_index = index & 63ULL;
  usize const pack       = index >> 6;
  arr[pack] |= (bit_u64 << pack_index);
}

constexpr void and_bit(u64 *arr, usize index, bool bit)
{
  u64 const   bit_u64    = bit;
  u8 const    pack_index = index & 63ULL;
  usize const pack       = index >> 6;
  arr[pack] &= ~(1ULL << pack_index) | (bit_u64 << pack_index);
}

}        // namespace bitvec

template <typename T>
void trivial_relocate(T *arr, usize src_index, usize dst_index)
{
  mem::copy(arr + src_index, arr + dst_index, 1);
}

/// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
/// https://bruop.github.io/frustum_culling/
///
/// exploits the fact that in clip-space all vertices in the view frustum will
/// obey:
///
/// -w <= x << w
/// -w <= y << w
///  0 <= z << w
///
constexpr bool is_outside_frustum(Mat4 const &mvp, Box const &box)
{
  constexpr u8   NUM_CORNERS = 8;
  constexpr auto to_vec4     = [](Vec3 a) { return Vec4{a.x, a.y, a.z, 1}; };
  Vec4 const     corners[NUM_CORNERS] = {
      mvp * to_vec4(box.offset),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, 0}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, 0, box.extent.z}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, box.extent.z}),
      mvp * to_vec4(box.offset + box.extent),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, box.extent.z})};
  u8 left   = 0;
  u8 right  = 0;
  u8 top    = 0;
  u8 bottom = 0;
  u8 back   = 0;

  for (u8 i = 0; i < NUM_CORNERS; i++)
  {
    Vec4 const corner = corners[i];

    if (corner.x < -corner.w)
    {
      left++;
    }

    if (corner.x > corner.w)
    {
      right++;
    }

    if (corner.y < -corner.w)
    {
      bottom++;
    }

    if (corner.y > corner.w)
    {
      top++;
    }

    if (corner.z < 0)
    {
      back++;
    }
  }

  return left == NUM_CORNERS || right == NUM_CORNERS || top == NUM_CORNERS ||
         bottom == NUM_CORNERS || back == NUM_CORNERS;
}

}        // namespace ash
