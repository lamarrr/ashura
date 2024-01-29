#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

// SizeType: size type of the container: u8, u16, u32, u64
template <typename T, typename SizeType>
struct TrivialVec
{
  T       *data     = nullptr;
  SizeType size     = 0;
  SizeType capacity = 0;

  constexpr void reset(AllocatorImpl const &allocator)
  {
    allocator.deallocate_typed(data, capacity);
    data     = nullptr;
    size     = 0;
    capacity = 0;
  }

  [[nodiscard]] constexpr bool reserve(AllocatorImpl const &allocator,
                                       SizeType             target_capacity)
  {
    if (target_capacity <= capacity)
    {
      return true;
    }
    T *new_data = allocator.reallocate_typed(data, capacity, target_capacity);
    if (new_data == nullptr)
    {
      return false;
    }
    data     = new_data;
    capacity = target_capacity;
    return true;
  }

  [[nodiscard]] constexpr bool grow__(AllocatorImpl const &allocator,
                                      SizeType             target_capacity);

  [[nodiscard]] constexpr bool push(AllocatorImpl const &allocator,
                                    T const             &element)
  {
    SizeType const target_size     = size + 1;
    SizeType const target_capacity = target_size > capacity ?
                                         (target_size + (target_size >> 1)) :
                                         target_size;
    if (!reserve(allocator, data, capacity, target_capacity))
    {
      return false;
    }
    data[size] = element;
    size++;
    return true;
  }

  [[nodiscard]] constexpr bool extend(AllocatorImpl const &allocator,
                                      T const             *push_elements,
                                      SizeType             num_push_elements)
  {
    SizeType const target_size     = size + num_push_elements;
    SizeType const target_capacity = target_size > capacity ?
                                         (target_size + (target_size >> 1)) :
                                         target_size;
    if (!reserve(allocator, data, capacity, target_capacity))
    {
      return false;
    }

    mem::copy(push_elements, data + size, num_push_elements);
    size += num_push_elements;
    return true;
  }

  constexpr void erase(SizeType first_erase, SizeType num_erase)
  {
    // num_erase can be as large as SizeT::max()
    // essencially a slice
    first_erase = first_erase > size ? size : first_erase;
    num_erase =
        num_erase > (size - first_erase) ? (size - first_erase) : num_erase;
    SizeType relocate_begin = first_erase + num_erase;
    SizeType num_relocate   = size - relocate_begin;
    // can't use memcpy as it is potentially overlapping
    alg::copy(Span{data + relocate_begin, num_relocate},
              Span{data + first_erase, num_relocate});
    // calculate new size
    size -= num_erase;
  }

  [[nodiscard]] constexpr bool fit(AllocatorImpl const &allocator)
  {
    if (size == capacity)
    {
      return true;
    }

    T *new_data = allocator.reallocate_typed(data, capacity, size);
    if (new_data == nullptr)
    {
      return false;
    }

    data     = new_data;
    capacity = size;
    return true;
  }

  [[nodiscard]] constexpr bool insert(AllocatorImpl const &allocator,
                                      SizeType insert_index, T const &);

  [[nodiscard]] constexpr bool insert_range(AllocatorImpl const &allocator,
                                            SizeType insert_index, T const *,
                                            SizeType num_insert);
};

};        // namespace ash
