
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
    for (size_t i = 0; i < SrcSize; i++)
    {
      new (data() + i) T{arr[i]};
    }
  }

  constexpr Array(Array const &other) : size_{other.size_}
  {
    for (size_t i = 0; i < other.size_; i++)
    {
      new (data() + i) T{other.data()[i]};
    }
  }

  constexpr Array(Array &&other) : size_{other.size_}
  {
    for (size_t i = 0; i < other.size_; i++)
    {
      new (data() + i) T{(T &&) (other.data()[i])};
    }

    for (size_t i = 0; i < other.size_; i++)
    {
      (other.data() + i)->~T();
    }

    other.size_ = 0;
  }

  constexpr Array &operator=(Array const &other)
  {
    this->~Array();
    new (this) Array{other};
    return *this;
  }

  constexpr Array &operator=(Array &&other)
  {
    this->~~Array();
    new (this) Array{(Array &&) other};
    return *this;
  }

  constexpr ~Array()
  {
    for (size_t i = 0; i < size_; i++)
    {
      data()[i].~T();
    }
  }

  constexpr size_t size() const
  {
    return size_;
  }

  constexpr T *begin()
  {
    return data();
  }

  constexpr T const *begin() const
  {
    return data();
  }

  constexpr T *end()
  {
    return data() + size_;
  }

  constexpr T const *end() const
  {
    return data() + size_;
  }

  constexpr T *data()
  {
    return &storage_[0].value;
  }

  constexpr T const *data() const
  {
    return &storage_[0].value;
  }

  constexpr stx::Span<T> span()
  {
    return stx::Span{data(), size_};
  }

  constexpr stx::Span<T const> span() const
  {
    return stx::Span{data(), size_};
  }

  T &operator[](Index index)
  {
    STX_ARRAY_ENSURE(index < size_, "index out of bounds");
    return data()[index];
  }

  T const &operator[](Index index) const
  {
    STX_ARRAY_ENSURE(index < size_, "index out of bounds");
    return data()[index];
  }

  constexpr T &get_unsafe(Index index);

  constexpr T const &get_unsafe(Index index) const;

  constexpr void clear()
  {
    this->~Array();
    size_ = 0;
  }

  constexpr void erase();

  constexpr void erase_unsafe();

  template <typename... Args>
  Result<Void, AllocError> push_inplace(Args &&...args)
  {
    if (size_ >= Capacity)
    {
      return stx::Err(stx::AllocError::NoMemory);
    }
    push_inplace_unsafe(((Args &&) args)...);
    return stx::Ok(stx::Void{});
  }

  template <typename... Args>
  constexpr void push_inplace_unsafe(Args &&...args)
  {
    new (&(storage_[size_].value)) T{((Args &&) args)...};
    size_++;
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

  union Storage
  {
    T       value;
    uint8_t rep[sizeof(T)] = {};
  } storage_[capacity];
  size_t size_ = 0;
};

STX_END_NAMESPACE
