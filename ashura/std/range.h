/// SPDX-License-Identifier: MIT
#pragma once
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
    return IterEnd{};
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
    return IterEnd{};
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
};

template <typename BaseIter, typename... Iters>
struct ZipRange
{
  Tuple<BaseIter, Iters...> begins_{};

  constexpr auto begin() const
  {
    return ZipIter<BaseIter, Iters...>{.iters_{begins_}};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

/// @brief The size of the head range will be used as the total size of the whole range
template <Range Base, Range... Ranges>
constexpr auto zip(Base && base, Ranges &&... ranges)
{
  return ZipRange<decltype(begin(base)), decltype(end(base)),
                  decltype(begin(ranges))...>{
      .begins_{begin(base), begin(ranges)...},
      .end_{end(base)}
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
    return IterEnd{};
  }
};

template <typename Index, Range R>
constexpr auto enumerate(R && range)
{
  return EnumerateRange<decltype(begin(range)), Index>{.iter_{begin(range)}};
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
constexpr bool all(R && range, Predicate && predicate)
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
constexpr bool any(R && range, Predicate && predicate)
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
constexpr bool none(R && range, Predicate && predicate)
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

/// size is 0 if not found, size is 1 if found
//
// [ ] use range slicing? not all ranges are equal
//
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
constexpr bool is_sorted(Span<T> indices, Cmp && cmp = {})
{
  return std::is_sorted(indices.pbegin(), indices.pend(),
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
void iota(R && range, T && first)
{
  // [ ] use range methods
  for (auto & value : range)
  {
    value = first++;
  };
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
    *out_iter = static_cast<T &&>(init);
    init      = op(*out_iter, *in_iter);
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
    *out_iter = op(static_cast<T &&>(init), *in_iter);
    init      = *out_iter;
    ++in_iter;
    ++out_iter;
  }

  return init;
}

template <typename Index, typename T>
struct PrefixRunIter
{
  Index *       run_iter_ = nullptr;
  Index const * end_      = nullptr;
  T *           data_     = nullptr;

  constexpr PrefixRunIter & operator++()
  {
    ++run_iter_;
    return *this;
  }

  constexpr auto operator*() const
  {
    Index const begin = *run_iter_;
    Index const end   = *(run_iter_ + 1);
    return Span<T>{data_ + begin, data_ + end};
  }

  constexpr bool operator!=(IterEnd) const
  {
    return run_iter_ != end_;
  }
};

template <typename Index, typename T>
struct PrefixRunRange
{
  Index *       run_begin_;
  Index const * run_end_;
  T *           data_;

  constexpr auto begin() const
  {
    return PrefixRunIter<Index, T>{
        .run_iter_ = run_begin_, .end_ = run_end_, .data_ = data_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

/// @param ends run-ends of the data. Must be sorted.
template <typename Index, typename T>
constexpr PrefixRunRange<Index, T> prefix_run(Span<T>           data,
                                              Span<Index const> runs)
{
  return PrefixRunRange<Index, T>{.run_begin_ = runs.pbegin(),
                                  .run_end_   = runs.pend(),
                                  .data_      = data.data()};
}

template <typename Index, typename T>
struct SuffixRunIter
{
  Index *       run_iter_  = nullptr;
  Index const * run_end_   = nullptr;
  Index         run_start_ = 0;
  T *           data_      = nullptr;

  constexpr SuffixRunIter & operator++()
  {
    run_start_ = *run_iter_;
    ++run_iter_;
    return *this;
  }

  constexpr auto operator*() const
  {
    return Span<T>{data_ + run_start_, data_ + *run_iter_};
  }

  constexpr bool operator!=(IterEnd) const
  {
    return run_iter_ != run_end_;
  }
};

template <typename Index, typename T>
struct SuffixRunRange
{
  Index *       run_begin_;
  Index const * run_end_;
  T *           data_;

  constexpr auto begin() const
  {
    return SuffixRunIter<Index, T>{.run_iter_  = run_begin_,
                                   .run_end_   = run_end_,
                                   .run_start_ = 0,
                                   .data_      = data_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

/// @param ends run-ends of the data. Must be sorted.
template <typename Index, typename T>
constexpr SuffixRunRange<Index, T> suffix_run(Span<T>           data,
                                              Span<Index const> runs)
{
  return SuffixRunRange<Index, T>{.run_begin_ = runs.pbegin(),
                                  .run_end_   = runs.pend(),
                                  .data_      = data.data()};
}

/// @brief search for first element less than or equal to value
template <typename T, typename U, typename Cmp = Less>
constexpr Span<T> lower_bound(Span<T> span, U && value, Cmp && cmp = {})
{
  usize size = span.size();
  T *   iter = span.pbegin();

  while (size != 0)
  {
    usize const half_size = size >> 1;

    if (cmp(iter[half_size], value))
    {
      size = half_size;
    }
    else
    {
      size -= half_size + 1;
      iter += half_size;
    }
  }

  return Span<T>{iter, span.pend()};
}

/// @brief search for first element greater than value
template <typename T, typename U, typename Cmp = Less>
constexpr Span<T> upper_bound(Span<T> span, U && value, Cmp && cmp = {})
{
  usize size = span.size();
  T *   iter = span.pbegin();

  while (size != 0)
  {
    usize const half_size = size >> 1;

    if (cmp(value, iter[half_size]))
    {
      size -= half_size + 1;
      iter += half_size;
    }
    else
    {
      size = half_size;
    }
  }

  return Span<T>{iter, span.pend()};
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
    return IterEnd{};
  }
};

template <typename T>
constexpr WindowRange<T> window(Span<T> span, usize window_size, usize advance)
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

template <typename T>
constexpr WindowRange<T> window(Span<T> span, usize window_size)
{
  return window<T>(span, window_size, window_size);
}

}        // namespace  ash
