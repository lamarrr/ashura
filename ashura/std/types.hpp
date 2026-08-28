/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.hpp"
#include "ashura/std/traits.hpp"
#include <bit>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

#define ASH_CONCAT_IMPL(a, b) a##b
#define ASH_CONCAT(a, b)      ASH_CONCAT_IMPL(a, b)

#define ASH_UNIQUE_NAME(prefix) ASH_CONCAT(prefix, __COUNTER__)

namespace ash
{

typedef char8_t  c8;
typedef char16_t c16;
typedef char32_t c32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#if ASH_CFG(COMPILER, GCC) || ASH_CFG(COMPILER, CLANG)
typedef __uint128_t u128;
#else
struct alignas(16) u128
{
    u64 repr_[2];
};
#endif

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#if ASH_CFG(COMPILER, GCC) || ASH_CFG(COMPILER, CLANG)
typedef __int128_t i128;
#else

struct alignas(16) i128
{
    u64 repr_[2];
};
#endif

typedef size_t    usize;
typedef ptrdiff_t isize;

typedef uintptr_t uptr;
typedef intptr_t  iptr;

struct f8
{
    u8 repr_;

    constexpr f8(float);
    constexpr f8(f8 const &)             = default;
    constexpr f8(f8 &&)                  = default;
    constexpr f8 & operator=(f8 const &) = default;
    constexpr f8 & operator=(f8 &&)      = default;
    constexpr ~f8()                      = default;
};

#if ASH_CFG(COMPILER, GCC) || ASH_CFG(COMPILER, CLANG)
typedef _Float16 f16;
#else
struct f16
{
    u16 repr_;

    constexpr f16(float);
    constexpr f16(f16 const &)             = default;
    constexpr f16(f16 &&)                  = default;
    constexpr f16 & operator=(f16 const &) = default;
    constexpr f16 & operator=(f16 &&)      = default;
    constexpr ~f16()                       = default;
};
#endif

typedef float  f32;
typedef double f64;

/// regular void
struct Void
{
};

/// @brief In-place type constructor flag. Intended for functions that take
/// generic types and want to overload with a second type that constructs the
/// type using the provided arguments.
struct Inplace
{
};

inline constexpr Inplace inplace{};

struct Uninit
{
};

inline constexpr Uninit uninit{};

struct FromParts
{
};

inline constexpr FromParts from_parts{};

struct WithinCapacity
{
};

inline constexpr WithinCapacity within_capacity{};

template <typename T>
inline constexpr usize bitsizeof = sizeof(T) * 8;

inline constexpr u8 U8_MIN = 0;
inline constexpr u8 U8_MAX = 0xFF;

inline constexpr i8 I8_MIN = -0x7F - 1;
inline constexpr i8 I8_MAX = 0x7F;

inline constexpr u16 U16_MIN = 0;
inline constexpr u16 U16_MAX = 0xFFFF;

inline constexpr i16 I16_MIN = -0x7FFF - 1;
inline constexpr i16 I16_MAX = 0x7FFF;

inline constexpr u32 U32_MIN = 0;
inline constexpr u32 U32_MAX = 0xFFFF'FFFFU;

inline constexpr i32 I32_MIN = -0x7FFF'FFFF - 1;
inline constexpr i32 I32_MAX = 0x7FFF'FFFF;

inline constexpr u64 U64_MIN = 0;
inline constexpr u64 U64_MAX = 0xFFFF'FFFF'FFFF'FFFFULL;

inline constexpr i64 I64_MIN = -0x7FFF'FFFF'FFFF'FFFFLL - 1;
inline constexpr i64 I64_MAX = 0x7FFF'FFFF'FFFF'FFFFLL;

inline constexpr usize USIZE_MIN = 0;
inline constexpr usize USIZE_MAX = SIZE_MAX;

inline constexpr isize ISIZE_MIN = PTRDIFF_MIN;
inline constexpr isize ISIZE_MAX = PTRDIFF_MAX;

inline constexpr c32 UTF32_MIN = 0x0000'0000;
inline constexpr c32 UTF32_MAX = 0x0010'FFFF;

inline constexpr f16 F16_MIN          = -65504.0F;            // -2^16 * (1 - 2^-10)
inline constexpr f16 F16_MIN_POSITIVE = 0.00006103515625F;    // 2^-10
inline constexpr f16 F16_MAX          = 65504.0F;             // 2^16 * (1 - 2^-10)
inline constexpr f16 F16_EPS          = 0.00006103515625F;
inline constexpr f16 F16_INF          = INFINITY;

inline constexpr f32 F32_MIN          = -FLT_MAX;
inline constexpr f32 F32_MIN_POSITIVE = FLT_MIN;
inline constexpr f32 F32_MAX          = FLT_MAX;
inline constexpr f32 F32_EPS          = FLT_EPSILON;
inline constexpr f32 F32_INF          = INFINITY;

inline constexpr f64 F64_MIN          = -DBL_MAX;
inline constexpr f64 F64_MIN_POSITIVE = DBL_MIN;
inline constexpr f64 F64_MAX          = DBL_MAX;
inline constexpr f64 F64_EPS          = DBL_EPSILON;
inline constexpr f64 F64_INF          = INFINITY;

inline constexpr f32 PI = 3.14159265358979323846F;

template <typename T>
struct NumTraits;

template <>
struct NumTraits<u8>
{
    static constexpr u8   MIN            = U8_MIN;
    static constexpr u8   MAX            = U8_MAX;
    static constexpr bool SIGNED         = false;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u16>
{
    static constexpr u16  MIN            = U16_MIN;
    static constexpr u16  MAX            = U16_MAX;
    static constexpr bool SIGNED         = false;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u32>
{
    static constexpr u32  MIN            = U32_MIN;
    static constexpr u32  MAX            = U32_MAX;
    static constexpr bool SIGNED         = false;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u64>
{
    static constexpr u64  MIN            = U64_MIN;
    static constexpr u64  MAX            = U64_MAX;
    static constexpr bool SIGNED         = false;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i8>
{
    static constexpr i8   MIN            = I8_MIN;
    static constexpr i8   MAX            = I8_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i16>
{
    static constexpr i16  MIN            = I16_MIN;
    static constexpr i16  MAX            = I16_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i32>
{
    static constexpr i32  MIN            = I32_MIN;
    static constexpr i32  MAX            = I32_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i64>
{
    static constexpr i64  MIN            = I64_MIN;
    static constexpr i64  MAX            = I64_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<f16>
{
    static constexpr f16  MIN            = F16_MIN;
    static constexpr f16  MAX            = F16_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = true;
};

template <>
struct NumTraits<f32>
{
    static constexpr f32  MIN            = F32_MIN;
    static constexpr f32  MAX            = F32_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = true;
};

template <>
struct NumTraits<f64>
{
    static constexpr f64  MIN            = F64_MIN;
    static constexpr f64  MAX            = F64_MAX;
    static constexpr bool SIGNED         = true;
    static constexpr bool FLOATING_POINT = true;
};

template <typename T>
struct NumTraits<T const> : NumTraits<T>
{
};

template <typename T>
struct NumTraits<T volatile> : NumTraits<T>
{
};

template <typename T>
struct NumTraits<T const volatile> : NumTraits<T>
{
};

enum class Order : i8
{
    Less    = -1,
    Equal   = 0,
    Greater = 1,
};

constexpr Order reverse_order(Order ord)
{
    return static_cast<Order>(-static_cast<i8>(ord));
}

struct Add
{
    constexpr auto operator()(auto const & a, auto const & b) const
    {
        return a + b;
    }
};

struct Sub
{
    constexpr auto operator()(auto const & a, auto const & b) const
    {
        return a - b;
    }
};

struct Mul
{
    constexpr auto operator()(auto const & a, auto const & b) const
    {
        return a * b;
    }
};

struct Div
{
    constexpr auto operator()(auto const & a, auto const & b) const
    {
        return a / b;
    }
};

struct Eq
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a == b;
    }
};

struct NEq
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a != b;
    }
};

struct Less
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a < b;
    }
};

struct LEq
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a <= b;
    }
};

struct Gt
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a > b;
    }
};

struct GEq
{
    constexpr bool operator()(auto const & a, auto const & b) const
    {
        return a >= b;
    }
};

struct Cmp
{
    constexpr Order operator()(auto const & a, auto const & b) const
    {
        if (a == b)
        {
            return Order::Equal;
        }
        if (a > b)
        {
            return Order::Greater;
        }
        return Order::Less;
    }
};

struct Min
{
    template <typename T>
    constexpr T operator()(T const & a, T const & b) const
    {
        return a < b ? a : b;
    }

    template <typename T, typename... T1>
    constexpr T operator()(T const & a, T const & b, T1 const &... cs) const
    {
        T m = this->template operator()<T>(a, b);
        // clang-format off
    ((m = this->template operator()<T>(static_cast<T &&>(m), cs)), ...);
        // clang-format on
        return m;
    }
};

struct Max
{
    template <typename T>
    constexpr T operator()(T const & a, T const & b) const
    {
        return a > b ? a : b;
    }

    template <typename T, typename... T1>
    constexpr T operator()(T const & a, T const & b, T1 const &... cs) const
    {
        T m = this->template operator()<T>(a, b);
        // clang-format off
    ((m = this->template operator()<T>(static_cast<T &&>(m), cs)), ...);
        // clang-format on
        return m;
    }
};

struct Swap
{
    template <typename T>
    constexpr void operator()(T & a, T & b) const
    {
        T s{static_cast<T &&>(a)};
        a = static_cast<T &&>(b);
        b = static_cast<T &&>(s);
    }
};

struct Clamp
{
    template <typename T>
    constexpr T operator()(T const & value, T const & min, T const & max) const
    {
        return value < min ? min : (value > max ? max : value);
    }
};

constexpr Add   add;
constexpr Sub   sub;
constexpr Mul   mul;
constexpr Div   div;
constexpr Eq    eq;
constexpr NEq   neq;
constexpr Less  lt;
constexpr LEq   leq;
constexpr Gt    gt;
constexpr GEq   geq;
constexpr Cmp   cmp;
constexpr Min   min;
constexpr Max   max;
constexpr Swap  swap;
constexpr Clamp clamp;

// TODO: default eq, neq, lt, gt, leq overridable

constexpr u8 sat_add(u8 a, u8 b)
{
    return ((a + b) < a) ? U8_MAX : (a + b);
}

constexpr u16 sat_add(u16 a, u16 b)
{
    return ((a + b) < a) ? U16_MAX : (a + b);
}

constexpr u32 sat_add(u32 a, u32 b)
{
    return ((a + b) < a) ? U32_MAX : (a + b);
}

constexpr u64 sat_add(u64 a, u64 b)
{
    return ((a + b) < a) ? U64_MAX : (a + b);
}

constexpr i8 sat_add(i8 a, u8 b)
{
    return (i8) clamp((i16) ((i16) a + (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_add(i16 a, i16 b)
{
    return (i16) clamp((i32) a + (i32) b, (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_add(i32 a, i32 b)
{
    return (i32) clamp((i64) a + (i64) b, (i64) I32_MIN, (i64) I32_MAX);
}

constexpr i64 sat_add(i64 a, i64 b)
{
    if (a > 0)
    {
        b = I64_MAX - a;
    }
    else if (b < (I64_MIN - a))
    {
        b = I64_MIN - a;
    }

    return a + b;
}

constexpr u8 sat_sub(u8 a, u8 b)
{
    return (u8) clamp((i16) ((i16) a - (i16) b), (i16) U8_MIN, (i16) U8_MAX);
}

constexpr u16 sat_sub(u16 a, u16 b)
{
    return (u16) clamp((i32) ((i32) a - (i32) b), (i32) U16_MIN, (i32) U16_MAX);
}

constexpr u32 sat_sub(u32 a, u32 b)
{
    return (u32) clamp((i64) ((i64) a - (i64) b), (i64) U32_MIN, (i64) U32_MAX);
}

constexpr u64 sat_sub(u64 a, u64 b)
{
    if (a < b)
    {
        return 0;
    }

    return a - b;
}

constexpr i8 sat_sub(i8 a, i8 b)
{
    return (i8) clamp((i16) ((i16) a - (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_sub(i16 a, i16 b)
{
    return (i16) clamp((i32) ((i32) a - (i32) b), (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_sub(i32 a, i32 b)
{
    return (i32) clamp((i64) ((i64) a - (i64) b), (i64) I32_MIN, (i64) I32_MAX);
}

constexpr u8 sat_mul(u8 a, u8 b)
{
    return (u8) min((u16) ((u16) a * (u16) b), (u16) U8_MAX);
}

constexpr u16 sat_mul(u16 a, u16 b)
{
    return (u16) min((u32) a * (u32) b, (u32) U16_MAX);
}

constexpr u32 sat_mul(u32 a, u32 b)
{
    return (u32) min((u64) a * (u64) b, (u64) U32_MAX);
}

constexpr i8 sat_mul(i8 a, i8 b)
{
    return (i8) clamp((i16) ((i16) a * (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_mul(i16 a, i16 b)
{
    return (i16) clamp((i32) a * (i32) b, (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_mul(i32 a, i32 b)
{
    return (i32) clamp((i64) a * (i64) b, (i64) I32_MIN, (i64) I32_MAX);
}

template <Unsigned T>
constexpr T ring_add(T index, T dist, T size)
{
    return (index + dist) % size;
}

template <Unsigned T>
constexpr T ring_sub(T index, T dist, T size)
{
    dist = dist % size;
    return (index >= dist) ? (index - dist) : (size - (dist - index));
}

using std::bit_cast;

template <typename T>
constexpr bool has_bits(T src, T cmp)
{
    return (src & cmp) == cmp;
}

template <typename T>
constexpr bool has_any_bit(T src, T cmp)
{
    return (src & cmp) != (T) 0;
}

constexpr bool is_pow2(u8 x)
{
    return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u16 x)
{
    return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u32 x)
{
    return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u64 x)
{
    return (x & (x - 1)) == 0ULL;
}

constexpr u64 operator""_B(unsigned long long int x)
{
    return static_cast<u64>(x);
}

constexpr u64 operator""_KB(unsigned long long int x)
{
    return static_cast<u64>(x << 10);
}

constexpr u64 operator""_MB(unsigned long long int x)
{
    return static_cast<u64>(x << 20);
}

constexpr u64 operator""_GB(unsigned long long int x)
{
    return static_cast<u64>(x << 30);
}

constexpr u64 operator""_TB(unsigned long long int x)
{
    return static_cast<u64>(x << 40);
}

template <typename Repr>
constexpr usize atom_size_for(usize num_bits)
{
    return (num_bits + (bitsizeof<Repr> - 1)) / bitsizeof<Repr>;
}

template <typename E>
using enum_ut = std::underlying_type_t<E>;

template <typename E>
constexpr enum_ut<E> enum_uv(E a)
{
    return static_cast<enum_ut<E>>(a);
}

template <typename E>
constexpr enum_ut<E> enum_uv_or(E a, E b)
{
    return static_cast<enum_ut<E>>(enum_uv(a) | enum_uv(b));
}

template <typename E>
constexpr enum_ut<E> enum_uv_and(E a, E b)
{
    return static_cast<enum_ut<E>>(enum_uv(a) & enum_uv(b));
}

template <typename E>
constexpr enum_ut<E> enum_uv_not(E a)
{
    return static_cast<enum_ut<E>>(~enum_uv(a));
}

template <typename E>
constexpr enum_ut<E> enum_uv_xor(E a, E b)
{
    return static_cast<enum_ut<E>>(enum_uv(a) ^ enum_uv(b));
}

template <typename E>
constexpr E enum_or(E a, E b)
{
    return static_cast<E>(enum_uv_or(a, b));
}

template <typename E>
constexpr E enum_and(E a, E b)
{
    return static_cast<E>(enum_uv_and(a, b));
}

template <typename E>
constexpr E enum_xor(E a, E b)
{
    return static_cast<E>(enum_uv_xor(a, b));
}

template <typename E>
constexpr E enum_not(E a)
{
    return static_cast<E>(enum_uv_not(a));
}

#define ASH_BIT_ENUM_OPS(E)              \
    constexpr E operator|(E a, E b)      \
    {                                    \
        return ::ash::enum_or(a, b);     \
    }                                    \
                                         \
    constexpr E operator&(E a, E b)      \
    {                                    \
        return ::ash::enum_and(a, b);    \
    }                                    \
                                         \
    constexpr E operator^(E a, E b)      \
    {                                    \
        return ::ash::enum_xor(a, b);    \
    }                                    \
                                         \
    constexpr E operator~(E a)           \
    {                                    \
        return ::ash::enum_not(a);       \
    }                                    \
                                         \
    constexpr E & operator|=(E & a, E b) \
    {                                    \
        a = a | b;                       \
        return a;                        \
    }                                    \
                                         \
    constexpr E & operator&=(E & a, E b) \
    {                                    \
        a = a & b;                       \
        return a;                        \
    }                                    \
                                         \
    constexpr E & operator^=(E & a, E b) \
    {                                    \
        a = a ^ b;                       \
        return a;                        \
    }

template <typename T>
struct ref
{
    using Type = T;

    T * repr_;

    constexpr ref(T & v) : repr_{&v}
    {
    }

    constexpr ref(Uninit) : repr_{nullptr}
    {
    }

    constexpr ref(ref const &)             = default;
    constexpr ref(ref &&)                  = default;
    constexpr ref & operator=(ref const &) = default;
    constexpr ref & operator=(ref &&)      = default;
    constexpr ~ref()                       = default;

    constexpr T & unref() const
    {
        return *repr_;
    }

    constexpr T * operator->() const
    {
        return repr_;
    }

    constexpr operator T &() const
    {
        return *repr_;
    }

    constexpr T & operator*() const
    {
        return *repr_;
    }

    constexpr T * ptr() const
    {
        return repr_;
    }
};

template <typename T>
ref(T &) -> ref<T>;

template <typename T>
using InitList = std::initializer_list<T>;

/// @brief A slice is a pair of integers: `offset` and `span` that represent a
/// range of elements in any container. .offset can range from 0-`S::MAX`,
/// `S::MAX` means the end of the range. .span can range from 0-`S::MAX`,
/// `S::MAX` means to the end of the range.
///
///
///
/// `.offset` and `.offset + .span` can be more than the number of elements in
/// the target container. To resolve the slice once the size of the container is
/// known, use the resolve operator: `Slice::operator()(usize n)`
///
/// To prevent `.offset + .span` from overflow or out-of-bounds call the resolve
/// operator
///
template <typename S>
struct [[nodiscard]] CoreSlice
{
    static constexpr S END = NumTraits<S>::MAX;

    S offset = 0;
    S span   = 0;

    static constexpr CoreSlice offsets(S begin, S end)
    {
        return CoreSlice{.offset = begin, .span = static_cast<S>(end - begin)};
    }

    static constexpr CoreSlice elements(S first, S last)
    {
        return CoreSlice{.offset = first, .span = static_cast<S>(last - first + 1)};
    }

    static constexpr CoreSlice slice(S offset, S span)
    {
        return CoreSlice{.offset = offset, .span = span};
    }

    static constexpr CoreSlice all()
    {
        return CoreSlice{.offset = 0, .span = END};
    }

    constexpr S begin() const
    {
        return offset;
    }

    constexpr S end() const
    {
        return offset + span;
    }

    constexpr S first() const
    {
        return offset;
    }

    constexpr S last() const
    {
        return end() - 1;
    }

    constexpr CoreSlice operator()(S size) const
    {
        return CoreSlice::offsets(min(offset, size), min(sat_add(offset, span), size));
    }

    constexpr CoreSlice operator()() const
    {
        return CoreSlice::offsets(offset, sat_add(offset, span));
    }

    constexpr bool is_empty() const
    {
        return span == 0;
    }

    constexpr bool contains(CoreSlice other) const
    {
        return begin() <= other.begin() && end() >= other.end();
    }

    constexpr bool intersects(CoreSlice other) const
    {
        return max(begin(), other.begin()) < min(end(), other.end());
    }

    constexpr CoreSlice intersection(CoreSlice other) const
    {
        auto m0 = max(begin(), other.begin());
        auto m1 = min(end(), other.end());
        return m0 > m1 ? CoreSlice::slice(0, 0) : CoreSlice::offsets(m0, m1);
    }

    constexpr CoreSlice extend(S extension) const
    {
        return CoreSlice::offsets(begin(), sat_add(end(), extension));
    }

    constexpr bool contains(S item) const
    {
        return begin() <= item && end() > item;
    }

    constexpr bool in_range(S size) const
    {
        return begin() < size && end() >= size;
    }

    constexpr CoreSlice<u32> as_u32() const
    {
        return CoreSlice<u32>{static_cast<u32>(offset), static_cast<u32>(span)};
    }

    constexpr CoreSlice<u64> as_u64() const
    {
        return CoreSlice<u64>{static_cast<u64>(offset), static_cast<u64>(span)};
    }

    constexpr CoreSlice<usize> as_usize() const
    {
        return CoreSlice<usize>{static_cast<usize>(offset), static_cast<usize>(span)};
    }
};

template <typename S>
constexpr bool operator==(CoreSlice<S> const & a, CoreSlice<S> const & b)
{
    return a.offset == b.offset && a.span == b.span;
}

using Slice   = CoreSlice<usize>;
using Slice8  = CoreSlice<u8>;
using Slice16 = CoreSlice<u16>;
using Slice32 = CoreSlice<u32>;
using Slice64 = CoreSlice<u64>;

struct IterEnd
{
};

inline constexpr IterEnd iter_end;

template <typename T>
struct SpanIter;

template <typename T, usize N>
constexpr SpanIter<T> begin(T (&a)[N]);

template <typename T>
constexpr auto begin(T && a) -> decltype(a.begin())
{
    return a.begin();
}

template <typename T, usize N>
constexpr auto end(T (&)[N])
{
    return iter_end;
}

template <typename T>
constexpr auto end(T && a) -> decltype(a.end())
{
    return a.end();
}

template <typename T, usize N>
constexpr T * data(T (&a)[N])
{
    return a;
}

template <typename T>
constexpr auto data(T && a) -> decltype(a.data())
{
    return a.data();
}

template <typename T, usize N>
constexpr usize size(T (&)[N])
{
    return N;
}

template <typename T>
constexpr auto size(T && a) -> decltype(a.size())
{
    return a.size();
}

/// @brief Iterator Model. Iterators are only required to
/// produce values, they are not required to provide
/// references to the values
template <typename T>
concept Iter = requires (T it) {
    {
        // value producer
        *it
    };
    {
        // in-place (pre-fix) advancement
        ++it
    };
};

/// @brief Range Model. Ranges are read-only by default.
template <typename R>
concept Range = requires (R r) {
    {
        // can get an iterator to its beginning element
        begin(r)
    } -> Iter;
    {
        // returns boolean when asked if it has ended
        !(begin(r) != end(r))
    };
};

template <typename T>
concept OutIter = Iter<T>;

template <typename T>
concept OutRange = Range<T>;

template <typename Iter>
concept SizedIter = requires (Iter iter) {
    { static_cast<usize>(iter.size()) };
};

template <typename Iter>
concept BoundedSizeIter = requires (Iter iter) {
    { static_cast<usize>(iter.max_size()) };
};

template <typename Iter, typename T>
concept IterOf = requires (Iter iter) {
    { static_cast<T>(*iter) };
};

template <typename Iter>
struct IterView
{
    Iter iter_;

    constexpr auto begin() const
    {
        return iter_;
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr auto size() const requires (SizedIter<Iter>)
    {
        return iter_.size();
    }

    constexpr auto max_size() const requires (BoundedSizeIter<Iter>)
    {
        return iter_.max_size();
    }
};

template <typename T>
concept Sized = requires (T t) {
    { size(t) };
};

template <typename T, u8 N>
constexpr u8 size8(T (&)[N])
{
    return N;
}

template <Sized T>
constexpr auto size8(T && a)
{
    return static_cast<u8>(a.size());
}

template <typename T, u16 N>
constexpr u16 size16(T (&)[N])
{
    return N;
}

template <Sized T>
constexpr auto size16(T && a)
{
    return static_cast<u16>(a.size());
}

template <typename T, u32 N>
constexpr u32 size32(T (&)[N])
{
    return N;
}

template <Sized T>
constexpr auto size32(T && a)
{
    return static_cast<u32>(a.size());
}

template <typename T, u64 N>
constexpr u64 size64(T (&)[N])
{
    return N;
}

template <Sized T>
constexpr auto size64(T && a)
{
    return static_cast<u64>(a.size());
}

template <typename T, usize N>
constexpr usize size_bytes(T (&)[N])
{
    return sizeof(T) * N;
}

template <typename T>
constexpr auto size_bytes(T && a) -> decltype(a.size_bytes())
{
    return a.size_bytes();
}

template <typename T>
constexpr auto is_empty(T && a)
{
    return size(a) == 0;
}

template <typename T>
constexpr auto is_empty(T && a) -> decltype(a.is_empty())
{
    return a.is_empty();
}

template <usize Alignment, typename T>
[[nodiscard]] constexpr T * assume_aligned_to(T * ptr)
{
    if (std::is_constant_evaluated())
    {
        return ptr;
    }
    else
    {
        return static_cast<T *>(__builtin_assume_aligned(ptr, Alignment));
    }
}

namespace impl
{

namespace atom
{
template <typename Atom, usize BitWidth>
constexpr Atom get_bits(Atom s, usize i)
{
    constexpr auto mask = ((Atom) 1U << BitWidth) - 1;
    auto           pos  = i * BitWidth;
    return (s >> pos) & mask;
}

template <typename Atom, usize BitWidth>
constexpr Atom clear_bits(Atom s, usize i)
{
    constexpr auto mask = (((Atom) 1U) << BitWidth) - 1;
    auto           pos  = i * BitWidth;
    return s & ~(mask << pos);
}

template <typename Atom, usize BitWidth>
constexpr Atom set_bits(Atom s, usize i)
{
    constexpr auto mask = ((Atom) 1U << BitWidth) - 1;
    auto           pos  = i * BitWidth;
    return s | (mask << pos);
}

template <typename Atom, usize BitWidth>
constexpr Atom assign_bits(Atom s, usize i, Atom value)
{
    auto pos = i * BitWidth;
    return clear_bits<Atom, BitWidth>(s, i) | (value << pos);
}

template <typename Atom, usize BitWidth>
constexpr Atom flip_bits(Atom s, usize i)
{
    constexpr auto mask = ((Atom) 1U << BitWidth) - 1;
    auto           pos  = i * BitWidth;
    return s ^ (mask << pos);
}

}    // namespace atom

template <typename Atom>
constexpr Atom get_bit(Atom * p_atoms, usize i)
{
    auto atom_idx = i / bitsizeof<Atom>;
    auto bit_idx  = i & (bitsizeof<Atom> - 1);

    return atom::get_bits<Atom, 1>(p_atoms[atom_idx], bit_idx);
}

template <typename Atom>
constexpr void clear_bit(Atom * p_atoms, usize i)
{
    auto atom_idx = i / bitsizeof<Atom>;
    auto bit_idx  = i & (bitsizeof<Atom> - 1);

    auto & a = p_atoms[atom_idx];
    a        = atom::clear_bits<Atom, 1>(a, bit_idx);
}

template <typename Atom>
constexpr void set_bit(Atom * p_atoms, usize i)
{
    auto atom_idx = i / bitsizeof<Atom>;
    auto bit_idx  = i & (bitsizeof<Atom> - 1);

    auto & a = p_atoms[atom_idx];
    a        = atom::set_bits<Atom, 1>(a, bit_idx);
}

template <typename Atom>
constexpr void assign_bit(Atom * p_atoms, usize i, Atom value)
{
    auto atom_idx = i / bitsizeof<Atom>;
    auto bit_idx  = i & (bitsizeof<Atom> - 1);

    auto & a = p_atoms[atom_idx];
    a        = atom::assign_bits<Atom, 1>(a, bit_idx, value);
}

template <typename Atom>
constexpr void flip_bit(Atom * p_atoms, usize i)
{
    auto atom_idx = i / bitsizeof<Atom>;
    auto bit_idx  = i & (bitsizeof<Atom> - 1);

    auto & a = p_atoms[atom_idx];
    a        = atom::flip_bits<Atom, 1>(a, bit_idx);
}

template <typename Atom>
constexpr usize find_set_bit(Atom * p_atom, usize num_atoms)
{
    auto begin = p_atom;
    auto iter  = p_atom;
    auto end   = p_atom + num_atoms;

    while (iter != end && *iter == 0)
    {
        iter++;
    }

    auto idx = static_cast<usize>(iter - begin) * bitsizeof<Atom>;

    if (iter == end)
    {
        return idx;
    }

    return idx | std::countr_zero(*iter);
}

template <typename Atom>
constexpr usize find_clear_bit(Atom * p_atom, usize num_atoms)
{
    auto begin = p_atom;
    auto iter  = p_atom;
    auto end   = p_atom + num_atoms;

    while (iter != end && *iter == NumTraits<Atom>::MAX)
    {
        iter++;
    }

    auto idx = static_cast<usize>(iter - begin) * bitsizeof<Atom>;

    if (iter == end)
    {
        return idx;
    }

    return idx | std::countr_one(*iter);
}

}    // namespace impl

constexpr u16 byte_swap(u16 x)
{
    return __builtin_bswap16(x);
}

constexpr u32 byte_swap(u32 x)
{
    return __builtin_bswap32(x);
}

constexpr u64 byte_swap(u64 x)
{
    return __builtin_bswap64(x);
}

template <typename Lambda>
struct defer
{
    Lambda lambda;
    constexpr defer(defer &&)                  = delete;
    constexpr defer(defer const &)             = delete;
    constexpr defer & operator=(defer &&)      = delete;
    constexpr defer & operator=(defer const &) = delete;

    constexpr defer(Lambda && l) : lambda{static_cast<Lambda &&>(l)}
    {
    }

    constexpr ~defer()
    {
        lambda();
    }
};

template <typename Lambda>
defer(Lambda &&) -> defer<Lambda>;

struct Noop
{
    template <typename... Args>
    using Sig = void(Args...);

    template <typename... Args>
    constexpr operator Sig<Args...> *() const
    {
        return [](Args...) {};
    }

    template <typename... Args>
    constexpr void operator()(Args &&...) const
    {
    }
};

constexpr Noop noop;

template <typename Char>
constexpr usize cstr_len(Char * c_str)
{
    Char const * it = c_str;
    while (*it != 0)
    {
        it++;
    }

    return static_cast<usize>(it - c_str);
}

template <typename T = void>
struct Pin
{
    typedef T Type;

    T v;

    template <typename... Args>
    constexpr Pin(Args &&... args) : v{static_cast<Args &&>(args)...}
    {
    }

    constexpr Pin(Pin const &)             = delete;
    constexpr Pin(Pin &&)                  = delete;
    constexpr Pin & operator=(Pin const &) = delete;
    constexpr Pin & operator=(Pin &&)      = delete;
    constexpr ~Pin()                       = default;
};

template <>
struct Pin<void>
{
    typedef void Type;

    constexpr Pin()                        = default;
    constexpr Pin(Pin const &)             = delete;
    constexpr Pin(Pin &&)                  = delete;
    constexpr Pin & operator=(Pin const &) = delete;
    constexpr Pin & operator=(Pin &&)      = delete;
    constexpr ~Pin()                       = default;
};

/// @brief Uninitialized storage
template <usize Alignment, usize Capacity>
struct InplaceStorage
{
    alignas(Alignment) mutable u8 storage_[Capacity];
};

template <usize Alignment>
struct InplaceStorage<Alignment, 0>
{
    static constexpr u8 * storage_ = nullptr;
};

template <typename T>
using Storage = InplaceStorage<alignof(T), sizeof(T)>;

struct alignas(u64) Version
{
    u16 variant = 0;
    u16 major   = 0;
    u16 minor   = 0;
    u16 patch   = 0;
};

#define ASH_VERSION                                 \
    (::ash::Version{.variant = ASH_VARIANT_VERSION, \
                    .major   = ASH_MAJOR_VERSION,   \
                    .minor   = ASH_MINOR_VERSION,   \
                    .patch   = ASH_PATCH_VERSION})

template <typename T>
T declval() noexcept
{
    static_assert(false, "declval not allowed in an evaluated context");
}

/// @brief Consume transfers ownership of an handle type.
/// Thus "poisoning" the source value by resetting it to a null/empty/poison
/// state. This distinction is necessary because not all types have meaningful
/// `default`, `null`, or `poison` semantics.
template <typename T>
struct Consume
{
    static constexpr T consume(T & v)
    {
        return static_cast<T &&>(v);
    }
};

template <typename T>
struct Consume<T *>
{
    static constexpr auto consume(T *& v)
    {
        auto * out = v;
        v          = nullptr;
        return out;
    }
};

}    // namespace ash
