#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include "ashura/std/error.h"
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
    return hash_bytes(str.as_u8());
  }
};

constexpr StrEqual  str_equal;
constexpr StrHasher str_hash;

template <typename K, typename V>
struct HashMapEntry
{
  K key{};
  V value{};
};

template <typename K, typename V>
struct HashMapProbe
{
  usize              distance = -1;
  HashMapEntry<K, V> entry{};
};

/// Robin-hood open-address probing hashmap
template <typename K, typename V, typename Hasher, typename KeyCmp>
struct HashMap
{
  using KeyType    = K;
  using ValueType  = V;
  using HasherType = Hasher;
  using KeyCmpType = KeyCmp;
  using EntryType  = HashMapEntry<K, V>;
  using ProbeType  = HashMapProbe<K, V>;

  static constexpr usize PROBE_SENTINEL = -1;

  constexpr void reset()
  {
    clear();
    allocator_.deallocate_typed(probes_, num_probes_);
    probes_     = nullptr;
    num_probes_ = 0;
  }

  constexpr void clear()
  {
    for (ProbeType *probe = probes_; probe < probes_ + num_probes_; probe++)
    {
      if constexpr (!TriviallyDestructible<ProbeType>)
      {
        if (probe->distance != PROBE_SENTINEL)
        {
          probe->entry.~EntryType();
        }
      }
      probe->distance = PROBE_SENTINEL;
    }
    num_entries_ = 0;
  }

  [[nodiscard]] constexpr V *operator[](K const &key) const
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return nullptr;
    }
    Hash const hash           = hasher_(key);
    usize      probe_index    = hash % num_probes_;
    usize      probe_distance = 0;
    while (probe_distance <= max_probe_distance_)
    {
      ProbeType *probe = probes_ + probe_index;
      if (probe->distance == PROBE_SENTINEL)
      {
        break;
      }
      if (cmp_(probe->entry.key, key))
      {
        return &probe->entry.value;
      }
      probe_index = (probe_index + 1) % num_probes_;
      probe_distance++;
    }
    return nullptr;
  }

  [[nodiscard]] constexpr bool has(K const &key) const
  {
    return (*this)[key] != nullptr;
  }

  static constexpr bool needs_rehash_(usize num_entries, usize num_probes)
  {
    if (num_probes == 0)
    {
      return true;
    }
    // 7/8 => .875 load factor
    return ((num_entries * 8ULL) / num_probes) > 7ULL;
  }

  void reinsert_(Span<ProbeType> probes)
  {
    for (ProbeType &probe : probes)
    {
      if (probe.distance != PROBE_SENTINEL)
      {
        EntryType entry{(EntryType &&) probe.entry};
        probe.entry.~EntryType();
        Hash  hash           = hasher_(entry.key);
        usize probe_index    = hash % num_probes_;
        usize probe_distance = 0;
        while (true)
        {
          ProbeType *dst_probe = probes_ + probe_index;
          if (dst_probe->distance == PROBE_SENTINEL)
          {
            new (dst_probe) ProbeType{.distance = probe_distance,
                                      .entry    = (EntryType &&) entry};
            break;
          }
          if (dst_probe->distance < probe_distance)
          {
            swap(entry, dst_probe->entry);
            swap(probe_distance, dst_probe->distance);
          }
          probe_distance++;
          probe_index = (probe_index + 1) % num_probes_;
        }
        max_probe_distance_ = max(max_probe_distance_, probe_distance);
        num_entries_++;
      }
    }
  }

  bool rehash_()
  {
    usize      new_num_probes = (num_probes_ == 0) ? 1 : (num_probes_ * 2);
    ProbeType *new_probes =
        allocator_.allocate_typed<ProbeType>(new_num_probes);
    if (new_probes == nullptr)
    {
      return false;
    }

    for (ProbeType *probe = new_probes; probe < new_probes + new_num_probes;
         probe++)
    {
      probe->distance = PROBE_SENTINEL;
    }

    ProbeType *old_probes     = probes_;
    usize      old_num_probes = num_probes_;
    probes_                   = new_probes;
    num_probes_               = new_num_probes;
    num_entries_              = 0;
    max_probe_distance_       = 0;

    reinsert_(Span{old_probes, old_num_probes});
    allocator_.deallocate_typed(old_probes, old_num_probes);
    return true;
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] constexpr bool insert(bool &exists, V *replaced_uninit,
                                      KeyArg &&key_arg, Args &&...value_args)
  {
    exists = false;

    if (needs_rehash_(num_entries_ + 1, num_probes_))
    {
      if (!rehash_())
      {
        return false;
      }
    }

    Hash const hash           = hasher_(key_arg);
    usize      probe_index    = hash % num_probes_;
    usize      insert_index   = PROBE_SENTINEL;
    usize      probe_distance = 0;
    EntryType  entry{.key   = K{(KeyArg &&) key_arg},
                     .value = V{((Args &&) value_args)...}};

    while (true)
    {
      ProbeType *probe = probes_ + probe_index;
      if (probe->distance == PROBE_SENTINEL)
      {
        insert_index = probe_index;
        new (probe) ProbeType{.distance = probe_distance,
                              .entry    = (EntryType &&) entry};
        num_entries_++;
        break;
      }
      if (insert_index == PROBE_SENTINEL &&
          probe_distance <= max_probe_distance_ &&
          cmp_(entry.key, probe->entry.key))
      {
        exists       = true;
        insert_index = probe_index;
        if (replaced_uninit != nullptr)
        {
          new (replaced_uninit) V{(V &&) probe->entry.value};
          probe->entry.value = V{((Args &&) value_args)...};
        }
        break;
      }
      if (probe_distance > probe->distance)
      {
        swap(probe->entry, entry);
        swap(probe->distance, probe_distance);
        if (insert_index == PROBE_SENTINEL)
        {
          insert_index = probe_index;
        }
      }
      probe_index = (probe_index + 1) % num_probes_;
      probe_distance++;
    }

    max_probe_distance_ = max(max_probe_distance_, probe_distance);
    return true;
  }

  constexpr bool erase(K const &key)
  {
    CHECK(false);
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return false;
    }
    Hash const hash           = hasher_(key);
    usize      probe_index    = hash % num_probes_;
    usize      probe_distance = 0;

    while (probe_distance <= max_probe_distance_)
    {
      ProbeType *probe = probes_ + probe_index;
      if (probe->distance == PROBE_SENTINEL)
      {
        return false;
      }
      if (cmp_(probe->entry.key, key))
      {
        // TODO(lamarrr): need to wrap around???? and use indices instead
        ProbeType *shift_dst_it    = probe;
        ProbeType *shift_src_begin = probe + 1;
        ProbeType *shift_src_end   = shift_src_begin;
        while (shift_src_end < probes_ + num_probes_ &&
               shift_src_end->distance != 0 &&
               shift_src_end->distance != PROBE_SENTINEL)
        {
          shift_src_end++;
        }

        shift_dst_it->entry.~EntryType();
        shift_dst_it->distance = PROBE_SENTINEL;

        for (ProbeType *shift_src_it = shift_src_begin;
             shift_src_it < shift_src_end; shift_src_it++, shift_dst_it++)
        {
          new (&shift_dst_it->entry)
              EntryType{(EntryType &&) shift_src_it->entry};
          shift_dst_it->distance = shift_src_it->distance - 1;
        }

        num_entries_--;
        return true;
      }
      probe_index = (probe_index + 1) % num_probes_;
      probe_distance++;
    }
    return false;
  }

  Hasher        hasher_{};
  KeyCmp        cmp_{};
  AllocatorImpl allocator_          = default_allocator;
  ProbeType    *probes_             = nullptr;
  usize         num_probes_         = 0;
  usize         num_entries_        = 0;
  usize         max_probe_distance_ = 0;
};

template <typename V>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual>;

}        // namespace ash
