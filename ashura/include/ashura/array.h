
#pragma once

#include "stx/allocator.h"
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/void.h"

namespace stx
{

template <typename T, size_t capacity_>
struct Array
{
  static_assert(capacity_ > 0);
  static constexpr size_t capacity = capacity_;
  using Type                       = T;
  using Reference                  = T &;
  using Iterator                   = T *;
  using ConstIterator              = T const *;
  using Pointer                    = T *;
  using Size                       = size_t;
  using Index                      = size_t;

  constexpr Array() : size_{0}
  {
  }

  template <size_t SrcSize>
  constexpr Array(T const (&arr)[SrcSize]) : size_{SrcSize}
  {
    static_assert(SrcSize <= capacity);
  }

  constexpr Array(Array const &);
  constexpr Array(Array &&);
  constexpr Array &operator=(Array const &);
  constexpr Array &operator=(Array &&);

  constexpr ~Array()
  {
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      for (size_t i = 0; i < size_; i++)
      {
        data_[i].~T();
      }
    }
  }

  constexpr size_t size() const
  {
    return size_;
  }

  constexpr T *begin()
  {
    return data_;
  }

  constexpr T const *begin() const
  {
    return data_;
  }

  constexpr T *end()
  {
    return data_ + size_;
  }

  constexpr T const *end() const
  {
    return data_ + size_;
  }

  constexpr T *data()
  {
    return data_;
  }

  constexpr T const *data() const
  {
    return data_;
  }

  constexpr stx::Span<T> span()
  {
  }

  constexpr stx::Span<T const> span() const
  {
  }

  T &operator[](Index index)
  {
  }

  T const &operator[](Index index) const
  {
  }

  constexpr T &get_unsafe(Index index);

  constexpr T const &get_unsafe(Index index) const;

  constexpr void clear();

  constexpr void erase();

  constexpr void erase_unsafe();

  Result<Void, AllocError> push(T &&item);

  constexpr void push_unsafe(T &&item);

  template <typename... Args>
  Result<Void, AllocError> push_inplace(Args &&...args);

  template <typename... Args>
  constexpr void push_inplace_unsafe(Args &&...args);

  Result<Void, AllocError> extend(stx::Span<T const> span);

  constexpr void extend_unsafe(stx::Span<T const> span);

  Result<Void, AllocError> extend_move(stx::Span<T> span);

  constexpr void extend_move_unsafe(stx::Span<T> span);

  union
  {
    T data_[capacity];
  };
  size_t size_ = 0;
};

}        // namespace stx
