#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include <iostream>
#include <string.h>

#define HALT_IF()
#define PRINT_DEBUG(IDENTIFIER) \
  std::cout << #IDENTIFIER << ": " << IDENTIFIER << std::endl

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
  using KeyType                                   = K;
  using ValueType                                 = V;
  using EntryType                                 = DictEntry<K, V>;
  using HasherType                                = Hasher;
  using KeyCmpType                                = KeyCmp;
  static constexpr usize INITIAL_NUM_BUCKETS_LOG2 = 1;

  usize num_buckets() const
  {
    return ((usize) 1) << num_buckets_log2_;
  }

  usize num_entries() const
  {
    return num_entries_;
  }

  usize entries_capacity() const
  {
    return num_buckets() * bucket_capacity_;
  }

  usize bucket_capacity() const
  {
    return bucket_capacity_;
  }

  [[nodiscard]] bool init()
  {
    static constexpr usize INITIAL_NUM_BUCKETS = 1 << INITIAL_NUM_BUCKETS_LOG2;

    usize *bucket_sizes =
        allocator_.allocate_zeroed_typed<usize>(INITIAL_NUM_BUCKETS);

    if (bucket_sizes == nullptr)
    {
      return false;
    }

    num_entries_             = 0;
    num_buckets_log2_        = INITIAL_NUM_BUCKETS_LOG2;
    bucket_capacity_         = 0;
    p_bucket_sizes_          = bucket_sizes;
    p_entries_               = nullptr;
    p_entries_capacity_      = 0;
    p_bucket_sizes_capacity_ = INITIAL_NUM_BUCKETS;
    return true;
  }

  void destroy()
  {
    clear();
    allocator_.deallocate_typed(p_entries_, p_entries_capacity_);
    allocator_.deallocate_typed(p_bucket_sizes_, p_bucket_sizes_capacity_);
    p_entries_capacity_      = 0;
    p_bucket_sizes_capacity_ = 0;
    num_buckets_log2_        = 0;
    bucket_capacity_         = 0;
  }

  void clear()
  {
    if constexpr (!TriviallyDestructible<EntryType>)
    {
      for (usize ibucket = 0; ibucket < num_buckets(); ibucket++)
      {
        usize const bucket_size = p_bucket_sizes_[ibucket];
        EntryType  *bucket_it   = p_entries_ + ibucket * bucket_capacity_;
        for (usize i = 0; i < bucket_size; i++, bucket_it++)
        {
          bucket_it->~EntryType();
        }
      }
    }
    mem::zero(p_bucket_sizes_, num_buckets());
    num_entries_ = 0;
  }

  [[nodiscard]] V *operator[](K const &key) const
  {
    Hash const  hash         = hasher_(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize const bucket_size  = p_bucket_sizes_[bucket_index];
    EntryType  *bucket_it    = p_entries_ + bucket_index * bucket_capacity_;
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (cmp_(bucket_it->key, key))
      {
        return &bucket_it->value;
      }
    }
    return nullptr;
  }

  bool remove(K const &key)
  {
    Hash const  hash         = hasher_(key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize      &bucket_size  = p_bucket_sizes_[bucket_index];
    EntryType  *bucket_it    = p_entries_ + bucket_index * bucket_capacity_;
    for (usize i = 0; i < bucket_size; i++, bucket_it++)
    {
      if (cmp_(bucket_it->key, key))
      {
        if (i != bucket_size - 1)
        {
          swap(bucket_it[i], bucket_it[bucket_size - 1]);
        }
        (bucket_it + bucket_size - 1)->~T();
        bucket_size--;
        num_entries_--;
        return true;
      }
    }
    return false;
  }

  // reinsert temporarily relocated elements
  void reinsert_(EntryType *entry)
  {
    Hash const  hash         = hasher_(entry->key);
    usize const bucket_index = hash & (num_buckets() - 1);
    usize      &bucket_size  = p_bucket_sizes_[bucket_index];
    EntryType *dst = p_entries_ + bucket_index * bucket_capacity_ + bucket_size;
    mem::relocate(entry, dst, 1);
    bucket_size++;
    num_entries_++;
  }

  void reinsert_(EntryType *entries, usize num_insert)
  {
    for (usize i = 0; i < num_insert; i++)
    {
      reinsert_(entries + i);
    }
  }

  // double the number of buckets
  [[nodiscard]] bool increase_buckets()
  {
    usize const num_buckets          = ((usize) 1) << num_buckets_log2_;
    usize const new_num_buckets_log2 = num_buckets_log2_ + 1;
    usize const new_num_buckets      = ((usize) 1) << new_num_buckets_log2;
    usize const bucket_capacity      = bucket_capacity_;
    usize const new_entries_capacity = new_num_buckets * bucket_capacity_;
    usize const num_entries          = num_entries_;

    usize *new_bucket_sizes = allocator_.reallocate_typed(
        p_bucket_sizes_, p_bucket_sizes_capacity_, new_num_buckets);
    if (new_bucket_sizes == nullptr)
    {
      return false;
    }

    p_bucket_sizes_capacity_ = new_num_buckets;
    p_bucket_sizes_          = new_bucket_sizes;

    EntryType *entries_array =
        allocator_.allocate_typed<EntryType>(num_entries);
    if (entries_array == nullptr)
    {
      return false;
    }

    {
      EntryType *src = p_entries_;
      for (usize ibucket = 0, ientry = 0; ibucket < num_buckets; ibucket++)
      {
        usize const bucket_size = p_bucket_sizes_[ibucket];
        mem::relocate(src, entries_array + ientry, bucket_size);
        ientry += bucket_size;
        src += bucket_capacity;
      }
    }

    mem::zero(p_bucket_sizes_, p_bucket_sizes_capacity_);
    num_entries_ = 0;

    EntryType *new_entries = allocator_.reallocate_typed(
        p_entries_, p_entries_capacity_, new_entries_capacity);

    if (new_entries == nullptr)
    {
      reinsert_(entries_array, num_entries);
      allocator_.deallocate_typed(entries_array, num_entries);
      return false;
    }

    p_entries_          = new_entries;
    p_entries_capacity_ = new_entries_capacity;
    num_buckets_log2_   = new_num_buckets_log2;

    reinsert_(entries_array, num_entries);
    allocator_.deallocate_typed(entries_array, num_entries);
    return true;
  }

  [[nodiscard]] static constexpr bool needs_rehash(usize num_entries,
                                                   usize num_buckets_log2)
  {
    // max load factor of .875, scale of 8, 8 * .875 = 7
    usize load_factor = (num_entries << 3) >> num_buckets_log2;
    return load_factor > 7;
  }

  [[nodiscard]] bool increase_bucket_slots()
  {
    usize const bucket_capacity         = bucket_capacity_;
    usize const target_bucket_capacity  = bucket_capacity_ + 1;
    usize const target_entries_capacity = target_bucket_capacity
                                          << num_buckets_log2_;

    if constexpr (TriviallyRelocatable<EntryType>)
    {
      EntryType *new_entries = allocator_.reallocate_typed(
          p_entries_, p_entries_capacity_, target_entries_capacity);
      if (new_entries == nullptr)
      {
        return false;
      }

      p_entries_          = new_entries;
      p_entries_capacity_ = target_entries_capacity;
      bucket_capacity_    = target_bucket_capacity;

      for (usize i = num_buckets(); i != 0;)
      {
        i--;
        usize from = i * bucket_capacity;
        usize to   = i * target_bucket_capacity;
        mem::relocate(p_entries_ + from, p_entries_ + to, p_bucket_sizes_[i]);
      }

      return true;
    }
    else
    {
      EntryType *new_entries =
          allocator_.allocate_typed<EntryType>(target_entries_capacity);

      if (new_entries == nullptr)
      {
        return false;
      }

      for (usize i = num_buckets(); i != 0;)
      {
        i--;
        usize from = i * bucket_capacity;
        usize to   = i * target_bucket_capacity;
        mem::relocate(p_entries_ + from, new_entries + to, p_bucket_sizes_[i]);
      }

      allocator_.deallocate_typed(p_entries_, p_entries_capacity_);

      p_entries_          = new_entries;
      p_entries_capacity_ = target_entries_capacity;
      bucket_capacity_    = target_bucket_capacity;
      return true;
    }
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] bool insert(ValueType *overwritten, KeyArg &&key_arg,
                            Args &&...value_args)
  {
    Hash const hash = hasher_(key_arg);

    {
      usize const bucket_index = hash & (num_buckets() - 1);
      usize const bucket_size  = p_bucket_sizes_[bucket_index];
      EntryType  *bucket_it    = p_entries_ + bucket_index * bucket_capacity_;
      for (usize i = 0; i < bucket_size; i++, bucket_it++)
      {
        if (cmp_(bucket_it->key, key_arg))
        {
          if (overwritten != nullptr)
          {
            *overwritten     = (ValueType &&) bucket_it->value;
            bucket_it->value = ValueType{((Args &&) value_args)...};
          }
          return true;
        }
      }
    }

    if (needs_rehash(num_entries_ + 1, num_buckets_log2_) &&
        !increase_buckets())
    {
      return false;
    }

    usize const bucket_index = hash & (num_buckets() - 1);
    usize      &bucket_size  = p_bucket_sizes_[bucket_index];

    if (((bucket_size + 1) > bucket_capacity()) && !increase_bucket_slots())
    {
      return false;
    }

    EntryType *entry =
        p_entries_ + bucket_index * bucket_capacity_ + bucket_size;
    new (entry) EntryType{.key{key_arg}, .value{((Args &&) value_args)...}};

    bucket_size++;
    num_entries_++;
    return true;
  }

  KeyCmp        cmp_{};
  Hasher        hasher_{};
  AllocatorImpl allocator_               = default_allocator;
  usize         num_entries_             = 0;
  usize         num_buckets_log2_        = 0;
  usize         bucket_capacity_         = 0;
  usize        *p_bucket_sizes_          = nullptr;
  EntryType    *p_entries_               = nullptr;
  usize         p_entries_capacity_      = 0;
  usize         p_bucket_sizes_capacity_ = 0;
};

template <typename V>
using StrDict = Dict<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash