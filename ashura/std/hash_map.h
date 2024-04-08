#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include <new>
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

  constexpr void reset()
  {
    clear();
    allocator_.deallocate_typed(probes_, num_probes_);
    allocator_.deallocate_typed(probe_distances_, num_probes_);
    probes_          = nullptr;
    probe_distances_ = nullptr;
    num_probes_      = 0;
  }

  constexpr void clear()
  {
    if constexpr (!TriviallyDestructible<Entry>)
    {
      for (usize i = 0; i < num_probes_; i++)
      {
        if (probe_distances_[i] != PROBE_SENTINEL)
        {
          (probes_ + i)->~Entry();
        }
      }
    }

    for (usize i = 0; i < num_probes_; i++)
    {
      probe_distances_[i] = PROBE_SENTINEL;
    }

    num_entries_        = 0;
    max_probe_distance_ = 0;
  }

  [[nodiscard]] constexpr V *operator[](K const &key) const
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return nullptr;
    }
    Hash const hash           = hasher_(key);
    usize      probe_index    = hash & (num_probes_ - 1);
    Distance   probe_distance = 0;
    while (probe_distance <= max_probe_distance_)
    {
      if (probe_distances_[probe_index] == PROBE_SENTINEL)
      {
        break;
      }
      Entry *probe = probes_ + probe_index;
      if (cmp_(probe->key, key))
      {
        return &probe->value;
      }
      probe_index = (probe_index + 1) & (num_probes_ - 1);
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
    // 3/4 => .75 load factor
    return num_probes == 0 || ((num_entries * 10ULL) / num_probes) > 6ULL;
  }

  constexpr void reinsert_(Span<Entry>          src_probes,
                           Span<Distance const> src_probe_distances)
  {
    for (usize src_probe_index = 0; src_probe_index < src_probes.size();
         src_probe_index++)
    {
      if (src_probe_distances[src_probe_index] != PROBE_SENTINEL)
      {
        Entry entry{(Entry &&) src_probes[src_probe_index]};
        src_probes[src_probe_index].~Entry();
        Hash     hash           = hasher_(entry.key);
        usize    probe_index    = hash & (num_probes_ - 1);
        Distance probe_distance = 0;
        while (true)
        {
          Entry    *dst_probe          = probes_ + probe_index;
          Distance *dst_probe_distance = probe_distances_ + probe_index;

          if (*dst_probe_distance == PROBE_SENTINEL)
          {
            new (dst_probe) Entry{(Entry &&) entry};
            *dst_probe_distance = probe_distance;
            break;
          }
          if (*dst_probe_distance < probe_distance)
          {
            swap(entry, *dst_probe);
            swap(probe_distance, *dst_probe_distance);
          }
          probe_distance++;
          probe_index = (probe_index + 1) & (num_probes_ - 1);
        }
        max_probe_distance_ = max(max_probe_distance_, probe_distance);
        num_entries_++;
      }
    }
  }

  constexpr bool rehash_()
  {
    usize  new_num_probes = (num_probes_ == 0) ? 1 : (num_probes_ * 2);
    Entry *new_probes     = allocator_.allocate_typed<Entry>(new_num_probes);
    if (new_probes == nullptr)
    {
      return false;
    }

    Distance *new_probe_distances =
        allocator_.allocate_typed<Distance>(new_num_probes);

    if (new_probe_distances == nullptr)
    {
      allocator_.deallocate_typed<Entry>(new_probes, new_num_probes);
      return false;
    }

    for (usize i = 0; i < new_num_probes; i++)
    {
      new_probe_distances[i] = PROBE_SENTINEL;
    }

    Entry    *old_probes          = probes_;
    Distance *old_probe_distances = probe_distances_;
    usize     old_num_probes      = num_probes_;
    probes_                       = new_probes;
    probe_distances_              = new_probe_distances;
    num_probes_                   = new_num_probes;
    num_entries_                  = 0;
    max_probe_distance_           = 0;

    reinsert_(Span{old_probes, old_num_probes},
              {old_probe_distances, old_num_probes});
    allocator_.deallocate_typed(old_probes, old_num_probes);
    allocator_.deallocate_typed(old_probe_distances, old_num_probes);
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
    usize      probe_index    = hash & (num_probes_ - 1);
    usize      insert_index   = USIZE_MAX;
    Distance   probe_distance = 0;
    Entry      entry{.key   = K{(KeyArg &&) key_arg},
                     .value = V{((Args &&) value_args)...}};

    while (true)
    {
      Entry    *dst_probe          = probes_ + probe_index;
      Distance *dst_probe_distance = probe_distances_ + probe_index;
      if (*dst_probe_distance == PROBE_SENTINEL)
      {
        insert_index        = probe_index;
        *dst_probe_distance = probe_distance;
        new (dst_probe) Entry{(Entry &&) entry};
        num_entries_++;
        break;
      }
      if (insert_index == USIZE_MAX && probe_distance <= max_probe_distance_ &&
          cmp_(entry.key, dst_probe->key))
      {
        exists       = true;
        insert_index = probe_index;
        if (replaced_uninit != nullptr)
        {
          new (replaced_uninit) V{(V &&) dst_probe->value};
          dst_probe->value = V{((Args &&) value_args)...};
        }
        break;
      }
      if (probe_distance > *dst_probe_distance)
      {
        swap(*dst_probe, entry);
        swap(*dst_probe_distance, probe_distance);
        if (insert_index == USIZE_MAX)
        {
          insert_index = probe_index;
        }
      }
      probe_index = (probe_index + 1) & (num_probes_ - 1);
      probe_distance++;
    }

    max_probe_distance_ = max(max_probe_distance_, probe_distance);
    return true;
  }

  constexpr void erase_probe_(usize erase_index)
  {
    usize probe_index = erase_index;
    do
    {
      usize     next_probe_index    = (probe_index + 1) & (num_probes_ - 1);
      Entry    *probe               = probes_ + probe_index;
      Entry    *next_probe          = probes_ + next_probe_index;
      Distance *probe_distance      = probe_distances_ + probe_index;
      Distance *next_probe_distance = probe_distances_ + next_probe_index;

      if (*next_probe_distance == 0 || *next_probe_distance == PROBE_SENTINEL)
      {
        *probe_distance = PROBE_SENTINEL;
        probe->~Entry();
        break;
      }

      *probe_distance      = *next_probe_distance - 1;
      *probe               = (Entry &&) *next_probe;
      *next_probe_distance = PROBE_SENTINEL;
      next_probe->~Entry();
      probe_index = next_probe_index;
    } while (probe_index != erase_index);
    num_entries_--;
  }

  constexpr bool erase(K const &key)
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return false;
    }
    Hash const hash           = hasher_(key);
    usize      probe_index    = hash & (num_probes_ - 1);
    Distance   probe_distance = 0;

    while (probe_distance <= max_probe_distance_)
    {
      Distance *dst_probe_distance = probe_distances_ + probe_index;
      if (*dst_probe_distance == PROBE_SENTINEL)
      {
        return false;
      }
      Entry *dst_probe = probes_ + probe_index;
      if (cmp_(dst_probe->key, key))
      {
        erase_probe_(probe_index);
        return true;
      }
      probe_index = (probe_index + 1) & (num_probes_ - 1);
      probe_distance++;
    }
    return false;
  }

  template <typename Fn>
  constexpr void for_each(Fn &&fn)
  {
    for (usize i = 0; i < num_probes_; i++)
    {
      if (probe_distances_[i] != PROBE_SENTINEL)
      {
        fn(probes_[i].key, probes_[i].value);
      }
    }
  }

  Hasher        hasher_{};
  KeyCmp        cmp_{};
  AllocatorImpl allocator_          = default_allocator;
  Entry        *probes_             = nullptr;
  Distance     *probe_distances_    = nullptr;
  usize         num_probes_         = 0;
  usize         num_entries_        = 0;
  Distance      max_probe_distance_ = 0;
};

template <typename V, typename D = u16>
using StrHashMap = HashMap<Span<char const>, V, StrHasher, StrEqual, u16>;

}        // namespace ash
