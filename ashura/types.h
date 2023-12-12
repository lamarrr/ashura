#pragma once
#include <cfloat>
#include <cinttypes>
#include <cstddef>
#include <cstdint>

namespace ash
{

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;
using f32   = float;
using f64   = double;
using usize = size_t;
using isize = ptrdiff_t;

constexpr u8 U8_MIN = 0;
constexpr u8 U8_MAX = 0xFFU;

constexpr i8 I8_MIN = -0x7F - 1;
constexpr i8 I8_MAX = 0x7F;

constexpr u16 U16_MIN = 0;
constexpr u16 U16_MAX = 0xFFFFU;

constexpr i16 I16_MIN = -0x7FFF - 1;
constexpr i16 I16_MAX = 0x7FFF;

constexpr u32 U32_MIN = 0;
constexpr u32 U32_MAX = 0xFFFFFFFFU;

constexpr i32 I32_MIN = -0x7FFFFFFF - 1;
constexpr i32 I32_MAX = 0x7FFFFFFF;

constexpr u64 U64_MIN = 0;
constexpr u64 U64_MAX = 0xFFFFFFFFFFFFFFFFULL;

constexpr i64 I64_MIN = -0x7FFFFFFFFFFFFFFFLL - 1;
constexpr i64 I64_MAX = 0x7FFFFFFFFFFFFFFFLL;

constexpr usize USIZE_MIN = 0;
constexpr usize USIZE_MAX = SIZE_MAX;

constexpr isize ISIZE_MIN = PTRDIFF_MIN;
constexpr isize ISIZE_MAX = PTRDIFF_MAX;

constexpr f32 F32_MIN          = -FLT_MAX;
constexpr f32 F32_MIN_POSITIVE = FLT_MIN;
constexpr f32 F32_MAX          = FLT_MAX;
constexpr f32 F32_EPSILON      = FLT_EPSILON;

constexpr f64 F64_MIN          = -DBL_MAX;
constexpr f64 F64_MIN_POSITIVE = DBL_MIN;
constexpr f64 F64_MAX          = DBL_MAX;
constexpr f32 F64_EPSILON      = DBL_EPSILON;

constexpr usize MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

constexpr f32 PI = 3.14159265358979323846f;

struct Slice
{
  usize offset = 0;
  usize size   = 0;
};

template <typename T>
struct Span
{
  T    *data = nullptr;
  usize size = 0;

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size;
  }

  constexpr bool is_empty() const
  {
    return size == 0;
  }

  constexpr T *begin() const
  {
    return data;
  }

  constexpr T *end() const
  {
    return data + size;
  }

  constexpr T &operator[](usize i) const
  {
    return data[i];
  }

  constexpr Span<T> operator[](Slice slice) const
  {
    // written such that overflow will not occur even if both offset and size
    // are set to USIZE_MAX
    slice.offset = slice.offset > size ? size : slice.offset;
    slice.size =
        (size - slice.offset) > slice.size ? slice.size : (size - slice.offset);
    return Span<T>{data + slice.offset, slice.size};
  }

  constexpr operator Span<T const>() const
  {
    return Span<T const>{data, size};
  }

  constexpr Span<T> slice(usize offset) const
  {
    return (*this)[Slice{offset, USIZE_MAX}];
  }

  constexpr Span<T> slice(usize offset, usize size) const
  {
    return (*this)[Slice{offset, size}];
  }

  constexpr Span<T> slice(Slice s) const
  {
    return (*this)[s];
  }
};

template <typename T>
constexpr Span<T const> as_const(Span<T> span)
{
  return Span<T const>{span.data, span.size};
}

template <typename T>
constexpr Span<u8> as_u8(Span<T> span)
{
  return Span<u8>{reinterpret_cast<u8 *>(span.data), span.size_bytes()};
}

template <typename T>
constexpr Span<char> as_char(Span<T> span)
{
  return Span<char>{reinterpret_cast<char *>(span.data), span.size_bytes()};
}

template <typename T>
constexpr Span<u8 const> as_u8(Span<T const> span)
{
  return Span<u8 const>{reinterpret_cast<u8 const *>(span.data),
                        span.size_bytes()};
}

template <typename T>
constexpr Span<char const> as_char(Span<T const> span)
{
  return Span<char const>{reinterpret_cast<char const *>(span.data),
                          span.size_bytes()};
}

template <typename T, usize N>
constexpr Span<T> span_from_array(T (&array)[N])
{
  return Span<T>{array, N};
}

template <typename StdContainer>
constexpr auto span_from_std_container(StdContainer &container)
{
  return Span{container.data(), container.size()};
}

// A span with bit access semantics
template <typename UnsignedInteger>
struct BitSpan
{
  static constexpr u32 NUM_BITS_PER_PACK = sizeof(UnsignedInteger) << 3;

  Span<UnsignedInteger> body          = {};
  Slice                 current_slice = {};

  constexpr bool is_empty() const
  {
    return current_slice.size == 0;
  }

  constexpr bool operator[](usize offset) const
  {
    offset   = current_slice.offset + offset;
    bool bit = (body.data[offset / NUM_BITS_PER_PACK] >>
                (offset % NUM_BITS_PER_PACK)) &
               1U;
    return bit;
  }

  constexpr void set(usize offset, bool bit) const
  {
    offset                      = current_slice.offset + offset;
    UnsignedInteger &out        = body.data[offset / NUM_BITS_PER_PACK];
    usize            bit_offset = offset % NUM_BITS_PER_PACK;
    out                         = (out & ~((UnsignedInteger) 1 << bit_offset)) |
          ((UnsignedInteger) bit << bit_offset);
  }

  constexpr void toggle(usize offset) const
  {
    offset               = current_slice.offset + offset;
    UnsignedInteger &out = body.data[offset / NUM_BITS_PER_PACK];
    out                  = out ^ (1ULL << (offset % NUM_BITS_PER_PACK));
  }

  constexpr BitSpan operator[](Slice slice) const
  {
    slice.offset += current_slice.offset;
    slice.offset =
        slice.offset > current_slice.size ? current_slice.size : slice.offset;
    slice.size = (current_slice.size - slice.offset) > slice.size ?
                     slice.size :
                     (current_slice.size - slice.offset);
    return BitSpan{body, slice};
  }

  constexpr BitSpan slice(Slice s) const
  {
    return (*this)[s];
  }
};

}        // namespace ash
