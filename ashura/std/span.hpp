/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.hpp"

namespace ash
{

template <typename U, typename T>
concept SpanCompatible = Convertible<U (*)[], T (*)[]>;

template <typename Container>
using ContainerDataType =
  std::remove_pointer_t<decltype(data(std::declval<Container &>()))>;

template <typename Container>
concept SpanContainer = requires (Container c) {
    { data(c) };
    { size(c) };
};

template <typename Container, typename T>
concept SpanCompatibleContainer =
  SpanContainer<Container> && SpanCompatible<ContainerDataType<Container>, T>;

template <typename T>
struct [[nodiscard]] SpanIter
{
    using Type = T;
    using Ref  = T &;

    T *       iter_ = nullptr;
    T const * end_  = nullptr;

    constexpr SpanIter & operator++()
    {
        ++iter_;
        return *this;
    }

    constexpr T & operator*() const
    {
        return *iter_;
    }

    constexpr bool operator!=(IterEnd) const
    {
        return iter_ != end_;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
    }

    constexpr usize size() const
    {
        return static_cast<usize>(end_ - iter_);
    }
};

template <typename T>
struct [[nodiscard]] RevSpanIter
{
    using Type = T;
    using Ref  = T &;

    T *       iter_  = nullptr;
    T const * begin_ = nullptr;

    constexpr RevSpanIter & operator++()
    {
        --iter_;
        return *this;
    }

    constexpr T & operator*() const
    {
        return *(iter_ - 1);
    }

    constexpr bool operator!=(IterEnd) const
    {
        return iter_ != begin_;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
    }

    constexpr usize size() const
    {
        return static_cast<usize>(iter_ - begin_);
    }
};

template <typename T, usize N>
constexpr SpanIter<T> begin(T (&a)[N])
{
    return SpanIter<T>{.iter_ = a, .end_ = a + N};
}

template <typename T>
struct [[nodiscard]] Span
{
    using Type    = T;
    using Repr    = T;
    using Iter    = SpanIter<T>;
    using RevIter = RevSpanIter<T>;
    using Rev     = IterView<RevIter>;
    using View    = Span<T>;

    T *   data_ = nullptr;
    usize size_ = 0;

    constexpr Span() = default;

    constexpr Span(T * data, usize size) : data_{data}, size_{size}
    {
    }

    constexpr Span(T * begin, T const * end) :
      data_{begin},
      size_{static_cast<usize>(end - begin)}
    {
    }

    constexpr Span(Iter iter) : Span{iter.iter_, iter.end_}
    {
    }

    template <usize N>
    constexpr Span(T (&data)[N]) : data_{data}, size_{N}
    {
    }

    template <SpanCompatibleContainer<T> C>
    constexpr Span(C & cont) : data_{ash::data(cont)}, size_{ash::size(cont)}
    {
    }

    template <SpanCompatible<T> U>
    constexpr Span(Span<U> span) : data_{span.data_}, size_{span.size_}
    {
    }

    constexpr Span(Span const &) = default;

    constexpr Span(Span &&) = default;

    constexpr Span & operator=(Span const &) = default;

    constexpr Span & operator=(Span &&) = default;

    constexpr ~Span() = default;

    constexpr T * data() const
    {
        return data_;
    }

    constexpr usize size() const
    {
        return size_;
    }

    constexpr usize size_bytes() const
    {
        return sizeof(T) * size();
    }

    constexpr bool is_empty() const
    {
        return size() == 0;
    }

    constexpr auto begin() const
    {
        return Iter{.iter_ = pbegin(), .end_ = pend()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr auto rev() const
    {
        return Rev{
          .iter_ = RevIter{.iter_ = pend(), .begin_ = pbegin()}
        };
    }

    constexpr T * pbegin() const
    {
        return data();
    }

    constexpr T * pend() const
    {
        return data() + size();
    }

    constexpr T & first() const
    {
        return get(0);
    }

    // TODO: structure all index-based getters and setters to use range checks
    constexpr T & last() const
    {
        return get(size() - 1);
    }

    constexpr T & unsafe_get(usize index) const
    {
        return data()[index];
    }

    constexpr T & operator[](usize index) const
    {
        return data()[index];
    }

    constexpr T & get(usize index) const
    {
        return data()[index];
    }

    template <typename... Args>
    constexpr void set(usize index, Args &&... args) const requires (NonConst<T>)
    {
        data()[index] = T{static_cast<Args &&>(args)...};
    }

    constexpr auto as_const() const
    {
        return Span<T const>{data(), size()};
    }

    constexpr auto as_u8() const requires (NonConst<T>)
    {
        return Span<u8>{reinterpret_cast<u8 *>(data()), size_bytes()};
    }

    constexpr auto as_u8() const requires (Const<T>)
    {
        return Span<u8 const>{reinterpret_cast<u8 const *>(data()), size_bytes()};
    }

    constexpr auto as_char() const requires (NonConst<T>)
    {
        return Span<char>{reinterpret_cast<char *>(data()), size_bytes()};
    }

    constexpr auto as_char() const requires (Const<T>)
    {
        return Span<char const>{reinterpret_cast<char const *>(data()), size_bytes()};
    }

    constexpr auto as_c8() const requires (NonConst<T>)
    {
        return Span<c8>{reinterpret_cast<c8 *>(data()), size_bytes()};
    }

    constexpr auto as_c8() const requires (Const<T>)
    {
        return Span<c8 const>{reinterpret_cast<c8 const *>(data()), size_bytes()};
    }

    constexpr Slice as_slice_of(Span<T const> parent) const
    {
        return Slice{static_cast<usize>(data() - parent.data()), size()};
    }

    constexpr Span slice(Slice s) const
    {
        s = s(size());
        return Span{data() + s.offset, s.span};
    }

    constexpr Span slice(Slice16 s) const
    {
        return slice(s.as_usize());
    }

    constexpr Span slice(Slice32 s) const
    {
        return slice(s.as_usize());
    }

    constexpr Span slice(usize offset, usize span) const
    {
        return slice(Slice{offset, span});
    }

    constexpr Span slice(usize offset) const
    {
        return slice(offset, USIZE_MAX);
    }

    template <typename U>
    Span<U> reinterpret() const
    {
        return Span<U>{reinterpret_cast<U *>(data()), size_bytes() / sizeof(U)};
    }

    constexpr View view() const
    {
        return *this;
    }
};

template <typename T, usize N>
Span(T (&)[N]) -> Span<T>;

template <typename T>
Span(T *, T *) -> Span<T>;

template <typename T>
Span(T *, T const *) -> Span<T>;

template <typename T>
Span(T *, usize) -> Span<T>;

template <typename T>
Span(SpanIter<T>, IterEnd) -> Span<T>;

template <SpanContainer C>
Span(C & container) -> Span<std::remove_pointer_t<decltype(data(container))>>;

template <typename T>
constexpr Span<T const> span(InitList<T> list)
{
    return Span<T const>{list.begin(), list.size()};
}

template <typename T, usize N>
constexpr Span<T> span(T (&array)[N])
{
    return Span<T>{array, N};
}

template <SpanContainer C>
constexpr auto span(C & c)
{
    return Span{data(c), size(c)};
}

template <typename T, usize N>
constexpr Span<T> view(T (&array)[N])
{
    return span(array);
}

template <typename R>
constexpr auto view(R & range) -> decltype(range.view())
{
    return range.view();
}

template <typename T>
constexpr Span<u8 const> as_u8_span(T const & obj)
{
    return Span<T const>{&obj, 1}.as_u8();
}

template <typename T>
constexpr Span<u8> as_u8_span(T & obj)
{
    return Span<T>{&obj, 1}.as_u8();
}

typedef Span<char const> Str;
typedef Span<char>       MutStr;

typedef Span<c16 const> Str16;
typedef Span<c16>       MutStr16;

typedef Span<c8 const> Str8;
typedef Span<c8>       MutStr8;

typedef Span<c32 const> Str32;
typedef Span<c32>       MutStr32;

inline namespace str_literal
{

constexpr Str operator""_s(char const * lit, usize n)
{
    return Str{lit, n};
}

constexpr Str8 operator""_s(c8 const * lit, usize n)
{
    return Str8{lit, n};
}

constexpr Str16 operator""_s(c16 const * lit, usize n)
{
    return Str16{lit, n};
}

constexpr Str32 operator""_s(c32 const * lit, usize n)
{
    return Str32{lit, n};
}

}    // namespace str_literal

template <typename R>
struct BitSpanIter
{
    R *   storage_ = nullptr;
    usize iter_    = 0;
    usize end_     = 0;

    constexpr bool operator*() const
    {
        return impl::get_bit(storage_, iter_);
    }

    constexpr BitSpanIter & operator++()
    {
        ++iter_;
        return *this;
    }

    constexpr bool operator!=(IterEnd) const
    {
        return iter_ != end_;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
    }
};

template <typename R>
struct BitSpan
{
    using Type = bool;
    using Repr = R;
    using Iter = BitSpanIter<R>;

    R *   storage_ = nullptr;
    usize size_    = 0;

    constexpr BitSpan() = default;

    constexpr BitSpan(R * storage, usize size) : storage_{storage}, size_{size}
    {
    }

    constexpr BitSpan(Span<R> storage, usize size) : BitSpan{storage.data(), size}
    {
    }

    constexpr BitSpan(Span<R> storage) :
      BitSpan{storage.data(), storage.size() * bitsizeof<R>}
    {
    }

    constexpr BitSpan(BitSpan const &) = default;

    constexpr BitSpan(BitSpan &&) = default;

    constexpr BitSpan & operator=(BitSpan const &) = default;

    constexpr BitSpan & operator=(BitSpan &&) = default;

    constexpr ~BitSpan() = default;

    constexpr usize size() const
    {
        return size_;
    }

    constexpr usize atom_size() const
    {
        return atom_size_for<Repr>(size());
    }

    constexpr bool is_empty() const
    {
        return size() == 0;
    }

    constexpr auto begin() const
    {
        return Iter{.storage_ = storage_, .iter_ = 0, .end_ = size()};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr Span<R> repr() const
    {
        return Span<R>{storage_, atom_size()};
    }

    constexpr bool unsafe_get(usize index) const
    {
        return impl::get_bit(storage_, index);
    }

    constexpr bool operator[](usize index) const
    {
        return impl::get_bit(storage_, index);
    }

    constexpr bool get(usize index) const
    {
        return impl::get_bit(storage_, index);
    }

    constexpr void set(usize index, bool value) const requires (NonConst<R>)
    {
        impl::assign_bit(storage_, index, static_cast<Repr>(value));
    }

    constexpr bool get_bit(usize index) const
    {
        return impl::get_bit(storage_, index);
    }

    constexpr void set_bit(usize index) const requires (NonConst<R>)
    {
        impl::set_bit(storage_, index);
    }

    constexpr void clear_bit(usize index) const requires (NonConst<R>)
    {
        impl::clear_bit(storage_, index);
    }

    constexpr void flip_bit(usize index) const requires (NonConst<R>)
    {
        impl::flip_bit(storage_, index);
    }

    constexpr void clear_all_bits() const requires (NonConst<R>)
    {
        for (auto i = 0uz; i < atom_size(); ++i)
        {
            storage_[i] = 0;
        }
    }

    constexpr void set_all_bits() const requires (NonConst<R>)
    {
        for (auto i = 0uz; i < atom_size(); ++i)
        {
            storage_[i] = NumTraits<R>::MAX;
        }
    }

    constexpr usize find_set_bit()
    {
        return impl::find_set_bit(storage_, atom_size());
    }

    constexpr usize find_clear_bit()
    {
        return impl::find_clear_bit(storage_, atom_size());
    }

    constexpr BitSpan<R const> as_const() const
    {
        return BitSpan<R const>{storage_, size_};
    }

    constexpr operator BitSpan<R const>() const
    {
        return as_const();
    }
};

template <typename T, usize N>
BitSpan(T (&)[N]) -> BitSpan<T>;

template <typename T, usize N>
BitSpan(T (&)[N], usize) -> BitSpan<T>;

template <typename T>
BitSpan(T *, usize) -> BitSpan<T>;

template <SpanContainer C>
BitSpan(C & container) -> BitSpan<std::remove_pointer_t<decltype(data(container))>>;

template <SpanContainer C>
BitSpan(C & container, usize)
  -> BitSpan<std::remove_pointer_t<decltype(data(container))>>;

template <typename Char>
constexpr Span<Char> cstr(Char * c_str)
{
    return {c_str, cstr_len(c_str)};
}

template <typename T>
struct Consume<Span<T>>
{
    static constexpr auto consume(Span<T> & v)
    {
        auto out = v;
        v        = Span<T>{};
        return out;
    }
};

template <typename T>
struct Consume<BitSpan<T>>
{
    static constexpr auto consume(BitSpan<T> & v)
    {
        auto out = v;
        v        = BitSpan<T>{};
        return out;
    }
};

}    // namespace ash
