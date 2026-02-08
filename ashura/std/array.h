/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/span.h"

namespace ash
{

template <typename T, usize N>
struct Array
{
    using Type      = T;
    using View      = Span<T>;
    using ConstView = Span<T const>;
    using Iter      = SpanIter<T>;
    using ConstIter = SpanIter<T const>;

    static constexpr usize SIZE = N;

    T data_[SIZE]{};

    static constexpr bool is_empty()
    {
        return false;
    }

    constexpr T * data()
    {
        return data_;
    }

    constexpr T const * data() const
    {
        return data_;
    }

    static constexpr usize size()
    {
        return SIZE;
    }

    static constexpr usize capacity()
    {
        return size();
    }

    static constexpr usize size_bytes()
    {
        return sizeof(T) * size();
    }

    constexpr T const * pbegin() const
    {
        return data();
    }

    constexpr T const * pend() const
    {
        return data() + size();
    }

    constexpr T * pbegin()
    {
        return data();
    }

    constexpr T * pend()
    {
        return data() + size();
    }

    constexpr auto begin()
    {
        return Iter{.iter_ = pbegin(), .end_ = pend()};
    }

    constexpr auto begin() const
    {
        return ConstIter{.iter_ = pbegin(), .end_ = pend()};
    }

    constexpr auto end()
    {
        return iter_end;
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr T & first()
    {
        return get(0);
    }

    constexpr T const & first() const
    {
        return get(0);
    }

    constexpr T & last()
    {
        return get(size() - 1);
    }

    constexpr T const & last() const
    {
        return get(size() - 1);
    }

    constexpr T & get(usize index)
    {
        return data()[index];
    }

    constexpr T const & get(usize index) const
    {
        return data()[index];
    }

    template <typename... Args>
    constexpr void set(usize index, Args &&... args)
    {
        data()[index] = T{static_cast<Args &&>(args)...};
    }

    constexpr T & operator[](usize index)
    {
        return data()[index];
    }

    constexpr T const & operator[](usize index) const
    {
        return data()[index];
    }

    constexpr operator T *()
    {
        return data();
    }

    constexpr operator T const *() const
    {
        return data();
    }

    constexpr auto view() const
    {
        return ConstView{data(), size()};
    }

    constexpr auto view()
    {
        return View{data(), size()};
    }
};

// TODO: bounds checks
template <typename T>
struct Array<T, 0>
{
    using Type      = T;
    using View      = Span<T>;
    using ConstView = Span<T const>;
    using Iter      = SpanIter<T>;
    using ConstIter = SpanIter<T const>;

    static constexpr usize SIZE = 0;

    static constexpr bool is_empty()
    {
        return true;
    }

    constexpr T * data()
    {
        return nullptr;
    }

    constexpr T const * data() const
    {
        return nullptr;
    }

    static constexpr usize size()
    {
        return SIZE;
    }

    static constexpr usize capacity()
    {
        return size();
    }

    static constexpr usize size_bytes()
    {
        return sizeof(T) * size();
    }

    constexpr T const * pbegin() const
    {
        return data();
    }

    constexpr T const * pend() const
    {
        return data() + size();
    }

    constexpr T * pbegin()
    {
        return data();
    }

    constexpr T * pend()
    {
        return data() + size();
    }

    constexpr auto begin()
    {
        return Iter{.iter_ = pbegin(), .end_ = pend()};
    }

    constexpr auto begin() const
    {
        return ConstIter{.iter_ = pbegin(), .end_ = pend()};
    }

    constexpr auto end()
    {
        return iter_end;
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr T & first() requires (SIZE > 1)
    {
        return get(0);
    }

    constexpr T const & first() const requires (SIZE > 1)
    {
        return get(0);
    }

    constexpr T & last() requires (SIZE > 1)
    {
        return get(size() - 1);
    }

    constexpr T const & last() const requires (SIZE > 1)
    {
        return get(size() - 1);
    }

    constexpr T & get(usize index) requires (SIZE > 1)
    {
        return data()[index];
    }

    constexpr T const & get(usize index) const requires (SIZE > 1)
    {
        return data()[index];
    }

    template <typename... Args>
    constexpr void set(usize index, Args &&... args) requires (SIZE > 1)
    {
        data()[index] = T{static_cast<Args &&>(args)...};
    }

    constexpr T & operator[](usize index) requires (SIZE > 1)
    {
        return data()[index];
    }

    constexpr T const & operator[](usize index) const
    {
        return data()[index];
    }

    constexpr operator T *()
    {
        return data();
    }

    constexpr operator T const *() const
    {
        return data();
    }

    constexpr auto view() const
    {
        return ConstView{data(), size()};
    }

    constexpr auto view()
    {
        return View{data(), size()};
    }
};

template <typename T, typename... U>
Array(T, U...) -> Array<T, 1 + sizeof...(U)>;

template <typename T, usize N>
struct IsTriviallyRelocatable<Array<T, N>>
{
    static constexpr bool value = TriviallyRelocatable<T>;
};

template <typename R, usize N>
struct Bits
{
    using Type      = bool;
    using Repr      = R;
    using View      = BitSpan<Repr>;
    using ConstView = BitSpan<Repr const>;
    using Iter      = BitSpanIter<R>;
    using ConstIter = BitSpanIter<R const>;

    static constexpr usize SIZE = N;

    Array<R, atom_size_for<R>(N)> storage_;

    constexpr Bits()                         = default;
    constexpr Bits(Bits const &)             = default;
    constexpr Bits(Bits &&)                  = default;
    constexpr Bits & operator=(Bits const &) = default;
    constexpr Bits & operator=(Bits &&)      = default;
    constexpr ~Bits()                        = default;

    constexpr auto begin() const
    {
        return Iter{.storage_ = storage_.data(), .iter_ = 0, .end_ = size()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr usize size() const
    {
        return SIZE;
    }

    constexpr bool is_empty() const
    {
        return size() == 0;
    }

    constexpr bool operator[](usize index) const
    {
        return get(index);
    }

    constexpr bool get(usize index) const
    {
        return view().get(index);
    }

    constexpr bool first() const
    {
        return get(0);
    }

    constexpr bool last() const
    {
        return get(size() - 1);
    }

    constexpr void set(usize index, bool value)
    {
        view().set(index, value);
    }

    constexpr bool get_bit(usize index) const
    {
        return get(index);
    }

    constexpr void set_bit(usize index)
    {
        view().set_bit(index);
    }

    constexpr void clear_bit(usize index)
    {
        view().clear_bit(index);
    }

    constexpr void flip_bit(usize index)
    {
        view().flip_bit(index);
    }

    constexpr void swap(usize a, usize b)
    {
        bool av = get(a);
        bool bv = get(b);
        set(a, bv);
        set(b, av);
    }

    constexpr auto view() const
    {
        return ConstView{storage_.view(), size()};
    }

    constexpr auto view()
    {
        return View{storage_.view(), size()};
    }
};

}    // namespace ash
