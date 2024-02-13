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

  usize num_buckets() const
  {
    return m_bucket_sizes.size();
  }

  usize bucket_capacity() const
  {
    return m_buckets_capacity;
  }

  usize num_entries() const
  {
    return m_num_entries;
  }

  void reset();

  [[nodiscard]] V *operator[](K const &key) const
  {
    if (num_buckets() == 0)
    {
      return nullptr;
    }
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash % num_buckets();
    usize const bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it    = m_entries + bucket_index * bucket_capacity();
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (m_cmp(bucket_it->key, key))
      {
        return &bucket_it->value;
      }
    } 
    return nullptr;
  }

  void remove(K const &key)
  {
    if (num_buckets() == 0)
    {
      return;
    }
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash % num_buckets();
    usize const bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it    = m_entries + bucket_index * bucket_capacity();
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (m_cmp(bucket_it->key, key))
      {
        for (usize src = i + 1; src < bucket_size; src++)
        {
          bucket_it[src - 1] = (EntryType &&) (bucket_it[src]);
        }
        (bucket_it + bucket_size - 1)->~T();
      }
    }
  }

  // num_entries/num_buckets > .875, increase number of buckets or buckets
  // capacity?
  template <typename... Args>
  [[nodiscard]] bool push(K const &key, Args &&...value_args)
  {
    if (num_buckets() == 0)
    {
      // increase num buckets
    }

    if (((m_num_entries * 100) / num_buckets()) > 875)
    {
      // increase bucket count and rehash
    }

    // what if after rehashing a number of elements fall into the same slot? and
    // then can't fit in?
  }

  // push if not exists

  KeyCmp        m_cmp              = {};
  Hasher        m_hasher           = {};
  AllocatorImpl m_allocator        = default_allocator;
  usize         m_num_entries      = 0;
  usize         m_buckets_capacity = 0;
  Vec<usize>    m_bucket_sizes     = {};
  EntryType    *m_entries          = nullptr;
};

template <typename V>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash