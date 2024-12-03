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

template <typename Index, typename Iter, typename IterEnd>
struct EnumerateIter
{
  Index   index_{};
  Iter    iter_{};
  IterEnd end_{};

  constexpr EnumerateIter & operator++()
  {
    ++index_;
    ++iter_;
    return *this;
  }

  constexpr auto operator*() const
  {
    return Tuple<Index, decltype(*iter_)>{index_, iter_};
  }

  constexpr bool operator!=(IterEnd) const
  {
    return iter_ != end_;
  }
};

template <typename Index, typename IterBegin, typename IterEnd>
struct EnumerateRange
{
  IterBegin begin_;
  IterEnd   end_;

  constexpr auto begin() const
  {
    return EnumerateIter<Index, IterBegin, IterEnd>{
        .index_ = 0, .iter_ = begin_, .end_ = end_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

/// @warning Equality is only determined by the first iterator
template <typename BaseIter, typename BaseIterEnd, typename... Iters>
struct ZipIter
{
  Tuple<BaseIter, Iters...> iters_{};
  BaseIterEnd               end0_{};

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

  constexpr bool operator!=(IterEnd) const
  {
    return iters_.v0 != end0_;
  }
};

template <typename BaseIter, typename BaseIterEnd, typename... Iters>
struct ZipRange
{
  Tuple<BaseIter, Iters...> begins_{};
  BaseIterEnd               end0_{};

  constexpr auto begin() const
  {
    return ZipIter<BaseIter, BaseIterEnd, Iters...>{.iters_{begins_},
                                                    .end0_{end0_}};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

template <typename I>
constexpr IndexRange<I> range(I max)
{
  return IndexRange<I>{.i0 = 0, .i1 = max};
}

template <typename I>
constexpr IndexRange<I> range(I min, I max)
{
  return IndexRange<I>{.i0 = min, .i1 = max};
}

template <typename I>
constexpr SkipIndexRange<I> range(I min, I max, I advance)
{
  return SkipIndexRange<I>{.i0 = min, .i1 = max, .advance = advance};
}

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

template <typename Index, Range R>
constexpr auto enumerate(R && range)
{
  return EnumerateRange<Index, decltype(begin(range)), decltype(end(range))>{
      .begin_{begin(range)}, .end_{end(range)}};
}

template <OutputRange A, OutputRange B, typename SwapOp = Swap>
constexpr void swap_range(A && a, B && b, SwapOp && swap_op = {})
{
  auto a_iter = begin(a);
  auto a_end  = end(a);
  auto b_iter = begin(b);

  while (a_iter != a_end)
  {
    swap_op(*a_iter, *b_iter);
    ++a_iter;
    ++b_iter;
  }
}

template <OutputRange R, typename U>
constexpr void fill(R && dst, U && value)
{
  auto iter    = begin(dst);
  auto dst_end = end(dst);

  while (iter != dst_end)
  {
    *iter = value;
    ++iter;
  }
}

template <Range R, typename Predicate>
constexpr bool all(R && range, Predicate && predicate)
{
  auto iter      = begin(range);
  auto range_end = end(range);

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
  auto iter      = begin(range);
  auto range_end = end(range);

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
  auto iter      = begin(range);
  auto range_end = end(range);

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
  auto iter      = begin(range);
  auto range_end = end(range);

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

  auto body_iter = begin(body);
  auto head_iter = begin(head);
  auto head_end  = end(head);

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

template <Range B, Range F, typename Cmp = Eq>
constexpr bool ends_with(B && body, F && foot, Cmp && cmp = {})
{
  if (size(foot) > size(body))
  {
    return false;
  }

  auto foot_iter = begin(foot);
  auto foot_end  = end(foot);
  auto body_iter = end(body) - size(foot);

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
template <typename T, typename U, typename Cmp = Eq>
constexpr Span<T> find(Span<T> span, U && value, Cmp && cmp = {})
{
  usize offset = 0;
  for (; offset < span.size(); ++offset)
  {
    if (cmp(span[offset], value))
    {
      break;
    }
  }
  return span.slice(offset, 1);
}

template <typename T, typename Predicate>
constexpr Span<T> find_if(Span<T> span, Predicate && predicate)
{
  usize offset = 0;
  for (; offset < span.size(); ++offset)
  {
    if (predicate(span[offset]))
    {
      break;
    }
  }
  return span.slice(offset, 1);
}

template <Range R, typename Target, typename Cmp = Eq>
constexpr usize count(R && range, Target && target, Cmp && cmp = {})
{
  usize count     = 0;
  auto  iter      = begin(range);
  auto  range_end = end(range);

  while (iter != range_end)
  {
    if (cmp(*iter, target))
    {
      count++;
    }
  }

  return count;
}

template <Range R, typename Predicate>
constexpr usize count_if(R && range, Predicate && predicate)
{
  usize count = 0;

  auto iter      = begin(range);
  auto range_end = end(range);

  while (iter != range_end)
  {
    if (predicate(*iter))
    {
      count++;
    }
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

  auto a_iter = begin(a);
  auto a_end  = end(a);
  auto b_iter = begin(b);

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

template <Range I, OutputRange O, typename Map>
constexpr void transform(I && in, O && out, Map && mapper)
{
  auto in_iter  = begin(in);
  auto in_end   = end(in);
  auto out_iter = begin(out);

  while (in_iter != in_end)
  {
    *out_iter = mapper(*in_iter);
    ++in_iter;
    ++out_iter;
  }
}

template <Range R, typename Init, typename Reduce = Add>
constexpr Init reduce(R && range, Init && init, Reduce && reducer = {})
{
  auto iter      = begin(range);
  auto range_end = end(range);

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
  auto iter      = begin(range);
  auto range_end = end(range);

  while (iter != range_end)
  {
    init = reducer(static_cast<Init &&>(init), mapper(*iter));
    ++iter;
  }

  return static_cast<Init &&>(init);
}

template <OutputRange R, typename E, typename F, typename Cmp = Eq>
constexpr void replace(R && range, E && target, F && replacement,
                       Cmp && cmp = {})
{
  auto iter      = begin(range);
  auto range_end = end(range);

  while (iter != range_end)
  {
    if (cmp(*iter, target))
    {
      *iter = replacement;
    }
  }
}

template <OutputRange R, typename F, typename Test>
constexpr void replace_if(R && range, F && replacement, Test && test)
{
  auto iter      = begin(range);
  auto range_end = end(range);

  while (iter != range_end)
  {
    if (test(*iter))
    {
      *iter = replacement;
    }
  }
}

template <typename T, typename Cmp = Eq>
constexpr void unique(Span<T>, Cmp && cmp = {});        // destroy? retain?

template <OutputRange R, typename SwapOp = Swap>
constexpr void reverse(R && range, SwapOp && swap = {})
{
  if (is_empty(range))
  {
    return;
  }

  // [ ] Use range API

  auto head = begin(range);
  auto tail = end(range) - 1;

  while (head < tail)
  {
    swap(*head, *tail);
    ++head;
    --tail;
  }
}

template <typename T, typename U, typename Op, typename Cmp = Eq>
constexpr void split(Span<T> span, Span<U> delimeter, Op op, Cmp && cmp = {});

// first check if src begins with other
// keep advancing whilst src begins with other
// once it doesn't, store present offset,
// slice from present offset to end, and compare for other
// if equal, move back from end - other.size
// if equal again, move back
// move back until it is no longer equal
template <typename T, typename U, typename Cmp = Eq>
constexpr Span<T> strip(Span<T> src, Span<U> other, Cmp && cmp = {});

template <typename S, typename Cmp = Less>
constexpr void sort(S && span, Cmp && cmp = {})
{
  std::sort(begin(span), end(span), cmp);
}

template <typename I, typename Cmp>
constexpr void indirect_sort(Span<I> indices, Cmp && cmp = {})
{
  sort(indices, [&](I a, I b) { return cmp(a, b); });
}

template <typename S, typename Cmp = Less>
constexpr void stable_sort(S && span, Cmp && cmp = {})
{
  std::stable_sort(begin(span), end(span), cmp);
}

template <typename I, typename Cmp = Less>
constexpr void indirect_stable_sort(Span<I> indices, Cmp && cmp = {})
{
  stable_sort(indices, [&](I a, I b) { return cmp(a, b); });
}

template <OutputRange R, typename Predicate>
constexpr Tuple<Slice, Slice> partition(R && range, Predicate && predicate)
{
  auto       iter      = begin(range);
  auto       range_end = end(range);
  auto const first     = begin(range);

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

  usize const first_size  = static_cast<usize>(next - first);
  usize const second_size = static_cast<usize>(iter - next);

  return Tuple{
      Slice{0,          first_size },
      Slice{first_size, second_size}
  };
}

template <typename T>
void iota(Span<T> s, T first)
{
  for (auto & v : s)
  {
    v = first;
    ++first;
  };
}

template <typename T, typename Op = Add>
constexpr T inclusive_scan(Span<T const> in, Span<T> out, T && init = {},
                           Op && op = {})
{
  T const * in_iter  = begin(in);
  T const * in_end   = end(in);
  T *       out_iter = begin(out);

  while (in_iter != in_end)
  {
    *out_iter = static_cast<T &&>(init);
    init      = op(*out_iter, *in_iter);
    ++in_iter;
    ++out_iter;
  }

  return init;
}

template <typename T, typename Op = Add>
constexpr T prefix_sum(Span<T const> in, Span<T> out, T && init = {},
                       Op && op = {})
{
  return inclusive_scan(in, out, static_cast<T &&>(init),
                        static_cast<Op &&>(op));
}

template <typename T, typename Op = Add>
constexpr T exclusive_scan(Span<T const> in, Span<T> out, T && init = {},
                           Op && op = {})
{
  T const * in_iter  = begin(in);
  T const * in_end   = end(in);
  T *       out_iter = begin(out);

  while (in_iter != in_end)
  {
    *out_iter = op(static_cast<T &&>(init), *in_iter);
    init      = *out_iter;
    ++in_end;
    ++out_iter;
  }

  return init;
}

template <typename T, typename Op = Add>
constexpr T suffix_sum(Span<T const> in, Span<T> out, T && init = {},
                       Op && op = {})
{
  return exclusive_scan(in, out, static_cast<T &&>(init),
                        static_cast<Op &&>(op));
}

template <typename Index, typename T>
struct PrefixRunIter
{
  Index * run_iter_ = nullptr;
  T *     data_     = nullptr;
  Index * end_      = nullptr;

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

  constexpr bool operator!=(IterEnd const &) const
  {
    return run_iter_ != end_;
  }
};

template <typename Index, typename T>
struct PrefixRunSpan
{
  Index * run_begin_;
  Index * run_end_;
  T *     data_;

  constexpr auto begin() const
  {
    return PrefixRunIter<Index, T>{
        .run_iter_ = run_begin_, .run_start_ = 0, .data_ = data_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }
};

template <typename Index, typename T>
struct SuffixRunIter
{
  Index * run_iter_  = nullptr;
  Index * run_end_   = nullptr;
  Index   run_start_ = 0;
  T *     data_      = nullptr;

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

  constexpr bool operator!=(IterEnd const &) const
  {
    return run_iter_ != run_end_;
  }
};

template <typename Index, typename T>
struct SuffixRunSpan
{
  Index * run_begin_;
  Index * run_end_;
  T *     data_;

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
constexpr PrefixRunSpan<Index, T> prefix_run(Span<T>           data,
                                             Span<Index const> ends)
{
  return PrefixRunSpan<Index, T>{
      .run_begin_ = ends.begin(), .run_end_ = ends.end(), .data_ = data.data()};
}

/// @param ends run-ends of the data. Must be sorted.
template <typename Index, typename T>
constexpr SuffixRunSpan<Index, T> suffix_run(Span<T>           data,
                                             Span<Index const> ends)
{
  return SuffixRunSpan<Index, T>{
      .run_begin_ = ends.begin(), .run_end_ = ends.end(), .data_ = data.data()};
}

// TODO: tuple transform: use for sparsevec push

// [ ] window
//
// is_sorted()
// apply() - in-place
// binary_search()
// lower_bound()
// upper_bound()
// fold()

}        // namespace  ash
