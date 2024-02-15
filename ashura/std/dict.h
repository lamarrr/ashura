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
struct DictEntry
{
  using KeyType   = K;
  using ValueType = V;

  K key;
  V value;
};

// Linear Probing hash map, Based on:
// https://developer.nvidia.com/blog/maximizing-performance-with-massively-parallel-hash-maps-on-gpus/
template <typename K, typename V, typename Hasher, typename KeyCmp>
struct Dict
{
  using KeyType                               = K;
  using ValueType                             = V;
  using EntryType                             = DictEntry<K, V>;
  using HasherType                            = Hasher;
  using KeyCmpType                            = KeyCmp;
  static constexpr usize INIT_BUCKET_CAPACITY = 8;
  static constexpr usize INIT_NUM_BUCKETS     = 4;

  usize num_buckets() const
  {
    return ((usize) 1) << m_num_buckets_log2;
  }

  usize num_entries() const
  {
    return m_num_entries;
  }

  usize entries_capacity() const
  {
    return num_buckets() << m_bucket_capacity_log2;
  }

  usize bucket_capacity() const
  {
    return ((usize) 1) << m_bucket_capacity_log2;
  }

  [[nodiscard]] bool init()
  {
    INIT_BUCKET_CAPACITY;
    INIT_NUM_BUCKETS;
  }

  void destroy()
  {
    if constexpr (!TriviallyDestructible<EntryType>)
    {
      // destroy valid elements
    }
    m_allocator.deallocate_typed(m_entries, m_entries_capacity);
    m_allocator.deallocate_typed(m_bucket_sizes, m_bucket_sizes_capacity);
    m_entries_capacity      = 0;
    m_bucket_sizes_capacity = 0;
    m_num_entries           = 0;
    m_num_buckets_log2      = 0;
    m_bucket_capacity_log2  = 0;
  }

  void clear()
  {
    if constexpr (!TriviallyDestructible<EntryType>)
    {
      // destroy valid elements
    }
    mem::fill(m_bucket_sizes, entries_capacity());
    m_num_entries = 0;
  }

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
    usize      &bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it = m_entries + bucket_index << m_bucket_capacity_log2;
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (m_cmp(bucket_it->key, key))
      {
        if (i != bucket_size - 1)
        {
          swap(bucket_it[i], bucket_it[bucket_size - 1]);
        }
        (bucket_it + bucket_size - 1)->~T();
        bucket_size--;
        m_num_entries--;
        return;
      }
    }
  }

  // the elements will be relocated
  void reinsert(Span<EntryType> entries)
  {
    for (EntryType &entry : entries)
    {
      Hash const       hash         = m_hasher(entry.key);
      usize const      bucket_index = hash & (num_buckets() - 1);
      usize           &bucket_size  = m_bucket_sizes[bucket_index];
      EntryType *const dst =
          m_entries + (bucket_index << m_num_buckets_log2) + bucket_size;
      mem::relocate(&entry, dst, 1);
      bucket_size++;
      m_num_entries++;
    }
  }

  // double the number of buckets
  [[nodiscard]] bool grow_hash()
  {
    usize const num_buckets          = ((usize) 1) << m_num_buckets_log2;
    usize const new_num_buckets_log2 = m_num_buckets_log2 + 1;
    usize const new_num_buckets      = ((usize) 1) << new_num_buckets_log2;
    usize const bucket_capacity      = ((usize) 1) << m_bucket_capacity_log2;
    usize const entries_capacity     = num_buckets << m_bucket_capacity_log2;
    usize const new_entries_capacity = new_num_buckets
                                       << m_bucket_capacity_log2;
    usize const num_entries = m_num_entries;

    usize *new_bucket_sizes = m_allocator.reallocate_typed(
        m_bucket_sizes, m_bucket_sizes_capacity, new_num_buckets);
    if (new_bucket_sizes == nullptr)
    {
      return false;
    }

    m_bucket_sizes_capacity = new_num_buckets;
    m_bucket_sizes          = new_bucket_sizes;

    EntryType *entries_array =
        m_allocator.allocate_typed<EntryType>(num_entries);
    if (entries_array == nullptr)
    {
      return false;
    }

    {
      EntryType *src = m_entries;
      for (usize ibucket = 0, ientry = 0; ibucket < num_buckets; ibucket++)
      {
        usize const bucket_size = m_bucket_sizes[ibucket];
        mem::relocate(src, entries_array + ientry, bucket_size);
        ientry += bucket_size;
        src += bucket_capacity;
      }
    }

    mem::fill(m_bucket_sizes, m_bucket_sizes_capacity, 0);
    m_num_entries = 0;

    EntryType *new_entries = m_allocator.reallocate_typed(
        m_entries, m_entries_capacity, new_entries_capacity);

    if (new_entries == nullptr)
    {
      reinsert({entries_array, num_entries});
      m_allocator.deallocate_typed(entries_array, num_entries);
      return false;
    }

    m_num_buckets_log2 = new_num_buckets_log2;
    reinsert({entries_array, num_entries});
    m_allocator.deallocate_typed(entries_array, num_entries);
    return true;
  }

  [[nodiscard]] bool needs_rehash() const
  {
    // max load factor of .875
    // scale of 8, 8 * .875 = 7
    usize const load_factor = ((m_num_entries + 1) << 3) >> m_num_buckets_log2;
    return load_factor > 7;
  }

  [[nodiscard]] bool grow_buckets()
  {
    usize const new_bucket_capacity_log2 = m_bucket_capacity_log2 + 1;
    usize const new_bucket_capacity = ((usize) 1) << new_bucket_capacity_log2;
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] bool push_overwrite(bool *replaced, KeyArg &&key_arg,
                                    Args &&...value_args)
  {
    if (needs_rehash())
    {
      if (!grow_hash())
      {
        return false;
      }
    }

    Hash const  hash         = m_hasher(key_arg);
    usize const bucket_index = hash & (num_buckets() - 1);

    {
      usize &bucket_size = m_bucket_sizes[bucket_index];
      if (bucket_size < bucket_capacity())
      {
        EntryType *entry =
            m_entries + (bucket_index << m_bucket_capacity_log2) + bucket_size;
        new (entry) EntryType{.key{key_arg}, .value{((Args &&) value_args)...}};
        bucket_size++;
        m_num_entries++;
        return true;
      }
    }

    if (!grow_buckets())
    {
      return false;
    }

    {
      usize     &bucket_size = m_bucket_sizes[bucket_index];
      EntryType *entry =
          m_entries + (bucket_index << m_bucket_capacity_log2) + bucket_size;
      new (entry) EntryType{.key{key_arg}, .value{((Args &&) value_args)...}};
      bucket_size++;
      m_num_entries++;
    }

    return true;
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] bool push_no_overwrite(bool *exists, KeyArg &&key_arg,
                                       Args &&...value_args)
  {
  }

  // push if not exists

  KeyCmp        m_cmp                   = {};
  Hasher        m_hasher                = {};
  AllocatorImpl m_allocator             = default_allocator;
  usize         m_num_entries           = 0;
  usize         m_num_buckets_log2      = 0;
  usize         m_bucket_capacity_log2  = 0;
  usize        *m_bucket_sizes          = nullptr;
  EntryType    *m_entries               = nullptr;
  usize         m_entries_capacity      = 0;
  usize         m_bucket_sizes_capacity = 0;
};

template <typename V>
using StrDict = Dict<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash