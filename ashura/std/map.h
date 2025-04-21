/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/hash.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

template <typename K, typename V>
struct MapEntry
{
  using Key   = K;
  using Value = V;

  K key{};
  V value{};
};

/// @brief Robin-hood open-address probing HashMap
/// @tparam K key type
/// @tparam V value type
/// @tparam H key hasher functor type
/// @tparam KCmp key comparator type
/// @tparam D unsigned integer to use to encode probe distances, should be same
/// or larger than usize
/// @details the Map doesn't use any divide operation
template <typename K, typename V, typename H, typename KCmp, typename D = usize>
struct [[nodiscard]] Map
{
  using Key      = K;
  using Value    = V;
  using Hasher   = H;
  using KeyCmp   = KCmp;
  using Entry    = MapEntry<K, V>;
  using Distance = D;

  static constexpr Distance PROBE_SENTINEL = -1;

  /// @brief always pointing to a valid element or one past the end of the map
  struct Iter
  {
    Distance *       iter_  = nullptr;
    Distance const * end_   = nullptr;
    Entry *          probe_ = nullptr;

    /// @brief seek the next non-empty probe, this iterator inclusive
    constexpr void seek()
    {
      while (iter_ != end_)
      {
        if (*iter_ != PROBE_SENTINEL)
        {
          break;
        }
        ++iter_;
        ++probe_;
      }
    }

    constexpr Iter & operator++()
    {
      // advancement past the current element must occur
      ++iter_;
      ++probe_;

      seek();

      return *this;
    }

    constexpr Entry & operator*() const
    {
      return *probe_;
    }

    constexpr bool operator!=(IterEnd) const
    {
      return iter_ != end_;
    }
  };

  struct View
  {
    Distance *       iter_  = nullptr;
    Distance const * end_   = nullptr;
    Entry *          probe_ = nullptr;

    constexpr auto begin() const
    {
      return Iter{.iter_ = iter_, .end_ = end_, .probe_ = probe_};
    }

    constexpr auto end() const
    {
      return IterEnd{};
    }
  };

  Distance *   probe_dists_;
  Entry *      probes_;
  usize        num_probes_;
  usize        num_entries_;
  Distance     max_probe_dist_;
  AllocatorRef allocator_;
  Hasher       hasher_;
  KeyCmp       cmp_;

  constexpr Map(AllocatorRef allocator = {}, Hasher hasher = {},
                KeyCmp cmp = {}) :
    probe_dists_{nullptr},
    probes_{nullptr},
    num_probes_{0},
    num_entries_{0},
    max_probe_dist_{0},
    allocator_{allocator},
    hasher_{static_cast<Hasher &&>(hasher)},
    cmp_{static_cast<KeyCmp &&>(cmp)}
  {
  }

  constexpr Map(Map const &) = delete;

  constexpr Map & operator=(Map const &) = delete;

  constexpr Map(Map && other) :
    probe_dists_{other.probe_dists_},
    probes_{other.probes_},
    num_probes_{other.num_probes_},
    num_entries_{other.num_entries_},
    max_probe_dist_{other.max_probe_dist_},
    allocator_{other.allocator_},
    hasher_{static_cast<Hasher &&>(other.hasher_)},
    cmp_{static_cast<KeyCmp &&>(other.cmp_)}
  {
    other.probe_dists_    = nullptr;
    other.probes_         = nullptr;
    other.num_probes_     = 0;
    other.num_entries_    = 0;
    other.max_probe_dist_ = 0;
    other.allocator_      = default_allocator;
    other.hasher_         = {};
    other.cmp_            = {};
  }

  constexpr Map & operator=(Map && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) Map{static_cast<Map &&>(other)};
    return *this;
  }

  constexpr ~Map()
  {
    uninit();
  }

  constexpr void destruct_probes__()
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
  }

  constexpr void uninit()
  {
    destruct_probes__();
    allocator_->ndealloc(num_probes_, probe_dists_);
    allocator_->ndealloc(num_probes_, probes_);
  }

  constexpr void reset()
  {
    uninit();
    probe_dists_    = nullptr;
    probes_         = nullptr;
    num_probes_     = 0;
    num_entries_    = 0;
    max_probe_dist_ = 0;
    allocator_      = {};
  }

  constexpr void clear()
  {
    destruct_probes__();

    for (usize i = 0; i < num_probes_; i++)
    {
      probe_dists_[i] = PROBE_SENTINEL;
    }

    num_entries_    = 0;
    max_probe_dist_ = 0;
  }

  constexpr bool is_empty() const
  {
    return num_entries_ == 0;
  }

  constexpr usize size() const
  {
    return num_entries_;
  }

  constexpr u32 size32() const
  {
    return (u32) num_entries_;
  }

  constexpr u64 size64() const
  {
    return (u64) num_entries_;
  }

  constexpr usize capacity() const
  {
    return num_probes_;
  }

  [[nodiscard]] constexpr OptionRef<V> try_get(auto const & key,
                                               hash64       hash) const
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return none;
    }

    usize    probe_idx  = hash & (num_probes_ - 1);
    Distance probe_dist = 0;

    while (probe_dist <= max_probe_dist_)
    {
      if (probe_dists_[probe_idx] == PROBE_SENTINEL)
      {
        break;
      }

      Entry * probe = probes_ + probe_idx;

      if (cmp_(probe->key, key))
      {
        return probe->value;
      }

      probe_idx = (probe_idx + 1) & (num_probes_ - 1);
      probe_dist++;
    }

    return none;
  }

  [[nodiscard]] constexpr OptionRef<V> try_get(auto const & key) const
  {
    hash64 const hash = hasher_(key);
    return try_get(key, hash);
  }

  [[nodiscard]] constexpr V & operator[](auto const & key) const
  {
    return try_get(key).unwrap();
  }

  [[nodiscard]] constexpr bool has(auto const & key) const
  {
    return try_get(key).is_some();
  }

  [[nodiscard]] constexpr bool has(auto const & key, hash64 hash) const
  {
    return try_get(key, hash).is_some();
  }

  static constexpr bool needs_rehash_(usize num_entries, usize num_probes)
  {
    /// 75% load factor
    return (num_entries + (num_entries >> 2)) >= num_probes;
  }

  constexpr void reinsert_(Entry * src_probes, Distance const * src_probe_dists,
                           usize n)
  {
    for (usize src_probe_idx = 0; src_probe_idx < n; src_probe_idx++)
    {
      if (src_probe_dists[src_probe_idx] != PROBE_SENTINEL)
      {
        Entry entry{static_cast<Entry &&>(src_probes[src_probe_idx])};
        src_probes[src_probe_idx].~Entry();
        hash64   hash       = hasher_(entry.key);
        usize    probe_idx  = hash & (num_probes_ - 1);
        Distance probe_dist = 0;

        while (true)
        {
          Entry *    dst_probe      = probes_ + probe_idx;
          Distance * dst_probe_dist = probe_dists_ + probe_idx;

          if (*dst_probe_dist == PROBE_SENTINEL)
          {
            new (dst_probe) Entry{static_cast<Entry &&>(entry)};
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
    Distance * new_probe_dists;

    if (!allocator_->nalloc(new_num_probes, new_probe_dists))
    {
      return false;
    }

    Entry * new_probes;

    if (!allocator_->nalloc(new_num_probes, new_probes))
    {
      allocator_->ndealloc(new_num_probes, new_probe_dists);
      return false;
    }

    for (usize i = 0; i < new_num_probes; i++)
    {
      new_probe_dists[i] = PROBE_SENTINEL;
    }

    Entry *    old_probes      = probes_;
    Distance * old_probe_dists = probe_dists_;
    usize      old_num_probes  = num_probes_;
    probes_                    = new_probes;
    probe_dists_               = new_probe_dists;
    num_probes_                = new_num_probes;
    num_entries_               = 0;
    max_probe_dist_            = 0;

    reinsert_(old_probes, old_probe_dists, old_num_probes);
    allocator_->ndealloc(old_num_probes, old_probe_dists);
    allocator_->ndealloc(old_num_probes, old_probes);
    return true;
  }

  constexpr bool rehash_()
  {
    usize new_num_probes = (num_probes_ == 0) ? 1 : (num_probes_ << 1);
    return rehash_n_(new_num_probes);
  }

  /// @brief Insert a new entry into the Map
  /// @param exists set to true if the object already exists
  /// @param replaced if true, the original value is replaced if it exists,
  /// otherwise the entry is added
  /// @return The inserted or existing value if the insert was successful
  /// without a memory allocation error, otherwise an Err
  [[nodiscard]] constexpr Result<Tuple<K &, V &>>
    insert(K key, V value, bool * exists = nullptr, bool replace = true)
  {
    if (exists != nullptr)
    {
      *exists = false;
    }

    if (needs_rehash_(num_entries_ + 1, num_probes_) && !rehash_()) [[unlikely]]
    {
      return Err{};
    }

    hash64 const hash       = hasher_(key);
    usize        probe_idx  = hash & (num_probes_ - 1);
    usize        insert_idx = USIZE_MAX;
    Distance     probe_dist = 0;
    Entry entry{.key{static_cast<K &&>(key)}, .value{static_cast<V &&>(value)}};

    while (true)
    {
      Entry *    dst_probe      = probes_ + probe_idx;
      Distance * dst_probe_dist = probe_dists_ + probe_idx;

      if (*dst_probe_dist == PROBE_SENTINEL)
      {
        insert_idx      = probe_idx;
        *dst_probe_dist = probe_dist;
        new (dst_probe) Entry{static_cast<Entry &&>(entry)};
        num_entries_++;
        break;
      }

      if (insert_idx == USIZE_MAX && probe_dist <= max_probe_dist_ &&
          cmp_(entry.key, dst_probe->key))
      {
        insert_idx = probe_idx;
        if (exists != nullptr)
        {
          *exists = true;
        }
        if (replace)
        {
          dst_probe->value = static_cast<V &&>(entry.value);
        }
        break;
      }

      if (probe_dist > *dst_probe_dist) [[unlikely]]
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
    Entry * probe   = probes_ + insert_idx;
    return Ok{
      Tuple<K &, V &>{probe->key, probe->value}
    };
  }

  constexpr void pop_probe_(usize pop_idx)
  {
    usize insert_idx = pop_idx;
    usize probe_idx  = (pop_idx + 1) & (num_probes_ - 1);

    while (probe_idx != pop_idx)
    {
      Entry *    probe      = probes_ + probe_idx;
      Distance * probe_dist = probe_dists_ + probe_idx;

      if (*probe_dist == 0 || *probe_dist == PROBE_SENTINEL)
      {
        break;
      }

      Entry *    insert_probe      = probes_ + insert_idx;
      Distance * insert_probe_dist = probe_dists_ + insert_idx;

      obj::relocate_nonoverlapping(Span{probe, 1}, insert_probe);

      *insert_probe_dist = *probe_dist - 1;
      *probe_dist        = PROBE_SENTINEL;
      probe_idx          = (probe_idx + 1) & (num_probes_ - 1);
      insert_idx         = (insert_idx + 1) & (num_probes_ - 1);
    }
  }

  constexpr bool erase(auto const & key)
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return false;
    }

    hash64   hash       = hasher_(key);
    usize    probe_idx  = hash & (num_probes_ - 1);
    Distance probe_dist = 0;

    while (probe_dist <= max_probe_dist_)
    {
      Distance * dst_probe_dist = probe_dists_ + probe_idx;

      if (*dst_probe_dist == PROBE_SENTINEL)
      {
        return false;
      }

      Entry * dst_probe = probes_ + probe_idx;

      if (cmp_(dst_probe->key, key))
      {
        dst_probe->~Entry();
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

  constexpr View view() const
  {
    Iter iter{.iter_  = probe_dists_,
              .end_   = probe_dists_ + num_probes_,
              .probe_ = probes_};

    iter.seek();

    return View{.iter_ = iter.iter_, .end_ = iter.end_, .probe_ = iter.probe_};
  }

  constexpr Iter begin() const
  {
    return view().begin();
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

template <typename K, typename V, typename H, typename KCmp, typename D>
struct IsTriviallyRelocatable<Map<K, V, H, KCmp, D>>
{
  static constexpr bool value =
    TriviallyRelocatable<H> && TriviallyRelocatable<KCmp>;
};

template <typename V, typename D = usize>
using StrMap = Map<Str, V, SpanHash, StrEq, D>;

template <typename V, typename D = usize>
using StrVecMap = Map<Vec<char>, V, SpanHash, StrEq, D>;

template <typename K, typename V, typename D = usize>
using BitMap = Map<K, V, BitHash, BitEq, D>;

}    // namespace ash
