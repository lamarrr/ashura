#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T>
struct Vec
{
  using Type          = T;
  using Rep           = T;
  using Iterator      = T *;
  using ConstIterator = T const *;

  AllocatorImpl m_allocator = heap_allocator;
  T            *m_data      = nullptr;
  usize         m_capacity  = 0;
  usize         m_size      = 0;

  [[nodiscard]] constexpr bool is_empty() const
  {
    return m_size == 0;
  }

  [[nodiscard]] constexpr T *data() const
  {
    return m_data;
  }

  [[nodiscard]] constexpr usize size() const
  {
    return m_size;
  }

  [[nodiscard]] constexpr usize capacity() const
  {
    return m_capacity;
  }

  [[nodiscard]] constexpr T *begin() const
  {
    return m_data;
  }

  [[nodiscard]] constexpr T *end() const
  {
    return m_data + m_size;
  }

  constexpr operator Span<T>() const
  {
    return Span<T>{m_data, m_size};
  }

  [[nodiscard]] constexpr T &operator[](usize index) const
  {
    return m_data[index];
  }

  [[nodiscard]] constexpr T *try_get(usize index) const
  {
    if (index < m_size)
    {
      return m_data + index;
    }

    return nullptr;
  }

  constexpr void clear()
  {
    if constexpr (!TriviallyDestructible<T>)
    {
      for (T *iter = begin(); iter < end(); iter++)
      {
        iter->~T();
      }
    }
    m_size = 0;
  }

  constexpr void reset()
  {
    if constexpr (!TriviallyDestructible<T>)
    {
      for (T *iter = begin(); iter < end(); iter++)
      {
        iter->~T();
      }
    }
    m_allocator.deallocate_typed(m_data, m_capacity);
    m_data     = nullptr;
    m_size     = 0;
    m_capacity = 0;
  }

  [[nodiscard]] bool reserve(usize target_capacity)
  {
    if (m_capacity >= target_capacity)
    {
      return true;
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      T *new_data =
          m_allocator.reallocate_typed(m_data, m_capacity, target_capacity);

      if (new_data == nullptr)
      {
        return false;
      }

      m_data = new_data;
    }
    else
    {
      T *new_data = m_allocator.allocate_typed<T>(target_capacity);

      if (new_data == nullptr)
      {
        return false;
      }

      for (usize i = 0; i < m_size; i++)
      {
        new (new_data + i) T{(T &&) m_data[i]};
      }

      for (usize i = 0; i < m_size; i++)
      {
        (m_data + i)->~T();
      }

      m_allocator.deallocate_typed(m_data, m_capacity);
      m_data = new_data;
    }

    m_capacity = target_capacity;
    return true;
  }

  [[nodiscard]] bool fit()
  {
    if (m_size == m_capacity)
    {
      return true;
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      T *new_data = m_allocator.reallocate_typed(m_data, m_capacity, m_size);

      if (new_data == nullptr)
      {
        return false;
      }

      m_data = new_data;
    }
    else
    {
      T *new_data = m_allocator.allocate_typed<T>(m_size);

      if (new_data == nullptr)
      {
        return false;
      }

      for (usize i = 0; i < m_size; i++)
      {
        new (new_data + i) T{(T &&) m_data[i]};
      }

      for (usize i = 0; i < m_size; i++)
      {
        (m_data + i)->~T();
      }

      m_allocator.deallocate_typed(m_data, m_capacity);
      m_data = new_data;
    }

    m_capacity = m_size;
    return true;
  }

  [[nodiscard]] bool grow(usize target_size)
  {
    if (m_capacity >= target_size)
    {
      return true;
    }

    return reserve(max(target_size, m_capacity + (m_capacity >> 1)));
  }

  void erase(usize first, usize num = 1)
  {
    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(m_data + first + num, m_data + first, m_size - (first + num));
    }
    else
    {
      for (usize i = first; i < m_size - num; i++)
      {
        m_data[i] = (T &&) (m_data[i + num]);
      }

      for (usize i = m_size - num; i < m_size; i++)
      {
        (m_data + i)->~T();
      }
    }
    m_size -= num;
  }

  template <typename... Args>
  [[nodiscard]] bool push(Args &&...args)
  {
    if (!grow(m_size + 1))
    {
      return false;
    }

    new (m_data + m_size) T{((Args &&) args)...};

    m_size++;

    return true;
  }

  constexpr void pop(usize num = 1)
  {
    if constexpr (!TriviallyDestructible<T>)
    {
      for (usize i = m_size - num; i < m_size; i++)
      {
        (m_data + i)->~T();
      }
    }

    m_size -= num;
  }

  [[nodiscard]] constexpr bool try_pop(usize num = 1)
  {
    if (m_size < num)
    {
      return false;
    }

    pop(num);

    return true;
  }

  [[nodiscard]] bool shift_uninitialized(usize first, usize distance)
  {
    if (!grow(m_size + distance))
    {
      return false;
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(m_data + first, m_data + first + distance, m_size - first);
    }
    else
    {
      // move construct tail elements
      usize const tail_first = max(first, min(m_size, distance) - m_size);
      for (usize i = tail_first; i < m_size; i++)
      {
        new (m_data + i + distance) T{(T &&) m_data[i]};
      }

      // move non-tail elements towards end
      for (usize i = first; i < tail_first; i++)
      {
        m_data[i + distance] = (T &&) m_data[i];
      }

      if constexpr (!TriviallyDestructible<T>)
      {
        // destruct previous position of non-tail elements
        for (usize i = first; i < tail_first; i++)
        {
          (m_data + i)->~T();
        }
      }
    }

    m_size += distance;

    return true;
  }

  template <typename... Args>
  [[nodiscard]] bool insert(usize dst, Args &&...args)
  {
    if (!shift_uninitialized(dst, 1))
    {
      return false;
    }

    new (m_data + dst) T{((Args &&) args)...};
    return true;
  }

  [[nodiscard]] bool insert_span_copy(usize dst, Span<T const> span)
  {
    if (!shift_uninitialized(dst, span.size()))
    {
      return false;
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), m_data + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (m_data + dst + i) T{span[i]};
      }
    }

    return true;
  }

  [[nodiscard]] bool insert_span_move(usize dst, Span<T> span)
  {
    if (!shift_uninitialized(dst, span.size()))
    {
      return false;
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), m_data + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (m_data + dst + i) T{(T &&) span[i]};
      }
    }

    return true;
  }

  [[nodiscard]] bool extend_uninitialized(usize extension)
  {
    if (!grow(m_size + extension))
    {
      return false;
    }

    m_size += extension;

    return true;
  }

  [[nodiscard]] bool extend_defaulted(usize extension)
  {
    usize const pos = m_size;
    if (!extend_uninitialized(extension))
    {
      return false;
    }

    for (usize i = pos; i < m_size; i++)
    {
      new (m_data + i) T{};
    }

    return true;
  }

  [[nodiscard]] bool extend_copy(Span<T const> span)
  {
    usize const pos = m_size;
    if (!extend_uninitialized(span.size()))
    {
      return false;
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), m_data + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (m_data + pos + i) T{span[i]};
      }
    }

    return true;
  }

  [[nodiscard]] bool extend_move(Span<T> span)
  {
    usize const pos = m_size;
    if (!extend_uninitialized(span.size()))
    {
      return false;
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), m_data + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (m_data + pos + i) T{(T &&) span[i]};
      }
    }

    return true;
  }

  constexpr void swap(usize a, usize b) const
  {
    ::ash::swap(m_data[a], m_data[b]);
  }

  [[nodiscard]] bool resize_uninitialized(usize new_size)
  {
    if (new_size <= m_size)
    {
      erase(new_size, m_size - new_size);
      return true;
    }

    return extend_uninitialized(new_size - m_size);
  }

  [[nodiscard]] bool resize_defaulted(usize new_size)
  {
    if (new_size <= m_size)
    {
      erase(new_size, m_size - new_size);
      return true;
    }

    return extend_defaulted(new_size - m_size);
  }
};

// Adapter
template <typename RepT>
struct BitVec
{
  using Type          = bool;
  using Rep           = RepT;
  using Iterator      = BitIterator<Rep>;
  using ConstIterator = BitIterator<Rep const>;

  Vec<Rep> vec      = {};
  usize    num_bits = 0;

  [[nodiscard]] constexpr BitRef<Rep> operator[](usize index) const
  {
    return *(begin() + index);
  }

  [[nodiscard]] constexpr operator BitSpan<Rep>() const
  {
    return BitSpan<Rep>{vec.data(), num_bits};
  }

  [[nodiscard]] constexpr bool is_empty() const
  {
    return num_bits == 0;
  }

  [[nodiscard]] constexpr Rep *data() const
  {
    return vec.data();
  }

  [[nodiscard]] constexpr BitIterator<Rep> begin() const
  {
    return BitIterator<Rep>{vec.data(), 0};
  }

  [[nodiscard]] constexpr BitIterator<Rep> end() const
  {
    return BitIterator<Rep>{vec.data(), num_bits};
  }

  [[nodiscard]] constexpr usize size() const
  {
    return num_bits;
  }

  [[nodiscard]] constexpr usize capacity() const
  {
    return vec.capacity() * NumTraits<Rep>::NUM_BITS;
  }

  void clear()
  {
    num_bits = 0;
    vec.clear();
  }

  void reset()
  {
    num_bits = 0;
    vec.reset();
  }

  static constexpr usize num_packs(usize num_bits)
  {
    return (num_bits >> NumTraits<Rep>::LOG2_NUM_BITS) +
           ((num_bits & (NumTraits<Rep>::NUM_BITS - 1)) == 0 ? 0 : 1);
  }

  [[nodiscard]] bool reserve(usize target_capacity)
  {
    return vec.reserve(num_packs(target_capacity));
  }

  [[nodiscard]] bool grow(usize target_size)
  {
    return vec.grow(num_packs(target_size));
  }

  [[nodiscard]] bool push(bool bit)
  {
    if (!grow(num_bits + 1))
    {
      return false;
    }
    this->operator[](num_bits) = bit;
    num_bits++;
    return true;
  }

  constexpr void pop(usize num = 1)
  {
    num_bits -= num;
    usize diff = vec.size() - num_packs(num_bits);
    if (diff > 0)
    {
      vec.pop(diff);
    }
  }

  [[nodiscard]] constexpr bool try_pop(usize num = 1)
  {
    if (num_bits < num)
    {
      return false;
    }
    pop(num);
    return true;
  }

  void erase(usize index, usize num = 1)
  {
    for (BitIterator out = begin() + index, src = out + num; src != end();
         out++, src++)
    {
      *out = *src;
    };
    pop(num);
  }

  template <typename T>
  [[nodiscard]] bool extend_span_copy(BitSpan<T const> span);

  template <typename T>
  [[nodiscard]] bool extend_span_move(BitSpan<T const> span);

  [[nodiscard]] bool extend_uninitialized(usize extension)
  {
    if (!vec.extend_uninitialized(num_packs(num_bits + extension) -
                                  num_packs(num_bits)))
    {
      return false;
    }

    num_bits += extension;

    return true;
  }

  [[nodiscard]] bool extend_defaulted(usize extension)
  {
    usize const pos = num_bits;
    if (!extend_uninitialized(extension))
    {
      return false;
    }

    for (usize i = pos; i < num_bits; i++)
    {
      this->operator[](i) = false;
    }

    return true;
  }

  [[nodiscard]] bool resize_uninitialized(usize new_size)
  {
    if (new_size <= num_bits)
    {
      erase(new_size, num_bits - new_size);
      return true;
    }

    return extend_uninitialized(new_size - num_bits);
  }

  [[nodiscard]] bool resize_defaulted(usize new_size)
  {
    if (new_size <= num_bits)
    {
      erase(new_size, num_bits - new_size);
      return true;
    }

    return extend_defaulted(new_size - num_bits);
  }

  // flip
  // insert

  constexpr void swap(usize a, usize b) const
  {
    BitRef<Rep> a_r = this->operator[](a);
    BitRef<Rep> b_r = this->operator[](b);
    bool        a_v = a_r;
    a_r             = b_r;
    b_r             = a_v;
  }
};

template <typename Rep>
constexpr auto to_span(BitVec<Rep> &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

template <typename Rep>
constexpr auto to_span(BitVec<Rep> const &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

}        // namespace ash
