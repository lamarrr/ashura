#pragma once
#include "stx/option.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/try_ok.h"
#include "stx/try_some.h"
#include "stx/vec.h"
#include <cinttypes>

STX_BEGIN_NAMESPACE

static constexpr size_t BIT_PACK_SIZE      = sizeof(size_t);
static constexpr size_t BIT_PACK_BIT_COUNT = BIT_PACK_SIZE << 3;
using Bit = size_t;        // must always be 0 or 1

struct BitConstRef
{
  size_t const *pack       = nullptr;
  size_t        pack_index = 0;

  constexpr operator Bit() const
  {
    return ((*pack) >> pack_index) & 1;
  }

  constexpr Bit operator&(Bit other) const
  {
    return ((Bit) * this) & other;
  }

  constexpr Bit operator|(Bit other) const
  {
    return ((Bit) (*this)) | other;
  }

  constexpr Bit operator~() const
  {
    return ~((Bit) (*this));
  }
};

struct BitRef
{
  size_t *pack       = nullptr;
  size_t  pack_index = 0;

  constexpr operator BitConstRef() const
  {
    return BitConstRef{pack, pack_index};
  }

  constexpr operator bool() const
  {
    return ((*pack) >> pack_index) & 1;
  }

  constexpr BitRef &operator=(Bit bit)
  {
    (*pack) |= (bit << pack_index);
    return *this;
  }

  constexpr BitRef &operator|=(Bit bit)
  {
    (*pack) |= (bit << pack_index);
    return *this;
  }

  constexpr BitRef &operator&=(Bit bit)
  {
    (*pack) &= (~(((Bit) 1) << pack_index)) | (bit << pack_index);
    return *this;
  }

  constexpr Bit operator&(Bit other) const
  {
    return ((Bit) * this) & other;
  }

  constexpr Bit operator|(Bit other) const
  {
    return ((Bit) (*this)) | other;
  }

  constexpr Bit operator~() const
  {
    return ~((Bit) (*this));
  }
};

constexpr bool operator==(BitConstRef a, BitConstRef b)
{
  return ((Bit) a) == ((Bit) b);
}

constexpr bool operator!=(BitConstRef a, BitConstRef b)
{
  return ((Bit) a) != ((Bit) b);
}

constexpr bool operator==(BitConstRef a, BitRef b)
{
  return ((Bit) a) == ((Bit) b);
}

constexpr bool operator!=(BitConstRef a, BitRef b)
{
  return ((Bit) a) != ((Bit) b);
}

constexpr bool operator==(BitRef a, BitConstRef b)
{
  return ((Bit) a) == ((Bit) b);
}

constexpr bool operator!=(BitRef a, BitConstRef b)
{
  return ((Bit) a) != ((Bit) b);
}

constexpr bool operator==(BitRef a, BitRef b)
{
  return ((Bit) a) == ((Bit) b);
}

constexpr bool operator!=(BitRef a, BitRef b)
{
  return ((Bit) a) != ((Bit) b);
}

struct BitConstIterator
{
  size_t const *data  = nullptr;
  size_t        index = 0;

  constexpr BitConstRef operator*() const
  {
    return BitConstRef{data + (index / BIT_PACK_BIT_COUNT),
                       index % BIT_PACK_BIT_COUNT};
  }

  constexpr BitConstIterator &operator++()
  {
    index++;
    return *this;
  }

  constexpr BitConstIterator operator++(int)
  {
    BitConstIterator cpy{*this};
    ++(*this);
    return cpy;
  }

  constexpr BitConstIterator operator+(size_t advance) const
  {
    return BitConstIterator{data, index + advance};
  }

  constexpr BitConstIterator operator-(size_t step) const
  {
    return BitConstIterator{data, index - step};
  }
};

struct BitIterator
{
  size_t *data  = nullptr;
  size_t  index = 0;

  constexpr BitRef operator*() const
  {
    return BitRef{data + (index / BIT_PACK_BIT_COUNT),
                  index % BIT_PACK_BIT_COUNT};
  }

  constexpr BitIterator &operator++()
  {
    index++;
    return *this;
  }

  constexpr BitIterator operator++(int)
  {
    BitIterator cpy{*this};
    ++(*this);
    return cpy;
  }

  constexpr operator BitConstIterator() const
  {
    return BitConstIterator{data, index};
  }

  constexpr BitIterator operator+(size_t advance) const
  {
    return BitIterator{data, index + advance};
  }

  constexpr BitIterator operator-(size_t step) const
  {
    return BitIterator{data, index - step};
  }
};

constexpr bool operator==(BitConstIterator a, BitConstIterator b)
{
  return a.data == b.data && a.index == b.index;
}

constexpr bool operator!=(BitConstIterator a, BitConstIterator b)
{
  return !(a == b);
}

constexpr bool operator==(BitConstIterator a, BitIterator b)
{
  return a.data == b.data && a.index == b.index;
}

constexpr bool operator!=(BitConstIterator a, BitIterator b)
{
  return !(a == b);
}

constexpr bool operator==(BitIterator a, BitConstIterator b)
{
  return a.data == b.data && a.index == b.index;
}

constexpr bool operator!=(BitIterator a, BitConstIterator b)
{
  return !(a == b);
}

constexpr bool operator==(BitIterator a, BitIterator b)
{
  return a.data == b.data && a.index == b.index;
}

constexpr bool operator!=(BitIterator a, BitIterator b)
{
  return !(a == b);
}

struct BitVec
{
  stx::Vec<Bit> vec;
  size_t        num_bits = 0;

  BitIterator begin()
  {
    return BitIterator{vec.data(), 0};
  }

  BitIterator end()
  {
    return BitIterator{vec.data(), num_bits};
  }

  BitConstIterator begin() const
  {
    return BitConstIterator{vec.data(), 0};
  }

  BitConstIterator end() const
  {
    return BitConstIterator{vec.data(), num_bits};
  }

  Bit unsafe_get(size_t index)
  {
    size_t const pack_index = index / BIT_PACK_BIT_COUNT;
    size_t const bit_index  = index % BIT_PACK_BIT_COUNT;
    return (vec.data()[pack_index] >> bit_index) & 1U;
  }

  stx::Option<Bit> get(size_t index)
  {
    if (index >= num_bits)
    {
      return stx::None;
    }
    size_t const pack_index = index / BIT_PACK_BIT_COUNT;
    size_t const bit_index  = index % BIT_PACK_BIT_COUNT;
    return stx::Some((Bit) ((vec.data()[pack_index] >> bit_index) & 1U));
  }

  stx::Result<stx::Void, stx::AllocError> push(Bit bit)
  {
    size_t const index      = num_bits + 1;
    size_t const pack_index = index / BIT_PACK_BIT_COUNT;
    size_t const bit_index  = index % BIT_PACK_BIT_COUNT;

    TRY_OK(voidr, vec.resize(pack_index + 1, 0U));
    vec.data()[pack_index] |= (bit << bit_index);
    num_bits++;
    return stx::Ok(stx::Void{});
  }

  stx::Result<stx::Void, stx::AllocError> resize(size_t target_size,
                                                 Bit    default_bits = 0)
  {
    TRY_OK(voidr,
           vec.resize((target_size / BIT_PACK_BIT_COUNT) + 1, default_bits));
    num_bits = target_size;
    return stx::Ok(stx::Void{});
  }
};

STX_END_NAMESPACE
