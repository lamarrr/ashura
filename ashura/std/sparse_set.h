#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/trivial_vec.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T>
struct SparseVec
{
  using Type = T;

  AllocatorImpl allocator = {};
  T            *data      = nullptr;
  usize         capacity  = 0;
  usize         size      = 0;

  constexpr T &operator[](usize index) const
  {
    return data[index];
  }
  constexpr void clear();
  constexpr bool reserve(usize target_capacity);
  constexpr void destruct_element(usize index);
  constexpr void reset(usize count);
  constexpr void relocate(usize src, usize dst_uninit);
  constexpr T   *get(usize index);
  constexpr T   *begin() const;
  constexpr T   *end() const;
};

/// @brief An externally managed sparse set, the sparse set is always compacted
/// @tparam SizeT: size type, u8, u32, u64
///
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
/// @size: the number of valid elements in the sparse set
/// @capacity: the number of elements the sparse set has capacity for
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
template <typename SizeType>
struct SparseSet
{
  static_assert(!IntTraits<SizeType>::SIGNED);
  static constexpr SizeType STUB         = IntTraits<SizeType>::MAX;
  static constexpr SizeType RELEASE_MASK = ~(STUB >> 1);
  static constexpr SizeType MAX_ELEMENTS = STUB >> 1;
  static constexpr SizeType MAX_ID       = MAX_ELEMENTS;

  AllocatorImpl allocator            = {};
  SizeType      size                 = 0;
  SizeType      capacity             = 0;
  SizeType     *index_to_id          = nullptr;
  SizeType     *id_to_index          = nullptr;
  SizeType      free_id_head         = STUB;
  SizeType      index_to_id_capacity = 0;
  SizeType      id_to_index_capacity = 0;

  template <typename... T>
  constexpr void clear(SparseVec<T> &...sparse)
  {
    (sparse.clear(), ...);
    size = 0;
    if (capacity > 0)
    {
      free_id_head = 0;
      for (SizeType i = 0; i < capacity - 1; i++)
      {
        id_to_index[i] = (i + 1) | RELEASE_MASK;
      }
      id_to_index[capacity - 1] = STUB;
    }
    else
    {
      free_id_head = STUB;
    }
  }

  template <typename... T>
  constexpr void reset(SparseVec<T> &...sparse)
  {
    (sparse.reset(), ...);
    tvec::reset(allocator, id_to_index, id_to_index_capacity);
    tvec::reset(allocator, index_to_id, index_to_id_capacity);
    free_id_head = STUB;
    num_free     = 0;
    num_slots    = 0;
  }

  [[nodiscard]] constexpr bool is_valid_id(SizeType id) const
  {
    return id < capacity && !(id_to_index[id] & RELEASE_MASK);
  }

  [[nodiscard]] constexpr bool is_valid_index(SizeType index) const
  {
    return index < size;
  }

  [[nodiscard]] constexpr SizeType to_index(SizeType id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr SizeType operator[](SizeType id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr bool try_to_index(SizeType id, SizeType &index) const
  {
    if (!is_valid_id(id))
    {
      return false;
    }

    index = to_index(id);
    return true;
  }

  template <typename... T>
  constexpr void erase2(SizeType id, SparseVec<T> &...sparse)
  {
    SizeType const index = id_to_index[id];
    SizeType const last  = size - 1;
    (sparse.destruct_element(index), ...);
    if (index != last)
    {
      (sparse.relocate(last, index), ...);
      (sparse.size--, ...);
    }
    id_to_index[index_to_id[last]] = index;
    index_to_id[index]             = index_to_id[last];
    id_to_index[id]                = free_id_head | RELEASE_MASK;
    free_id_head                   = id;
    size--;
  }

  template <typename Relocate>
  [[nodiscard]] constexpr bool try_release(SizeType id, Relocate &&relocate_op)
  {
    if (!is_valid_id(id))
    {
      return false;
    }
    release(id, relocate_op);
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

    if (!tvec::reserve(allocator, id_to_index, id_to_index_capacity,
                       target_capacity) ||
        !tvec::reserve(allocator, index_to_id, index_to_id_capacity,
                       target_capacity))
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

  template <typename... T>
  [[nodiscard]] constexpr bool push(SizeType &out_id, SizeType &out_index,
                                    SparseVec<T> &...sparse)
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
    out_index            = index;
    num_free--;
    return true;
  }

  void reorder(SizeType const *indices);
};

}        // namespace ash
