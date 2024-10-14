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
struct [[nodiscard]] Vec
{
  using Type = T;
  using Repr = T;

  AllocatorImpl allocator_ = default_allocator;
  T            *data_      = nullptr;
  usize         capacity_  = 0;
  usize         size_      = 0;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T *data() const
  {
    return data_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr T *begin() const
  {
    return data_;
  }

  constexpr T *end() const
  {
    return data_ + size_;
  }

  constexpr T &operator[](usize index) const
  {
    return data_[index];
  }

  constexpr T &get(usize index) const
  {
    return data_[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&...args) const
  {
    data_[index] = T{((Args &&) args)...};
  }

  constexpr T *try_get(usize index) const
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
      if (!allocator_.nrealloc(capacity_, target_capacity, data_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(target_capacity, new_data))
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
      if (!allocator_.nrealloc(capacity_, size_, data_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_data;
      if (!allocator_.nalloc(size_, new_data))
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

  Result<Void, Void> shift_uninit(usize first, usize distance)
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
  constexpr Result<Void, Void> insert(usize pos, Args &&...args)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, 1))
    {
      return Err{};
    }

    new (data_ + pos) T{((Args &&) args)...};
    return Ok{};
  }

  constexpr Result<Void, Void> insert_span_copy(usize pos, Span<T const> span)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, span.size()))
    {
      return Err{};
    }

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

  constexpr Result<Void, Void> insert_span_move(usize pos, Span<T> span)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, span.size()))
    {
      return Err{};
    }

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

  constexpr Result<Void, Void> extend_uninit(usize extension)
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
    if (!extend_uninit(extension))
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
    if (!extend_uninit(span.size()))
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
    if (!extend_uninit(span.size()))
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

  constexpr Result<Void, Void> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
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

template <typename R>
struct [[nodiscard]] BitVec
{
  using Type = bool;
  using Repr = R;

  Vec<R> repr_     = {};
  usize  bit_size_ = 0;

  constexpr bool operator[](usize index) const
  {
    return ::ash::get_bit(span(repr_), index);
  }

  constexpr Vec<R> const &repr() const
  {
    return repr_;
  }

  constexpr Vec<R> &repr()
  {
    return repr_;
  }

  constexpr usize size() const
  {
    return bit_size_;
  }

  constexpr bool is_empty() const
  {
    return bit_size_ == 0;
  }

  constexpr bool has_trailing() const
  {
    return bit_size_ != (repr_.size_ * sizeof(R) * 8);
  }

  constexpr usize capacity() const
  {
    return repr_.capacity_ * sizeof(R) * 8;
  }

  constexpr void clear()
  {
    bit_size_ = 0;
    repr_.clear();
  }

  constexpr void reset()
  {
    bit_size_ = 0;
    repr_.reset();
  }

  constexpr bool get(usize index) const
  {
    return ::ash::get_bit(span(repr_), index);
  }

  constexpr void set(usize index, bool value) const
  {
    ::ash::assign_bit(span(repr_), index, value);
  }

  constexpr bool get_bit(usize index) const
  {
    return ::ash::get_bit(span(repr_), index);
  }

  constexpr bool set_bit(usize index) const
  {
    return ::ash::set_bit(span(repr_), index);
  }

  constexpr bool clear_bit(usize index) const
  {
    return ::ash::clear_bit(span(repr_), index);
  }

  constexpr void flip_bit(usize index) const
  {
    ::ash::flip_bit(span(repr_), index);
  }

  constexpr Result<Void, Void> reserve(usize target_capacity)
  {
    return repr_.reserve(bit_packs<R>(target_capacity));
  }

  constexpr Result<Void, Void> fit()
  {
    return repr_.fit();
  }

  constexpr Result<Void, Void> grow(usize target_size)
  {
    return repr_.grow(bit_packs<R>(target_size));
  }

  constexpr Result<Void, Void> push(bool bit)
  {
    usize index = bit_size_;
    if (!extend_uninit(1))
    {
      return Err{};
    }
    set(index, bit);
    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(bit_size_, num);
    bit_size_ -= num;
    usize diff = repr_.size() - bit_packs<R>(bit_size_);
    repr_.pop(diff);
  }

  constexpr Result<Void, Void> try_pop(usize num = 1)
  {
    if (bit_size_ < num)
    {
      return Err{};
    }
    pop(num);
    return Ok{};
  }

  constexpr Result<Void, Void> insert(usize pos, bool value)
  {
    pos = min(pos, bit_size_);
    if (!extend_uninit(1))
    {
      return Err{};
    }
    for (usize src = pos, dst = src + 1; src < bit_size_; src++, dst++)
    {
      set(dst, get(src));
    }
    set(pos, value);
    return Ok{};
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(bit_size_);
    for (usize dst = slice.offset, src = slice.end(); src != bit_size_;
         dst++, src++)
    {
      set(dst, get(src));
    }
    pop(slice.span);
  }

  constexpr Result<Void, Void> extend_uninit(usize extension)
  {
    if (!repr_.extend_uninit(bit_packs<R>(bit_size_ + extension) -
                             bit_packs<R>(bit_size_)))
    {
      return Err{};
    }

    bit_size_ += extension;

    return Ok{};
  }

  constexpr Result<Void, Void> extend_defaulted(usize extension)
  {
    usize const pos = bit_size_;
    if (!extend_uninit(extension))
    {
      return Err{};
    }

    for (usize i = pos; i < bit_size_; i++)
    {
      set(i, false);
    }

    return Ok{};
  }

  constexpr Result<Void, Void> resize_uninit(usize new_size)
  {
    if (new_size <= bit_size_)
    {
      erase(new_size, bit_size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - bit_size_);
  }

  constexpr Result<Void, Void> resize_defaulted(usize new_size)
  {
    if (new_size <= bit_size_)
    {
      erase(new_size, bit_size_ - new_size);
      return Ok{};
    }

    return extend_defaulted(new_size - bit_size_);
  }

  constexpr void swap(usize a, usize b) const
  {
    bool av = get(a);
    bool bv = get(b);
    set(a, bv);
    set(b, av);
  }
};

template <typename Rep>
constexpr auto bit_span(BitVec<Rep> &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

template <typename Rep>
constexpr auto bit_span(BitVec<Rep> const &container) -> BitSpan<Rep>
{
  return BitSpan{container.data(), container.size()};
}

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

template <typename T, usize C>
struct [[nodiscard]] InplaceVec
{
  using Type = T;
  using Repr = T;

  static constexpr usize Capacity = C;

  alignas(T) u8 data_[C * sizeof(T)];
  usize size_ = 0;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T *data() const
  {
    return (T *) data_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  constexpr usize capacity() const
  {
    return Capacity;
  }

  constexpr T *begin() const
  {
    return data();
  }

  constexpr T *end() const
  {
    return data() + size_;
  }

  constexpr T &operator[](usize index) const
  {
    return data()[index];
  }

  constexpr T &get(usize index) const
  {
    return data()[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&...args) const
  {
    data()[index] = T{((Args &&) args)...};
  }

  constexpr T *try_get(usize index) const
  {
    if (index < size_)
    {
      return data() + index;
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
    size_ = 0;
  }

  constexpr void uninit()
  {
    reset();
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
      mem::move(data() + slice.end(), data() + slice.offset,
                size_ - slice.end());
    }
    else
    {
      for (usize i = slice.offset; i < size_ - slice.span; i++)
      {
        data()[i] = (T &&) (data()[i + slice.span]);
      }

      for (usize i = size_ - slice.span; i < size_; i++)
      {
        (data() + i)->~T();
      }
    }
    size_ -= slice.span;
  }

  template <typename... Args>
  constexpr Result<Void, Void> push(Args &&...args)
  {
    if ((size_ + 1) > Capacity)
    {
      return Err{};
    }

    new (data() + size_) T{((Args &&) args)...};

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
        (data() + i)->~T();
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

  Result<Void, Void> shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);
    if ((size_ + distance) > Capacity)
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(data() + first, data() + first + distance, size_ - first);
    }
    else
    {
      // move construct tail elements
      usize const tail_first = max(first, min(size_, distance) - size_);
      for (usize i = tail_first; i < size_; i++)
      {
        new (data() + i + distance) T{(T &&) data()[i]};
      }

      // move non-tail elements towards end
      for (usize i = first; i < tail_first; i++)
      {
        data()[i + distance] = (T &&) data()[i];
      }

      if constexpr (!TriviallyDestructible<T>)
      {
        // destruct previous position of non-tail elements
        for (usize i = first; i < tail_first; i++)
        {
          (data() + i)->~T();
        }
      }
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<Void, Void> insert(usize pos, Args &&...args)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, 1))
    {
      return Err{};
    }

    new (data() + pos) T{((Args &&) args)...};
    return Ok{};
  }

  constexpr Result<Void, Void> insert_span_copy(usize pos, Span<T const> span)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, span.size()))
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), data() + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data() + pos + i) T{span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> insert_span_move(usize pos, Span<T> span)
  {
    pos = min(pos, size_);
    if (!shift_uninit(pos, span.size()))
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), data() + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data() + pos + i) T{(T &&) span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_uninit(usize extension)
  {
    if ((size_ + extension) > Capacity)
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<Void, Void> extend_defaulted(usize extension)
  {
    usize const pos = size_;
    if (!extend_uninit(extension))
    {
      return Err{};
    }

    for (usize i = pos; i < size_; i++)
    {
      new (data() + i) T{};
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_copy(Span<T const> span)
  {
    usize const pos = size_;
    if (!extend_uninit(span.size()))
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span.data(), data() + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data() + pos + i) T{span[i]};
      }
    }

    return Ok{};
  }

  constexpr Result<Void, Void> extend_move(Span<T> span)
  {
    usize const pos = size_;
    if (!extend_uninit(span.size()))
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span.data(), data() + pos, span.size());
    }
    else
    {
      for (usize i = 0; i < span.size(); i++)
      {
        new (data() + pos + i) T{(T &&) span[i]};
      }
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b) const
  {
    ::ash::swap(data()[a], data()[b]);
  }

  constexpr Result<Void, Void> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
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

}        // namespace ash
