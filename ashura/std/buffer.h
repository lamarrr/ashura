/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief A buffer is similar to a `Vec` but it has a few differences:
///
/// - It doesn't manage its memory, the memory is managed elsewhere
/// - The capacity of the memory is fixed and can't change; hence, it can't shrink or increase its memory usage
/// - It can only store trivially copyable data
///
template <typename T>
requires (TriviallyCopyable<T> && NonConst<T>)
struct [[nodiscard]] Buffer
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;
  using View = Span<T>;

  T *   storage_;
  usize size_;
  usize capacity_;

  /// @brief Construct a Buffer from pre-allocated memory
  /// @param storage the pre-allocated memory
  /// @param initial assumed size of the buffer
  /// @param capacity initial assumed capacity of the buffer
  constexpr Buffer(T * storage, usize size, usize capacity) :
    storage_{storage},
    size_{size},
    capacity_{capacity}
  {
  }

  /// @brief Construct an empty buffer from pre-allocated memory
  /// @param span the memory block to use as storage
  constexpr Buffer(Span<T> span) :
    storage_{span.data()},
    size_{0},
    capacity_{span.size()}
  {
  }

  /// @brief Default-construct a buffer
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

  /// @brief Checks if the buffer is empty
  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  /// @brief return a pointer to the stored data
  constexpr T * data() const
  {
    return storage_;
  }

  /// @brief return the number of stored elements
  constexpr usize size() const
  {
    return size_;
  }

  /// @brief return the size of the stored elements in bytes
  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  /// @brief return the total number of elements the buffer is capable of storing
  constexpr usize capacity() const
  {
    return capacity_;
  }

  /// @brief return an iterator to the buffer's elements
  constexpr auto begin() const
  {
    return Iter{.iter_ = data(), .end_ = data() + size_};
  }

  /// @brief returns an iterator terminator
  constexpr auto end() const
  {
    return IterEnd{};
  }

  /// @brief returns a reference to the first element in the buffer
  constexpr T & first() const
  {
    return get(0);
  }

  /// @brief returns a reference to the last element in the buffer
  constexpr T & last() const
  {
    return get(size_ - 1);
  }

  /// @brief returns a reference to the element at index `index`
  /// @param index index of the element
  constexpr T & operator[](usize index) const
  {
    return get(index);
  }

  /// @brief returns a reference to the element at index `index`
  /// @param index index of the element
  constexpr T & get(usize index) const
  {
    return data()[index];
  }

  /// @brief sets the value at `index`
  /// @param index index of the element to set
  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const
  {
    data()[index] = T{static_cast<Args &&>(args)...};
  }

  /// @brief trivially clear all elements contained in the buffer
  constexpr void clear()
  {
    size_ = 0;
  }

  /// @brief clear the Buffer and release its allocated memory
  constexpr void reset()
  {
    storage_  = nullptr;
    size_     = 0;
    capacity_ = 0;
  }

  /// @brief erase elements from `[first, first+num)`
  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  /// @brief erase elements referenced by `slice`
  constexpr void erase(Slice slice)
  {
    slice = slice(size_);
    mem::move(Span{data() + slice.end(), size_ - slice.end()},
              data() + slice.begin());
    size_ -= slice.span;
  }

  /// @brief push an element into the Buffer
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

  /// @brief remove `num` elements from the Buffer
  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    size_ -= num;
  }

  /// @brief try to remove `num` elements from the Buffer if there's up to `num` elements in the Buffer
  /// @returns true if there's at least `num` elements in the Buffer
  [[nodiscard]] constexpr bool try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return false;
    }

    pop(num);

    return true;
  }

  /// @brief Shift element `first` to the end of the buffer by `distance` and leave the
  /// shifted-from memory region uninitialized
  /// @returns true if the shift is possible
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

  /// @brief insert element at position `pos`
  /// @returns true if there's enough memory to perform the insertion
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

  /// @brief copy-insert elements to position `pos`
  /// @returns true if there's enough memory to perform the insertion
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

  /// @brief move-insert element to position `pos`
  /// @returns true if there's enough memory to perform the insertion
  [[nodiscard]] constexpr bool insert_span_move(usize pos, Span<T> span)
  {
    return insert_span(pos, span);
  }

  /// @brief extend the size of the Buffer by `extension` and leave the extended region uninitialized
  /// @returns true if there's enough memory to perform the operation
  [[nodiscard]] constexpr bool extend_uninit(usize extension)
  {
    if ((size_ + extension) > capacity_) [[unlikely]]
    {
      return false;
    }

    size_ += extension;

    return true;
  }

  /// @brief extend the size of the Buffer by `extension` and leave the extended region uninitialized
  /// @returns true if there's enough memory to perform the operation
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

  /// @brief extend the the Buffer with elements in `span`
  /// @returns true if there's enough memory to perform the operation
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

  /// @brief swap elements at index `a` and `b`
  constexpr void swap(usize a, usize b) const
  {
    ash::swap(data()[a], data()[b]);
  }

  /// @brief resize the Buffer to size `new_size`. If the size is greater than the previous size, the
  /// extended region is left uninitialized.
  [[nodiscard]] constexpr bool resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return true;
    }

    return extend_uninit(new_size - size_);
  }

  /// @brief return a non-owning view of the Buffer's elements
  constexpr View view() const
  {
    return View{data(), size()};
  }
};

template <typename T>
Buffer(Span<T>) -> Buffer<T>;

template <typename T, usize N>
Buffer(T (&)[N]) -> Buffer<T>;

/// @brief A single-threaded non-thread-safe RingBuffer
/// Properties:
/// - It has a fixed capacity
/// - It only stores trivial elements
/// - It does not manage its memory
template <typename T>
requires (TriviallyCopyable<T> && NonConst<T>)
struct [[nodiscard]] RingBuffer
{
  using Type = T;
  using Repr = T;

  /// @brief the memory storage for the elements
  T * storage_;

  /// @param number of elements that are available to be consumed
  usize size_;

  /// @param capacity must be a power of 2
  usize capacity_;

  /// @param next element to be yielded to the consumer
  usize consume_next_;

  /// @brief Construct a RingBuffer using pre-allocated memory
  /// @param storage the memory storage to use for the elements
  /// @param size the initial assumed size of the buffer
  /// @param capacity the initial non-zero power-of-2 capacity for the buffer
  /// @param consume_next the index of the first element to be consumed
  constexpr RingBuffer(T * storage, usize size, usize capacity,
                       usize consume_next) :
    storage_{storage},
    size_{size},
    capacity_{capacity},
    consume_next_{consume_next}
  {
  }

  /// @brief Construct an empty RingBuffer
  constexpr RingBuffer() :
    storage_{nullptr},
    size_{0},
    capacity_{0},
    consume_next_{0}
  {
  }

  constexpr RingBuffer(RingBuffer && other) :
    storage_{other.storage_},
    size_{other.size_},
    capacity_{other.capacity_},
    consume_next_{other.consume_next_}
  {
    other.storage_      = nullptr;
    other.size_         = 0;
    other.capacity_     = 0;
    other.consume_next_ = 0;
  }

  constexpr RingBuffer & operator=(RingBuffer && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    storage_      = other.storage_;
    size_         = other.size_;
    capacity_     = other.capacity_;
    consume_next_ = other.consume_next_;

    other.storage_      = nullptr;
    other.size_         = 0;
    other.capacity_     = 0;
    other.consume_next_ = 0;

    return *this;
  }

  constexpr RingBuffer(RingBuffer const &)             = delete;
  constexpr RingBuffer & operator=(RingBuffer const &) = delete;
  constexpr ~RingBuffer()                              = default;

  /// @brief check if the RingBuffer is empty
  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  /// @brief Returns the pointer to the elements of the RingBuffer
  constexpr T * storage() const
  {
    return storage_;
  }

  /// @brief Returns the number of elements in the RingBuffer
  constexpr usize size() const
  {
    return size_;
  }

  /// @brief Returns byte-size of the elements in the RingBuffer
  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  /// @brief Returns element capacity of the RingBuffer
  constexpr usize capacity() const
  {
    return capacity_;
  }

  /// @brief Try to pop one element from the RingBuffer
  /// @param out output memory destination
  /// @returns true if the operation was successful
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

  /// @brief Try to push one element from the RingBuffer
  /// @returns true if there's enough memory to push an element onto the RingBuffer
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
