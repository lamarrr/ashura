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

template <typename K, typename V, typename Hasher, typename KeyCmp>
struct Dict
{
  using KeyType    = K;
  using ValueType  = V;
  using HasherType = Hasher;
  using KeyCmpType = KeyCmp;

  void destroy()
  {
    this->~Dict();
  }

  void clear()
  {
    impl_.clear();
  }

  [[nodiscard]] ValueType *operator[](KeyType const &key)
  {
    auto it = impl_.find(key);
    return it == impl_.end() ? nullptr : &it->second;
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] bool emplace(KeyArg &&key_arg, Args &&...value_args)
  {
    impl_.emplace((KeyArg &&) key_arg, ((Args &&) value_args)...);
    return true;
  }

  std::unordered_map<K, V, Hasher, KeyCmp> impl_;
};

template <typename V>
using StrDict = Dict<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash
