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
  using UidT                 = T;
  T     *data                = nullptr;
  SizeT *free_sizes          = nullptr;
  // SizeT *free_list = nullptr;
  SizeT *id_map              = nullptr;
  SizeT  data_capacity       = 0;
  SizeT  free_sizes_capacity = 0;
  SizeT  id_map_capacity     = 0;
  SizeT  pool_size           = 0;
  SizeT  num_pools           = 0;
  // SizeT  next_free           = -1;
  // SizeT  num_free            = 0;
  // TODO(lamarrr): next consistent in all methods

  constexpr SizeT items_capacity() const
  {
    return pool_size * num_pools;
  }

  constexpr SizeT count_items() const
  {
    SizeT count = 0;
    for (SizeT i = 0; i < num_pools; i++)
    {
      count += pool_size - free_sizes[i];
    }
  }

  constexpr T *get_unsafe(UidT item) const
  {
    return data + id_map[item];
  }

  constexpr bool is_valid(UidT item) const
  {
    if (item > items_capacity())
    {
      return false;
    }

    for (SizeT i = 0; i < num_pools; i++)
    {
      // check
      free_sizes[i];
    }

    return true;
  }

  constexpr T *get(UidT item) const
  {
    return is_valid(item) ? get_unsafe(item) : nullptr;
  }

  bool reserve_pools(AllocatorImpl const &allocator, SizeT num_reserve_pools)
  {
    if (num_pools >= num_reserve_pools)
    {
      return true;
    }

    SizeT *new_free_sizes = allocator.reallocate_typed(
        free_sizes, free_sizes_capacity, num_reserve_pools);
    if (new_free_sizes == nullptr)
    {
      return false;
    }

    free_sizes          = new_free_sizes;
    free_sizes_capacity = num_reserve_pools;

    SizeT *new_id_map = allocator.reallocate_typed(
        id_map, id_map_capacity, pool_size * num_reserve_pools);
    if (new_id_map == nullptr)
    {
      return false;
    }

    id_map          = new_id_map;
    id_map_capacity = pool_size * num_reserve_pools;

    T *new_data = allocator.reallocate_typed(data, data_capacity,
                                             pool_size * num_reserve_pools);
    if (new_data == nullptr)
    {
      return false;
    }

    data          = new_data;
    data_capacity = pool_size * num_reserve_pools;

    for (SizeT i = pool_size * num_pools; i < pool_size * num_reserve_pools;
         i++)
    {
      id_map[i] = -1;
    }

    for (SizeT i = num_pools; i < num_reserve_pools; i++)
    {
      free_sizes[i] = pool_size;
    }

    num_pools = num_reserve_pools;
    return true;
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
    SizeT    index = id_map[item];
    SizeT    pool  = index / pool_size;
    T *const pool_end =
        data + pool * pool_size + (pool_size - free_sizes[pool]);
    T *item_ptr = data + index;
    item_ptr++;
    // adjust id to index mapping
    while (item_ptr < pool_end)
    {
      mem::copy(item_ptr, Span{item_ptr - 1, 1});
      item_ptr++;
    }
    id_map[item] = -1;
    free_sizes[pool]++;
  }

  bool erase(UidT item)
  {
    if (!is_valid(item))
    {
      return false;
    }

    if (id_map[item] == -1)
    {
      return false;
    }

    erase_unsafe(item);
    return true;
  }

  void clear()
  {
    for (SizeT i = 0; i < num_pools; i++)
    {
      free_sizes[i] = pool_size;
    }
  }

  void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(data, data_capacity);
    allocator.deallocate_typed(free_sizes, data_capacity);
    allocator.deallocate_typed(id_map, data_capacity);
    data                = nullptr;
    free_sizes          = nullptr;
    id_map              = nullptr;
    data_capacity       = 0;
    free_sizes_capacity = 0;
    id_map_capacity     = 0;
    num_pools           = 0;
  }
};

}        // namespace gfx
}        // namespace ash
