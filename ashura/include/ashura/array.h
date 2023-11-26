
#pragma once

#include "stx/allocator.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/void.h"
#include <cstddef>
#include <new>

STX_BEGIN_NAMESPACE

#define STX_ARRAY_ENSURE(condition, message)                                      \
  do                                                                              \
  {                                                                               \
    if (!(condition))                                                             \
    {                                                                             \
      ::stx::panic("condition: '" #condition "' failed. explanation: " #message); \
    }                                                                             \
  } while (0)

template <typename T, size_t Capacity>
struct Array
{
  static_assert(Capacity > 0);
  static constexpr size_t capacity = Capacity;

  constexpr Array() : rep{}, size{0}
  {
  }

  template <size_t SrcSize>
  constexpr Array(T const (&arr)[SrcSize]) : rep{}, size{SrcSize}
  {
    static_assert(SrcSize <= capacity);
    for (size_t i = 0; i < SrcSize; i++)
    {
      new (data + i) T{arr[i]};
    }
  }

  constexpr Array(Array const &other) : rep{}, size{other.size}
  {
    for (size_t i = 0; i < other.size; i++)
    {
      new (data + i) T{other.data[i]};
    }
  }

  constexpr Array(Array &&other) : rep{}, size{other.size}
  {
    for (size_t i = 0; i < other.size; i++)
    {
      new (data + i) T{(T &&) (other.data[i])};
    }

    for (size_t i = 0; i < other.size; i++)
    {
      (other.data + i)->~T();
    }

    other.size = 0;
  }

  constexpr Array &operator=(Array const &other)
  {
    if (size == other.size)
    {
      for (size_t i = 0; i < size; i++)
      {
        data[i] = other.data[i];
      }
    }
    else if (size > other.size)
    {
      for (size_t i = 0; i < other.size; i++)
      {
        data[i] = other.data[i];
      }
      for (size_t i = other.size; i < size; i++)
      {
        data[i].~T();
      }
    }
    else
    {
      for (size_t i = 0; i < size; i++)
      {
        data[i] = other.data[i];
      }
      for (size_t i = size; i < other.size; i++)
      {
        new (data + i) T{other.data[i]};
      }
    }
    size = other.size;
    return *this;
  }

  constexpr Array &operator=(Array &&other)
  {
    if (size == other.size)
    {
      for (size_t i = 0; i < size; i++)
      {
        data[i] = (T &&) other.data[i];
      }
    }
    else if (size > other.size)
    {
      for (size_t i = 0; i < other.size; i++)
      {
        data[i] = (T &&) other.data[i];
      }
      for (size_t i = other.size; i < size; i++)
      {
        data[i].~T();
      }
    }
    else
    {
      for (size_t i = 0; i < size; i++)
      {
        data[i] = (T &&) other.data[i];
      }
      for (size_t i = size; i < other.size; i++)
      {
        new (data + i) T{(T &&) other.data[i]};
      }
    }

    for (size_t i = 0; i < other.size; i++)
    {
      other.data[i].~T();
    }

    size       = other.size;
    other.size = 0;
    return *this;
  }

  constexpr ~Array()
  {
    for (size_t i = 0; i < size; i++)
    {
      data[i].~T();
    }
  }

  constexpr T *begin()
  {
    return data;
  }

  constexpr T const *begin() const
  {
    return data;
  }

  constexpr T *end()
  {
    return data + size;
  }

  constexpr T const *end() const
  {
    return data + size;
  }

  constexpr stx::Span<T> span()
  {
    return stx::Span{data, size};
  }

  constexpr stx::Span<T const> span() const
  {
    return stx::Span{data, size};
  }

  T &operator[](size_t index)
  {
    STX_ARRAY_ENSURE(index < size, "index out of bounds");
    return data[index];
  }

  T const &operator[](size_t index) const
  {
    STX_ARRAY_ENSURE(index < size, "index out of bounds");
    return data[index];
  }

  constexpr T &get_unsafe(size_t index);

  constexpr T const &get_unsafe(size_t index) const;

  constexpr void clear()
  {
    for (size_t i = 0; i < size; i++)
    {
      data[i].~T();
    }
    size = 0;
  }

  constexpr void erase();

  constexpr void erase_unsafe();

  template <typename... Args>
  Result<Void, AllocError> push_inplace(Args &&...args)
  {
    if (size >= Capacity)
    {
      return stx::Err(stx::AllocError::NoMemory);
    }
    push_inplace_unsafe(((Args &&) args)...);
    return stx::Ok(stx::Void{});
  }

  template <typename... Args>
  constexpr void push_inplace_unsafe(Args &&...args)
  {
    new (data + size) T{((Args &&) args)...};
    size++;
  }

  Result<Void, AllocError> push(T &&item)
  {
    return push_inplace((T &&) item);
  }

  constexpr void push_unsafe(T &&item)
  {
    return push_inplace_unsafe((T &&) item);
  }

  Result<Void, AllocError> extend(stx::Span<T const> span);

  constexpr void extend_unsafe(stx::Span<T const> span);

  Result<Void, AllocError> extend_move(stx::Span<T> span);

  constexpr void extend_move_unsafe(stx::Span<T> span);

  union
  {
    T       data[capacity];
    uint8_t rep[capacity * sizeof(T)] = {};
  };
  size_t size = 0;
};

STX_END_NAMESPACE
