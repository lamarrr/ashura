#pragma once
#include "ashura/std/algorithms.h"
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief
/// @tparam SizeT: size type, u8, u32, u64
///
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
template <typename SizeType = u64>
struct SparseSet
{
  static_assert(!IntTraits<SizeType>::SIGNED);

  static constexpr SizeType RELEASE_MASK =
      ((SizeType) 1) << (IntTraits<SizeType>::NUM_BITS - 1);
  static constexpr SizeType MAX_ELEMENTS = ~RELEASE_MASK;
  static constexpr SizeType MAX_ID       = ~RELEASE_MASK;

  SizeType *index_to_id          = nullptr;
  SizeType *id_to_index          = nullptr;
  SizeType  free_id_head         = -1;
  SizeType  free_index_head      = -1;
  SizeType  num_free             = 0;
  SizeType  num_slots            = 0;
  SizeType  index_to_id_capacity = 0;
  SizeType  id_to_index_capacity = 0;

  constexpr void clear()
  {
    SizeType head   = num_slots ? 0 : (SizeType) -1;
    free_id_head    = head;
    free_index_head = head;
    for (SizeType i = 0; i < num_slots - 1; i++)
    {
      index_to_id[i] = (i + 1) | RELEASE_MASK;
      id_to_index[i] = (i + 1) | RELEASE_MASK;
    }
    if (num_slots > 0)
    {
      index_to_id[num_slots - 1] = -1;
      id_to_index[num_slots - 1] = -1;
    }
    num_free = num_slots;
  }

  constexpr void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(index_to_id, index_to_id_capacity);
    allocator.deallocate_typed(id_to_index, id_to_index_capacity);
    index_to_id          = nullptr;
    id_to_index          = nullptr;
    free_id_head         = -1;
    free_index_head      = -1;
    num_free             = 0;
    num_slots            = 0;
    index_to_id_capacity = 0;
    id_to_index_capacity = 0;
  }

  [[nodiscard]] constexpr bool is_valid(SizeType id) const
  {
    return id < num_slots && !(id_to_index[id] & RELEASE_MASK);
  }

  [[nodiscard]] constexpr SizeType unsafe_to_index(SizeType id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr bool to_index(SizeType id, SizeType &index) const
  {
    if (!is_valid(id))
    {
      return false;
    }

    index = unsafe_to_index(id);
    return true;
  }

  constexpr void unsafe_remove(SizeType id)
  {
    SizeType const index = id_to_index[id];
    index_to_id[index]   = RELEASE_MASK | free_index_head;
    id_to_index[id]      = RELEASE_MASK | free_id_head;
    free_id_head         = id;
    free_index_head      = index;
    num_free++;
  }

  [[nodiscard]] constexpr bool remove(SizeType id)
  {
    if (!is_valid(id))
    {
      return false;
    }
    unsafe_remove(id);
    return true;
  }

  bool resize(AllocatorImpl const &allocator, SizeType size)
  {
    // if smaller, compact and then
    //
    // reserve needs to be called on elements as well to sync capacity
    // no free ids available,
    // allocate new id
    // don't use id until all operations are successfull, so we can return it
    // if it fails
    // no free indices available, i.e. array full
    // allocate new index and resize array
  }

  [[nodiscard]] constexpr bool push(AllocatorImpl const &allocator,
                                    SizeType            &out_id)
  {
    // still need to check if it is actually freee???
    if (!reserve(allocator, num_slots + 1))
    {
      return false;
    }
    SizeType const index = free_index_head;
    SizeType const id    = free_id_head;
    free_id_head         = id_to_index[free_id_head] & ~RELEASE_MASK;
    free_index_head      = index_to_id[free_index_head] & ~RELEASE_MASK;
    id_to_index[id]      = index;
    index_to_id[index]   = id;
    out_id               = id;
    return true;
  }

  template <typename Relocate>
  constexpr void compact(Relocate relocate_op)
  {
    // starting from index num_valid elements to end of the array, move the
    // valid elements into the holes that have indices lower than (num_slots -
    // num_free) as gotten from the implict free index list
    //
    SizeType const num_valid           = num_slots - num_free;
    SizeType       dst                 = free_index_head;
    SizeType       prev_free_index     = -1;
    SizeType       new_free_index_head = -1;

    for (SizeType src = num_valid; src < num_slots; src++)
    {
      for (; src < num_slots && !(index_to_id[src] & RELEASE_MASK); src++)
      {
      }

      while (dst >= num_valid)
      {
        if (new_free_index_head == -1)
        {
          new_free_index_head = dst;
        }
        prev_free_index = dst;
        dst             = index_to_id[dst] & ~RELEASE_MASK;
      }

      // update the free index list, consuming this index from the list
      if (prev_free_index != -1)
      {
        // should be a masked free index or -1
        index_to_id[prev_free_index] = index_to_id[dst];
      }

      relocate_op(src, dst);
      SizeType const next_dst       = index_to_id[dst] & ~RELEASE_MASK;
      id_to_index[index_to_id[src]] = dst;
      index_to_id[dst]              = index_to_id[src];
      index_to_id[src]              = new_free_index_head | RELEASE_MASK;
      new_free_index_head           = src;
      dst                           = next_dst;
    }

    free_index_head = new_free_index_head;
  }
};

}        // namespace ash
