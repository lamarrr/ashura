#pragma once
#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include <algorithm>

namespace ash
{

template <typename T>
constexpr void default_construct(Span<T> span)
{
  for (T *iter = span.begin(); iter < span.end(); iter++)
  {
    new (iter) T{};
  }
}

template <typename Src, typename Dst>
constexpr void move_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    new (*output) Dst{(Src &&) (*input)};
  }
}

template <typename Src, typename Dst>
constexpr void copy_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    new (*output) Dst{*input};
  }
}

template <typename T>
constexpr void destruct(Span<T> span)
{
  for (T *iter = span.begin(); iter < span.end(); iter++)
  {
    iter->~T();
  }
}

template <typename Src, typename Dst>
constexpr void move(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    *output = (Src &&) *input;
  }
}

template <typename Src, typename Dst>
constexpr void copy(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    *output = *input;
  }
}

template <typename A, typename B, typename SwapOpT = Swap>
constexpr void swap_range(Span<A> a, Span<B> b, SwapOpT &&swap_op = {})
{
  A *a_iter = a.begin();
  B *b_iter = b.begin();
  for (; a_iter < a.end(); a_iter++, b_iter++)
  {
    swap_op(*a_iter, *b_iter);
  }
}

// destroys elements that don't match a predicate,
// the uninitialized range, no-op
// for trivially destructible types
template <typename T, typename Predicate>
constexpr Span<T> destruct_if(Span<T> span, usize &destroy_start);

// moves the elements to the end of the range if predicate is true
constexpr void relocate_if();

template <typename T, typename U>
constexpr void fill(Span<T> dst, U const &value)
{
  for (T &element : dst)
  {
    element = value;
  }
}

template <typename T, typename Predicate>
constexpr bool all_of(Span<T> span, Predicate &&predicate)
{
  for (T const &element : span)
  {
    if (!predicate(element))
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename Predicate>
constexpr bool any_of(Span<T> span, Predicate &&predicate)
{
  for (T const &element : span)
  {
    if (predicate(element))
    {
      return true;
    }
  }
  return false;
}

template <typename T, typename U, typename Cmp = Equal>
constexpr bool contains(Span<T> span, U const &value, Cmp &&cmp = {})
{
  for (T const &element : span)
  {
    if (cmp(element, value))
    {
      return true;
    }
  }
  return false;
}

template <typename B, typename H, typename Cmp = Equal>
constexpr bool begins_with(Span<B> body, Span<H> header, Cmp &&cmp = {})
{
  if (header.size > body.size)
  {
    return false;
  }
  for (usize i = 0; i < header.size; i++)
  {
    if (!cmp(body.data[i], header.data[i]))
    {
      return false;
    }
  }
  return true;
}

template <typename B, typename F, typename Cmp = Equal>
constexpr bool ends_with(Span<B> body, Span<F> footer, Cmp &&cmp = {})
{
  if (footer.size > body.size)
  {
    return false;
  }
  for (usize ibody = body.size - footer.size, ifooter = 0;
       ifooter < footer.size; ifooter++)
  {
    if (!cmp(body.data[ibody], footer.data[ifooter]))
    {
      return false;
    }
  }
  return true;
}

// size is 0 if not found, size is 1 if found
template <typename T, typename U, typename Cmp = Equal>
constexpr Span<T> find(Span<T> span, U const &value, Cmp &&cmp = {})
{
  usize offset = 0;
  for (; offset < span.size; offset++)
  {
    if (cmp(span.data[offset], value))
    {
      break;
    }
  }
  return span[Slice{offset, 1}];
}

template <typename T, typename Predicate>
constexpr Span<T> find_if(Span<T> span, Predicate &&predicate)
{
  usize offset = 0;
  for (; offset < span.size; offset++)
  {
    if (predicate(span.data[offset]))
    {
      break;
    }
  }
  return span[Slice{offset, 1}];
}

template <typename T, typename Predicate>
constexpr Span<T> skip_until(Span<T>, Predicate &&predicate);

template <typename T, typename Predicate>
constexpr Span<T> skip_while(Span<T>, Predicate &&predicate);

template <typename T, typename Predicate>
constexpr Span<T> skip_to_last(Span<T>, Predicate &&predicate);

template <typename T, typename U, typename Cmp = Equal>
constexpr void find_mismatch(Span<T>, Span<U>, Span<T> &, Span<U> &,
                             Cmp &&cmp = {});

template <typename T, typename Element, typename Cmp = Equal>
constexpr usize count(Span<T> span, Element const &target, Cmp &&cmp = {})
{
  usize count = 0;
  for (T &element : span)
  {
    if (cmp(element, target))
    {
      count++;
    }
  }
  return count;
}

template <typename T, typename Predicate>
constexpr usize count_if(Span<T> span, Predicate &&predicate)
{
  usize count = 0;
  for (T &element : span)
  {
    if (predicate(element))
    {
      count++;
    }
  }
  return count;
}

template <typename A, typename B, typename Cmp = Equal>
constexpr bool range_equal(Span<A> a, Span<B> b, Cmp &&cmp = {})
{
  for (usize i = 0; i < a.size; i++)
  {
    if (!cmp(a.data[i], b.data[i]))
    {
      return false;
    }
    return true;
  }
}

template <typename Input, typename Output, typename Map>
constexpr void map(Span<Input> input, Span<Output> output, Map &&mapper)
{
  for (usize i = 0; i < input.size; i++)
  {
    output.data[i] = mapper(input.data[i]);
  }
}

template <typename Input, typename Init, typename Reduce = Add>
constexpr Init reduce(Span<Input> span, Init init, Reduce &&reducer = {})
{
  for (Input &input : span)
  {
    init = reducer(init, input);
  }

  return (Init &&) init;
}

template <typename Input, typename Output, typename Init, typename Map,
          typename Reduce>
constexpr Init map_reduce(Span<Input> input, Init init, Map &&mapper,
                          Reduce &&reducer)
{
  for (Input &element : input)
  {
    init = reducer(init, mapper(element));
  }

  return (Init &&) init;
}

template <typename T, typename E, typename R, typename Cmp = Equal>
constexpr void replace(Span<T> span, E const &target, R const &replacement,
                       Cmp &&cmp = {})
{
  for (T &element : span)
  {
    if (cmp(element, target))
    {
      element = replacement;
    }
  }
}

template <typename T, typename R, typename Test>
constexpr void replace_if(Span<T> span, R const &replacement, Test &&test)
{
  for (T &element : span)
  {
    if (test(element))
    {
      element = replacement;
    }
  }
}

template <typename T, typename Predicate>
constexpr void partition(Span<T>, Span<T> &, Span<T> &, Predicate &&);

template <typename T, typename Cmp = Equal>
constexpr void unique(Span<T>, Cmp &&cmp = {});        // destroy? retain?

template <typename T, typename Swap = Swap>
constexpr void reverse(Span<T> span, Swap &&swap = {})
{
  for (usize fwd = 0, bwd = span.size - 1; fwd < span.size / 2; fwd++, bwd--)
  {
    swap(span.data[fwd], span.data[bwd]);
  }
}

template <typename T, typename Swap = Swap>
constexpr void rotate(Span<T>, Swap &&swap = {});

template <typename T, typename Cmp = Min>
constexpr Span<T> range_min(Span<T> span, Cmp &&cmp = {});        // return Span

template <typename T, typename Cmp = Max>
constexpr Span<T> range_max(Span<T> span, Cmp &&cmp = {});        // return Span

template <typename T, typename LexOrd = Compare>
constexpr void range_min_max(Span<T> span, Span<T> &min, Span<T> &max,
                             LexOrd &&ord = {});

// once gotten, it will call op(span) for each range
// TODO(lamarrr)
template <typename T, typename U, typename Op, typename Cmp = Equal>
constexpr void split(Span<T> span, Span<U> delimeter, Op op, Cmp &&cmp = {});

// first check if src begins with other
// keep advancing whilst src begins with other
// once it doesn't, store present offset,
// slice from present offset to end, and compare for other
// if equal, move back from end - other.size
// if equal again, move back
// move back until it is no longer equal
template <typename T, typename U, typename Cmp = Equal>
constexpr Span<T> strip(Span<T> src, Span<U> other, Cmp &&cmp = {});
// title() span, no string_view types

// returns the
template <typename T, typename Predicate>
constexpr void find_reflection(Span<T> span, Span<T> &head, Span<T> &body,
                               Span<T> &tail, Predicate &&predicate);

template <typename T, typename IndexType, typename Cmp = Lesser>
constexpr void indirect_sort(T *data, Span<IndexType> indices, Cmp &&cmp = {})
{
  std::sort(
      indices.begin(), indices.end(),
      [data, &cmp](IndexType a, IndexType b) { return cmp(data[a], data[b]); });
}

template <typename T, typename IndexType, typename Cmp = Lesser>
constexpr void stable_indirect_sort(T *data, Span<IndexType> indices,
                                    Cmp &&cmp = {})
{
  std::stable_sort(
      indices.begin(), indices.end(),
      [data, &cmp](IndexType a, IndexType b) { return cmp(data[a], data[b]); });
}

template <typename T, typename IndexType, typename Fn, typename Cmp = Equal>
void iter_partitions_indirect(T *data, Span<IndexType> indices, Fn &&op,
                              Cmp &&cmp = {})
{
  IndexType *partition_begin = indices.begin();
  IndexType *iter            = indices.begin();

  while (iter < indices.end())
  {
    while (iter < indices.end() && cmp(data[*partition_begin], data[*iter]))
    {
      iter++;
    }
    op(Span{partition_begin, static_cast<usize>(iter - partition_begin)});
    partition_begin = iter;
  }
}
}        // namespace  ash
