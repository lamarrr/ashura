#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
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
    return hash_bytes(str.as_u8());
  }
};

constexpr StrEqual  str_equal;
constexpr StrHasher str_hash;

template <typename K, typename V>
struct HashMapEntry
{
  using Key   = K;
  using Value = V;

  K key{};
  V value{};
};

/// Robin-hood open-address probing hashmap
template <typename K, typename V, typename H, typename KCmp, typename D>
struct HashMap
{
  using Key      = K;
  using Value    = V;
  using Hasher   = H;
  using KeyCmp   = KCmp;
  using Entry    = HashMapEntry<K, V>;
  using Distance = D;

  static constexpr Distance PROBE_SENTINEL = -1;

  Hasher        hasher_         = {};
  KeyCmp        cmp_            = {};
  AllocatorImpl allocator_      = default_allocator;
  Entry        *probes_         = nullptr;
  Distance     *probe_dists_    = nullptr;
  usize         num_probes_     = 0;
  usize         num_entries_    = 0;
  Distance      max_probe_dist_ = 0;

  constexpr void reset()
  {
    clear();
    allocator_.ndealloc(probes_, num_probes_);
    allocator_.ndealloc(probe_dists_, num_probes_);
    probes_      = nullptr;
    probe_dists_ = nullptr;
    num_probes_  = 0;
  }

  constexpr void uninit()
  {
    reset();
    allocator_ = default_allocator;
  }

  constexpr void clear()
  {
    if constexpr (!TriviallyDestructible<Entry>)
    {
      for (usize i = 0; i < num_probes_; i++)
      {
        if (probe_dists_[i] != PROBE_SENTINEL)
        {
          (probes_ + i)->~Entry();
        }
      }
    }

    for (usize i = 0; i < num_probes_; i++)
    {
      probe_dists_[i] = PROBE_SENTINEL;
    }

    num_entries_    = 0;
    max_probe_dist_ = 0;
  }

  [[nodiscard]] constexpr V *operator[](K const &key) const
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return nullptr;
    }
    Hash const hash       = hasher_(key);
    usize      probe_idx  = hash & (num_probes_ - 1);
    Distance   probe_dist = 0;
    while (probe_dist <= max_probe_dist_)
    {
      if (probe_dists_[probe_idx] == PROBE_SENTINEL)
      {
        break;
      }
      Entry *probe = probes_ + probe_idx;
      if (cmp_(probe->key, key))
      {
        return &probe->value;
      }
      probe_idx = (probe_idx + 1) & (num_probes_ - 1);
      probe_dist++;
    }
    return nullptr;
  }

  [[nodiscard]] constexpr bool has(K const &key) const
  {
    return (*this)[key] != nullptr;
  }

  static constexpr bool needs_rehash_(usize num_entries, usize num_probes)
  {
    // 6/10 => .6 load factor
    return num_probes == 0 || ((num_entries * 10ULL) / num_probes) > 6ULL;
  }

  constexpr void reinsert_(Entry *src_probes, Distance const *src_probe_dists,
                           usize n)
  {
    for (usize src_probe_idx = 0; src_probe_idx < n; src_probe_idx++)
    {
      if (src_probe_dists[src_probe_idx] != PROBE_SENTINEL)
      {
        Entry entry{(Entry &&) src_probes[src_probe_idx]};
        src_probes[src_probe_idx].~Entry();
        Hash     hash       = hasher_(entry.key);
        usize    probe_idx  = hash & (num_probes_ - 1);
        Distance probe_dist = 0;
        while (true)
        {
          Entry    *dst_probe      = probes_ + probe_idx;
          Distance *dst_probe_dist = probe_dists_ + probe_idx;

          if (*dst_probe_dist == PROBE_SENTINEL)
          {
            new (dst_probe) Entry{(Entry &&) entry};
            *dst_probe_dist = probe_dist;
            break;
          }
          if (*dst_probe_dist < probe_dist)
          {
            swap(entry, *dst_probe);
            swap(probe_dist, *dst_probe_dist);
          }
          probe_dist++;
          probe_idx = (probe_idx + 1) & (num_probes_ - 1);
        }
        max_probe_dist_ = max(max_probe_dist_, probe_dist);
        num_entries_++;
      }
    }
  }

  constexpr bool rehash_n_(usize new_num_probes)
  {
    Entry *new_probes;
    if (!allocator_.nalloc(new_num_probes, &new_probes))
    {
      return false;
    }

    Distance *new_probe_dists;
    if (!allocator_.nalloc(new_num_probes, &new_probe_dists))
    {
      allocator_.ndealloc(new_probes, new_num_probes);
      return false;
    }

    for (usize i = 0; i < new_num_probes; i++)
    {
      new_probe_dists[i] = PROBE_SENTINEL;
    }

    Entry    *old_probes      = probes_;
    Distance *old_probe_dists = probe_dists_;
    usize     old_num_probes  = num_probes_;
    probes_                   = new_probes;
    probe_dists_              = new_probe_dists;
    num_probes_               = new_num_probes;
    num_entries_              = 0;
    max_probe_dist_           = 0;

    reinsert_(old_probes, old_probe_dists, old_num_probes);
    allocator_.ndealloc(old_probes, old_num_probes);
    allocator_.ndealloc(old_probe_dists, old_num_probes);
    return true;
  }

  constexpr bool rehash_()
  {
    usize new_num_probes = (num_probes_ == 0) ? 1 : (num_probes_ * 2);
    return rehash_n_(new_num_probes);
  }

  constexpr bool reserve(usize n)
  {
    if (n <= num_probes_)
    {
      return true;
    }
    return rehash_n_(n);
  }

  template <typename KeyArg, typename... Args>
  [[nodiscard]] constexpr bool insert(bool &exists, V *replaced_uninit,
                                      KeyArg &&key_arg, Args &&...value_args)
  {
    exists = false;

    if (needs_rehash_(num_entries_ + 1, num_probes_) && !rehash_())
    {
      return false;
    }

    Hash const hash       = hasher_(key_arg);
    usize      probe_idx  = hash & (num_probes_ - 1);
    usize      insert_idx = USIZE_MAX;
    Distance   probe_dist = 0;
    Entry      entry{.key   = K{(KeyArg &&) key_arg},
                     .value = V{((Args &&) value_args)...}};

    while (true)
    {
      Entry    *dst_probe      = probes_ + probe_idx;
      Distance *dst_probe_dist = probe_dists_ + probe_idx;
      if (*dst_probe_dist == PROBE_SENTINEL)
      {
        insert_idx      = probe_idx;
        *dst_probe_dist = probe_dist;
        new (dst_probe) Entry{(Entry &&) entry};
        num_entries_++;
        break;
      }
      if (insert_idx == USIZE_MAX && probe_dist <= max_probe_dist_ &&
          cmp_(entry.key, dst_probe->key))
      {
        exists     = true;
        insert_idx = probe_idx;
        if (replaced_uninit != nullptr)
        {
          new (replaced_uninit) V{(V &&) dst_probe->value};
          dst_probe->value = (V &&) entry.value;
        }
        break;
      }
      if (probe_dist > *dst_probe_dist)
      {
        swap(*dst_probe, entry);
        swap(*dst_probe_dist, probe_dist);
        if (insert_idx == USIZE_MAX)
        {
          insert_idx = probe_idx;
        }
      }
      probe_idx = (probe_idx + 1) & (num_probes_ - 1);
      probe_dist++;
    }

    max_probe_dist_ = max(max_probe_dist_, probe_dist);
    return true;
  }

  constexpr void pop_probe_(usize pop_idx)
  {
    usize insert_idx = pop_idx;
    usize probe_idx  = (pop_idx + 1) & (num_probes_ - 1);
    while (probe_idx != pop_idx)
    {
      Entry    *probe      = probes_ + probe_idx;
      Distance *probe_dist = probe_dists_ + probe_idx;

      if (*probe_dist == 0 || *probe_dist == PROBE_SENTINEL)
      {
        break;
      }

      Entry    *insert_probe      = probes_ + insert_idx;
      Distance *insert_probe_dist = probe_dists_ + insert_idx;

      mem::relocate(probe, insert_probe, 1);
      *insert_probe_dist = *probe_dist - 1;
      *probe_dist        = PROBE_SENTINEL;
      probe_idx          = (probe_idx + 1) & (num_probes_ - 1);
      insert_idx         = (insert_idx + 1) & (num_probes_ - 1);
    }
  }

  constexpr bool erase(K const &key, V *erased_uninit = nullptr)
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return false;
    }
    Hash const hash       = hasher_(key);
    usize      probe_idx  = hash & (num_probes_ - 1);
    Distance   probe_dist = 0;

    while (probe_dist <= max_probe_dist_)
    {
      Distance *dst_probe_dist = probe_dists_ + probe_idx;
      if (*dst_probe_dist == PROBE_SENTINEL)
      {
        return false;
      }
      Entry *dst_probe = probes_ + probe_idx;
      if (cmp_(dst_probe->key, key))
      {
        if (erased_uninit != nullptr)
        {
          mem::relocate(&dst_probe->value, erased_uninit, 1);
          dst_probe->key.~K();
        }
        else
        {
          dst_probe->~Entry();
        }
        *dst_probe_dist = PROBE_SENTINEL;
        pop_probe_(probe_idx);
        num_entries_--;
        return true;
      }
      probe_idx = (probe_idx + 1) & (num_probes_ - 1);
      probe_dist++;
    }
    return false;
  }

  template <typename Fn>
  constexpr void for_each(Fn &&fn)
  {
    for (usize i = 0; i < num_probes_; i++)
    {
      if (probe_dists_[i] != PROBE_SENTINEL)
      {
        fn(probes_[i].key, probes_[i].value);
      }
    }
  }
};

template <typename V, typename D = u32>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual, u16>;

}        // namespace ash
