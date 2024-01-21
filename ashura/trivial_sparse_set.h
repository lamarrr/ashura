#pragma once
#include "ashura/allocator.h"
#include "ashura/mem.h"

namespace ash
{
namespace gfx
{

/// @brief
/// @tparam T: contained type, must be movable
/// @tparam SizeT: size type, u8, u32, u64, u128
//
// https://skypjack.github.io/2019-05-06-ecs-baf-part-3/
template <typename T, typename SizeT>
struct TrivialSparseSet
{
  using UidT             = T;
  T     *data            = nullptr;
  SizeT *id_map          = nullptr;
  SizeT  data_capacity   = 0;
  SizeT  id_map_capacity = 0;
  SizeT  next_free       = -1;
  SizeT  num_free        = 0;
  SizeT  num_item_slots  = 0;
  // TODO(lamarrr): next consistent in all methods

  constexpr T *get_unsafe(UidT item) const
  {
    return data + id_map[item];
  }

  constexpr bool is_valid(UidT item) const
  {
    return true;
  }

  constexpr T *get(UidT item) const
  {
    return is_valid(item) ? get_unsafe(item) : nullptr;
  }

  bool push(AllocatorImpl const &allocator, T const &item, UidT &out_id)
  {
    // need fast id reclamation
    //
    // find free pool (starting from the last)
    // if none found, reserve_pools(num_pools + 1)
    // insert into new pool
    // allocate id, starting from the last pool
    return true;
  }

  void erase_unsafe(UidT item)
  {
  }

  bool erase(UidT item)
  {
    if (!is_valid(item))
    {
      return false;
    }
    return true;
  }

  void clear();

  void reset(AllocatorImpl const &allocator);
};

}        // namespace gfx
}        // namespace ash
