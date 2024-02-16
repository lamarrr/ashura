#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"
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
  using KeyType                                    = K;
  using ValueType                                  = V;
  using EntryType                                  = DictEntry<K, V>;
  using HasherType                                 = Hasher;
  using KeyCmpType                                 = KeyCmp;
  static constexpr usize INIT_BUCKET_CAPACITY_LOG2 = 8;
  static constexpr usize INIT_NUM_BUCKETS_LOG2     = 2;

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
    // todo(lamarrr):
    m_num_buckets_log2     = INIT_NUM_BUCKETS_LOG2;
    m_bucket_capacity_log2 = INIT_BUCKET_CAPACITY_LOG2;
  }

  void destroy()
  {
    clear();
    m_allocator.deallocate_typed(m_p_entries, m_p_entries_capacity);
    m_allocator.deallocate_typed(m_p_bucket_sizes, m_p_bucket_sizes_capacity);
    m_p_entries_capacity      = 0;
    m_p_bucket_sizes_capacity = 0;
    m_num_buckets_log2        = 0;
    m_bucket_capacity_log2    = 0;
  }

  void clear()
  {
    if constexpr (!TriviallyDestructible<EntryType>)
    {
      for (usize ibucket = 0; ibucket < num_buckets(); ibucket++)
      {
        usize const bucket_size = m_bucket_sizes[ibucket];
        EntryType  *bucket_it =
            m_p_entries + (ibucket << m_bucket_capacity_log2);
        for (usize i = 0; i < bucket_size; i++, bucket_it++)
        {
          bucket_it->~EntryType();
        }
      }
    }
    mem::fill(m_p_bucket_sizes, num_buckets(), 0);
    m_num_entries = 0;
  }

  [[nodiscard]] V *operator[](K const &key) const
  {
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize const bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it =
        m_p_entries + (bucket_index << m_bucket_capacity_log2);
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (m_cmp(bucket_it->key, key))
      {
        return &bucket_it->value;
      }
    }
    return nullptr;
  }

  bool remove(K const &key)
  {
    Hash const  hash         = hasher(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize      &bucket_size  = m_bucket_sizes[bucket_index];
    EntryType  *bucket_it =
        m_p_entries + (bucket_index << m_bucket_capacity_log2);
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
        return true;
      }
    }
    return false;
  }

  // relocate elements relocated to a temporary array to the hash map.
  void reinsert(Span<EntryType> entries)
  {
    for (EntryType &entry : entries)
    {
      Hash const       hash         = m_hasher(entry.key);
      usize const      bucket_index = hash & (num_buckets() - 1);
      usize           &bucket_size  = m_bucket_sizes[bucket_index];
      EntryType *const dst =
          m_p_entries + (bucket_index << m_num_buckets_log2) + bucket_size;
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
        m_p_bucket_sizes, m_p_bucket_sizes_capacity, new_num_buckets);
    if (new_bucket_sizes == nullptr)
    {
      return false;
    }

    m_p_bucket_sizes_capacity = new_num_buckets;
    m_p_bucket_sizes          = new_bucket_sizes;

    EntryType *entries_array =
        m_allocator.allocate_typed<EntryType>(num_entries);
    if (entries_array == nullptr)
    {
      return false;
    }

    {
      EntryType *src = m_p_entries;
      for (usize ibucket = 0, ientry = 0; ibucket < num_buckets; ibucket++)
      {
        usize const bucket_size = m_bucket_sizes[ibucket];
        mem::relocate(src, entries_array + ientry, bucket_size);
        ientry += bucket_size;
        src += bucket_capacity;
      }
    }

    mem::fill(m_p_bucket_sizes, m_p_bucket_sizes_capacity, 0);
    m_num_entries = 0;

    EntryType *new_entries = m_allocator.reallocate_typed(
        m_p_entries, m_p_entries_capacity, new_entries_capacity);

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
    usize const bucket_capacity_log2     = m_bucket_capacity_log2;
    usize const bucket_capacity          = ((usize) 1) << bucket_capacity_log2;
    usize const new_bucket_capacity_log2 = bucket_capacity_log2 + 1;
    usize const new_bucket_capacity = ((usize) 1) << new_bucket_capacity_log2;
    usize const new_entries_capacity =
        ((usize) 1) << (m_num_buckets_log2 + bucket_capacity_log2);

    if constexpr (TriviallyRelocatable<EntryType>)
    {
      EntryType *new_entries = m_allocator.reallocate_typed(
          m_p_entries, m_p_entries_capacity, new_entries_capacity);
      if (new_entries == nullptr)
      {
        return false;
      }

      for (usize i = num_buckets(); i != 0; i--)
      {
        mem::relocate(m_p_entries + (i << bucket_capacity_log2),
                      m_p_entries + (i << new_bucket_capacity_log2),
                      m_p_bucket_sizes[i]);
      }

      m_p_entries            = new_entries;
      m_p_entries_capacity   = new_entries_capacity;
      m_bucket_capacity_log2 = new_bucket_capacity_log2;
      return true;
    }
    else
    {
      EntryType *new_entries =
          m_allocator.allocate_typed<EntryType>(new_entries_capacity);

      if (new_entries == nullptr)
      {
        return false;
      }

      for (usize i = 0; i < num_buckets(); i++)
      {
        mem::relocate(m_p_entries + (i << bucket_capacity_log2),
                      new_entries + (i << new_bucket_capacity_log2),
                      m_p_bucket_sizes[i]);
      }

      m_allocator.deallocate_typed(m_p_entries, m_p_entries_capacity);

      m_p_entries            = new_entries;
      m_p_entries_capacity   = new_entries_capacity;
      m_bucket_capacity_log2 = new_bucket_capacity_log2;
      return true;
    }
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] bool push_overwrite(bool *replaced, KeyArg &&key_arg,
                                    Args &&...value_args)
  {
    *replaced = false;
    // TODO(lamarrr): abstract this for no-overwrite
    if (needs_rehash())
    {
      if (!grow_hash())
      {
        return false;
      }
    }

    Hash const  hash         = m_hasher(key_arg);
    usize const bucket_index = hash & (num_buckets() - 1);

    // TODO(lamarrr): replace if exists

    {
      usize &bucket_size = m_bucket_sizes[bucket_index];
      if (bucket_size < bucket_capacity())
      {
        EntryType *entry = m_p_entries +
                           (bucket_index << m_bucket_capacity_log2) +
                           bucket_size;
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
          m_p_entries + (bucket_index << m_bucket_capacity_log2) + bucket_size;
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
    *exists = false;
  }

  // push if not exists

  KeyCmp        m_cmp{};
  Hasher        m_hasher{};
  AllocatorImpl m_allocator               = default_allocator;
  usize         m_num_entries             = 0;
  usize         m_num_buckets_log2        = 0;
  usize         m_bucket_capacity_log2    = 0;
  usize        *m_p_bucket_sizes          = nullptr;
  EntryType    *m_p_entries               = nullptr;
  usize         m_p_entries_capacity      = 0;
  usize         m_p_bucket_sizes_capacity = 0;
};

template <typename V>
using StrDict = Dict<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash