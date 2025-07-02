/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Minimum alignment of the Vec types. This fits into 1 AVX-512 lane.
/// Only InplaceVec doesn't use this alignment as it is usually in-place and
/// compacted along with other struct members/stack variables.
inline constexpr usize MIN_VEC_ALIGNMENT = 64;

template <typename T>
requires (NonConst<T>)
struct [[nodiscard]] Vec
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;
  using View = Span<T>;

  static constexpr usize ALIGNMENT = max(alignof(Type), MIN_VEC_ALIGNMENT);

  Type *       storage_   = nullptr;
  usize        size_      = 0;
  usize        capacity_  = 0;
  AllocatorRef allocator_ = {};

  explicit constexpr Vec(AllocatorRef allocator) :
    storage_{nullptr},
    size_{0},
    capacity_{0},
    allocator_{allocator}
  {
  }

  constexpr Vec() : Vec{default_allocator}
  {
  }

  constexpr Vec(AllocatorRef allocator, Type * storage, usize capacity,
                usize size) :
    storage_{storage},
    size_{size},
    capacity_{capacity},
    allocator_{allocator}
  {
  }

  constexpr Vec(Vec const &) = delete;

  constexpr Vec & operator=(Vec const &) = delete;

  constexpr Vec(Vec && other) :
    storage_{other.storage_},
    size_{other.size_},
    capacity_{other.capacity_},
    allocator_{other.allocator_}
  {
    other.storage_   = nullptr;
    other.size_      = 0;
    other.capacity_  = 0;
    other.allocator_ = default_allocator;
  }

  constexpr Vec & operator=(Vec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) Vec{static_cast<Vec &&>(other)};
    return *this;
  }

  constexpr ~Vec()
  {
    uninit();
  }

  static constexpr Result<Vec> make(usize capacity, AllocatorRef allocator = {})
  {
    Vec out{allocator};

    if (!out.reserve(capacity))
    {
      return Err{};
    }

    return Ok{static_cast<Vec &&>(out)};
  }

  constexpr Result<Vec> clone(AllocatorRef allocator) const
  {
    Vec out{allocator};

    if (!out.extend(*this))
    {
      return Err{};
    }

    return Ok{static_cast<Vec &&>(out)};
  }

  constexpr Result<Vec> clone() const
  {
    return clone(allocator_);
  }

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr Type * data() const
  {
    return assume_aligned<ALIGNMENT>(storage_);
  }

  static constexpr usize alignment()
  {
    return ALIGNMENT;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(Type) * size();
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr usize capacity_bytes() const
  {
    return sizeof(Type) * capacity();
  }

  constexpr auto begin() const
  {
    return Iter{.iter_ = data(), .end_ = data() + size()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr Type & first() const
  {
    return get(0);
  }

  constexpr Type & last() const
  {
    return get(size() - 1);
  }

  constexpr Type & operator[](usize index) const
  {
    return get(index);
  }

  constexpr Type & get(usize index) const
  {
    return data()[index];
  }

  constexpr Option<Type &> try_get(usize index) const
  {
    if (index >= size()) [[unlikely]]
    {
      return none;
    }

    return data()[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const
  {
    data()[index] = Type{static_cast<Args &&>(args)...};
  }

  constexpr void clear()
  {
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
    allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
  }

  constexpr void reset()
  {
    uninit();
    storage_   = nullptr;
    size_      = 0;
    capacity_  = 0;
    allocator_ = default_allocator;
  }

  constexpr Result<> reserve(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<Type>)
    {
      if (!allocator_->pnrealloc(ALIGNMENT, capacity_, target_capacity,
                                 storage_)) [[unlikely]]
      {
        return Err{};
      }
    }
    else
    {
      Type * new_storage;
      if (!allocator_->pnalloc(ALIGNMENT, target_capacity, new_storage))
        [[unlikely]]
      {
        return Err{};
      }

      obj::relocate_nonoverlapping(Span{data(), size_}, new_storage);
      allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
      storage_ = new_storage;
    }

    capacity_ = target_capacity;
    return Ok{};
  }

  constexpr Result<> reserve_extend(usize extension)
  {
    return reserve(size_ + extension);
  }

  constexpr Result<> fit()
  {
    if (size_ == capacity_)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<Type>)
    {
      if (!allocator_->pnrealloc(ALIGNMENT, capacity_, size_, storage_))
        [[unlikely]]
      {
        return Err{};
      }
    }
    else
    {
      Type * new_storage;
      if (!allocator_->pnalloc(ALIGNMENT, size_, new_storage)) [[unlikely]]
      {
        return Err{};
      }

      obj::relocate_nonoverlapping(Span{data(), size_}, new_storage);
      allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
      storage_ = new_storage;
    }

    capacity_ = size_;
    return Ok{};
  }

  static constexpr usize growth(usize capacity)
  {
    return capacity << 1;
  }

  constexpr Result<> grow(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return Ok{};
    }

    return reserve(max(target_capacity, growth(capacity_)));
  }

  constexpr Result<> grow_extend(usize extension)
  {
    return grow(size() + extension);
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(size_);
    if constexpr (TriviallyRelocatable<Type>)
    {
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move_assign(Span{data() + slice.end(), size_ - slice.end()},
                       data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
    }
    size_ -= slice.span;
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if (!grow(size_ + 1)) [[unlikely]]
    {
      return Err{};
    }

    new (data() + size_) Type{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size() < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  Result<> shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);
    if (!grow(size_ + distance)) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<Type>)
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
      obj::move_assign(Span{data() + first, tail_first - first},
                       data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> insert(usize pos, Args &&... args)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, 1)) [[unlikely]]
    {
      return Err{};
    }

    new (data() + pos) Type{static_cast<Args &&>(args)...};
    return Ok{};
  }

  constexpr Result<> insert_span(usize pos, Span<Type const> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> insert_span_move(usize pos, Span<Type> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if (!grow(size_ + extension)) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend(Span<Type const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<Type> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b) const
  {
    ash::swap(data()[a], data()[b]);
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
  }

  constexpr Result<> resize(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend(new_size - size_);
  }

  constexpr View view() const
  {
    return View{data(), size()};
  }
};

template <typename T>
constexpr Result<Vec<T>> vec(AllocatorRef allocator, Span<T const> data)
{
  Result out = Vec<T>::make(data.size(), allocator);

  if (!out)
  {
    return out;
  }

  out.v().extend(data).unwrap();

  return out;
}

template <typename T>
constexpr Result<Vec<T>> vec_move(AllocatorRef allocator, Span<T> data)
{
  Result out = Vec<T>::make(data.size(), allocator);

  if (!out)
  {
    return out;
  }

  out.v().extend_move(data).unwrap();

  return out;
}

/// @brief A vector with elements pinned to memory, The address of the vector is
/// stable over its lifetime and guarantees that there's never reference
/// invalidation. The elements are never relocated during their lifetime. It can
/// only pop elements and add elements while within its capacity. It also never
/// reallocates nor grow in capacity.
template <typename T>
requires (NonConst<T>)
struct [[nodiscard]] PinVec
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;
  using View = Span<T>;

  static constexpr usize ALIGNMENT = max(alignof(Type), MIN_VEC_ALIGNMENT);

  Type *       storage_;
  usize        size_;
  usize        capacity_;
  AllocatorRef allocator_;

  constexpr PinVec() :
    storage_{nullptr},
    size_{0},
    capacity_{0},
    allocator_{default_allocator}
  {
  }

  constexpr PinVec(AllocatorRef allocator, Type * storage, usize capacity,
                   usize size) :
    storage_{storage},
    size_{size},
    capacity_{capacity},
    allocator_{allocator}
  {
  }

  constexpr PinVec(PinVec const &) = delete;

  constexpr PinVec & operator=(PinVec const &) = delete;

  constexpr PinVec(PinVec && other) :
    storage_{other.storage_},
    size_{other.size_},
    capacity_{other.capacity_},
    allocator_{other.allocator_}
  {
    other.storage_   = nullptr;
    other.size_      = 0;
    other.capacity_  = 0;
    other.allocator_ = default_allocator;
  }

  constexpr PinVec & operator=(PinVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    uninit();
    new (this) PinVec{static_cast<PinVec &&>(other)};

    return *this;
  }

  constexpr ~PinVec()
  {
    uninit();
  }

  static constexpr Result<PinVec> make(usize        capacity,
                                       AllocatorRef allocator = {})
  {
    Type * storage;
    if (!allocator->pnalloc(ALIGNMENT, capacity, storage)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{
      PinVec{allocator, storage, capacity, 0}
    };
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
    allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
  }

  constexpr void reset()
  {
    uninit();
    storage_   = nullptr;
    size_      = 0;
    capacity_  = 0;
    allocator_ = {};
  }

  constexpr Result<PinVec> clone(AllocatorRef allocator) const
  {
    Result<PinVec> out = PinVec::make(allocator, capacity_);

    if (!out)
    {
      return out;
    }

    obj::copy_construct(view(), out.v().view());

    return out;
  }

  Result<PinVec> clone() const
  {
    return clone(allocator_);
  }

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr Type * data() const
  {
    return assume_aligned<ALIGNMENT>(storage_);
  }

  static constexpr usize alignment()
  {
    return ALIGNMENT;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(Type) * size();
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr usize capacity_bytes() const
  {
    return sizeof(Type) * capacity();
  }

  constexpr auto begin() const
  {
    return Iter{.iter_ = data(), .end_ = data() + size()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr Type & first() const
  {
    return get(0);
  }

  constexpr Type & last() const
  {
    return get(size() - 1);
  }

  constexpr Type & operator[](usize index) const
  {
    return get(index);
  }

  constexpr Type & get(usize index) const
  {
    return data()[index];
  }

  constexpr void clear()
  {
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if ((size_ + 1) > capacity_) [[unlikely]]
    {
      return Err{};
    }

    new (data() + size_) Type{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if ((size_ + extension) > capacity_) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend(Span<Type const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<Type> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<Type>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr View view() const
  {
    return View{data(), size()};
  }
};

template <typename R>
requires (NonConst<R>)
struct [[nodiscard]] BitVec
{
  using Type = bool;
  using Repr = R;
  using Iter = BitSpanIter<R>;
  using View = BitSpan<R>;

  Vec<R> repr_ = {};
  usize  size_ = 0;

  explicit constexpr BitVec(AllocatorRef allocator) : repr_{allocator}
  {
  }

  constexpr BitVec() : BitVec{default_allocator}
  {
  }

  constexpr BitVec(BitVec const &) = delete;

  constexpr BitVec & operator=(BitVec const &) = delete;

  constexpr BitVec(BitVec && other) :
    repr_{static_cast<Vec<R> &&>(other.repr_)},
    size_{other.size_}
  {
    other.size_ = 0;
  }

  constexpr BitVec & operator=(BitVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) BitVec{static_cast<BitVec &&>(other)};
    return *this;
  }

  constexpr ~BitVec() = default;

  constexpr auto const & repr() const
  {
    return repr_;
  }

  constexpr auto & repr()
  {
    return repr_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr auto begin() const
  {
    return Iter{.storage_ = repr_.data(), .iter_ = 0, .end_ = size()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr usize capacity() const
  {
    return repr_.capacity() * bitsizeof<R>;
  }

  constexpr void clear()
  {
    repr_.clear();
    size_ = 0;
  }

  constexpr void uninit()
  {
    repr_.uninit();
  }

  constexpr void reset()
  {
    repr_.reset();
    size_ = 0;
  }

  constexpr Type operator[](usize index) const
  {
    return get(index);
  }

  constexpr Type first() const
  {
    return get(0);
  }

  constexpr Type last() const
  {
    return get(size() - 1);
  }

  constexpr Type get(usize index) const
  {
    return view().get(index);
  }

  constexpr void set(usize index, Type value) const
  {
    view().set(index, value);
  }

  constexpr Type get_bit(usize index) const
  {
    return get(index);
  }

  constexpr void set_bit(usize index) const
  {
    view().set_bit(index);
  }

  constexpr void clear_bit(usize index) const
  {
    view().clear_bit(index);
  }

  constexpr void flip_bit(usize index) const
  {
    view().flip_bit(index);
  }

  constexpr Result<> reserve(usize target_capacity)
  {
    return repr_.reserve(atom_size_for<R>(target_capacity));
  }

  constexpr Result<> reserve_extend(usize extension)
  {
    return reserve(size() + extension);
  }

  constexpr Result<> fit()
  {
    return repr_.fit();
  }

  constexpr Result<> grow(usize target_capacity)
  {
    return repr_.grow(atom_size_for<R>(target_capacity));
  }

  constexpr Result<> grow_extend(usize extension)
  {
    return grow(size() + extension);
  }

  constexpr Result<> push(bool bit)
  {
    usize const index = size_;

    if (!extend_uninit(1)) [[unlikely]]
    {
      return Err{};
    }

    set(index, bit);
    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(size_, num);
    size_ -= num;
    auto const diff = repr_.size() - atom_size_for<R>(size_);
    repr_.pop(diff);
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size() < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);
    return Ok{};
  }

  constexpr Result<> insert(usize pos, bool value)
  {
    pos = min(pos, size_);
    if (!extend_uninit(1)) [[unlikely]]
    {
      return Err{};
    }
    for (usize src = pos, dst = src + 1; src < size_; src++, dst++)
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
    slice = slice(size_);
    for (usize dst = slice.begin(), src = slice.end(); src != size_;
         ++dst, ++src)
    {
      set(dst, get(src));
    }
    pop(slice.span);
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if (!repr_.extend_uninit(atom_size_for<R>(size_ + extension) -
                             atom_size_for<R>(size_))) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    for (usize i = pos; i < size_; i++)
    {
      set(i, false);
    }

    return Ok{};
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
  }

  constexpr Result<> resize(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend(new_size - size_);
  }

  constexpr void swap(usize a, usize b) const
  {
    bool av = get(a);
    bool bv = get(b);
    set(a, bv);
    set(b, av);
  }

  constexpr auto view() const
  {
    return View{repr_, size()};
  }
};

template <typename T, usize Capacity, usize Alignment = alignof(T)>
requires (NonConst<T>)
struct [[nodiscard]] InplaceVec
  : InplaceStorage<Alignment, sizeof(T) * Capacity>
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;
  using View = Span<T>;

  static constexpr usize ALIGNMENT = Alignment;
  static constexpr usize CAPACITY  = Capacity;

  usize size_ = 0;

  constexpr InplaceVec() = default;

  constexpr InplaceVec(InplaceVec const & other) : size_{other.size()}
  {
    obj::copy_construct(Span{other.data(), other.size()}, data());
  }

  constexpr InplaceVec & operator=(InplaceVec const & other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{other};
    return *this;
  }

  constexpr InplaceVec(InplaceVec && other) : size_{other.size()}
  {
    obj::relocate_nonoverlapping(Span{other.data(), other.size_}, data());
    other.size_ = 0;
  }

  constexpr InplaceVec & operator=(InplaceVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{static_cast<InplaceVec &&>(other)};
    return *this;
  }

  constexpr ~InplaceVec()
  {
    uninit();
  }

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr Type * data() const
  {
    return assume_aligned<ALIGNMENT>(reinterpret_cast<Type *>(this->storage_));
  }

  static constexpr usize alignment()
  {
    return ALIGNMENT;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(Type) * size();
  }

  static constexpr usize capacity()
  {
    return Capacity;
  }

  constexpr auto begin() const
  {
    return Iter{.iter_ = data(), .end_ = data() + size()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr Type & first() const
  {
    return get(0);
  }

  constexpr Type & last() const
  {
    return get(size() - 1);
  }

  constexpr Type & operator[](usize index) const
  {
    return get(index);
  }

  constexpr Type & get(usize index) const
  {
    return data()[index];
  }

  constexpr Option<Type &> try_get(usize index) const
  {
    if (index >= size()) [[unlikely]]
    {
      return none;
    }

    return data()[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const
  {
    data()[index] = Type{static_cast<Args &&>(args)...};
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

    if constexpr (TriviallyRelocatable<Type>)
    {
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move_assign(Span{data() + slice.end(), size_ - slice.end()},
                       data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
    }

    size_ -= slice.span;
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if ((size_ + 1) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    new (data() + size_) Type{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size() < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  Result<> shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);

    if ((size_ + distance) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<Type>)
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
      obj::move_assign(Span{data() + first, tail_first - first},
                       data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> insert(usize pos, Args &&... args)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, 1)) [[unlikely]]
    {
      return Err{};
    }

    new (data() + pos) Type{static_cast<Args &&>(args)...};
    return Ok{};
  }

  constexpr Result<> insert_span(usize pos, Span<Type const> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<Type>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> insert_span_move(usize pos, Span<Type> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<Type>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if ((size_ + extension) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend(Span<Type const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<Type>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<Type> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<Type>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_construct(span, data() + pos);
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b)
  {
    ash::swap(data()[a], data()[b]);
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
  }

  constexpr Result<> resize(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend(new_size - size_);
  }

  constexpr auto view() const
  {
    return View{data(), size()};
  }
};

/// @brief Sparse Vector (a.k.a Sparse Set) are used for stable ID-tagging of
/// objects in high-perf scenarious i.e. ECS, where a stable identity is needed
/// for objects and they need to be processed in batches for efficiency. They
/// have an indirect index into their elements, although they don't guarantee
/// stability of the addresses of the elements they guarantee that the IDs
/// persist until the id is released. Unlike typical Sparse Sets, Sparse Vec's
/// elements are always contiguous without holes in them, making them suitable
/// for operations like batch-processing and branchless SIMD.
///
/// @tparam V dense containers for the properties, i.e. Vec<i32>, Vec<f32>
/// @param index_to_id id of data, ordered relative to {data}
/// @param id_to_index map of id to index in {data}
/// @param size the number of valid elements in the sparse set
/// @param capacity the number of elements the sparse set has capacity for,
/// includes reserved but unallocated ids pointing to valid but uninitialized
/// memory
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASED_MASK
///
///
template <typename... V>
requires (NonConst<V> && ... && true)
struct SparseVec
{
  using Dense = Tuple<V...>;
  using Id    = usize;
  using Ids   = Vec<Id>;

  static constexpr Id RELEASED_MASK = ~(NumTraits<Id>::MAX >> 1);
  static constexpr Id STUB          = NumTraits<Id>::MAX;

  struct Iter
  {
    Tuple<typename V::Iter...> iters_;

    constexpr auto operator*() const
    {
      return apply(
        [](auto &... iters) { return Tuple<decltype(*iters)...>{*iters...}; },
        iters_);
    }

    constexpr Iter & operator++()
    {
      apply([](auto &... iters) { (++iters, ...); }, iters_);
      return *this;
    }

    constexpr bool operator!=(IterEnd) const
    {
      return apply(
        [](auto &... iters) {
          static constexpr bool ZERO_SIZED = (sizeof...(V) == 0);
          return ((!ZERO_SIZED) && ... && (iters != IterEnd{}));
        },
        iters_);
    }
  };

  struct View
  {
    Tuple<typename V::View...> views_;

    constexpr auto begin() const
    {
      return apply(
        [](auto &&... views) {
          return Tuple<typename V::View::Iter...>{ash::begin(views)...};
        },
        views_);
    }

    constexpr auto end() const
    {
      return IterEnd{};
    }
  };

  Ids   index_to_id_;
  Ids   id_to_index_;
  Dense dense;
  Id    free_id_head_;

  explicit constexpr SparseVec(Ids index_to_id, Ids id_to_index, Dense dense,
                               Id free_id_head) :
    index_to_id_{static_cast<Ids &&>(index_to_id)},
    id_to_index_{static_cast<Ids &&>(id_to_index)},
    dense{static_cast<Dense &&>(dense)},
    free_id_head_{free_id_head}
  {
  }

  explicit constexpr SparseVec(AllocatorRef allocator) :
    index_to_id_{allocator},
    id_to_index_{allocator},
    dense{V{allocator}...},
    free_id_head_{STUB}
  {
  }

  constexpr SparseVec() : SparseVec{default_allocator}
  {
  }

  constexpr SparseVec(SparseVec const &) = delete;

  constexpr SparseVec & operator=(SparseVec const &) = delete;

  constexpr SparseVec(SparseVec && other) :
    index_to_id_{static_cast<Ids &&>(other.index_to_id_)},
    id_to_index_{static_cast<Ids &&>(other.id_to_index_)},
    dense{static_cast<Dense &&>(other.dense)},
    free_id_head_{other.free_id_head_}
  {
    other.free_id_head_ = STUB;
  }

  constexpr SparseVec & operator=(SparseVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    index_to_id_        = static_cast<Ids &&>(other.index_to_id_);
    id_to_index_        = static_cast<Ids &&>(other.id_to_index_);
    dense               = static_cast<Dense &&>(other.dense);
    free_id_head_       = other.free_id_head_;
    other.free_id_head_ = STUB;
    return *this;
  }

  constexpr ~SparseVec() = default;

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr Id size() const
  {
    return static_cast<Id>(index_to_id_.size());
  }

  constexpr auto begin() const
  {
    return Iter{.iters_ = apply(
                  [](auto &... dense) {
                    return Tuple<decltype(ash::begin(dense))...>{
                      ash::begin(dense)...};
                  },
                  dense)};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr auto view() const
  {
    return apply([](auto &... dense) { return View{ash::view(dense)...}; },
                 dense);
  }

  constexpr void clear()
  {
    apply([](auto &... d) { (d.clear(), ...); }, dense);
    id_to_index_.clear();
    index_to_id_.clear();
    free_id_head_ = STUB;
  }

  constexpr void reset()
  {
    apply([](auto &... d) { (d.reset(), ...); }, dense);
    id_to_index_.reset();
    index_to_id_.reset();
    free_id_head_ = STUB;
  }

  constexpr void uninit()
  {
    apply([](auto &... d) { (d.uninit(), ...); }, dense);
    id_to_index_.uninit();
    index_to_id_.uninit();
  }

  constexpr bool is_valid_id(Id id) const
  {
    if (id >= id_to_index_.size())
    {
      return false;
    }

    return (id_to_index_[id] & RELEASED_MASK) == 0;
  }

  constexpr bool is_valid_index(Id index) const
  {
    return index < size();
  }

  constexpr auto operator[](Id id) const
  {
    CHECK(is_valid_id(id), "Invalid ID: {}", id);
    auto const index = id_to_index_[id];
    return apply(
      [index](auto &... dense) {
        return Tuple<decltype(dense[index])...>{dense[index]...};
      },
      dense);
  }

  constexpr Id to_index(Id id) const
  {
    CHECK(is_valid_id(id), "Invalid ID: {}", id);
    return id_to_index_[id];
  }

  constexpr Result<Id, Void> try_to_index(Id id) const
  {
    if (!is_valid_id(id)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{id_to_index_[id]};
  }

  constexpr Id to_id(Id index) const
  {
    CHECK(is_valid_index(index), "Invalid index: {}", index);
    return index_to_id_[index];
  }

  constexpr Result<Id, Void> try_to_id(Id index) const
  {
    if (!is_valid_index(index)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{to_id(index)};
  }

  constexpr void erase(Id id)
  {
    CHECK(is_valid_id(id), "Invalid ID: {}", id);
    auto const index = id_to_index_[id];
    auto const last  = size() - 1;

    if (index != last)
    {
      apply([&](auto &... dense) { (dense.swap(index, last), ...); }, dense);
    }

    apply([](auto &... dense) { (dense.pop(), ...); }, dense);

    // swap indices of this element and the last element
    if (index != last)
    {
      auto const last_id    = index_to_id_[last];
      id_to_index_[last_id] = index;
      index_to_id_[index]   = last_id;
    }

    id_to_index_[id] = free_id_head_ | RELEASED_MASK;
    free_id_head_    = id;
    index_to_id_.pop();
  }

  constexpr Result<> try_erase(Id id)
  {
    if (!is_valid_id(id)) [[unlikely]]
    {
      return Err{};
    }
    erase(id);
    return Ok{};
  }

  constexpr Result<> reserve(Id target_capacity)
  {
    bool const err = !apply(
      [&](auto &... dense) {
        return ((id_to_index_.reserve(target_capacity) &&
                 index_to_id_.reserve(target_capacity)) &&
                ... && dense.reserve(target_capacity));
      },
      dense);

    if (err) [[unlikely]]
    {
      return Err{};
    }

    return Ok{};
  }

  constexpr Result<> reserve_extend(Id extension)
  {
    return reserve(size() + extension);
  }

  constexpr Result<> grow(Id target_capacity)
  {
    bool const err = !apply(
      [&](auto &... dense) {
        return ((id_to_index_.grow(target_capacity) &&
                 index_to_id_.grow(target_capacity)) &&
                ... && dense.grow(target_capacity));
      },
      dense);

    if (err) [[unlikely]]
    {
      return Err{};
    }

    return Ok{};
  }

  constexpr Result<> grow_extend(Id extension)
  {
    return grow(size() + extension);
  }

  /// make new id and map the unique id to the end index
  constexpr Id create_id_()
  {
    auto const index = static_cast<Id>(index_to_id_.size());
    if (free_id_head_ != STUB)
    {
      auto const id        = free_id_head_;
      auto const next_free = id_to_index_[free_id_head_];
      id_to_index_[id]     = index | RELEASED_MASK;
      free_id_head_        = next_free;
      index_to_id_.push(id).discard();
      return id;
    }
    else
    {
      auto const id = static_cast<Id>(id_to_index_.size());
      id_to_index_.push(index).discard();
      index_to_id_.push(id).discard();
      return id;
    }
  }

  template <typename... Args>
  requires (sizeof...(Args) == sizeof...(V))
  constexpr Result<Id, Void> push(Args &&... args)
  {
    // grow here so we can handle all memory allocation
    // failures at once. the proceeding unwrap calls will not fail
    if (!grow(size() + 1)) [[unlikely]]
    {
      return Err{};
    }

    auto const id = create_id_();

    Tuple<Args &&...> arg_refs{static_cast<Args &&>(args)...};

    index_apply<sizeof...(V)>([&]<Id... I>() {
      (dense.template get<I>()
         .push(
           static_cast<index_pack<I, Args &&...>>(arg_refs.template get<I>()))
         .discard(),
       ...);
    });

    return Ok{id};
  }
};

template <typename T>
struct IsTriviallyRelocatable<Vec<T>>
{
  static constexpr bool value = true;
};

template <typename T>
struct IsTriviallyRelocatable<PinVec<T>>
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

inline void format(fmt::Sink sink, fmt::Spec spec, Vec<char> const & str)
{
  format(sink, spec, str.view());
}

inline void format(fmt::Sink sink, fmt::Spec spec, PinVec<char> const & str)
{
  format(sink, spec, str.view());
}

template <usize C>
void format(fmt::Sink sink, fmt::Spec spec, InplaceVec<char, C> const & str)
{
  format(sink, spec, str.view());
}

}    // namespace ash
