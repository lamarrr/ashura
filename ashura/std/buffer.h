/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T>
requires (TriviallyCopyable<T> && NonConst<T>)
struct [[nodiscard]] Buffer
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;
  using View = Span<T>;

  T *   storage_  = nullptr;
  usize size_     = 0;
  usize capacity_ = 0;

  constexpr Buffer(T * storage, usize size, usize capacity) :
    storage_{storage},
    size_{size},
    capacity_{capacity}
  {
  }

  constexpr Buffer(Span<T> span) :
    storage_{span.data()},
    size_{0},
    capacity_{span.size()}
  {
  }

  constexpr Buffer() : storage_{nullptr}, size_{0}, capacity_{0}
  {
  }

  constexpr Buffer(Buffer && other) :
    storage_{other.storage_},
    size_{other.size_},
    capacity_{other.capacity_}
  {
    other.storage_  = nullptr;
    other.size_     = 0;
    other.capacity_ = 0;
  }

  constexpr Buffer & operator=(Buffer && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    storage_  = other.storage_;
    size_     = other.size_;
    capacity_ = other.capacity_;

    other.storage_  = nullptr;
    other.size_     = 0;
    other.capacity_ = 0;

    return *this;
  }

  constexpr Buffer(Buffer const &)             = delete;
  constexpr Buffer & operator=(Buffer const &) = delete;
  constexpr ~Buffer()                          = default;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * data() const
  {
    return storage_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr auto begin() const
  {
    return Iter{.iter_ = data(), .end_ = data() + size_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr T & first() const
  {
    return get(0);
  }

  constexpr T & last() const
  {
    return get(size_ - 1);
  }

  constexpr T & operator[](usize index) const
  {
    return get(index);
  }

  constexpr T & get(usize index) const
  {
    return data()[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const
  {
    data()[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr void clear()
  {
    size_ = 0;
  }

  constexpr void reset()
  {
    storage_  = nullptr;
    size_     = 0;
    capacity_ = 0;
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(size_);
    mem::move(Span{data() + slice.end(), size_ - slice.end()},
              data() + slice.begin());
    size_ -= slice.span;
  }

  template <typename... Args>
  [[nodiscard]] constexpr bool push(Args &&... args)
  {
    if (size_ + 1 > capacity_) [[unlikely]]
    {
      return false;
    }

    new (data() + size_) T{static_cast<Args &&>(args)...};

    size_++;

    return true;
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    size_ -= num;
  }

  [[nodiscard]] constexpr bool try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return false;
    }

    pop(num);

    return true;
  }

  [[nodiscard]] bool shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);
    if ((size_ + distance) > capacity_) [[unlikely]]
    {
      return false;
    }

    // potentially overlapping
    mem::move(Span{data() + first, size_ - first}, data() + first + distance);

    size_ += distance;

    return true;
  }

  template <typename... Args>
  [[nodiscard]] constexpr bool insert(usize pos, Args &&... args)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, 1)) [[unlikely]]
    {
      return false;
    }

    new (data() + pos) T{static_cast<Args &&>(args)...};
    return true;
  }

  [[nodiscard]] constexpr bool insert_span(usize pos, Span<T const> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return false;
    }

    mem::copy(span, data() + pos);

    return true;
  }

  [[nodiscard]] constexpr bool insert_span_move(usize pos, Span<T> span)
  {
    return insert_span(pos, span);
  }

  [[nodiscard]] constexpr bool extend_uninit(usize extension)
  {
    if ((size_ + extension) > capacity_) [[unlikely]]
    {
      return false;
    }

    size_ += extension;

    return true;
  }

  [[nodiscard]] constexpr bool extend(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return false;
    }

    obj::default_construct(Span{data() + pos, extension});

    return true;
  }

  [[nodiscard]] constexpr bool extend(Span<T const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return false;
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    mem::copy(span, data() + pos);

    return true;
  }

  constexpr void swap(usize a, usize b) const
  {
    ash::swap(data()[a], data()[b]);
  }

  [[nodiscard]] constexpr bool resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return true;
    }

    return extend_uninit(new_size - size_);
  }

  constexpr View view() const
  {
    return View{data(), size()};
  }
};

template <typename T>
Buffer(Span<T>) -> Buffer<T>;

template <typename T, usize N>
Buffer(T (&)[N]) -> Buffer<T>;

template <typename T>
requires (TriviallyCopyable<T> && NonConst<T>)
struct [[nodiscard]] RingBuffer
{
  using Type = T;
  using Repr = T;

  T *   storage_      = nullptr;
  usize size_         = 0;
  usize capacity_     = 0;
  usize consume_next_ = 0;

  /// @param capacity must be a power of 2
  constexpr RingBuffer(T * storage, usize size, usize capacity) :
    storage_{storage},
    size_{size},
    capacity_{capacity}
  {
  }

  constexpr RingBuffer() : storage_{nullptr}, size_{0}, capacity_{0}
  {
  }

  constexpr RingBuffer(RingBuffer && other) :
    storage_{other.storage_},
    size_{other.size_},
    capacity_{other.capacity_}
  {
    other.storage_  = nullptr;
    other.size_     = 0;
    other.capacity_ = 0;
  }

  constexpr RingBuffer & operator=(RingBuffer && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    storage_  = other.storage_;
    size_     = other.size_;
    capacity_ = other.capacity_;

    other.storage_  = nullptr;
    other.size_     = 0;
    other.capacity_ = 0;

    return *this;
  }

  constexpr RingBuffer(RingBuffer const &)             = delete;
  constexpr RingBuffer & operator=(RingBuffer const &) = delete;
  constexpr ~RingBuffer()                              = default;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * storage() const
  {
    return storage_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  [[nodiscard]] constexpr bool pop(T & out)
  {
    if (size_ == 0) [[unlikely]]
    {
      return false;
    }

    out = storage_[consume_next_];

    consume_next_ = (consume_next_ + 1) & (capacity_ - 1);
    size_--;

    return true;
  }

  template <typename... Args>
  [[nodiscard]] constexpr bool push(Args &&... args)
  {
    if (size_ == capacity_) [[unlikely]]
    {
      return false;
    }

    usize const produce_next = (consume_next_ + size_) & (capacity_ - 1);
    new (storage_ + produce_next) T{static_cast<Args &&>(args)...};

    size_++;

    return true;
  }
};

}    // namespace ash
