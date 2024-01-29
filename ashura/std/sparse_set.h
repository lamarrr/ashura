#pragma once
#include "ashura/std/algorithms.h"
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
namespace ash
{

/// @brief An externally managed sparse set
/// @tparam SizeT: size type, u8, u32, u64
///
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
template <typename SizeType>
struct SparseSet
{
  static_assert(!Traits<SizeType>::SIGNED);
  static constexpr SizeType STUB         = Traits<SizeType>::MAX;
  static constexpr SizeType RELEASE_MASK = ~(STUB >> 1);
  static constexpr SizeType MAX_ELEMENTS = STUB >> 1;
  static constexpr SizeType MAX_ID       = MAX_ELEMENTS;

  SizeType *index_to_id          = nullptr;
  SizeType *id_to_index          = nullptr;
  SizeType  free_id_head         = STUB;
  SizeType  free_index_head      = STUB;
  SizeType  num_free             = 0;
  SizeType  num_slots            = 0;
  SizeType  index_to_id_capacity = 0;
  SizeType  id_to_index_capacity = 0;

  // the minimum required capacity of the referred-to external array
  constexpr SizeType required_capacity() const
  {
    return num_slots;
  }

  // clear all slots and id allocations
  constexpr void clear()
  {
    free_id_head    = STUB;
    free_index_head = STUB;
    num_free        = 0;
    num_slots       = 0;
  }

  // release all allocated ids
  constexpr void release_ids()
  {
    SizeType head   = num_slots == 0 ? STUB : 0;
    free_id_head    = head;
    free_index_head = head;
    for (SizeType i = 0; i < num_slots - 1; i++)
    {
      index_to_id[i] = (i + 1) | RELEASE_MASK;
      id_to_index[i] = (i + 1) | RELEASE_MASK;
    }
    if (num_slots > 0)
    {
      index_to_id[num_slots - 1] = STUB;
      id_to_index[num_slots - 1] = STUB;
    }
    num_free = num_slots;
  }

  constexpr void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(index_to_id, index_to_id_capacity);
    allocator.deallocate_typed(id_to_index, id_to_index_capacity);
    index_to_id          = nullptr;
    id_to_index          = nullptr;
    free_id_head         = STUB;
    free_index_head      = STUB;
    num_free             = 0;
    num_slots            = 0;
    index_to_id_capacity = 0;
    id_to_index_capacity = 0;
  }

  [[nodiscard]] constexpr bool is_valid_id(SizeType id) const
  {
    return id < num_slots && !(id_to_index[id] & RELEASE_MASK);
  }

  [[nodiscard]] constexpr SizeType unsafe_to_index(SizeType id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr bool to_index(SizeType id, SizeType &index) const
  {
    if (!is_valid_id(id))
    {
      return false;
    }

    index = unsafe_to_index(id);
    return true;
  }

  constexpr void unsafe_release(SizeType id)
  {
    SizeType const index = id_to_index[id];
    index_to_id[index]   = RELEASE_MASK | free_index_head;
    id_to_index[id]      = RELEASE_MASK | free_id_head;
    free_id_head         = id;
    free_index_head      = index;
    num_free++;
  }

  [[nodiscard]] constexpr bool release(SizeType id)
  {
    if (!is_valid_id(id))
    {
      return false;
    }
    unsafe_release(id);
    return true;
  }

  [[nodiscard]] constexpr bool reserve_memory(AllocatorImpl const &allocator,
                                              SizeType target_capacity)
  {
    if (target_capacity > index_to_id_capacity)
    {
      SizeType *new_index_to_id = allocator.reallocate_typed(
          index_to_id, index_to_id_capacity, target_capacity);
      if (new_index_to_id == nullptr)
      {
        return false;
      }
      index_to_id_capacity = target_capacity;
      index_to_id          = new_index_to_id;
    }

    if (target_capacity > id_to_index_capacity)
    {
      SizeType *new_id_to_index = allocator.reallocate_typed(
          id_to_index, id_to_index_capacity, target_capacity);
      if (new_id_to_index == nullptr)
      {
        return false;
      }
      id_to_index_capacity = target_capacity;
      id_to_index          = new_id_to_index;
    }

    return true;
  }

  [[nodiscard]] constexpr bool reserve_new_ids(AllocatorImpl const &allocator,
                                               SizeType num_extra_slots)
  {
    if (num_extra_slots == 0)
    {
      return true;
    }

    SizeType const new_num_slots = num_slots + num_extra_slots;

    if (!reserve_memory(allocator, new_num_slots))
    {
      return false;
    }

    for (SizeType index = num_slots; index < new_num_slots - 1; index++)
    {
      index_to_id[index] = RELEASE_MASK | (index + 1);
      id_to_index[index] = RELEASE_MASK | (index + 1);
    }

    index_to_id[new_num_slots - 1] = RELEASE_MASK | free_index_head;
    id_to_index[new_num_slots - 1] = RELEASE_MASK | free_id_head;
    free_index_head                = num_slots;
    free_id_head                   = num_slots;
    num_slots += num_extra_slots;
    num_free += num_extra_slots;
    return true;
  }

  [[nodiscard]] constexpr bool allocate_id(SizeType &out_id)
  {
    if (num_free == 0)
    {
      return false;
    }

    SizeType const index = free_index_head;
    SizeType const id    = free_id_head;
    free_id_head         = ~RELEASE_MASK & id_to_index[free_id_head];
    free_index_head      = ~RELEASE_MASK & index_to_id[free_index_head];
    index_to_id[index]   = id;
    id_to_index[id]      = index;
    out_id               = id;
    num_free--;
    return true;
  }

  template <typename Relocate>
  constexpr void compact(Relocate &&relocate_op)
  {
    // starting from index num_valid elements to end of the array, move the
    // valid elements into the holes that have indices lower than (num_slots -
    // num_free) as gotten from the implict free index list
    //
    SizeType const num_valid           = num_slots - num_free;
    SizeType       dst                 = free_index_head;
    SizeType       prev_free_index     = STUB;
    SizeType       new_free_index_head = STUB;

    for (SizeType src = num_valid; src < num_slots; src++)
    {
      if (index_to_id[src] & RELEASE_MASK)
      {
        continue;
      }

      while (dst >= num_valid)
      {
        if (new_free_index_head == STUB)
        {
          new_free_index_head = dst;
        }
        prev_free_index = dst;
        dst             = index_to_id[dst] & ~RELEASE_MASK;
      }

      // update the free index list, consuming this index from the list
      if (prev_free_index != STUB)
      {
        // should be a masked free index or STUB
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
