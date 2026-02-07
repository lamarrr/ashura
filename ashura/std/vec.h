/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/growth.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T, usize MinAlignment = SIMD_ALIGNMENT>
requires (NonConst<T>)
struct [[nodiscard]] Vec
{
    using Type = T;
    using Repr = T;
    using Iter = SpanIter<T>;
    using View = Span<T>;

    static constexpr usize ALIGNMENT = max(MinAlignment, alignof(Type));

    Type *    storage_;
    usize     size_;
    usize     capacity_;
    Allocator allocator_;

    explicit constexpr Vec(Allocator allocator) :
      storage_{nullptr},
      size_{0},
      capacity_{0},
      allocator_{allocator}
    {
    }

    constexpr Vec(Allocator allocator, Type * storage, usize capacity, usize size) :
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
        other.allocator_ = noop_allocator;
    }

    constexpr Vec & operator=(Vec && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        this->~Vec();
        new (this) Vec{static_cast<Vec &&>(other)};
        return *this;
    }

    constexpr ~Vec()
    {
        uninit();
    }

    constexpr View leak()
    {
        auto old  = view();
        storage_  = nullptr;
        size_     = 0;
        capacity_ = 0;
        return old;
    }

    static constexpr Result<Vec> make(usize capacity, Allocator allocator)
    {
        Vec out{allocator};

        if (!out.reserve(capacity))
        {
            return Err{};
        }

        return Ok{static_cast<Vec &&>(out)};
    }

    constexpr Result<Vec> clone(Allocator allocator) const
    {
        Vec out{allocator};

        if (!out.append(*this))
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
        return assume_aligned_to<ALIGNMENT>(storage_);
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

    constexpr Allocator allocator() const
    {
        return allocator_;
    }

    constexpr Layout storage_layout() const
    {
        return layout_of<Type>.array(capacity()).align_to(ALIGNMENT);
    }

    constexpr auto begin() const
    {
        return Iter{.iter_ = data(), .end_ = data() + size()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr Type & first() const
    {
        return get(0);
    }

    constexpr Type & unsafe_get(usize index) const
    {
        return data()[index];
    }

    constexpr Type & last() const
    {
        ASH_BOUNDS_CHECK(0uz, size_);
        return unsafe_get(size_ - 1);
    }

    constexpr Type & operator[](usize index) const
    {
        return get(index);
    }

    constexpr Type & get(usize index) const
    {
        ASH_BOUNDS_CHECK(index, size_);
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
        obj::destruct(view());
        size_ = 0;
    }

    constexpr void uninit()
    {
        obj::destruct(view());
        allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
    }

    constexpr void reset()
    {
        uninit();
        storage_  = nullptr;
        size_     = 0;
        capacity_ = 0;
    }

    constexpr Result<> reserve(usize target_capacity)
    {
        if (capacity_ >= target_capacity) [[likely]]
        {
            return Ok{};
        }

        if constexpr (TriviallyRelocatable<Type>)
        {
            if (!allocator_->pnrealloc(ALIGNMENT, capacity_, target_capacity, storage_))
              [[unlikely]]
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

            obj::relocate_nonoverlapping(view(), new_storage);
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

    constexpr Result<> shrink_to_(usize max_capacity)
    {
        if (capacity_ <= max_capacity)
        {
            return Ok{};
        }

        if constexpr (TriviallyRelocatable<Type>)
        {
            if (!allocator_->pnrealloc(ALIGNMENT, capacity_, max_capacity, storage_))
              [[unlikely]]
            {
                return Err{};
            }
        }
        else
        {
            Type * new_storage;
            if (!allocator_->pnalloc(ALIGNMENT, max_capacity, new_storage)) [[unlikely]]
            {
                return Err{};
            }

            obj::relocate_nonoverlapping(view(), new_storage);
            allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
            storage_ = new_storage;
        }

        capacity_ = max_capacity;
        return Ok{};
    }

    constexpr Result<> shrink()
    {
        return shrink_to_(HalfGrowth::grow(size_));
    }

    constexpr Result<> shrink_clear()
    {
        obj::destruct(view());
        auto old_size = size_;
        size_         = 0;
        return shrink_to_(old_size);
    }

    constexpr Result<> fit()
    {
        return shrink_to_(size_);
    }

    constexpr Result<> grow(usize target_capacity)
    {
        if (capacity_ >= target_capacity) [[likely]]
        {
            return Ok{};
        }

        return reserve(max(target_capacity, HalfGrowth::grow(capacity_)));
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
        if constexpr (TriviallyMoveConstructible<Type> && TriviallyDestructible<Type>)
        {
            mem::move(Span{data() + slice.end(), size_ - slice.end()},
                      data() + slice.begin());
        }
        else
        {
            for (usize dst = slice.begin(), src = slice.end(); src < size_;
                 dst++, src++)
            {
                data()[dst] = std::move(data()[src]);
            }

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

        if constexpr (TriviallyMoveConstructible<Type> && TriviallyDestructible<Type>)
        {
            // potentially overlapping
            mem::move(Span{data() + first, size_ - first}, data() + first + distance);
        }
        else
        {
            auto const tail_first = max(first, min(size_, distance) - size_);

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
        auto const pos = size_;

        if (!extend_uninit(extension)) [[unlikely]]
        {
            return Err{};
        }

        obj::default_construct(Span{data() + pos, extension});

        return Ok{};
    }

    constexpr Result<> append(Span<Type const> span)
    {
        auto const pos = size_;

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

    constexpr Result<> append_move(Span<Type> span)
    {
        auto const pos = size_;

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

    template <typename... Args>
    constexpr Result<> push(WithinCapacity, Args &&... args)
    {
        if (size() >= capacity()) [[unlikely]]
        {
            return Err{};
        }

        return push(static_cast<Args &&>(args)...);
    }

    template <typename... Args>
    constexpr Result<> insert(WithinCapacity, usize pos, Args &&... args)
    {
        if (size() >= capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert(pos, static_cast<Args &&>(args)...);
    }

    constexpr Result<> insert_span(WithinCapacity, usize pos, Span<Type const> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert_span(pos, span);
    }

    constexpr Result<> insert_span_move(WithinCapacity, usize pos, Span<Type> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert_span_move(pos, span);
    }

    constexpr Result<> extend(WithinCapacity, usize extension)
    {
        if (size() + extension > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return extend(extension);
    }

    constexpr Result<> append(WithinCapacity, Span<Type const> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return append(span);
    }

    constexpr Result<> append_move(WithinCapacity, Span<Type> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return append_move(span);
    }

    constexpr Result<> resize(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize(new_size);
    }

    constexpr Result<> resize_uninit(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize_uninit(new_size);
    }
};

namespace vec
{

template <typename Span, usize MinAlignment = SIMD_ALIGNMENT>
constexpr auto copy(Allocator allocator, Span items)
{
    Result out = Vec<remove_const<typename Span::Type>, MinAlignment>::make(
      items.size(), allocator);

    if (!out)
    {
        return out;
    }

    out.v().append(items).unwrap();

    return out;
}

template <typename Span, usize MinAlignment = SIMD_ALIGNMENT>
requires (NonConst<typename Span::Type>)
constexpr auto move(Allocator allocator, Span items)
{
    Result out = Vec<typename Span::Type, MinAlignment>::make(items.size(), allocator);

    if (!out)
    {
        return out;
    }

    out.v().append_move(items).unwrap();

    return out;
}
}    // namespace vec

/// @brief A Vec with a small inline-reserved storage
/// @warning SmallVec does not have stable addressing
template <typename T, usize InlineCapacity, usize MinAlignment>
requires (NonConst<T> && InlineCapacity > 0)
struct [[nodiscard]] SmallVec
{
    using Type = T;
    using Repr = T;
    using Iter = SpanIter<T>;
    using View = Span<T>;

    static constexpr usize ALIGNMENT       = max(MinAlignment, alignof(Type));
    static constexpr usize INLINE_CAPACITY = InlineCapacity;

    using InlineStorage = InplaceStorage<ALIGNMENT, sizeof(T) * INLINE_CAPACITY>;

    Type *                storage_;
    usize                 size_;
    usize                 capacity_;
    Allocator             allocator_;
    mutable InlineStorage inline_;

    explicit constexpr SmallVec(Allocator allocator) :
      storage_{inline_storage()},
      size_{0},
      capacity_{inline_capacity()},
      allocator_{allocator},
      inline_{}
    {
    }

    constexpr SmallVec(SmallVec const &) = delete;

    constexpr SmallVec & operator=(SmallVec const &) = delete;

    constexpr SmallVec(SmallVec && other) :
      storage_{nullptr},
      size_{other.size_},
      capacity_{other.capacity_},
      allocator_{other.allocator_},
      inline_{}
    {
        if (other.is_inline())
        {
            obj::relocate_nonoverlapping(other.view(), inline_storage());
            storage_ = inline_storage();
        }
        else
        {
            storage_ = other.storage_;
        }

        other.storage_   = other.inline_storage();
        other.size_      = 0;
        other.capacity_  = other.inline_capacity();
        other.allocator_ = noop_allocator;
    }

    constexpr SmallVec & operator=(SmallVec && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        this->~SmallVec();
        new (this) SmallVec{static_cast<SmallVec &&>(other)};
        return *this;
    }

    constexpr ~SmallVec()
    {
        uninit();
    }

    static constexpr Result<SmallVec> make(usize capacity, Allocator allocator)
    {
        SmallVec out{allocator};

        if (!out.reserve(capacity))
        {
            return Err{};
        }

        return Ok{static_cast<SmallVec &&>(out)};
    }

    constexpr Result<SmallVec> clone(Allocator allocator) const
    {
        SmallVec out{allocator};

        if (!out.append(*this))
        {
            return Err{};
        }

        return Ok{static_cast<SmallVec &&>(out)};
    }

    constexpr Result<SmallVec> clone() const
    {
        return clone(allocator_);
    }

    constexpr bool is_empty() const
    {
        return size() == 0;
    }

    constexpr Type * data() const
    {
        return assume_aligned_to<ALIGNMENT>(storage_);
    }

    constexpr Type * inline_storage() const
    {
        return assume_aligned_to<ALIGNMENT>(reinterpret_cast<Type *>(inline_.storage_));
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

    static constexpr usize inline_capacity()
    {
        return INLINE_CAPACITY;
    }

    static constexpr usize inline_capacity_bytes()
    {
        return sizeof(Type) * inline_capacity();
    }

    constexpr usize capacity() const
    {
        return capacity_;
    }

    constexpr usize capacity_bytes() const
    {
        return sizeof(Type) * capacity();
    }

    constexpr Allocator allocator() const
    {
        return allocator_;
    }

    constexpr Layout storage_layout() const
    {
        return layout_of<Type>.array(capacity()).align_to(ALIGNMENT);
    }

    static constexpr bool can_inline(usize target_capacity)
    {
        return INLINE_CAPACITY >= target_capacity;
    }

    constexpr bool is_inline() const
    {
        return storage_ == inline_storage();
    }

    constexpr auto begin() const
    {
        return Iter{.iter_ = data(), .end_ = data() + size()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr Type & first() const
    {
        return get(0);
    }

    constexpr Type & unsafe_get(usize index) const
    {
        return data()[index];
    }

    constexpr Type & last() const
    {
        ASH_BOUNDS_CHECK(0uz, size_);
        return unsafe_get(size_ - 1);
    }

    constexpr Type & operator[](usize index) const
    {
        return get(index);
    }

    constexpr Type & get(usize index) const
    {
        ASH_BOUNDS_CHECK(index, size_);
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
        obj::destruct(view());
        size_ = 0;
    }

    constexpr void uninit()
    {
        obj::destruct(view());
        if (is_inline())
        {
            return;
        }
        allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
    }

    constexpr void reset()
    {
        uninit();
        storage_  = nullptr;
        size_     = 0;
        capacity_ = 0;
    }

    constexpr Result<> reserve(usize target_capacity)
    {
        if (capacity_ >= target_capacity)
        {
            return Ok{};
        }

        if (is_inline())
        {
            if (can_inline(target_capacity))
            {
                capacity_ = target_capacity;
                return Ok{};
            }

            Type * new_storage;
            if (!allocator_->pnalloc(ALIGNMENT, target_capacity, new_storage))
              [[unlikely]]
            {
                return Err{};
            }

            obj::relocate_nonoverlapping(view(), new_storage);
            capacity_ = target_capacity;
            storage_  = new_storage;
            return Ok{};
        }
        else
        {
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

                obj::relocate_nonoverlapping(view(), new_storage);
                allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
                storage_ = new_storage;
            }

            capacity_ = target_capacity;
            return Ok{};
        }
    }

    constexpr Result<> reserve_extend(usize extension)
    {
        return reserve(size_ + extension);
    }

    constexpr Result<> shrink_to_(usize max_capacity)
    {
        if (capacity_ <= max_capacity)
        {
            return Ok{};
        }

        if (is_inline())
        {
            return Ok{};
        }

        if constexpr (TriviallyRelocatable<Type>)
        {
            if (!allocator_->pnrealloc(ALIGNMENT, capacity_, max_capacity, storage_))
              [[unlikely]]
            {
                return Err{};
            }
        }
        else
        {
            Type * new_storage;
            if (!allocator_->pnalloc(ALIGNMENT, max_capacity, new_storage)) [[unlikely]]
            {
                return Err{};
            }

            obj::relocate_nonoverlapping(view(), new_storage);
            allocator_->pndealloc(ALIGNMENT, capacity_, storage_);
            storage_ = new_storage;
        }

        capacity_ = max_capacity;
        return Ok{};
    }

    constexpr Result<> shrink()
    {
        return shrink_to_(HalfGrowth::grow(size_));
    }

    constexpr Result<> shrink_clear()
    {
        obj::destruct(view());
        auto old_size = size_;
        size_         = 0;
        return shrink_to_(old_size);
    }

    constexpr Result<> fit()
    {
        return shrink_to_(size_);
    }

    constexpr Result<> grow(usize target_capacity)
    {
        if (capacity_ >= target_capacity)
        {
            return Ok{};
        }

        return reserve(max(target_capacity, HalfGrowth::grow(capacity_)));
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
        if constexpr (TriviallyMoveConstructible<Type> && TriviallyDestructible<Type>)
        {
            mem::move(Span{data() + slice.end(), size_ - slice.end()},
                      data() + slice.begin());
        }
        else
        {
            for (usize dst = slice.begin(), src = slice.end(); src < size_;
                 dst++, src++)
            {
                data()[dst] = std::move(data()[src]);
            }

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
            auto const tail_first = max(first, min(size_, distance) - size_);

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
        auto const pos = size_;

        if (!extend_uninit(extension)) [[unlikely]]
        {
            return Err{};
        }

        obj::default_construct(Span{data() + pos, extension});

        return Ok{};
    }

    constexpr Result<> append(Span<Type const> span)
    {
        auto const pos = size_;

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

    constexpr Result<> append_move(Span<Type> span)
    {
        auto const pos = size_;

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

    template <typename... Args>
    constexpr Result<> push(WithinCapacity, Args &&... args)
    {
        if (size() >= capacity()) [[unlikely]]
        {
            return Err{};
        }

        return push(static_cast<Args &&>(args)...);
    }

    template <typename... Args>
    constexpr Result<> insert(WithinCapacity, usize pos, Args &&... args)
    {
        if (size() >= capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert(pos, static_cast<Args &&>(args)...);
    }

    constexpr Result<> insert_span(WithinCapacity, usize pos, Span<Type const> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert_span(pos, span);
    }

    constexpr Result<> insert_span_move(WithinCapacity, usize pos, Span<Type> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return insert_span_move(pos, span);
    }

    constexpr Result<> extend(WithinCapacity, usize extension)
    {
        if (size() + extension > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return extend(extension);
    }

    constexpr Result<> append(WithinCapacity, Span<Type const> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return append(span);
    }

    constexpr Result<> append_move(WithinCapacity, Span<Type> span)
    {
        if (size() + span.size() > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return append_move(span);
    }

    constexpr Result<> resize(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize(new_size);
    }

    constexpr Result<> resize_uninit(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize_uninit(new_size);
    }
};

namespace small_vec
{

template <usize InlineCapacity, usize MinAlignment, typename Span>
constexpr auto copy(Allocator allocator, Span items)
{
    using T = remove_const<typename Span::Type>;
    Result out =
      SmallVec<T, InlineCapacity, MinAlignment>::make(items.size(), allocator);

    if (!out)
    {
        return out;
    }

    out.v().append(within_capacity, items).unwrap();

    return out;
}

template <usize InlineCapacity, usize MinAlignment, typename Span>
requires (NonConst<typename Span::Type>)
constexpr auto move(Allocator allocator, Span items)
{
    using T = typename Span::Type;

    Result out =
      SmallVec<T, InlineCapacity, MinAlignment>::make(items.size(), allocator);

    if (!out)
    {
        return out;
    }

    out.v().append_move(within_capacity, items).unwrap();

    return out;
}
}    // namespace small_vec

/// @brief InplaceVec doesn't use SIMD_ALIGNMENT as it is usually in-place and
/// compacted along with other struct members/stack variables.
template <typename T, usize Capacity, usize MinAlignment>
requires (NonConst<T>)
struct [[nodiscard]] InplaceVec
  : InplaceStorage<max(MinAlignment, alignof(T)), sizeof(T) * Capacity>
{
    using Type = T;
    using Repr = T;
    using Iter = SpanIter<T>;
    using View = Span<T>;

    static constexpr usize ALIGNMENT = max(MinAlignment, alignof(T));
    static constexpr usize CAPACITY  = Capacity;

    usize size_ = 0;

    constexpr InplaceVec() = default;

    constexpr InplaceVec(Allocator) : InplaceVec{}
    {
    }

    constexpr InplaceVec(InitList<T> list) : InplaceVec{}
    {
        append(span(list)).unwrap();
    }

    constexpr InplaceVec(InplaceVec const & other) : size_{other.size()}
    {
        obj::copy_construct(other.view(), data());
    }

    constexpr InplaceVec & operator=(InplaceVec const & other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        this->~InplaceVec();
        new (this) InplaceVec{other};
        return *this;
    }

    constexpr InplaceVec(InplaceVec && other) : size_{other.size()}
    {
        obj::relocate_nonoverlapping(other.view(), data());
        other.size_ = 0;
    }

    constexpr InplaceVec & operator=(InplaceVec && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        this->~InplaceVec();
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
        return assume_aligned_to<ALIGNMENT>(reinterpret_cast<Type *>(this->storage_));
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
        return iter_end;
    }

    constexpr Type & first() const
    {
        return get(0);
    }

    constexpr Type & unsafe_get(usize index) const
    {
        return data()[index];
    }

    constexpr Type & last() const
    {
        ASH_BOUNDS_CHECK(0uz, size_);
        return unsafe_get(size() - 1);
    }

    constexpr Type & operator[](usize index) const
    {
        return get(index);
    }

    constexpr Type & get(usize index) const
    {
        ASH_BOUNDS_CHECK(index, size_);
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
        obj::destruct(view());
        size_ = 0;
    }

    constexpr void uninit()
    {
        obj::destruct(view());
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
        if constexpr (TriviallyMoveConstructible<Type> && TriviallyDestructible<Type>)
        {
            mem::move(Span{data() + slice.end(), size_ - slice.end()},
                      data() + slice.begin());
        }
        else
        {
            for (usize dst = slice.begin(), src = slice.end(); src < size_;
                 dst++, src++)
            {
                data()[dst] = std::move(data()[src]);
            }

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
            auto const tail_first = max(first, min(size_, distance) - size_);

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
        auto const pos = size_;

        if (!extend_uninit(extension)) [[unlikely]]
        {
            return Err{};
        }

        obj::default_construct(Span{data() + pos, extension});

        return Ok{};
    }

    constexpr Result<> append(Span<Type const> span)
    {
        auto const pos = size_;

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

    constexpr Result<> append_move(Span<Type> span)
    {
        auto const pos = size_;

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

namespace inplace_vec
{

template <usize InlineCapacity, usize MinAlignment, typename Span>
constexpr auto copy(Span items)
{
    using T = remove_const<typename Span::Type>;
    using V = InplaceVec<T, InlineCapacity, MinAlignment>;
    using R = Result<V>;

    V out{};

    if (!out.append(items))
    {
        return R{Err{}};
    }

    return Ok{std::move(out)};
}

template <usize InlineCapacity, usize MinAlignment, typename Span>
requires (NonConst<typename Span::Type>)
constexpr auto move(Span items)
{
    using T = typename Span::Type;
    using V = InplaceVec<T, InlineCapacity, MinAlignment>;
    using R = Result<V>;

    V out{};

    if (!out.append_move(items))
    {
        return R{Err{}};
    }

    return Ok{std::move(out)};
}

}    // namespace inplace_vec

template <typename Vec>
struct [[nodiscard]] CoreBitVec
{
    using Type = bool;
    using Repr = typename Vec::Type;
    using Iter = BitSpanIter<Repr>;
    using View = BitSpan<Repr>;

    Vec   repr_;
    usize size_;

    explicit constexpr CoreBitVec(Allocator allocator) : repr_{allocator}, size_{0}
    {
    }

    constexpr CoreBitVec(CoreBitVec const &) = delete;

    constexpr CoreBitVec & operator=(CoreBitVec const &) = delete;

    constexpr CoreBitVec(CoreBitVec && other) :
      repr_{static_cast<Vec &&>(other.repr_)},
      size_{other.size_}
    {
        other.size_ = 0;
    }

    constexpr CoreBitVec & operator=(CoreBitVec && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        this->~CoreBitVec();
        new (this) CoreBitVec{static_cast<CoreBitVec &&>(other)};
        return *this;
    }

    constexpr ~CoreBitVec() = default;

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

    constexpr View leak()
    {
        auto old = view();
        size_    = 0;
        repr_.leak();
        return old;
    }

    static constexpr Result<CoreBitVec> make(usize capacity, Allocator allocator)
    {
        CoreBitVec out{allocator};

        if (!out.reserve(capacity))
        {
            return Err{};
        }

        return Ok{static_cast<CoreBitVec &&>(out)};
    }

    constexpr auto begin() const
    {
        return Iter{.storage_ = repr_.data(), .iter_ = 0, .end_ = size()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr usize capacity() const
    {
        return repr_.capacity() * bitsizeof<Repr>;
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

    constexpr Type & unsafe_get(usize index) const
    {
        return data()[index];
    }

    constexpr Type last() const
    {
        ASH_BOUNDS_CHECK(0uz, size_);
        return unsafe_get(size() - 1);
    }

    constexpr Type get(usize index) const
    {
        ASH_BOUNDS_CHECK(index, size_);
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
        return repr_.reserve(atom_size_for<Repr>(target_capacity));
    }

    constexpr Result<> reserve_extend(usize extension)
    {
        return reserve(size() + extension);
    }

    constexpr Result<> fit()
    {
        return repr_.fit();
    }

    constexpr Result<> shrink()
    {
        return repr_.shrink();
    }

    constexpr Result<> shrink_clear()
    {
        return repr_.shrink_clear();
    }

    constexpr Result<> grow(usize target_capacity)
    {
        return repr_.grow(atom_size_for<Repr>(target_capacity));
    }

    constexpr Result<> grow_extend(usize extension)
    {
        return grow(size() + extension);
    }

    constexpr Result<> push(bool bit)
    {
        auto const index = size_;

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
        auto const diff = repr_.size() - atom_size_for<Repr>(size_);
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
        for (usize dst = slice.begin(), src = slice.end(); src != size_; ++dst, ++src)
        {
            set(dst, get(src));
        }
        pop(slice.span);
    }

    constexpr Result<> extend_uninit(usize extension)
    {
        if (!repr_.extend_uninit(atom_size_for<Repr>(size_ + extension) -
                                 atom_size_for<Repr>(size_))) [[unlikely]]
        {
            return Err{};
        }

        size_ += extension;

        return Ok{};
    }

    constexpr Result<> extend(usize extension)
    {
        auto const pos = size_;

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

    constexpr Result<> resize(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize(new_size);
    }

    constexpr Result<> resize_uninit(WithinCapacity, usize new_size)
    {
        if (new_size > capacity()) [[unlikely]]
        {
            return Err{};
        }

        return resize_uninit(new_size);
    }
};

template <typename R, usize MinAlignment = SIMD_ALIGNMENT>
using BitVec = CoreBitVec<Vec<R, MinAlignment>>;

template <typename R, usize MinBitCapacity, usize MinAlignment>
using SmallBitVec =
  CoreBitVec<SmallVec<R, atom_size_for<R>(MinBitCapacity), MinAlignment>>;

/// @brief Sparse Vector (a.k.a Sparse Set) are used for stable ID-tagging of
/// objects in high-perf scenarious i.e. ECS, where a stable identity is needed
/// for objects and they need to be processed in batches for efficiency. They
/// have an indirect index into their elements, although they don't guarantee
/// stability of the addresses of the elements, they guarantee that the IDs
/// persist until the id is released. Unlike typical Sparse Sets, Sparse Vec's
/// elements are always contiguous without holes in them, making them suitable
/// for operations like batch-processing and branchless SIMD.
///
/// @tparam IndexVec the index Vec type, i.e. Vec<usize>
/// @tparam V dense containers for the properties, i.e. Vec<i32>, Vec<f32>
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by FREE_MASK
///
template <typename IdType, typename IndexVecType, typename... V>
requires (NonConst<IdType> && NonConst<IndexVecType> &&
          Unsigned<typename IndexVecType::Type> &&
          (sizeof(IdType) == sizeof(typename IndexVecType::Type)) &&
          (NonConst<V> && ... && true))
struct CoreSparseVec
{
    using Dense   = Tuple<V...>;
    using Index   = typename IndexVecType::Type;
    using Indices = IndexVecType;
    using Id      = IdType;

    static constexpr Index FREE_MASK = ~(NumTraits<Index>::MAX >> 1);
    static constexpr Index NONE      = NumTraits<Index>::MAX;

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
                  return ((!ZERO_SIZED) && ... && (iters != iter_end));
              },
              iters_);
        }

        constexpr bool operator==(IterEnd) const
        {
            return !this->operator!=(iter_end);
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
            return iter_end;
        }
    };

    Indices index_to_id_;

    /// @brief Map of id to index
    Indices id_to_index_;

    Dense dense;

    Index free_id_head_;

    explicit constexpr CoreSparseVec(Indices index_to_id, Indices id_to_index,
                                     Dense dense, Index free_id_head) :
      index_to_id_{static_cast<Indices &&>(index_to_id)},
      id_to_index_{static_cast<Indices &&>(id_to_index)},
      dense{static_cast<Dense &&>(dense)},
      free_id_head_{free_id_head}
    {
    }

    explicit constexpr CoreSparseVec(Allocator allocator) :
      index_to_id_{allocator},
      id_to_index_{allocator},
      dense{V{allocator}...},
      free_id_head_{NONE}
    {
    }

    constexpr CoreSparseVec(CoreSparseVec const &) = delete;

    constexpr CoreSparseVec & operator=(CoreSparseVec const &) = delete;

    constexpr CoreSparseVec(CoreSparseVec && other) :
      index_to_id_{static_cast<Indices &&>(other.index_to_id_)},
      id_to_index_{static_cast<Indices &&>(other.id_to_index_)},
      dense{static_cast<Dense &&>(other.dense)},
      free_id_head_{other.free_id_head_}
    {
        other.free_id_head_ = NONE;
    }

    constexpr CoreSparseVec & operator=(CoreSparseVec && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }
        index_to_id_        = static_cast<Indices &&>(other.index_to_id_);
        id_to_index_        = static_cast<Indices &&>(other.id_to_index_);
        dense               = static_cast<Dense &&>(other.dense);
        free_id_head_       = other.free_id_head_;
        other.free_id_head_ = NONE;
        return *this;
    }

    constexpr ~CoreSparseVec() = default;

    constexpr bool is_empty() const
    {
        return size() == 0;
    }

    constexpr Index size() const
    {
        return static_cast<Index>(index_to_id_.size());
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
        return iter_end;
    }

    constexpr auto view() const
    {
        return apply([](auto &... dense) { return View{ash::view(dense)...}; }, dense);
    }

    constexpr void clear()
    {
        apply([](auto &... d) { (d.clear(), ...); }, dense);
        id_to_index_.clear();
        index_to_id_.clear();
        free_id_head_ = NONE;
    }

    constexpr void reset()
    {
        apply([](auto &... d) { (d.reset(), ...); }, dense);
        id_to_index_.reset();
        index_to_id_.reset();
        free_id_head_ = NONE;
    }

    constexpr void uninit()
    {
        apply([](auto &... d) { (d.uninit(), ...); }, dense);
        id_to_index_.uninit();
        index_to_id_.uninit();
    }

    constexpr bool is_valid_id(Id item) const
    {
        auto const id = static_cast<Index>(item);

        if (id >= id_to_index_.size())
        {
            return false;
        }

        return (id_to_index_[id] & FREE_MASK) == 0;
    }

    constexpr bool is_valid_index(Index index) const
    {
        return index < size();
    }

    constexpr auto operator[](Id item) const
    {
        auto const id = static_cast<Index>(item);

        auto const index = id_to_index_[id];
        return apply(
          [index](auto &... dense) {
              return Tuple<decltype(dense[index])...>{dense[index]...};
          },
          dense);
    }

    constexpr auto get(Id item) const
    {
        auto const id = static_cast<Index>(item);

        return this->operator[](id);
    }

    template <typename... Args>
    requires (sizeof...(Args) == sizeof...(V))
    constexpr void set(Id item, Args &&... args) const
    {
        auto const        id    = static_cast<Index>(item);
        auto              index = id_to_index_[id];
        Tuple<Args &&...> arg_refs{static_cast<Args &&>(args)...};

        index_apply<sizeof...(V)>([&]<Index... I>() {
            (dense.template get<I>().set(index, static_cast<index_pack<I, Args &&...>>(
                                                  arg_refs.template get<I>())),
             ...);
        });
    }

    constexpr Index to_index(Id item) const
    {
        auto const id = static_cast<Index>(item);
        return id_to_index_[id];
    }

    constexpr Result<Index, Void> try_to_index(Id item) const
    {
        auto const id = static_cast<Index>(item);

        if (!is_valid_id(id)) [[unlikely]]
        {
            return Err{};
        }

        return Ok{id_to_index_[id]};
    }

    constexpr Id to_id(Index index) const
    {
        return static_cast<Id>(index_to_id_[index]);
    }

    constexpr Result<Id, Void> try_to_id(Index index) const
    {
        if (!is_valid_index(index)) [[unlikely]]
        {
            return Err{};
        }

        return Ok{to_id(index)};
    }

    constexpr void erase(Id item)
    {
        auto const id    = static_cast<Index>(item);
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

        id_to_index_[id] = free_id_head_ | FREE_MASK;
        free_id_head_    = id;
        index_to_id_.pop();
    }

    constexpr Result<> try_erase(Id item)
    {
        auto const id = static_cast<Index>(item);

        if (!is_valid_id(id)) [[unlikely]]
        {
            return Err{};
        }
        erase(id);
        return Ok{};
    }

    constexpr Result<> reserve(Index target_capacity)
    {
        auto const err = !apply(
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

    constexpr Result<> reserve_extend(Index extension)
    {
        return reserve(size() + extension);
    }

    constexpr Result<> grow(Index target_capacity)
    {
        auto const err = !apply(
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

    constexpr Result<> grow_extend(Index extension)
    {
        return grow(size() + extension);
    }

    /// make new id and map the unique id to the end index
    constexpr Index create_id_()
    {
        auto const index = static_cast<Index>(index_to_id_.size());
        if (free_id_head_ != NONE)
        {
            auto const id        = free_id_head_;
            auto const next_free = id_to_index_[free_id_head_];
            id_to_index_[id]     = index | FREE_MASK;
            free_id_head_ = (next_free == NONE) ? NONE : (next_free & ~FREE_MASK);
            index_to_id_.push(id).discard();
            return id;
        }
        else
        {
            auto const id = static_cast<Index>(id_to_index_.size());
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

        index_apply<sizeof...(V)>([&]<Index... I>() {
            (dense.template get<I>()
               .push(static_cast<index_pack<I, Args &&...>>(arg_refs.template get<I>()))
               .discard(),
             ...);
        });

        return Ok{static_cast<Id>(id)};
    }
};

template <typename Id, typename... T>
using SparseVec =
  CoreSparseVec<Id, Vec<UnderlyingType<Id>, SIMD_ALIGNMENT>, Vec<T, SIMD_ALIGNMENT>...>;

template <typename Id, usize InlineCapacity, usize MinAlignment, typename... T>
using SmallSparseVec =
  CoreSparseVec<Id, SmallVec<UnderlyingType<Id>, InlineCapacity, MinAlignment>,
                SmallVec<T, InlineCapacity, MinAlignment>...>;

template <typename T, usize MinAlignment>
struct IsTriviallyRelocatable<Vec<T, MinAlignment>>
{
    static constexpr bool value = true;
};

template <typename V>
struct IsTriviallyRelocatable<CoreBitVec<V>>
{
    static constexpr bool value = TriviallyRelocatable<V>;
};

template <typename T, usize C, usize MinAlignment>
struct IsTriviallyRelocatable<InplaceVec<T, C, MinAlignment>>
{
    static constexpr bool value = TriviallyRelocatable<T>;
};

template <usize MinAlignment>
inline void format(fmt::Sink sink, fmt::Spec spec, Vec<char, MinAlignment> const & str)
{
    format(sink, spec, str.view());
}

template <usize InlineCapacity, usize MinAlignment>
void format(fmt::Sink sink, fmt::Spec spec,
            InplaceVec<char, InlineCapacity, MinAlignment> const & str)
{
    format(sink, spec, str.view());
}

using StrVec   = Vec<char, SIMD_ALIGNMENT>;
using StrVec8  = Vec<c8, SIMD_ALIGNMENT>;
using StrVec16 = Vec<c16, SIMD_ALIGNMENT>;
using StrVec32 = Vec<c32, SIMD_ALIGNMENT>;

}    // namespace ash
