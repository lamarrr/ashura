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
struct DictEntry
{
  using Key   = K;
  using Value = V;

  Key   key{};
  Value value{};

  template <typename KeyArg, typename ValueArg>
  DictEntry(KeyArg && key, ValueArg && value) :
    key{static_cast<Key &&>(key)},
    value{static_cast<ValueArg &&>(value)}
  {
  }
};

template <typename K>
struct DictEntry<K, Void>
{
  using Key   = K;
  using Value = Void const;

  Key                    key{};
  static constexpr Value value{};

  template <typename KeyArg, typename ValueArg>
  DictEntry(KeyArg && key, ValueArg &&) : key{static_cast<KeyArg &&>(key)}
  {
  }

  template <typename KeyArg>
  DictEntry(KeyArg && key) : key{static_cast<KeyArg &&>(key)}
  {
  }
};

// [ ] default-hash, default-cmp

/// @brief Robin-hood open-address probing HashMap
/// @tparam K key type
/// @tparam V value type
/// @tparam H key hasher functor type
/// @tparam KCmp key comparator type
/// @tparam D unsigned integer to use to encode probe distances, should be same
/// or larger than usize
/// @details the Dict doesn't use any divide operation
template <typename K, typename V, typename H, typename KCmp, typename D = usize>
struct [[nodiscard]] Dict
{
  using Entry    = DictEntry<K, V>;
  using Key      = typename Entry::Key;
  using Value    = typename Entry::Value;
  using Hasher   = H;
  using KeyCmp   = KCmp;
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

    constexpr auto operator*() const
    {
      return Tuple<Key const &, Value &>{probe_->key, probe_->value};
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

  constexpr Dict(AllocatorRef allocator = {}, Hasher hasher = {},
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

  constexpr Dict(Dict const &) = delete;

  constexpr Dict & operator=(Dict const &) = delete;

  constexpr Dict(Dict && other) :
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

  constexpr Dict & operator=(Dict && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) Dict{static_cast<Dict &&>(other)};
    return *this;
  }

  constexpr ~Dict()
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

  constexpr usize capacity() const
  {
    return num_probes_;
  }

  [[nodiscard]] constexpr Option<Value &> try_get(auto const & key,
                                                  usize        hash) const
  {
    if (num_probes_ == 0 || num_entries_ == 0)
    {
      return none;
    }

    auto     probe_idx  = hash & (num_probes_ - 1);
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

  [[nodiscard]] constexpr Option<Value &> try_get(auto const & key) const
  {
    auto const hash = hasher_(key);
    return try_get(key, hash);
  }

  [[nodiscard]] constexpr Value & get(auto const & key) const
  {
    return try_get(key).unwrap();
  }

  [[nodiscard]] constexpr Value & operator[](auto const & key) const
  {
    return get(key);
  }

  [[nodiscard]] constexpr bool has(auto const & key) const
  {
    return try_get(key).is_some();
  }

  [[nodiscard]] constexpr bool has(auto const & key, usize hash) const
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
        auto     hash       = hasher_(entry.key);
        auto     probe_idx  = hash & (num_probes_ - 1);
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
    auto       old_num_probes  = num_probes_;
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
    auto new_num_probes = (num_probes_ == 0) ? 1 : (num_probes_ << 1);
    return rehash_n_(new_num_probes);
  }

  constexpr Result<> reserve(usize target_capacity)
  {
    auto const target_num_probes = target_capacity << 1;
    if (num_probes_ >= target_num_probes)
    {
      return Ok{};
    }

    if (!rehash_n_(target_num_probes))
    {
      return Err{};
    }

    return Ok{};
  }

  /// @brief Insert a new entry into the Map
  /// @param exists set to true if the object already exists
  /// @param replaced if true, the original value is replaced if it exists,
  /// otherwise the entry is added
  /// @return The inserted or existing value if the insert was successful
  /// without a memory allocation error, otherwise an Err
  template <typename KeyArg, typename ValueArg>
  [[nodiscard]] constexpr Result<Tuple<Key const &, Value &>>
    push(KeyArg && key, ValueArg && value, bool * exists = nullptr,
         bool replace = true)
  {
    if (exists != nullptr)
    {
      *exists = false;
    }

    if (needs_rehash_(num_entries_ + 1, num_probes_) && !rehash_()) [[unlikely]]
    {
      return Err{};
    }

    auto const hash       = hasher_(key);
    auto       probe_idx  = hash & (num_probes_ - 1);
    auto       insert_idx = USIZE_MAX;
    Distance   probe_dist = 0;
    Entry entry{static_cast<KeyArg &&>(key), static_cast<ValueArg &&>(value)};

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
          swap(*dst_probe, entry);
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
      Tuple<Key const &, Value &>{probe->key, probe->value}
    };
  }

  constexpr void pop_probe_(usize pop_idx)
  {
    auto insert_idx = pop_idx;
    auto probe_idx  = (pop_idx + 1) & (num_probes_ - 1);

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

    auto     hash       = hasher_(key);
    auto     probe_idx  = hash & (num_probes_ - 1);
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

template <typename T, typename H, typename KCmp, typename D = usize>
using Set = Dict<T, Void, H, KCmp, D>;

template <typename K, typename V, typename H, typename KCmp, typename D>
struct IsTriviallyRelocatable<Dict<K, V, H, KCmp, D>>
{
  static constexpr bool value =
    TriviallyRelocatable<H> && TriviallyRelocatable<KCmp>;
};

template <typename V, typename D = usize>
using StrDict = Dict<Str, V, SpanHash, StrEq, D>;

template <typename V, typename D = usize>
using StringDict = Dict<Vec<char>, V, SpanHash, StrEq, D>;

template <typename K, typename V, typename D = usize>
using BitDict = Dict<K, V, BitHash, BitEq, D>;

}    // namespace ash
