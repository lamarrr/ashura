#pragma once

#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @tparam SizeT: size type, u8, u32, u64
///
/// @m_index_to_id: id of data, ordered relative to {data}
/// @m_id_to_index: map of id to index in {data}
/// @size: the number of valid elements in the sparse set
/// @capacity: the number of elements the sparse set has capacity for, includes
/// reserved but unallocated ids pointing to valid but uninitialized memory
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
template <typename SizeT>
struct SparseVec
{
  static_assert(!NumTraits<SizeT>::SIGNED && !NumTraits<SizeT>::FLOATING_POINT);
  using SizeType                         = SizeT;
  static constexpr SizeType STUB         = NumTraits<SizeType>::MAX;
  static constexpr SizeType RELEASE_MASK = ~(STUB >> 1);
  static constexpr SizeType MAX_ELEMENTS = STUB >> 1;
  static constexpr SizeType MAX_ID       = MAX_ELEMENTS;

  Vec<SizeType> m_index_to_id  = {};
  Vec<SizeType> m_id_to_index  = {};
  SizeType      m_size         = 0;
  SizeType      m_free_id_head = STUB;

  [[nodiscard]] constexpr bool is_empty() const
  {
    return m_size == 0;
  }

  [[nodiscard]] constexpr SizeType *data() const
  {
    return m_index_to_id.data();
  }

  [[nodiscard]] constexpr SizeType size() const
  {
    return m_size;
  }

  [[nodiscard]] constexpr SizeType *begin() const
  {
    return m_index_to_id.begin();
  }

  [[nodiscard]] constexpr SizeType *end() const
  {
    return m_index_to_id.end();
  }

  constexpr operator Span<SizeType>() const
  {
    return m_index_to_id;
  }

  template <typename... VecT>
  constexpr void clear(VecT &...dense)
  {
    (dense.clear(), ...);
    m_id_to_index.clear();
    m_index_to_id.clear();
    m_size         = 0;
    m_free_id_head = STUB;
  }

  template <typename... VecT>
  constexpr void reset(VecT &...dense)
  {
    (dense.reset(), ...);
    m_id_to_index.reset();
    m_index_to_id.reset();
    m_free_id_head = STUB;
    m_size         = 0;
  }

  [[nodiscard]] constexpr bool is_valid_id(SizeType id) const
  {
    return id < m_id_to_index.size() && !(m_id_to_index[id] & RELEASE_MASK);
  }

  [[nodiscard]] constexpr bool is_valid_index(SizeType index) const
  {
    return index < m_size;
  }

  [[nodiscard]] constexpr SizeType operator[](SizeType id) const
  {
    return m_id_to_index[id];
  }

  [[nodiscard]] constexpr SizeType to_index(SizeType id) const
  {
    return m_id_to_index[id];
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

  [[nodiscard]] constexpr SizeType to_id(SizeType index) const
  {
    return m_index_to_id[index];
  }

  [[nodiscard]] constexpr bool try_to_id(SizeType index, SizeType &id) const
  {
    if (!is_valid_index(index))
    {
      return false;
    }

    id = to_id(id);
    return true;
  }

  template <typename... VecT>
  constexpr void erase(SizeType id, VecT &...dense)
  {
    SizeType const index = m_id_to_index[id];
    SizeType const last  = m_size - 1;

    if (m_size != 1 && index != last)
    {
      (dense.swap(index, dense.size() - 1), ...);
    }

    (dense.pop(), ...);

    // TODO(lamarrr): pop index_to_id
    // don't forget to remove
    m_id_to_index[m_index_to_id[last]] = index;
    m_index_to_id[index]               = m_index_to_id[last];
    m_id_to_index[id]                  = m_free_id_head | RELEASE_MASK;
    m_free_id_head                     = id;
    m_size--;
  }

  template <typename... VecT>
  [[nodiscard]] constexpr bool try_erase(SizeType id, VecT &...dense)
  {
    if (!is_valid_id(id))
    {
      return false;
    }
    erase(id, dense...);
    return true;
  }

  template <typename... VecT>
  [[nodiscard]] constexpr bool reserve(SizeType target_capacity, VecT &...dense)
  {
    return ((m_id_to_index.reserve(target_capacity) &&
             m_index_to_id.reserve(target_capacity)) &&
            ... && dense.reserve(target_capacity));
  }

  template <typename... VecT>
  [[nodiscard]] constexpr bool grow(SizeType target_size, VecT &...dense)
  {
    return (
        (m_id_to_index.grow(target_size) && m_index_to_id.grow(target_size)) &&
        ... && dense.grow(target_size));
  }

  template <typename... VecT>
  [[nodiscard]] constexpr bool allocate(SizeType &out_id, SizeType &out_index,
                                        VecT &...dense)
  {
    if (!grow(m_size + 1, dense...))
    {
      return false;
    }

    // find free id, and grow all dense, and increase sizes of dense
    // also take callback to insert elements into indices?
    SizeType const index = m_size;
    SizeType const id    = m_free_id_head;
    m_free_id_head       = ~RELEASE_MASK & m_id_to_index[m_free_id_head];
    m_index_to_id[index] = id;
    m_id_to_index[id]    = index;
    out_id               = id;
    out_index            = index;
    m_size++;
    (dense.m_size++, ...);
    return true;
  }
};

}        // namespace ash
