#pragma once
#include "ashura/std/algorithms.h"
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

namespace tvec
{

    
template <typename T, typename SizeType>
constexpr void reset(AllocatorImpl const &allocator, T *&data,
                     SizeType &capacity)
{
  allocator.deallocate_typed(data, capacity);
  data     = nullptr;
  capacity = 0;
}

template <typename T, typename SizeType>
constexpr void reset(AllocatorImpl const &allocator, T *&data,
                     SizeType &capacity, SizeType &size)
{
  reset(allocator, data, capacity);
  size = 0;
}


template <typename T, typename SizeType>
[[nodiscard]] constexpr bool reserve(AllocatorImpl const &allocator, T *&data,
                                     SizeType &capacity,
                                     SizeType  target_capacity)
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

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool grow(AllocatorImpl const &allocator, T *&data,
                                  SizeType &capacity, SizeType target_capacity)
{
  target_capacity = max(target_capacity, capacity + capacity >> 1);
  return reserve(allocator, data, capacity, target_capacity);
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool push(AllocatorImpl const &allocator, T *&data,
                                  SizeType &capacity, SizeType &size,
                                  T const &element)
{
  SizeType const target_size = size + 1;

  if (target_size > capacity)
  {
    if (!grow(allocator, data, capacity, target_size))
    {
      return false;
    }
  }

  data[size] = element;
  size++;
  return true;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool
    extend(AllocatorImpl const &allocator, T *&data, SizeType &capacity,
           SizeType &size, T const *push_elements, SizeType num_push_elements)
{
  SizeType const target_size = size + num_push_elements;

  if (target_size > capacity)
  {
    if (!grow(allocator, data, capacity, target_size))
    {
      return false;
    }
  }

  mem::copy(push_elements, data + size, num_push_elements);
  size += num_push_elements;
  return true;
}

template <typename T, typename SizeType>
constexpr void erase(T *&data, SizeType &size, SizeType first_erase,
                     SizeType num_erase)
{
  // num_erase can be as large as SizeT::max()
  // essencially a slice
  first_erase = first_erase > size ? size : first_erase;
  num_erase =
      num_erase > (size - first_erase) ? (size - first_erase) : num_erase;
  SizeType relocate_begin = first_erase + num_erase;
  SizeType num_relocate   = size - relocate_begin;
  // can't use memcpy as it is potentially overlapping
  copy(Span{data + relocate_begin, num_relocate},
       Span{data + first_erase, num_relocate});
  // calculate new size
  size -= num_erase;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool fit(AllocatorImpl const &allocator, T *&data,
                                 SizeType &capacity, SizeType &size)
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

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool insert(AllocatorImpl const &allocator,
                                    SizeType insert_index, T const &);

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool insert_range(AllocatorImpl const &allocator,
                                          SizeType insert_index, T const *,
                                          SizeType num_insert);
}        // namespace tvec

};        // namespace ash
