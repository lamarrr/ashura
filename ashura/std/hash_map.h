#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/hash.h"
#include "ashura/std/op.h"
#include "ashura/std/storage.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <string.h>

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

template <typename K, typename V>
struct HashMapEntry
{
  using KeyType   = K;
  using ValueType = V;

  K key;
  V value;
};

// Linear Probing hash map
// https://developer.nvidia.com/blog/maximizing-performance-with-massively-parallel-hash-maps-on-gpus/
template <typename K, typename V, typename Hasher, typename KeyCmp>
struct HashMap
{
  using KeyType    = K;
  using ValueType  = V;
  using EntryType  = HashMapEntry<K, V>;
  using HasherType = Hasher;
  using KeyCmpType = KeyCmp;

  void reset();

  void               rehash();
  [[nodiscard]] bool should_rehash() const
  {
    // num_entries/num_buckets > .875
  }

  [[nodiscard]] V   *operator[](K const &);
  [[nodiscard]] bool has(K const &);
  [[nodiscard]] V   *get(K const &);
  void               remove(K const &);
  template <typename KArg, typename... Args>
  [[nodiscard]] bool push(KArg &&key_arg, Args &&...value_args);
  void               pop(K const &);

  KeyCmp        cmp              = {};
  AllocatorImpl allocator        = heap_allocator;
  usize         num_entries      = 0;
  usize         num_buckets      = 0;
  u8            buckets_capacity = 0;
  Vec<u8>       bucket_sizes     = {};
  void         *storage          = nullptr;
};

template <typename V>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash