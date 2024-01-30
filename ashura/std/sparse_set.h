#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/trivial_vec.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief An externally managed sparse set, the sparse set is always compacted
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
  SizeType  num_free             = 0;
  SizeType  num_slots            = 0;
  SizeType  index_to_id_capacity = 0;
  SizeType  id_to_index_capacity = 0;

  /// the minimum required capacity of the referred-to external array
  constexpr SizeType required_capacity() const
  {
    return num_slots;
  }

  /// the number of valid elements in the array
  constexpr SizeType num_valid() const
  {
    return num_slots - num_free;
  }

  /// clear all slots and id allocations
  /// all elements must have been destroyed
  constexpr void clear()
  {
    free_id_head = STUB;
    num_free     = 0;
    num_slots    = 0;
  }

  /// release all allocated ids.
  /// all elements must have been destroyed before calling this.
  constexpr void release_all()
  {
    if (num_slots > 0)
    {
      free_id_head = 0;
      for (SizeType i = 0; i < num_slots - 1; i++)
      {
        id_to_index[i] = (i + 1) | RELEASE_MASK;
      }
      id_to_index[num_slots - 1] = STUB;
      num_free                   = num_slots;
    }
  }

  constexpr void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(index_to_id, index_to_id_capacity);
    allocator.deallocate_typed(id_to_index, id_to_index_capacity);
    index_to_id          = nullptr;
    id_to_index          = nullptr;
    free_id_head         = STUB;
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

  template <typename Relocate>
  constexpr void unsafe_release(SizeType id, Relocate &&relocate_op)
  {
    SizeType const index = id_to_index[id];
    SizeType const last  = num_valid() - 1;
    if (index != last)
    {
      relocate_op(last, index);
    }
    id_to_index[index_to_id[last]] = index;
    index_to_id[index]             = index_to_id[last];
    id_to_index[id]                = free_id_head | RELEASE_MASK;
    free_id_head                   = id;
    num_free++;
  }

  // element at id must have already been destroyed.
  // Relocate: operation to move from initialized src to uninitialized dst, and
  // then destroy src
  template <typename Relocate>
  [[nodiscard]] constexpr bool release(SizeType id, Relocate &&relocate_op)
  {
    if (!is_valid_id(id))
    {
      return false;
    }
    unsafe_release(id, relocate_op);
    return true;
  }

  [[nodiscard]] constexpr bool reserve_memory(AllocatorImpl const &allocator,
                                              SizeType target_capacity)
  {
    return tvec::reserve(allocator, id_to_index, id_to_index_capacity,
                         target_capacity) &&
           tvec::reserve(allocator, index_to_id, index_to_id_capacity,
                         target_capacity);
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
      id_to_index[index] = RELEASE_MASK | (index + 1);
    }

    id_to_index[new_num_slots - 1] = RELEASE_MASK | free_id_head;
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

    SizeType const index = num_valid();
    SizeType const id    = free_id_head;
    free_id_head         = ~RELEASE_MASK & id_to_index[free_id_head];
    index_to_id[index]   = id;
    id_to_index[id]      = index;
    out_id               = id;
    num_free--;
    return true;
  }

  // TODO(lamarrr): sort_index_with_key, sort_id_with_key
};

}        // namespace ash
