/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/result.h"
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

  AllocatorImpl allocator_ = default_allocator;
  T            *data_      = nullptr;
  usize         capacity_  = 0;
  usize         size_      = 0;

  [[nodiscard]] constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr operator bool() const
  {
    return !is_empty();
  }

  [[nodiscard]] constexpr T *data() const
  {
    return data_;
  }

  [[nodiscard]] constexpr usize size() const
  {
    return size_;
  }

  [[nodiscard]] constexpr u32 size32() const
  {
    return (u32) size_;
  }

  [[nodiscard]] constexpr u64 size64() const
  {
    return (u64) size_;
  }

  [[nodiscard]] constexpr usize capacity() const
  {
    return capacity_;
  }

  [[nodiscard]] constexpr T *begin() const
  {
    return data_;
  }

  [[nodiscard]] constexpr T *end() const
  {
    return data_ + size_;
  }

  [[nodiscard]] constexpr T &operator[](usize index) const
  {
    return data_[index];
  }

  [[nodiscard]] constexpr T *try_get(usize index) const
  {
    if (index < size_)
    {
      return data_ + index;
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
    size_ = 0;
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
    allocator_.ndealloc(data_, capacity_);
    data_     = nullptr;
    size_     = 0;
    capacity_ = 0;
  }

  constexpr void uninit()
  {
    reset();
    allocator_ = default_allocator;
  }

  constexpr Result<Void, Void> reserve(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, target_capacity, &data_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(target_capacity, &new_data))
      {
        return Err{};
      }

      for (usize i = 0; i < size_; i++)
      {
        new (new_data + i) T{(T &&) data_[i]};
      }

      for (usize i = 0; i < size_; i++)
      {
        (data_ + i)->~T();
      }

      allocator_.ndealloc(data_, capacity_);
      data_ = new_data;
    }

    capacity_ = target_capacity;
    return Ok{};
  }

  constexpr Result<Void, Void> fit()
  {
    if (size_ == capacity_)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, size_, &data_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(size_, &new_data))
      {
        return Err{};
      }

      for (usize i = 0; i < size_; i++)
      {
        new (new_data + i) T{(T &&) data_[i]};
      }

      for (usize i = 0; i < size_; i++)
      {
        (data_ + i)->~T();
      }

      allocator_.ndealloc(data_, capacity_);
      data_ = new_data;
    }

    capacity_ = size_;
    return Ok{};
  }

  constexpr Result<Void, Void> grow(usize target_size)
  {
    if (capacity_ >= target_size)
    {
      return Ok{};
    }

    return reserve(max(target_size, capacity_ + (capacity_ >> 1)));
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(size_);
    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(data_ + slice.end(), data_ + slice.offset, size_ - slice.end());
    }
    else
    {
      for (usize i = slice.offset; i < size_ - slice.span; i++)
      {
        data_[i] = (T &&) (data_[i + slice.span]);
      }

      for (usize i = size_ - slice.span; i < size_; i++)
      {
        (data_ + i)->~T();
      }
    }
    size_ -= slice.span;
  }

  void erase_non_destructing();

  template <typename... Args>
  constexpr Result<Void, Void> push(Args &&...args)
  {
    if (!grow(size_ + 1))
    {
      return Err{};
    }

    new (data_ + size_) T{((Args &&) args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    if constexpr (!TriviallyDestructible<T>)
    {
      for (usize i = size_ - num; i < size_; i++)
      {
        (data_ + i)->~T();
      }
    }

    size_ -= num;
  }

  constexpr Result<Void, Void> try_pop(usize num = 1)
  {
    if (size_ < num)
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  Result<Void, Void> shift_uninitialized(usize first, usize distance)
  {
    first = min(first, size_);
    if (!grow(size_ + distance))
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(data_ + first, data_ + first + distance, size_ - first);
    }
    else
    {
      // move construct tail elements
      usize const tail_first = max(first, min(size_, distance) - size_);
      for (usize i = tail_first; i < size_; i++)
      {
        new (data_ + i + distance) T{(T &&) data_[i]};
      }

      // move non-tail elements towards end
      for (usize i = first; i < tail_first; i++)
      {
        data_[i + distance] = (T &&) data_[i];
      }

      if constexpr (!TriviallyDestructible<T>)
      {
        // destruct previous position of non-tail elements
        for (usize i = first; i < tail_first; i++)
        {
          (data_ + i)->~T();
        }
      }
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<Void, Void> insert(usize dst, Args &&...args)
  {
    dst = min(dst, size_);
    if (!shift_uninitialized(dst, 1))
    {
      return Err{};
    }

    new (data_ + dst) T{((Args &&) args)...};
    return Ok{};
  }

  constexpr Result<Void, Void> insert_span_copy(usize dst, Span<T const> span)
  {
    dst = min(dst, size_);
    if (!shift_uninitialized(dst, span.size()))
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), data_ + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + dst + i) T{span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> insert_span_move(usize dst, Span<T> span)
  {
    dst = min(dst, size_);
    if (!shift_uninitialized(dst, span.size()))
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), data_ + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + dst + i) T{(T &&) span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_uninitialized(usize extension)
  {
    if (!grow(size_ + extension))
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<Void, Void> extend_defaulted(usize extension)
  {
    usize const pos = size_;
    if (!extend_uninitialized(extension))
    {
      return Err{};
    }

    for (usize i = pos; i < size_; i++)
    {
      new (data_ + i) T{};
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_copy(Span<T const> span)
  {
    usize const pos = size_;
    if (!extend_uninitialized(span.size()))
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), data_ + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + pos + i) T{span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_move(Span<T> span)
  {
    usize const pos = size_;
    if (!extend_uninitialized(span.size()))
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), data_ + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + pos + i) T{(T &&) span[i]};
      }
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b) const
  {
    ::ash::swap(data_[a], data_[b]);
  }

  constexpr Result<Void, Void> resize_uninitialized(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninitialized(new_size - size_);
  }

  constexpr Result<Void, Void> resize_defaulted(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_defaulted(new_size - size_);
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

  constexpr Result<Void, Void> reserve(usize target_capacity)
  {
    return vec.reserve(num_packs(target_capacity));
  }

  constexpr Result<Void, Void> grow(usize target_size)
  {
    return vec.grow(num_packs(target_size));
  }

  constexpr Result<Void, Void> push(bool bit)
  {
    if (!grow(num_bits + 1))
    {
      return Err{};
    }
    this->operator[](num_bits) = bit;
    num_bits++;
    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num_bits, num);
    num_bits -= num;
    usize diff = vec.size() - num_packs(num_bits);
    if (diff > 0)
    {
      vec.pop(diff);
    }
  }

  constexpr Result<Void, Void> try_pop(usize num = 1)
  {
    if (num_bits < num)
    {
      return Err{};
    }
    pop(num);
    return Ok{};
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
  constexpr Result<Void, Void> extend_span_copy(BitSpan<T const> span);

  template <typename T>
  constexpr Result<Void, Void> extend_span_move(BitSpan<T const> span);

  constexpr Result<Void, Void> extend_uninitialized(usize extension)
  {
    if (!vec.extend_uninitialized(num_packs(num_bits + extension) -
                                  num_packs(num_bits)))
    {
      return Err{};
    }

    num_bits += extension;

    return Ok{};
  }

  constexpr Result<Void, Void> extend_defaulted(usize extension)
  {
    usize const pos = num_bits;
    if (!extend_uninitialized(extension))
    {
      return Err{};
    }

    for (usize i = pos; i < num_bits; i++)
    {
      this->operator[](i) = false;
    }

    return Ok{};
  }

  constexpr Result<Void, Void> resize_uninitialized(usize new_size)
  {
    if (new_size <= num_bits)
    {
      erase(new_size, num_bits - new_size);
      return Ok{};
    }

    return extend_uninitialized(new_size - num_bits);
  }

  constexpr Result<Void, Void> resize_defaulted(usize new_size)
  {
    if (new_size <= num_bits)
    {
      erase(new_size, num_bits - new_size);
      return Ok{};
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
constexpr auto span(BitVec<Rep> &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

template <typename Rep>
constexpr auto span(BitVec<Rep> const &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

}        // namespace ash
