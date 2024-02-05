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
  using Type = T;

  AllocatorImpl m_allocator = {};
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

  void erase_index(usize first, usize num)
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

  void erase_index(usize first)
  {
    erase_index(first, 1);
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

  void pop()
  {
    if constexpr (!TriviallyDestructible<T>)
    {
      (m_data + (m_size - 1))->~T();
    }

    m_size--;
  }

  [[nodiscard]] bool try_pop()
  {
    if (m_size == 0)
    {
      return false;
    }

    pop();

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
};

// Adapter
template <typename RepT>
struct BitVec
{
  using Type = bool;
  using Rep  = RepT;

  Vec<Rep> *m_vec      = nullptr;
  usize     m_num_bits = 0;

  constexpr BitRef<Rep> operator[](usize index)
  {
    return *(begin() + index);
  }

  constexpr operator BitSpan<Rep>() const
  {
    return BitSpan<Rep>{m_vec->data(), m_num_bits};
  }

  constexpr bool is_empty() const
  {
    return m_num_bits == 0;
  }

  constexpr BitIterator<Rep> begin() const
  {
    return BitIterator<Rep>{m_vec->data(), 0};
  }

  constexpr BitIterator<Rep> end() const
  {
    return BitIterator<Rep>{m_vec->data(), m_num_bits};
  }

  constexpr usize size() const
  {
    return m_num_bits;
  }

  bool push(bool);
  bool erase();
  bool extend();
};

}        // namespace ash
