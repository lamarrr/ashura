#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include <string.h>
#include <unordered_map>

namespace ash
{

struct StrEqual
{
  bool operator()(Span<char const> a, Span<char const> b) const
  {
    return a.size() == b.size() && (memcmp(a.data(), b.data(), a.size()) == 0);
  }
};

struct StrHasher
{
  Hash operator()(Span<char const> str) const
  {
    return hash_bytes(str.data(), str.size());
  }
};

constexpr StrEqual  str_equal;
constexpr StrHasher str_hash;

template <typename V>
using StrDict = std::unordered_map<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash