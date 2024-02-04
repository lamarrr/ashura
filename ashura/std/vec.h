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

  constexpr T &operator[](usize index) const
  {
    return m_data[index];
  }

  [[nodiscard]] constexpr T *try_get(usize index) const
  {
    if (index < m_size)
    {
      return m_data + m_size;
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

  [[nodiscard]] constexpr bool reserve(usize target_capacity)
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

  [[nodiscard]] constexpr bool grow(usize target_size)
  {
    if (m_capacity >= target_size)
    {
      return true;
    }

    return reserve(max(target_size, m_capacity + (m_capacity >> 1)));
  }

  constexpr void erase_index(usize first, usize num)
  {
  }

  constexpr void erase_index(usize first)
  {
  }

  [[nodiscard]] constexpr bool try_erase_index(usize first, usize num)
  {
    return false;
  }

  [[nodiscard]] constexpr bool try_erase_index(usize first)
  {
    return false;
  }

  // standard free function ????, memcpy if possible?
  // constexpr void               destroy_element(usize index);
  // [[nodiscard]] constexpr bool try_destroy_element(usize index);
  // constexpr void               relocate(usize src, usize dst_uninit);
  // [[nodiscard]] constexpr bool try_relocate(usize src, usize dst_uninit);

  template <typename... Args>
  [[nodiscard]] constexpr bool push(Args &&...args)
  {
    if (!grow(m_size + 1))
    {
      return false;
    }

    new (m_data + m_size) T{((Args &&) args)...};
    m_size++;
    return true;
  }

  constexpr void pop()
  {
    if constexpr (!TriviallyDestructible<T>)
    {
      (m_data + (m_size - 1))->~T();
    }
    m_size--;
  }

  [[nodiscard]] constexpr bool try_pop()
  {
    if (m_size == 0)
    {
      return false;
    }
    pop();
    return true;
  }

  [[nodiscard]] constexpr bool insert(usize dst)
  {
    return false;
  }

  [[nodiscard]] constexpr bool insert_range(usize dst)
  {
    return false;
  }

  [[nodiscard]] constexpr bool extend_copy(Span<T> span);

  [[nodiscard]] constexpr bool extend_move(Span<T> span);

  [[nodiscard]] constexpr bool extend_defaulted(usize extension);

  [[nodiscard]] constexpr bool extend_uninitialized(usize extension);

  [[nodiscard]] constexpr bool resize_defaulted();

  [[nodiscard]] constexpr bool resize_uninitialized();

  [[nodiscard]] constexpr bool fit()
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
};

// Adapter
template <typename RepT>
struct BitVec
{
  using Type = bool;
  using Rep  = RepT;

  Vec<Rep> *m_vec      = nullptr;
  usize     m_num_bits = 0;

  constexpr BitRef<Rep>      operator[](usize i);
  constexpr                  operator BitSpan<Rep>() const;
  constexpr bool             is_empty() const;
  constexpr BitIterator<Rep> begin() const;
  constexpr BitIterator<Rep> end() const;
  constexpr usize            size() const;
  constexpr bool             push(bool);
  constexpr bool             erase();
  constexpr bool             extend();
};

}        // namespace ash
