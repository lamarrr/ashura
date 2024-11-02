/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @brief Sparse Vector (a.k.a Sparse Set) are used for stable ID-tagging of
/// objects in high-perf scenarious i.e. ECS, where a stable identity is needed
/// for objects and they need to be processed in batches for efficiency. They
/// have an indirect index into their elements, although they don't guarantee
/// stability of the addresses of the elements they guarantee that the IDs
/// persist until the id is released. Unlike typical Sparse Sets, Sparse Vec's
/// elements are always contiguous without holes in them, making them suitable
/// for operations like batch-processing and branchless SIMD.
///
/// @tparam V dense containers for the properties, i.e. Vec<i32>, Vec<f32>
/// @param index_to_id id of data, ordered relative to {data}
/// @param id_to_index map of id to index in {data}
/// @param size the number of valid elements in the sparse set
/// @param capacity the number of elements the sparse set has capacity for,
/// includes reserved but unallocated ids pointing to valid but uninitialized
/// memory
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
///
template <typename... V>
struct SparseVec
{
  static constexpr u64 RELEASE_MASK = U64_MAX >> 1;
  static constexpr u64 STUB         = U64_MAX;

  Vec<u64>    index_to_id  = {};
  Vec<u64>    id_to_index  = {};
  Tuple<V...> dense        = {};
  u64         free_id_head = STUB;

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr u64 size() const
  {
    return static_cast<u64>(index_to_id.size());
  }

  constexpr void clear()
  {
    for_each([](auto &d) { d.clear(); }, dense);
    id_to_index.clear();
    index_to_id.clear();
    free_id_head = STUB;
  }

  constexpr void reset()
  {
    for_each([](auto &d) { d.reset(); }, dense);
    id_to_index.reset();
    index_to_id.reset();
    free_id_head = STUB;
  }

  constexpr void uninit()
  {
    for_each([](auto &d) { d.uninit(); }, dense);
    id_to_index.uninit();
    index_to_id.uninit();
  }

  constexpr bool is_valid_id(u64 id) const
  {
    return id < id_to_index.size() && !(id_to_index[id] & RELEASE_MASK);
  }

  constexpr bool is_valid_index(u64 index) const
  {
    return index < size();
  }

  constexpr u64 operator[](u64 id) const
  {
    return id_to_index[id];
  }

  constexpr u64 to_index(u64 id) const
  {
    return id_to_index[id];
  }

  constexpr Result<u64, Void> try_to_index(u64 id) const
  {
    if (!is_valid_id(id))
    {
      return Err{};
    }

    return Ok{to_index(id)};
  }

  constexpr u64 to_id(u64 index) const
  {
    return index_to_id[index];
  }

  constexpr Result<u64, Void> try_to_id(u64 index) const
  {
    if (!is_valid_index(index))
    {
      return Err{};
    }

    return Ok{to_id(index)};
  }

  constexpr void erase(u64 id)
  {
    u64 const index = id_to_index[id];
    u64 const last  = size() - 1;

    if (index != last)
    {
      for_each([&](auto &d) { d.swap(index, last); }, dense);
    }

    for_each([](auto &d) { d.pop(); }, dense);

    // adjust id and index mapping
    if (index != last)
    {
      id_to_index[index_to_id[last]] = index;
      index_to_id[index]             = index_to_id[last];
    }

    id_to_index[id] = free_id_head | RELEASE_MASK;
    free_id_head    = id;
    index_to_id.pop();
  }

  constexpr Result<Void, Void> try_erase(u64 id)
  {
    if (!is_valid_id(id))
    {
      return Err{};
    }
    erase(id);
    return Ok{};
  }

  constexpr Result<Void, Void> reserve(u64 target_capacity)
  {
    if (!(id_to_index.reserve(target_capacity) &&
          index_to_id.reserve(target_capacity)))
    {
      return Err{};
    }

    bool failed = false;
    for_each([&](auto &d) { failed |= !d.reserve(target_capacity); }, dense);

    if (failed)
    {
      return Err{};
    }

    return Ok{};
  }

  constexpr Result<Void, Void> grow(u64 target_size)
  {
    if (!(id_to_index.grow(target_size) && index_to_id.grow(target_size)))
    {
      return Err{};
    }

    bool failed = false;
    for_each([&](auto &d) { failed |= !d.grow(target_size); }, dense);

    if (failed)
    {
      return Err{};
    }

    return Ok{};
  }

  /// make new id and map the unique id to the unique index
  constexpr Result<u64, Void> make_id(u64 index)
  {
    if (free_id_head != STUB)
    {
      u64 id          = free_id_head;
      id_to_index[id] = index;
      free_id_head    = ~RELEASE_MASK & id_to_index[free_id_head];
      return Ok{id};
    }
    else
    {
      if (!id_to_index.push(index))
      {
        return Err{};
      }
      u64 id = static_cast<u64>(id_to_index.size() - 1);
      return Ok{id};
    }
  }

  template <usize I = 0>
  struct Pusher
  {
    template <typename Tuple, typename Head, typename... Tail>
    static constexpr void push(Tuple &t, Head &&head, Tail &&...tail)
    {
      get<I>(t).push((Head &&) head).unwrap();
      if constexpr (sizeof...(tail) != 0)
      {
        Pusher<I + 1>::push(t, ((Tail &&) tail)...);
      }
    }
  };

  template <typename... Args>
  constexpr Result<u64, Void> push(Args &&...args)
    requires(sizeof...(Args) == sizeof...(V))
  {
    u64 const index = size();

    if (!grow(size() + 1))
    {
      return Err{};
    }

    Result id = make_id(index);

    if (!id)
    {
      return Err{};
    }

    if (!index_to_id.push(id.unwrap()))
    {
      return Err{};
    }

    Pusher<0>::push(dense, ((Args &&) args)...);
    return id;
  }
};

}        // namespace ash
