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

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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

    constexpr bool operator!=(IterEnd) const
    {
        return iters_.v0 != iter_end;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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
struct ZipView
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

/// @brief The size of the head range will be used as the total size of the
/// whole range
template <Range BaseRange, Range... Ranges>
constexpr auto zip(BaseRange && base, Ranges &&... ranges)
{
    return ZipView<decltype(begin(base)), decltype(begin(ranges))...>{
      .iters_{begin(base), begin(ranges)...}
    };
}

template <typename Index, typename BaseIter, typename... Iters>
struct EnumerateIter
{
    Index                     index_{};
    Tuple<BaseIter, Iters...> iters_{};

    constexpr EnumerateIter & operator++()
    {
        ++index_;
        apply([](auto &... iter) { (++iter, ...); }, iters_);
        return *this;
    }

    constexpr auto operator*() const
    {
        return apply(
          [&](auto &... iters) {
              return Tuple<Index, decltype(*iters)...>{index_, (*iters)...};
          },
          iters_);
    }

    constexpr bool operator!=(IterEnd) const
    {
        return iters_.v0 != iter_end;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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

template <typename Index, typename BaseIter, typename... Iters>
struct EnumerateView
{
    Tuple<BaseIter, Iters...> iters_{};

    constexpr auto begin() const
    {
        return EnumerateIter<Index, BaseIter, Iters...>{.index_ = 0,
                                                        .iters_ = iters_};
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

template <typename Index = usize, Range BaseRange, Range... Ranges>
constexpr auto enumerate(BaseRange && base, Ranges &&... ranges)
{
    return EnumerateView<Index, decltype(begin(base)),
                         decltype(begin(ranges))...>{
      .iters_{begin(base), begin(ranges)...}
    };
}

template <typename Iter, typename Map>
struct MapIter
{
    Iter iter_{};
    Map  map_{};

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

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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
struct MapView
{
    Iter iter_{};
    Map  map_{};

    constexpr auto begin() const
    {
        return MapIter<Iter, Map>{.iter_ = iter_, .map_ = map_};
    }

    constexpr auto end() const
    {
        return iter_end;
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
constexpr auto map(Iter && iter, Map && map)
{
    return MapView<Iter, Map>{.iter_{static_cast<Iter &&>(iter)},
                              .map_{static_cast<Map &&>(map)}};
}

template <typename Iter, typename Predicate>
struct FilterIter
{
    Iter      valid_iter_{};
    Predicate predicate_{};

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

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
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
struct FilterView
{
    Iter      valid_iter_{};
    Predicate predicate_{};

    constexpr auto begin() const
    {
        return FilterIter<Iter, Predicate>{.valid_iter_ = valid_iter_,
                                           .predicate_  = predicate_};
    }

    constexpr auto end() const
    {
        return iter_end;
    }

    constexpr auto size() const requires (SizedIter<Iter>)
    {
        return valid_iter_.size();
    }

    constexpr auto max_size() const requires (BoundedSizeIter<Iter>)
    {
        return valid_iter_.max_size();
    }
};

template <typename Iter, typename Predicate>
constexpr FilterView<Iter, Predicate> filter(Iter &&      iter,
                                             Predicate && predicate)
{
    return FilterView<Iter, Predicate>{
      .valid_iter_{static_cast<Iter &&>(iter)},
      .predicate_{static_cast<Predicate &&>(predicate)}};
}

template <OutRange A, OutRange B, typename SwapOp = Swap>
constexpr void swap_range(A && a, B && b, SwapOp && swap_op = {})
{
    auto a_iter = begin(a);
    auto b_iter = begin(b);

    while (a_iter != iter_end)
    {
        swap_op(*a_iter, *b_iter);
        ++a_iter;
        ++b_iter;
    }
}

template <OutRange R, typename U>
constexpr void fill(R && dst, U && value)
{
    auto iter = begin(dst);

    while (iter != iter_end)
    {
        *iter = value;
        ++iter;
    }
}

template <Range R, typename Predicate>
constexpr bool all_is(R && range, Predicate && predicate)
{
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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

    while (head_iter != iter_end)
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
    auto iter = begin(span);

    while (iter != iter_end)
    {
        if (cmp(*iter, value))
        {
            break;
        }
        ++iter;
    }

    return Span<T>{iter};
}

template <typename T, typename Predicate>
constexpr Span<T> find_if(Span<T> span, Predicate && predicate)
{
    auto iter = begin(span);

    while (iter != iter_end)
    {
        if (predicate(*iter))
        {
            break;
        }
        ++iter;
    }

    return Span<T>{iter};
}

template <Range R, typename Target, typename Cmp = Eq>
constexpr usize count(R && range, Target && target, Cmp && cmp = {})
{
    auto count = 0uz;
    auto iter  = begin(range);

    while (iter != iter_end)
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
    auto count = 0uz;

    auto iter = begin(range);

    while (iter != iter_end)
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

    auto a_iter = begin(a);
    auto b_iter = begin(b);

    while (a_iter != iter_end)
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
    auto in_iter  = begin(in);
    auto out_iter = begin(out);

    while (in_iter != iter_end)
    {
        *out_iter = mapper(*in_iter);
        ++in_iter;
        ++out_iter;
    }
}

template <OutRange O, typename Map>
constexpr void transform(O && out, Map && mapper)
{
    auto out_iter = begin(out);

    while (out_iter != iter_end)
    {
        *out_iter = mapper(*out_iter);
        ++out_iter;
    }
}

template <Range R, typename Init, typename Reduce = Add>
constexpr Init reduce(R && range, Init && init, Reduce && reducer = {})
{
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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
    auto iter = begin(range);

    while (iter != iter_end)
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

/// @brief Given an ordered range, find first value in the range that satisfies
/// `cmp(x)`.
/// @warning each element in the range must be ordered relative to `cmp` or be
/// equal.

// [ ] return Option<T&>
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

    return Span<T>{span.pend(), 0uz};
}

template <typename T, typename Cmp, typename U>
constexpr Span<T> binary_find(Span<T> span, Cmp && cmp, U && value)
{
    return binary_find<T>(span, [value_ = static_cast<U &&>(value),
                                 cmp_   = static_cast<Cmp &&>(cmp)](
                                  auto const & a) { return cmp_(a, value_); });
}

/// @param window_advance_ must be non-zero
template <typename... T>
struct WindowIter
{
    Tuple<T *...> data_           = {};
    usize         iter_           = 0;
    usize         size_           = 0;
    usize         window_size_    = 0;
    usize         window_advance_ = 1;

    constexpr WindowIter & operator++()
    {
        iter_ += window_advance_;
        return *this;
    }

    constexpr auto operator*() const
    {
        return apply(
          [&](T *... p) {
              return Tuple<Span<T>...>{
                Span<T>{p + iter_, window_size_}
                ...
              };
          },
          data_);
    }

    constexpr bool operator==(IterEnd) const
    {
        return (iter_ + window_size_) >= size_;
    }
};

template <typename T0, typename... T>
struct WindowView
{
    using Iter = WindowIter<T0, T...>;

    Tuple<T0 *, T *...> data_;
    usize               size_;
    usize               window_size_;
    usize               window_advance_;

    constexpr WindowView(usize window_size, usize window_advance,
                         Span<T0> span0, Span<T>... spans) :
      data_{span0.pbegin(), spans.pbegin()...},
      size_{(window_size > span0.size()) ? 0 : span0.size()},
      window_size_{window_size},
      window_advance_{window_advance}
    {
    }

    constexpr auto begin() const
    {
        return Iter{.data_           = data_,
                    .iter_           = 0,
                    .size_           = size_,
                    .window_size_    = window_size_,
                    .window_advance_ = window_advance_};
    }

    constexpr auto end() const
    {
        return iter_end;
    }
};

template <typename T0, typename... T>
WindowView(usize, usize, Span<T0>, Span<T>...) -> WindowView<T0, T...>;

template <typename Index, typename... T>
struct RunBatchIter
{
    Index const * run_indices_ = nullptr;
    Tuple<T *...> items_       = {};
    Index         run_iter_    = 0;
    Index         num_runs_    = 0;

    constexpr RunBatchIter & operator++()
    {
        run_iter_++;
        return *this;
    }

    constexpr auto operator*() const
    {
        return apply(
          [&](T *... data) {
              auto begin = run_indices_[run_iter_];
              auto end   = run_indices_[run_iter_ + 1];
              return Tuple<Span<T>...>{
                {data + begin, end - begin}
                ...
              };
          },
          items_);
    }

    constexpr bool operator!=(IterEnd) const
    {
        return run_iter_ != num_runs_;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
    }
};

template <typename Index, typename... T>
struct RunBatchView
{
    using Iter = RunBatchIter<Index, T...>;

    Index const * run_indices_;
    Tuple<T *...> items_;
    Index         size_;

    constexpr RunBatchView(Span<Index const> run_indices, Span<T>... items) :
      run_indices_{run_indices.pbegin()},
      items_{items.pbegin()...},
      size_{(run_indices.size() < 2) ?
              0 :
              static_cast<Index>(run_indices.size() - 1)}
    {
    }

    constexpr auto begin() const
    {
        return Iter{.run_indices_ = run_indices_,
                    .items_       = items_,
                    .run_iter_    = 0,
                    .num_runs_    = size_};
    }

    constexpr auto end() const
    {
        return iter_end;
    }
};

template <typename Index, typename... T>
RunBatchView(Span<Index const>, Span<T>...) -> RunBatchView<Index, T...>;

template <typename Index, typename... T>
RunBatchView(Span<Index>, Span<T>...) -> RunBatchView<Index, T...>;

template <typename Index, typename... T>
struct RunItemIter
{
    using Slice = CoreSlice<Index>;

    Index const * run_indices_ = nullptr;
    Tuple<T *...> items_       = {};
    Index         item_iter_   = 0;
    Index         run_iter_    = 0;
    Index         num_items_   = 0;
    Index         num_runs_    = 0;

    constexpr Index run() const
    {
        return run_iter_;
    }

    constexpr Index item() const
    {
        return item_iter_;
    }

    constexpr RunItemIter & operator++()
    {
        item_iter_++;

        if (run_indices_[run_iter_ + 1] == item_iter_) [[unlikely]]
        {
            run_iter_++;
        }

        return *this;
    }

    /// @brief Move the iterator to the specified index. Good for linear seeks
    /// with small jumps.
    /// @details O(num_runs) complexity
    /// @warning `index` must be less than or equal to `num_items`
    constexpr void seek(Index index)
    {
        if (run_iter_ < num_runs_ &&
            (index == item_iter_ || Slice::offsets(run_indices_[run_iter_],
                                                   run_indices_[run_iter_ + 1])
                                      .contains(index))) [[likely]]
        {
        }
        else if (index < item_iter_)
        {
            while (run_iter_ > 0 && index < run_indices_[run_iter_])
            {
                item_iter_ = run_indices_[run_iter_];
                run_iter_--;
            }
        }
        else
        {
            // index > item_iter_

            while (run_iter_ < num_runs_ &&
                   index >= run_indices_[run_iter_ + 1])
            {
                item_iter_ = run_indices_[run_iter_ + 1];
                run_iter_++;
            }
        }

        item_iter_ = index;
    }

    /// @brief Move the iterator to the specified index. Good for
    /// random/non-deterministic seeks with possibly large jumps.
    /// @details O(log(N)) complexity
    constexpr void seek_far(Index index);

    constexpr auto operator*() const
    {
        return apply(
          [&](T *... data) { return Tuple<T &...>{data[run_iter_]...}; },
          items_);
    }

    constexpr bool operator!=(IterEnd) const
    {
        return item_iter_ != num_items_;
    }

    constexpr bool operator==(IterEnd) const
    {
        return !this->operator!=(iter_end);
    }
};

template <typename Index, typename... T>
struct RunItemView
{
    using Iter = RunItemIter<Index, T...>;

    Index const * run_indices_;
    Tuple<T *...> items_;
    Index         size_;
    Index         num_runs_;

    constexpr RunItemView(Span<Index const> run_indices, Span<T>... items) :
      run_indices_{run_indices.pbegin()},
      items_{items.pbegin()...},
      size_{(run_indices.size() < 2) ? 0 : run_indices.last()},
      num_runs_{(run_indices.size() < 2) ?
                  0 :
                  static_cast<Index>(run_indices.size() - 1)}
    {
    }

    constexpr Index size() const
    {
        return size_;
    }

    constexpr Index num_runs() const
    {
        return num_runs_;
    }

    constexpr Span<Index const> run_indices() const
    {
        return Span<Index const>{run_indices_,
                                 (num_runs_ == 0) ? 0 : (num_runs_ + 1)};
    }

    constexpr Tuple<Span<T>...> items() const
    {
        return apply(
          [&](T *... data) {
              return Tuple<Span<T>...>{
                Span<T>{data, size_}
                ...
              };
          },
          items_);
    }

    constexpr auto begin() const
    {
        return Iter{.run_indices_ = run_indices_,
                    .items_       = items_,
                    .item_iter_   = 0,
                    .run_iter_    = 0,
                    .num_items_   = size_,
                    .num_runs_    = num_runs_};
    }

    constexpr auto end() const
    {
        return iter_end;
    }
};

template <typename Index, typename... T>
RunItemView(Span<Index const>, Span<T>...) -> RunItemView<Index, T...>;

template <typename Index, typename... T>
RunItemView(Span<Index>, Span<T>...) -> RunItemView<Index, T...>;

}    // namespace  ash
