/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/option.h"
#include "ashura/std/tuple.h"
#include "ashura/std/types.h"
#include <algorithm>

namespace ash
{

template <typename I>
struct IndexIter
{
  I i_{};
  I max_{};

  constexpr IndexIter & operator++()
  {
    ++i_;
    return *this;
  }

  constexpr I operator*() const
  {
    return i_;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return i_ != max_;
  }

  constexpr I size() const
  {
    return max_ - i_;
  }
};

template <typename I>
struct IndexRange
{
  I min_{};
  I max_{};

  constexpr auto begin() const
  {
    return IndexIter<I>{.i_ = min_, .max_ = max_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }

  constexpr I size() const
  {
    return max_ - min_;
  }
};

template <typename I>
constexpr IndexRange<I> range(I max)
{
  return IndexRange<I>{.min_ = 0, .max_ = max};
}

template <typename I>
constexpr IndexRange<I> range(I min, I max)
{
  return IndexRange<I>{.min_ = min, .max_ = max};
}

// [ ] make these more concrete
// [ ] create view types of runs
// [ ] prefix, suffix
template <typename I>
constexpr IndexRange<I> range(CoreSlice<I> slice)
{
  return IndexRange<I>{.min_ = slice.begin(), .max_ = slice.end()};
}

/// @param inc_ non-zero increment
template <typename I>
struct SkipIndexIter
{
  I i_{};
  I incr_{1};
  I max_{};

  constexpr SkipIndexIter & operator++()
  {
    i_ += incr_;
    return *this;
  }

  constexpr I operator*() const
  {
    return i_;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return i_ != max_;
  }

  constexpr I size() const
  {
    return (max_ - i_) / incr_;
  }
};

/// @param incr_ non-zero increment
template <typename I>
struct SkipIndexRange
{
  I min_{};
  I max_{};
  I incr_{1};

  constexpr auto begin() const
  {
    return SkipIndexIter<I>{.i_ = min_, .max_ = max_, .incr_ = incr_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }

  constexpr auto size() const
  {
    return (max_ - min_) / incr_;
  }
};

template <typename I>
constexpr SkipIndexRange<I> range(I min, I max, I advance)
{
  return SkipIndexRange<I>{.min_ = min, .max_ = max, .incr_ = advance};
}

/// @warning Equality is only determined by the first iterator
template <typename BaseIter, typename... Iters>
struct ZipIter
{
  Tuple<BaseIter, Iters...> iters_{};

  constexpr ZipIter & operator++()
  {
    apply([](auto &... iters) { (++iters, ...); }, iters_);
    return *this;
  }

  constexpr auto operator*() const
  {
    return apply(
      [](auto... iters) { return Tuple<decltype(*iters)...>{(*iters)...}; },
      iters_);
  }

  constexpr bool operator!=(IterEnd end) const
  {
    return iters_.v0 != end;
  }

  constexpr auto size() const requires (SizedIter<BaseIter>)
  {
    return iters_.v0.size();
  }

  constexpr auto max_size() const requires (BoundedSizeIter<BaseIter>)
  {
    return iters_.v0.max_size();
  }
};

template <typename BaseIter, typename... Iters>
struct ZipRange
{
  Tuple<BaseIter, Iters...> iters_{};

  constexpr auto begin() const
  {
    return ZipIter<BaseIter, Iters...>{.iters_{iters_}};
  }

  constexpr auto end() const
  {
    return iter_end;
  }

  constexpr auto size() const requires (SizedIter<BaseIter>)
  {
    return iters_.v0.size();
  }

  constexpr auto max_size() const requires (BoundedSizeIter<BaseIter>)
  {
    return iters_.v0.max_size();
  }
};

/// @brief The size of the head range will be used as the total size of the whole range
template <Range Base, Range... Ranges>
constexpr auto zip(Base && base, Ranges &&... ranges)
{
  return ZipRange<decltype(begin(base)), decltype(begin(ranges))...>{
    .iters_{begin(base), begin(ranges)...}
  };
}

template <typename Iter, typename Index>
struct EnumerateIter
{
  Index index_{};
  Iter  iter_{};

  constexpr EnumerateIter & operator++()
  {
    ++index_;
    ++iter_;
    return *this;
  }

  constexpr auto operator*() const
  {
    return Tuple<Index, decltype(*iter_)>{index_, *iter_};
  }

  constexpr bool operator!=(IterEnd end) const
  {
    return iter_ != end;
  }
};

template <typename Iter, typename Index>
struct EnumerateRange
{
  Iter iter_;

  constexpr auto begin() const
  {
    return EnumerateIter<Iter, Index>{.index_ = 0, .iter_ = iter_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }
};

template <typename Index = usize, Range R>
constexpr auto enumerate(R && range)
{
  return EnumerateRange<decltype(begin(range)), Index>{.iter_{begin(range)}};
}

template <typename Iter, typename Map>
struct MapIter
{
  Iter iter_;
  Map  map_;

  constexpr decltype(auto) operator*() const
  {
    return map_(*iter_);
  }

  constexpr MapIter & operator++()
  {
    iter_++;
    return *this;
  }

  constexpr MapIter operator++(int)
  {
    auto old = *this;
    this->operator++();
    return old;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return iter_ != iter_end;
  }

  constexpr auto size() const requires (SizedIter<Iter>)
  {
    return iter_.size();
  }

  constexpr auto max_size() const requires (BoundedSizeIter<Iter>)
  {
    return iter_.max_size();
  }
};

template <typename Iter, typename Map>
constexpr MapIter<Iter, Map> map(Iter && iter, Map && map)
{
  return MapIter<Iter, Map>{.iter_{static_cast<Iter &&>(iter)},
                            .map_{static_cast<Map &&>(map)}};
}

template <typename Iter, typename Predicate>
struct FilterIter
{
  Iter      valid_iter_;
  Predicate predicate_;

  constexpr Option<decltype(*valid_iter_)> operator*() const
  {
    if (predicate_(*valid_iter_))
    {
      return *valid_iter_;
    }

    return none;
  }

  constexpr void seek()
  {
    while (!predicate_(*valid_iter_) && valid_iter_ != iter_end)
    {
      valid_iter_++;
    }
  }

  constexpr FilterIter & operator++()
  {
    valid_iter_++;
    return *this;
  }

  constexpr FilterIter operator++(int)
  {
    auto old = *this;
    this->operator++();
    return old;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return valid_iter_ != iter_end;
  }

  constexpr auto max_size() const
    requires (SizedIter<Iter> && !BoundedSizeIter<Iter>)
  {
    return valid_iter_.size();
  }

  constexpr auto max_size() const
    requires (BoundedSizeIter<Iter> && !SizedIter<Iter>)
  {
    return valid_iter_.max_size();
  }
};

template <typename Iter, typename Predicate>
constexpr FilterIter<Iter, Predicate> filter(Iter &&      iter,
                                             Predicate && predicate)
{
  return FilterIter<Iter, Predicate>{
    .valid_iter_{static_cast<Iter &&>(iter)},
    .predicate_{static_cast<Predicate &&>(predicate)}};
}

template <OutRange A, OutRange B, typename SwapOp = Swap>
constexpr void swap_range(A && a, B && b, SwapOp && swap_op = {})
{
  auto       a_iter = begin(a);
  auto const a_end  = end(a);
  auto       b_iter = begin(b);

  while (a_iter != a_end)
  {
    swap_op(*a_iter, *b_iter);
    ++a_iter;
    ++b_iter;
  }
}

template <OutRange R, typename U>
constexpr void fill(R && dst, U && value)
{
  auto       iter    = begin(dst);
  auto const dst_end = end(dst);

  while (iter != dst_end)
  {
    *iter = value;
    ++iter;
  }
}

template <Range R, typename Predicate>
constexpr bool all_is(R && range, Predicate && predicate)
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (!predicate(*iter))
    {
      return false;
    }
    ++iter;
  }
  return true;
}

template <Range R, typename Predicate>
constexpr bool any_is(R && range, Predicate && predicate)
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (predicate(*iter))
    {
      return true;
    }
    ++iter;
  }
  return false;
}

template <Range R, typename Predicate>
constexpr bool none_is(R && range, Predicate && predicate)
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (predicate(*iter))
    {
      return false;
    }
    ++iter;
  }
  return true;
}

template <Range R, typename U, typename Cmp = Eq>
constexpr bool contains(R && range, U && value, Cmp && cmp = {})
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (cmp(*iter, value))
    {
      return true;
    }
    ++iter;
  }
  return false;
}

template <Range B, Range H, typename Cmp = Eq>
constexpr bool begins_with(B && body, H && head, Cmp && cmp = {})
{
  if (size(head) > size(body))
  {
    return false;
  }

  auto       body_iter = begin(body);
  auto       head_iter = begin(head);
  auto const head_end  = end(head);

  while (head_iter != head_end)
  {
    if (!cmp(*head_iter, *body_iter))
    {
      return false;
    }

    ++head_iter;
    ++body_iter;
  }
  return true;
}

template <typename T, typename U, typename Cmp = Eq>
constexpr bool ends_with(Span<T> body, Span<U> foot, Cmp && cmp = {})
{
  if (foot.size() > body.size())
  {
    return false;
  }

  auto       foot_iter = foot.pbegin();
  auto const foot_end  = foot.pbegin();
  auto       body_iter = body.pend() - foot.size();

  while (foot_iter != foot_end)
  {
    if (!cmp(*foot_iter, *body_iter))
    {
      return false;
    }

    ++foot_iter;
    ++body_iter;
  }

  return true;
}

template <typename T, typename U, typename Cmp = Eq>
constexpr Span<T> find(Span<T> span, U && value, Cmp && cmp = {})
{
  auto       iter     = begin(span);
  auto const span_end = end(span);

  while (iter != span_end)
  {
    if (cmp(*iter, value))
    {
      break;
    }
    ++iter;
  }

  return Span<T>{iter, span_end};
}

template <typename T, typename Predicate>
constexpr Span<T> find_if(Span<T> span, Predicate && predicate)
{
  auto       iter     = begin(span);
  auto const span_end = end(span);

  while (iter != span_end)
  {
    if (predicate(*iter))
    {
      break;
    }
    ++iter;
  }

  return Span<T>{iter, span_end};
}

template <Range R, typename Target, typename Cmp = Eq>
constexpr usize count(R && range, Target && target, Cmp && cmp = {})
{
  usize      count     = 0;
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (cmp(*iter, target))
    {
      count++;
    }
    ++iter;
  }

  return count;
}

template <Range R, typename Predicate>
constexpr usize count_if(R && range, Predicate && predicate)
{
  usize count = 0;

  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (predicate(*iter))
    {
      count++;
    }
    ++iter;
  }

  return count;
}

template <Range A, Range B, typename Cmp = Eq>
constexpr bool range_eq(A && a, B && b, Cmp && cmp = {})
{
  if (size(a) != size(b))
  {
    return false;
  }

  auto       a_iter = begin(a);
  auto const a_end  = end(a);
  auto       b_iter = begin(b);

  while (a_iter != a_end)
  {
    if (!cmp(*a_iter, *b_iter))
    {
      return false;
    }
    ++a_iter;
    ++b_iter;
  }

  return true;
}

template <Range I, OutRange O, typename Map>
constexpr void transform(I && in, O && out, Map && mapper)
{
  auto       in_iter  = begin(in);
  auto const in_end   = end(in);
  auto       out_iter = begin(out);

  while (in_iter != in_end)
  {
    *out_iter = mapper(*in_iter);
    ++in_iter;
    ++out_iter;
  }
}

template <OutRange O, typename Map>
constexpr void transform(O && out, Map && mapper)
{
  auto       out_iter = begin(out);
  auto const out_end  = end(out);

  while (out_iter != out_end)
  {
    *out_iter = mapper(*out_iter);
    ++out_iter;
  }
}

template <Range R, typename Init, typename Reduce = Add>
constexpr Init reduce(R && range, Init && init, Reduce && reducer = {})
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    init = reducer(static_cast<Init &&>(init), *iter);
    ++iter;
  }

  return static_cast<Init &&>(init);
}

template <Range R, typename Init, typename Map, typename Reduce = Add>
constexpr Init transform_reduce(R && range, Init && init, Map && mapper,
                                Reduce && reducer = {})
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    init = reducer(static_cast<Init &&>(init), mapper(*iter));
    ++iter;
  }

  return static_cast<Init &&>(init);
}

template <OutRange R, typename E, typename F, typename Cmp = Eq>
constexpr void replace(R && range, E && target, F && replacement,
                       Cmp && cmp = {})
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (cmp(*iter, target))
    {
      *iter = replacement;
    }
  }
}

template <OutRange R, typename F, typename Test>
constexpr void replace_if(R && range, F && replacement, Test && test)
{
  auto       iter      = begin(range);
  auto const range_end = end(range);

  while (iter != range_end)
  {
    if (test(*iter))
    {
      *iter = replacement;
    }
    ++iter;
  }
}

template <typename T, typename SwapOp = Swap>
constexpr void reverse(Span<T> span, SwapOp && swap = {})
{
  auto * head = span.pbegin();
  auto * tail = span.pend();

  if (head == tail)
  {
    return;
  }

  tail--;

  while (head < tail)
  {
    swap(*head, *tail);
    ++head;
    --tail;
  }
}

template <typename T, typename U, typename Op, typename Cmp = Eq>
constexpr void split(Span<T> span, Span<U> delimeter, Op op, Cmp && cmp = {});

template <typename T, typename U, typename Cmp = Eq>
constexpr Span<T> strip(Span<T> src, Span<U> other, Cmp && cmp = {});

template <typename T, typename Cmp = Less>
constexpr void sort(Span<T> span, Cmp && cmp = {})
{
  std::sort(span.pbegin(), span.pend(), cmp);
}

template <typename I, typename Cmp>
constexpr void indirect_sort(Span<I> indices, Cmp && cmp = {})
{
  sort(indices, [&](I a, I b) { return cmp(a, b); });
}

template <typename T, typename Cmp = Less>
constexpr void stable_sort(Span<T> span, Cmp && cmp = {})
{
  std::stable_sort(span.pbegin(), span.pend(), cmp);
}

template <typename I, typename Cmp = Less>
constexpr void indirect_stable_sort(Span<I> indices, Cmp && cmp = {})
{
  stable_sort(indices, [&](I a, I b) { return cmp(a, b); });
}

template <typename T, typename Cmp = Less>
constexpr bool is_sorted(Span<T> values, Cmp && cmp = {})
{
  return std::is_sorted(values.pbegin(), values.pend(),
                        static_cast<Cmp &&>(cmp));
}

template <typename T, typename Predicate>
constexpr Tuple<Span<T>, Span<T>> partition(Span<T>      range,
                                            Predicate && predicate)
{
  auto       iter      = range.pbegin();
  auto const range_end = range.pend();
  auto const first     = range.pbegin();

  while (iter != range_end && predicate(*iter))
  {
    ++iter;
  }

  auto next = iter;

  while (iter != range_end)
  {
    if (predicate(*iter))
    {
      swap(*iter, *next);
      ++next;
    }
    ++iter;
  }

  return Tuple{
    Span<T>{first, next     },
    Span<T>{next,  range_end}
  };
}

template <Range R, typename T>
constexpr void iota(R && range, T && first)
{
  for (auto & value : range)
  {
    value = first++;
  }
}

template <typename T, typename I, typename O, typename Op = Add>
constexpr T inclusive_scan(Span<I const> in, Span<O> out, T init = {},
                           Op && op = {})
{
  I const *       in_iter  = in.pbegin();
  I const * const in_end   = in.pend();
  O *             out_iter = out.pbegin();

  while (in_iter != in_end)
  {
    *out_iter = op(static_cast<T &&>(init), *in_iter);
    init      = *out_iter;
    ++in_iter;
    ++out_iter;
  }

  return init;
}

template <typename T, typename I, typename O, typename Op = Add>
constexpr T exclusive_scan(Span<I const> in, Span<O> out, T init = {},
                           Op && op = {})
{
  I const *       in_iter  = in.pbegin();
  I const * const in_end   = in.pend();
  O *             out_iter = out.pbegin();

  while (in_iter != in_end)
  {
    *out_iter = static_cast<T &&>(init);
    init      = op(*out_iter, *in_iter);
    ++in_iter;
    ++out_iter;
  }

  return init;
}

// [ ] add tests
/// @details structured to break loop-dependence, we read from the indices arrays once
template <typename Index, typename... T>
struct RunIter
{
  Index         run_start_{0};
  Index         run_end_{0};
  Index const * ree_iter_ = nullptr;
  Index const * ree_end_  = nullptr;
  Tuple<T *...> data_{};

  constexpr RunIter & operator++()
  {
    run_start_ = run_end_;
    ree_iter_++;
    run_end_ = *ree_iter_;
    return *this;
  }

  constexpr auto operator*() const
  {
    return apply(
      [&](T *... data) {
        return Tuple<Span<T>...>{
          {data + run_start_, data + run_end_}
          ...
        };
      },
      data_);
  }

  constexpr bool operator!=(IterEnd) const
  {
    return ree_iter_ != ree_end_;
  }
};

template <typename Index, typename... T>
struct RunRange
{
  Index         run_start_{0};
  Index         run_end_{0};
  Index const * ree_begin_ = nullptr;
  Index const * ree_end_   = nullptr;
  Tuple<T *...> data_{};

  constexpr auto begin() const
  {
    return RunIter<Index, T...>{.run_start_ = run_start_,
                                .run_end_   = run_end_,
                                .ree_iter_  = ree_begin_,
                                .ree_end_   = ree_end_,
                                .data_{data_}};
  }

  constexpr auto end() const
  {
    return iter_end;
  }
};

/// @param ends run-ends of the data. Must be sorted.
template <typename Index, typename... T>
constexpr auto prefix_run(Span<Index const> runs, Span<T>... data)
{
  if (runs.size() < 2) [[unlikely]]
  {
    return RunRange<Index, T...>{};
  }
  return RunRange<Index, T...>{.run_start_ = runs[0],
                               .run_end_   = runs[1],
                               .ree_begin_ = runs.pbegin() + 1,
                               .ree_end_   = runs.pend(),
                               .data_{data.pbegin()...}};
}

/// @param ends run-ends of the data. Must be sorted.
template <typename I, typename Index, typename... T>
constexpr auto suffix_run(I start, Span<Index const> runs, Span<T>... data)
{
  if (runs.size() < 1) [[unlikely]]
  {
    return RunRange<Index, T...>{};
  }
  return RunRange<Index, T...>{.run_start_ = start,
                               .run_end_   = runs[0],
                               .ree_begin_ = runs.pbegin(),
                               .ree_end_   = runs.pend(),
                               .data_{data.pbegin()...}};
}

/// @brief Given an ordered range, find first value in the range that satisfies `cmp(x)`.
/// @warning each element in the range must be ordered relative to `cmp` or be equal.
template <typename T, typename Cmp>
constexpr Span<T> binary_find(Span<T> span, Cmp && cmp)
{
  T *   iter = span.pbegin();
  usize size = span.size();

  while (size > 1)
  {
    usize const h0_size = size >> 1;
    T * const   h0_last = iter + h0_size - 1;

    if (cmp(*h0_last))
    {
      size = h0_size;
    }
    else
    {
      iter += h0_size;
      size -= h0_size;
    }
  }

  if (size != 0 && cmp(*iter))
  {
    return Span<T>{iter, span.pend()};
  }

  return Span<T>{span.pend(), (usize) 0};
}

template <typename T, typename Cmp, typename U>
constexpr Span<T> binary_find(Span<T> span, Cmp && cmp, U && value)
{
  return binary_find<T>(
    span, [value_ = static_cast<U &&>(value), cmp_ = static_cast<Cmp &&>(cmp)](
            auto const & a) { return cmp_(a, value_); });
}

/// @param window_advance_ must be non-zero
template <typename T>
struct WindowIter
{
  T *       iter_ = nullptr;
  T const * end_  = nullptr;
  usize     window_size_{0};
  usize     window_advance_{1};

  constexpr WindowIter & operator++()
  {
    iter_ += window_advance_;
    return *this;
  }

  constexpr Span<T> operator*() const
  {
    return Span<T>{iter_, window_size_};
  }

  constexpr bool operator==(IterEnd) const
  {
    return (iter_ + window_size_) >= end_;
  }
};

template <typename T>
struct WindowRange
{
  T *       begin_ = nullptr;
  T const * end_   = nullptr;
  usize     window_size_{0};
  usize     window_advance_{1};

  constexpr auto begin() const
  {
    return WindowIter<T>{.iter_           = begin_,
                         .end_            = end_,
                         .window_size_    = window_size_,
                         .window_advance_ = window_advance_};
  }

  constexpr auto end() const
  {
    return iter_end;
  }
};

template <typename T>
constexpr WindowRange<T> window(Span<T> span, usize window_size,
                                usize advance = 1)
{
  if (window_size > span.size()) [[unlikely]]
  {
    return WindowRange<T>{};
  }

  return WindowRange<T>{.begin_          = span.pbegin(),
                        .end_            = span.pend(),
                        .window_size_    = window_size,
                        .window_advance_ = advance};
}

}    // namespace  ash
