#pragma once
#include "ashura/allocator.h"
#include "ashura/mem.h"
#include "ashura/traits.h"

namespace ash
{
namespace gfx
{

template <u8 Index, u8 Target, typename T, typename... U>
struct IndexTypePack
{
  using Type = typename IndexTypePack<Index + 1, Target, U...>::Type;
};

template <u8 Index, typename T, typename... U>
struct IndexTypePack<Index, Index, T, U...>
{
  using Type = T;
};

/// @brief
/// @tparam T: contained type, must be movable
/// @tparam SizeT: size type, u8, u32, u64
///
/// @data: contained elements
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
///
template <typename SizeType, typename... T>
struct TrivialSparseSet
{
  static_assert(!IntTraits<SizeType>::SIGNED);

  template <u8 Index>
  using Type = typename IndexTypePack<0, Index, T...>::Type;
  using UID  = SizeType;
  static constexpr SizeType NUM_COMPONENTS = sizeof...(T);
  static constexpr SizeType ID_MASK        = ((SizeType) 1)
                                      << (IntTraits<SizeType>::NUM_BITS - 1);
  void     *data[NUM_COMPONENTS]          = {};
  SizeType  data_capacity[NUM_COMPONENTS] = {};
  UID      *index_to_id                   = nullptr;
  SizeType *id_to_index                   = nullptr;
  SizeType  index_to_id_capacity          = 0;
  SizeType  id_to_index_capacity          = 0;
  UID       next_free_id                  = -1;
  SizeType  next_free_index               = -1;
  SizeType  num_items                     = 0;


template<u8 Index, typename Fn>
void _apply_recursive(Fn ... fn){
    fn(   (  Type<Index> *  )  data[Index]   );
    _apply_recursive(Index+1, fn);
}

template<typename Fn>
void _apply_recursive<NUM_COMPONENTS - 1>(){

}

  void clear()
  {

  }

  void reset(AllocatorImpl const &allocator);

  template <u8 Index>
  constexpr Type<Index> *get_unsafe(UID item) const
  {
  static_assert(Index < NUM_COMPONENTS);
    return ((Type<Index> *) data[Index]) + id_to_index[item];
  }

  constexpr bool is_valid(UID item) const
  {
    return true;
  }

  constexpr T *get(UID item) const
  {
    return is_valid(item) ? get_unsafe(item) : nullptr;
  }

  bool push(AllocatorImpl const &allocator, T const &item, UID &out_id)
  {
    if (next_free_id == -1)
    {
    }
    return true;
  }

  // OK - fast remove
  // OK - fast insert
  // OK - fast append
  // ? - fast compact

  void erase_unsafe(UID item)
  {
    SizeType const index = id_to_index[item];
    index_to_id[index]   = ID_MASK | next_free_index;
    id_to_index[item]    = ID_MASK | next_free_id;
    next_free_id         = item;
    next_free_index      = index;
  }

  bool erase(UID item)
  {
    if (!is_valid(item))
    {
      return false;
    }
    return true;
  }

  void compact()
  {
    // iterate linearly through the elements
    // iterate in any order through the ids
  }
};

}        // namespace gfx
}        // namespace ash
