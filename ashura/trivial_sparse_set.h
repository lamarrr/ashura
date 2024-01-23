#pragma once
#include "ashura/allocator.h"
#include "ashura/mem.h"
#include "ashura/traits.h"

namespace ash
{
namespace gfx
{

/// @brief
/// @tparam T: contained type, must be movable
/// @tparam SizeT: size type, u8, u32, u64
///
/// @data: contained elements
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
///
template <typename SizeType = u64>
struct TrivialSparseSet
{
  static_assert(!IntTraits<SizeType>::SIGNED);
  using UID                         = SizeType;
  static constexpr SizeType ID_MASK = ((SizeType) 1)
                                      << (IntTraits<SizeType>::NUM_BITS - 1);
  static constexpr SizeType MAX_ELEMENTS         = ~ID_MASK;
  UID                      *index_to_id          = nullptr;
  SizeType                 *id_to_index          = nullptr;
  UID                       next_free_id         = -1;
  SizeType                  next_free_index      = -1;
  SizeType                  num_slots            = 0;
  SizeType                  index_to_id_capacity = 0;
  SizeType                  id_to_index_capacity = 0;

  void clear()
  {
  }

  void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(index_to_id, index_to_id_capacity);
    allocator.deallocate_typed(id_to_index, id_to_index_capacity);
    index_to_id          = nullptr;
    id_to_index          = nullptr;
    next_free_id         = -1;
    next_free_index      = -1;
    num_slots            = 0;
    index_to_id_capacity = 0;
    id_to_index_capacity = 0;
  }

  [[nodiscard]] bool is_valid(UID id) const
  {
    return true;
  }

  SizeType unsafe_to_index(UID id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] bool to_index(UID id, SizeType &index) const
  {
    if (!is_valid(id))
    {
      return false;
    }

    index = unsafe_to_index(id);
    return true;
  }

  [[nodiscard]] bool push(AllocatorImpl const &allocator, T const &item,
                          UID &out_id)
  {
    UID id = -1;
    if (next_free_id == -1)
    {
      // no free ids available,
      // allocate new id
    }
    else
    {
      //  id = next_free_id & ~ID_MASK;
      // don't use id until all operations are successfull, so we can return it
      // if it fails
    }

    if (next_free_index == -1)
    {
      // no free indices available, i.e. array full
      // allocate new index and resize array
    }
    else
    {
    }

    return true;
  }

  // OK - fast remove
  // OK - fast insert
  // OK - fast append
  // ? - fast compact

  void unsafe_erase(UID id)
  {
    SizeType const index = id_to_index[id];
    index_to_id[index]   = ID_MASK | next_free_index;
    id_to_index[id]      = ID_MASK | next_free_id;
    next_free_id         = id;
    next_free_index      = index;
  }

  [[nodiscard]] bool erase(UID id)
  {
    if (!is_valid(id))
    {
      return false;
    }
    unsafe_erase(id);
    return true;
  }

  void compact()
  {
    // iterate linearly through the elements and index_to_ids from the back for
    // num_free valid elements
    //
    // move the valid elements into the holes that have indices lower than
    // (num_slots - num_free)
    //
    // move the valid elements into the holes that have indices higher than the
    // index of the present element??? holes???
  }
};

}        // namespace gfx
}        // namespace ash
