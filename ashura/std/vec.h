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
  T            *storage_   = nullptr;
  usize         capacity_  = 0;
  usize         size_      = 0;

  explicit constexpr Vec(AllocatorImpl allocator) : allocator_{allocator}
  {
  }

  constexpr Vec() : Vec{default_allocator}
  {
  }

  constexpr Vec(Vec const &) = delete;

  constexpr Vec &operator=(Vec const &) = delete;

  constexpr Vec(Vec &&other) :
      allocator_{other.allocator_},
      storage_{other.storage_},
      capacity_{other.capacity_},
      size_{other.size_}
  {
    other.allocator_ = default_allocator;
    other.storage_   = nullptr;
    other.capacity_  = 0;
    other.size_      = 0;
  }

  constexpr Vec &operator=(Vec &&other)
  {
    if (this == &other)
    {
      return *this;
    }
    uninit();
    new (this) Vec{(Vec &&) other};
    return *this;
  }

  constexpr ~Vec()
  {
    uninit();
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T *data() const
  {
    return storage_;
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
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
    allocator_.ndealloc(storage_, capacity_);
  }

  constexpr void reset()
  {
    uninit();
    allocator_ = default_allocator;
    storage_   = nullptr;
    size_      = 0;
    capacity_  = 0;
  }

  constexpr Result<Void, Void> reserve(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, target_capacity, storage_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_storage;
      if (!allocator_.nalloc(target_capacity, new_storage))
      {
        return Err{};
      }

      obj::relocate_non_overlapping(Span{data(), size_}, new_storage);
      allocator_.ndealloc(storage_, capacity_);
      storage_ = new_storage;
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
      if (!allocator_.nrealloc(capacity_, size_, storage_))
      {
        return Err{};
      }
    }
    else
    {
      T *new_storage;
      if (!allocator_.nalloc(size_, new_storage))
      {
        return Err{};
      }

      obj::relocate(Span{data(), size_}, new_storage);
      allocator_.ndealloc(storage_, capacity_);
      storage_ = new_storage;
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
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
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

    new (storage_ + size_) T{((Args &&) args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
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
      // potentially overlapping
      mem::move(Span{data() + first, size_ - first}, data() + first + distance);
    }
    else
    {
      usize const tail_first = max(first, min(size_, distance) - size_);

      // move construct tail elements to uninitialized placements
      obj::move_construct(Span{data() + tail_first, size_ - tail_first},
                          data() + tail_first + distance);

      // move non-tail elements towards end
      obj::move(Span{data() + first, tail_first - first},
                data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
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

    new (storage_ + pos) T{((Args &&) args)...};
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
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy(span, data() + pos);
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
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move(span, data() + pos);
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

    obj::default_construct(Span{data() + pos, extension});

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
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy(span, data() + pos);
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
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move(span, data() + pos);
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

template <typename R>
struct [[nodiscard]] BitVec
{
  using Type = bool;
  using Repr = R;

  Vec<R> repr_     = {};
  usize  bit_size_ = 0;

  explicit constexpr BitVec(AllocatorImpl allocator) : repr_{allocator}
  {
  }

  constexpr BitVec() : BitVec{default_allocator}
  {
  }

  constexpr BitVec(BitVec const &) = delete;

  constexpr BitVec &operator=(BitVec const &) = delete;

  constexpr BitVec(BitVec &&other) :
      repr_{(Vec<R> &&) other.repr_}, bit_size_{other.bit_size_}
  {
    other.bit_size_ = 0;
  }

  constexpr BitVec &operator=(BitVec &&other)
  {
    if (this == &other)
    {
      return *this;
    }
    uninit();
    new (this) BitVec{(BitVec &&) other};
    return *this;
  }

  constexpr ~BitVec() = default;

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
    for (usize dst = slice.begin(), src = slice.end(); src != bit_size_;
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

  // uninitialized storage
  alignas(T) u8 storage_[C * sizeof(T)];
  usize size_ = 0;

  constexpr InplaceVec() = default;

  constexpr InplaceVec(InplaceVec const &other) : size_{other.size_}
  {
    copy_construct(Span{other.data(), other.size_}, data());
  }

  constexpr InplaceVec &operator=(InplaceVec const &other)
  {
    if (this == &other)
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{other};
    return *this;
  }

  constexpr InplaceVec(InplaceVec &&other) : size_{other.size_}
  {
    obj::relocate(Span{other.data(), other.size_}, data());
    other.size_ = 0;
  }

  constexpr InplaceVec &operator=(InplaceVec &&other)
  {
    if (this == &other)
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{(InplaceVec &&) other};
    return *this;
  }

  constexpr ~InplaceVec()
  {
    uninit();
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T *data() const
  {
    return (T *) storage_;
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
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
  }

  constexpr void reset()
  {
    uninit();
    size_ = 0;
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
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
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
    obj::destruct(Span{data() + size_ - num, num});
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
      // potentially overlapping
      mem::move(Span{data() + first, size_ - first}, data() + first + distance);
    }
    else
    {
      usize const tail_first = max(first, min(size_, distance) - size_);

      // move construct tail elements to uninitialized placements
      obj::move_construct(Span{data() + tail_first, size_ - tail_first},
                          data() + tail_first + distance);

      // move non-tail elements towards end
      obj::move(Span{data() + first, tail_first - first},
                data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
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
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy(span, data() + pos);
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
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move(span, data() + pos);
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

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<Void, Void> extend_copy(Span<T const> span)
  {
    usize const pos = size_;
    if (!extend_uninit(span.size()))
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy(span, data() + pos);
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

    if constexpr (TriviallyMoveConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move(span, data() + pos);
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

template <typename T>
struct IsTriviallyRelocatable<Vec<T>>
{
  static constexpr bool value = true;
};

template <typename T>
struct IsTriviallyRelocatable<BitVec<T>>
{
  static constexpr bool value = TriviallyRelocatable<Vec<T>>;
};

template <typename T, usize C>
struct IsTriviallyRelocatable<InplaceVec<T, C>>
{
  static constexpr bool value = TriviallyRelocatable<T>;
};

}        // namespace ash
