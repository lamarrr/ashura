#pragma once
#include "ashura/types.h"

namespace ash
{

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
};

namespace span
{
template <typename T>
constexpr Span<T> slice(Span<T> self, usize offset)
{
  return self[Slice{offset, USIZE_MAX}];
}

template <typename T>
constexpr Span<T> slice(Span<T> self, usize offset, usize size)
{
  return self[Slice{offset, size}];
}

template <typename T>
constexpr BitSpan<T> slice(BitSpan<T> span, usize offset)
{
  return span[Slice{offset, USIZE_MAX}];
}

template <typename T>
constexpr BitSpan<T> slice(BitSpan<T> span, usize bit_offset, usize bit_size)
{
  return span[Slice{bit_offset, bit_size}];
}

template <typename T>
constexpr Span<T const> as_const(Span<T> self)
{
  return Span<T const>{self.data, self.size};
}

template <typename T>
constexpr Span<u8> as_u8(Span<T> self)
{
  return Span<u8>{reinterpret_cast<u8 *>(self.data), self.size_bytes()};
}

template <typename T>
constexpr Span<u8 const> as_u8(Span<T const> self)
{
  return Span<u8 const>{reinterpret_cast<u8 const *>(self.data),
                        self.size_bytes()};
}

template <typename T>
constexpr Span<char> as_char(Span<T> self)
{
  return Span<char>{reinterpret_cast<char *>(self.data), self.size_bytes()};
}

template <typename T>
constexpr Span<char const> as_char(Span<T const> self)
{
  return Span<char const>{reinterpret_cast<char const *>(self.data),
                          self.size_bytes()};
}

template <typename T, usize N>
constexpr Span<T> from_array(T (&array)[N])
{
  return Span<T>{array, N};
}

template <typename StdContainer>
constexpr auto from_std_container(StdContainer &container)
{
  return Span{container.data(), container.size()};
}
}        // namespace span
}        // namespace ash
