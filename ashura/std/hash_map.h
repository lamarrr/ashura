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

template <typename K, typename V>
struct HashBucket
{
  HashMapEntry<K, V> *data     = nullptr;
  usize               capacity = 0;
  usize               size     = 0;
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

  usize num_entries() const
  {
    return m_num_entries;
  }

  usize entries_capacity() const
  {
    return num_buckets() << m_bucket_capacity_log2;
  }

  // num buckets must be at least one
  void init();
  void reset();

  [[nodiscard]] V *operator[](K const &key) const
  {
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize const bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it = m_entries + bucket_index << m_bucket_capacity_log2;
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
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize const bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it = m_entries + bucket_index << m_bucket_capacity_log2;
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (m_cmp(bucket_it->key, key))
      {
        for (usize src = i + 1; src < bucket_size; src++)
        {
          bucket_it[src - 1] = (EntryType &&) (bucket_it[src]);
        }
        (bucket_it + bucket_size - 1)->~T();
        m_bucket_sizes[bucket_index]--;
      }
    }
  }

  // double the entries capacity
  bool rehash()
  {
    usize const    num_buckets          = ((usize) 1) << m_num_buckets_log2;
    usize const    new_num_buckets_log2 = m_num_buckets_log2 + 1;
    usize const    new_num_buckets      = ((usize) 1) << new_num_buckets_log2;
    Vec<EntryType> entries{m_allocator};
    // problem is that we don't know which is initialized and which isn't, so a
    // destroy would occur for all elements, leading to double-destroy
    if (!(m_bucket_sizes.resize_defaulted(new_num_buckets) &&
          m_entries.resize_uninitialized(new_num_buckets
                                         << m_bucket_capacity_log2) &&
          entries.reserve(m_num_entries) &&
          entries.resize_uninitialized(m_num_entries)))
    {
      return false;
    }

    // take max of bucket capacities, if load factor of any of bucket capacities
    // is ???
    // this will leave the memory uninitialized
    //
    //
    //
    //
    for (usize ibucket = 0, ientry = 0; ibucket < num_buckets; ibucket++)
    {
      mem::relocate(m_entries.data() + (ibucket << m_bucket_capacity_log2),
                    entries.data() + ientry, m_bucket_sizes[ibucket]);
    }

    for (usize ientry = 0; ientry < m_num_entries; ientry++)
    {
      // go from the back of the bucket towards the front
      // what if it remains same?
      // maintain forward insert, backward remove?
      // hash % (num_buckets)
      // hash % (num_buckets * 2)
      for (usize i = 0; i > 0; i--)
        ;
      // bucket placement will either remain same or move to the new buckets
    }
  }

  // increase bucket capacity

  bool needs_rehash() const
  {
    // max load factor of .875
    // portion of 128 divisions, 128 * .875 = 112
    usize const load_factor = ((m_num_entries + 1) << 7) >> m_num_buckets_log2;
    return load_factor > 112;
  }

  template <typename... Args>
  [[nodiscard]] bool push(K const &key, Args &&...value_args)
  {
    if (needs_rehash())
    {
      rehash();
    }

    // check if bucket isn't full, if full, increase bucket size >> 2
    insert();
  }

  // push if not exists

  KeyCmp         m_cmp                  = {};
  Hasher         m_hasher               = {};
  AllocatorImpl  m_allocator            = default_allocator;
  usize          m_num_entries          = 0;
  usize          m_num_buckets_log2     = 0;
  usize          m_bucket_capacity_log2 = 0;
  Vec<usize>     m_bucket_sizes         = {m_allocator};
  Vec<EntryType> m_entries              = {m_allocator};
};

template <typename V>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash