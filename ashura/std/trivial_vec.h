#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

// T: container element type
// SizeT: size type of the container: u8, u16, u32, u64
template <typename T, typename SizeType>
[[nodiscard]] constexpr bool
    trivial_vec_create_zeroed(AllocatorImpl const &allocator, T *&data,
                              SizeType target_size)
{
  data = allocator.allocate_zeroed_typed(data, target_size);
  return data != nullptr;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool trivial_vec_reserve(AllocatorImpl const &allocator,
                                                 T *&data, SizeType &capacity,
                                                 SizeType target_capacity)
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
[[nodiscard]] constexpr void trivial_vec_reset(AllocatorImpl const &allocator,
                                               T *&data, SizeType &size,
                                               SizeType &capacity)
{
  allocator.deallocate_typed(data, capacity);
  data     = nullptr;
  size     = 0;
  capacity = 0;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool
    trivial_vec_push(AllocatorImpl const &allocator, T *&data, SizeType &size,
                     SizeType &capacity, T const &element)
{
  SizeType const target_size = size + 1;
  SizeType const target_capacity =
      target_size > capacity ? (target_size + (target_size >> 1)) : target_size;
  if (!trivial_vec_reserve(allocator, data, capacity, target_capacity))
  {
    return false;
  }
  data[size] = element;
  size++;
  return true;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool
    trivial_vec_extend(AllocatorImpl const &allocator, T *&data, SizeType &size,
                       SizeType &capacity, T const *push_elements,
                       SizeType num_push_elements)
{
  SizeType const target_size = size + num_push_elements;
  SizeType const target_capacity =
      target_size > capacity ? (target_size + (target_size >> 1)) : target_size;
  if (!trivial_vec_reserve(allocator, data, capacity, target_capacity))
  {
    return false;
  }

  mem::copy(push_elements, data + size, num_push_elements);
  size += num_push_elements;
  return true;
}

template <typename T, typename SizeType>
constexpr void trivial_vec_erase(AllocatorImpl const &allocator, T *&data,
                                 SizeType &size, SizeType first_erase,
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
  alg::copy(Span{data + relocate_begin, num_relocate},
            Span{data + first_erase, num_relocate});
  // calculate new size
  size -= num_erase;
}

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool trivial_vec_fit(AllocatorImpl const &allocator,
                                             T *&data, SizeType size,
                                             SizeType &capacity)
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
[[nodiscard]] constexpr bool
    trivial_vec_insert(AllocatorImpl const &allocator, T *&data, SizeType size,
                       SizeType &capacity, SizeType insert_index, T const &);

template <typename T, typename SizeType>
[[nodiscard]] constexpr bool trivial_vec_insert_range(
    AllocatorImpl const &allocator, T *&data, SizeType size, SizeType &capacity,
    SizeType insert_index, T const *, SizeType num_insert);

};        // namespace ash
