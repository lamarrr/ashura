#pragma once
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

/// @index: max of Rep::NUM_BITS - 1
template <typename Rep>
struct BitRef
{
  Rep *pack  = nullptr;
  u16  index = 0;

  constexpr BitRef const &operator=(bool bit) const
    requires (!Const<Rep>)
  {
    *pack = (*pack & ~(((Rep) 1) << index)) | (((Rep) (bit)) << index);
    return *this;
  }

  constexpr operator bool() const
  {
    return (*pack >> index) & 1;
  }

  constexpr operator BitRef<Rep const>() const
  {
    return BitRef<Rep const>{pack, index};
  }

  bool operator|(bool other) const
  {
    return this->operator bool() || other;
  }

  bool operator&(bool other) const
  {
    return this->operator bool() && other;
  }

  bool operator~() const
  {
    return !(this->operator bool());
  }
};

/// @data: is never changed
template <typename Rep>
struct BitIterator
{
  Rep  *data  = nullptr;
  usize index = 0;

  constexpr BitIterator operator+(usize advance) const
  {
    return BitIterator{data, index + advance};
  }

  constexpr BitIterator &operator++()
  {
    index++;
    return *this;
  }

  constexpr BitIterator operator++(int)
  {
    BitIterator out{data, index};
    index++;
    return out;
  }

  constexpr operator BitIterator<Rep const>() const
  {
    return BitIterator{data, index};
  }
};

template <typename Rep>
constexpr bool operator==(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.data == b.data && a.index == b.index;
}

template <typename Rep>
constexpr bool operator!=(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.data != b.data || a.index != b.index;
}

template <typename Rep>
constexpr BitRef<Rep> operator*(BitIterator<Rep> it)
{
  constexpr u16 INDEX_SHIFT = IntTraits<Rep>::LOG2_NUM_BITS;
  constexpr u16 INDEX_MASK  = IntTraits<Rep>::NUM_BITS - 1;
  return BitRef{it.data + (it.index >> INDEX_SHIFT), (it.index & INDEX_MASK)};
}

/// UB if not pointing to the same data
template <typename Rep>
constexpr usize operator-(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index - b.index;
}

/// no slice support
template <typename Rep>
struct BitSpan
{
  Rep  *data     = nullptr;
  usize num_bits = 0;

  constexpr BitRef<Rep> operator[](usize index) const
  {
    constexpr u16 INDEX_SHIFT = IntTraits<Rep>::LOG2_NUM_BITS;
    constexpr u16 INDEX_MASK  = IntTraits<Rep>::NUM_BITS - 1;
    return BitRef{data + (index >> INDEX_SHIFT), (index & INDEX_MASK)};
  }

  constexpr operator BitSpan<Rep const>() const
  {
    return BitSpan<Rep const>{data, num_bits};
  }

  constexpr BitIterator<Rep> begin() const
  {
    return BitIterator<Rep>{data, 0};
  }

  constexpr BitIterator<Rep> end() const
  {
    return BitIterator<Rep>{data, num_bits};
  }

  constexpr bool is_empty() const
  {
    return num_bits == 0;
  }
};

}        // namespace ash