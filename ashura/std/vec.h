/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
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

  [[nodiscard]] bool reserve(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return true;
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, target_capacity, &data_))
      {
        return false;
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(target_capacity, &new_data))
      {
        return false;
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
    return true;
  }

  [[nodiscard]] bool fit()
  {
    if (size_ == capacity_)
    {
      return true;
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, size_, &data_))
      {
        return false;
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(size_, &new_data))
      {
        return false;
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
    return true;
  }

  [[nodiscard]] bool grow(usize target_size)
  {
    if (capacity_ >= target_size)
    {
      return true;
    }

    return reserve(max(target_size, capacity_ + (capacity_ >> 1)));
  }

  void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  void erase(Slice slice)
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
  [[nodiscard]] bool push(Args &&...args)
  {
    if (!grow(size_ + 1))
    {
      return false;
    }

    new (data_ + size_) T{((Args &&) args)...};

    size_++;

    return true;
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

  [[nodiscard]] constexpr bool try_pop(usize num = 1)
  {
    if (size_ < num)
    {
      return false;
    }

    pop(num);

    return true;
  }

  [[nodiscard]] bool shift_uninitialized(usize first, usize distance)
  {
    if (!grow(size_ + distance))
    {
      return false;
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

    return true;
  }

  template <typename... Args>
  [[nodiscard]] bool insert(usize dst, Args &&...args)
  {
    if (!shift_uninitialized(dst, 1))
    {
      return false;
    }

    new (data_ + dst) T{((Args &&) args)...};
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
      mem::copy(span.data(), data_ + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + dst + i) T{span[i]};
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
      mem::copy(span.data(), data_ + dst, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data_ + dst + i) T{(T &&) span[i]};
      }
    }

    return true;
  }

  [[nodiscard]] bool extend_uninitialized(usize extension)
  {
    if (!grow(size_ + extension))
    {
      return false;
    }

    size_ += extension;

    return true;
  }

  [[nodiscard]] bool extend_defaulted(usize extension)
  {
    usize const pos = size_;
    if (!extend_uninitialized(extension))
    {
      return false;
    }

    for (usize i = pos; i < size_; i++)
    {
      new (data_ + i) T{};
    }

    return true;
  }

  [[nodiscard]] bool extend_copy(Span<T const> span)
  {
    usize const pos = size_;
    if (!extend_uninitialized(span.size()))
    {
      return false;
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

    return true;
  }

  [[nodiscard]] bool extend_move(Span<T> span)
  {
    usize const pos = size_;
    if (!extend_uninitialized(span.size()))
    {
      return false;
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

    return true;
  }

  constexpr void swap(usize a, usize b) const
  {
    ::ash::swap(data_[a], data_[b]);
  }

  [[nodiscard]] bool resize_uninitialized(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return true;
    }

    return extend_uninitialized(new_size - size_);
  }

  [[nodiscard]] bool resize_defaulted(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return true;
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
    num = min(num_bits, num);
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
