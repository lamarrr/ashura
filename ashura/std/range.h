#pragma once
#include "ashura/std/types.h"
#include <algorithm>

namespace ash
{

template <typename T>
constexpr void default_construct(Span<T> span)
{
  for (T *iter = begin(span); iter != end(span); iter++)
  {
    new (iter) T{};
  }
}

template <typename Src, typename Dst>
constexpr void move_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = begin(src);
  Dst *output = begin(dst);
  for (; input != end(src); output++, input++)
  {
    new (*output) Dst{(Src &&) (*input)};
  }
}

template <typename Src, typename Dst>
constexpr void copy_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = begin(src);
  Dst *output = begin(dst);
  for (; input != end(src); output++, input++)
  {
    new (*output) Dst{*input};
  }
}

template <typename T>
constexpr void destruct(Span<T> span)
{
  for (T *iter = begin(span); iter != end(span); iter++)
  {
    iter->~T();
  }
}

template <typename Src, typename Dst>
constexpr void move(Span<Src> src, Span<Dst> dst)
{
  Src *input  = begin(src);
  Dst *output = begin(dst);
  for (; input != end(src); output++, input++)
  {
    *output = (Src &&) *input;
  }
}

template <typename Src, typename Dst>
constexpr void copy(Span<Src> src, Span<Dst> dst)
{
  Src *input  = begin(src);
  Dst *output = begin(dst);
  for (; input != end(src); output++, input++)
  {
    *output = *input;
  }
}

template <typename A, typename B, typename SwapOpT = Swap>
constexpr void swap_range(Span<A> a, Span<B> b, SwapOpT &&swap_op = {})
{
  A *a_iter = begin(a);
  B *b_iter = begin(b);
  for (; a_iter != end(a); a_iter++, b_iter++)
  {
    swap_op(*a_iter, *b_iter);
  }
}

/// destroys elements that don't match a predicate.
/// warning: the order of elements are not preserved!
template <typename T, typename Predicate>
constexpr void destruct_if(Span<T> span, usize &new_size);

template <OutputRange R, typename U>
constexpr void fill(R &&dst, U &&value)
{
  OutputIterator auto it = begin(dst);
  while (it != end(dst))
  {
    *it = value;
    it++;
  }
}

template <InputRange R, typename Predicate>
constexpr bool all_of(R &&range, Predicate &&predicate)
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    if (!predicate(*it))
    {
      return false;
    }
    it++;
  }
  return true;
}

template <InputRange R, typename Predicate>
constexpr bool any_of(R &&range, Predicate &&predicate)
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    if (predicate(*it))
    {
      return true;
    }
    it++;
  }
  return false;
}

template <InputRange R, typename Predicate>
constexpr bool none_of(R &&range, Predicate &&predicate)
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    if (predicate(*it))
    {
      return false;
    }
    it++;
  }
  return true;
}

template <InputRange R, typename U, typename Cmp = Equal>
constexpr bool contains(R &&range, U &&value, Cmp &&cmp = {})
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    if (cmp(*it, value))
    {
      return true;
    }
    it++;
  }
  return false;
}

template <InputRange B, InputRange H, typename Cmp = Equal>
constexpr bool begins_with(B &&body, H &&head, Cmp &&cmp = {})
{
  if (size(head) > size(body))
  {
    return false;
  }

  InputIterator auto body_it = begin(body);
  InputIterator auto head_it = begin(head);

  while (head_it != end(head))
  {
    if (!cmp(*head_it, *body_it))
    {
      return false;
    }

    head_it++;
    body_it++;
  }
  return true;
}

template <InputRange B, InputRange F, typename Cmp = Equal>
constexpr bool ends_with(B &&body, F &&foot, Cmp &&cmp = {})
{
  if (size(foot) > size(body))
  {
    return false;
  }

  InputIterator auto foot_it = begin(foot);
  InputIterator auto body_it = end(body) - size(foot);

  while (foot_it != end(foot))
  {
    if (!cmp(*foot_it, *body_it))
    {
      return false;
    }

    foot_it++;
    body_it++;
  }

  return true;
}

/// size is 0 if not found, size is 1 if found
template <typename T, typename U, typename Cmp = Equal>
constexpr Span<T> find(Span<T> span, U &&value, Cmp &&cmp = {})
{
  usize offset = 0;
  for (; offset < span.size(); offset++)
  {
    if (cmp(span[offset], value))
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
  for (; offset < span.size(); offset++)
  {
    if (predicate(span[offset]))
    {
      break;
    }
  }
  return span[Slice{offset, 1}];
}

template <InputRange R, typename Target, typename Cmp = Equal>
constexpr usize count(R &&range, Target &&target, Cmp &&cmp = {})
{
  usize count = 0;
  for (auto &element : range)
  {
    if (cmp(element, target))
    {
      count++;
    }
  }
  return count;
}

template <InputRange R, typename Predicate>
constexpr usize count_if(R &&range, Predicate &&predicate)
{
  usize count = 0;
  for (auto &element : range)
  {
    if (predicate(element))
    {
      count++;
    }
  }
  return count;
}

template <InputRange A, InputRange B, typename Cmp = Equal>
constexpr bool range_equal(A &&a, B &&b, Cmp &&cmp = {})
{
  if (size(a) != size(b))
  {
    return false;
  }

  InputIterator auto a_it = begin(a);
  InputIterator auto b_it = begin(b);

  while (a_it != end(a))
  {
    if (!cmp(*a_it, *b_it))
    {
      return false;
    }
    a_it++;
    b_it++;
  }

  return true;
}

template <InputRange I, OutputRange O, typename Map>
constexpr void map(I &&input, O &&output, Map &&mapper)
{
  InputIterator auto  input_it  = begin(input);
  OutputIterator auto output_it = begin(output);

  while (input_it != end(input))
  {
    *output_it = mapper(*input_it);
    input_it++;
    output_it++;
  }
}

template <InputRange R, typename Init, typename Reduce = Add>
constexpr Init reduce(R &&range, Init &&init, Reduce &&reducer = {})
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    init = reducer((Init &&) init, *it);
    it++;
  }

  return (Init &&) init;
}

template <InputRange R, typename Init, typename Map, typename Reduce>
constexpr Init map_reduce(R &&range, Init &&init, Map &&mapper,
                          Reduce &&reducer)
{
  InputIterator auto it = begin(range);
  while (it != end(range))
  {
    init = reducer((Init &&) init, mapper(*it));
    it++;
  }

  return (Init &&) init;
}

template <OutputRange R, typename E, typename F, typename Cmp = Equal>
constexpr void replace(R &&range, E &&target, F &&replacement, Cmp &&cmp = {})
{
  OutputIterator auto iter = begin(range);
  while (iter != end(range))
  {
    if (cmp(*iter, target))
    {
      *iter = replacement;
    }
  }
}

template <OutputRange R, typename F, typename Test>
constexpr void replace_if(R &&range, F &&replacement, Test &&test)
{
  OutputIterator auto iter = begin(range);
  while (iter != end(range))
  {
    if (test(*iter))
    {
      *iter = replacement;
    }
  }
}

template <typename T, typename Cmp = Equal>
constexpr void unique(Span<T>, Cmp &&cmp = {});        // destroy? retain?

template <OutputRange R, typename SwapOp = Swap>
constexpr void reverse(R &&range, SwapOp &&swap = {})
{
  OutputIterator auto head = begin(range);
  OutputIterator auto tail = end(range);

  while (head != --tail)
  {
    swap(*head, *tail);
    head++;
  }
}

template <typename T, typename SwapOp = Swap>
constexpr void rotate(Span<T>, SwapOp &&swap = {});

template <typename T, typename Cmp = Lesser>
constexpr Span<T> range_lex(Span<T> span, Cmp &&cmp = {})
{
  if (span.is_empty())
  {
    return Span<T>{};
  }

  T *const end = span.end();
  T       *min = span.begin();
  T       *it  = min + 1;

  while (it < end)
  {
    if (cmp(*it, *min))
    {
      min = it;
    }
    it++;
  }

  return Span<T>{min, 1};
}

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

template <typename S, typename Cmp = Lesser>
constexpr void sort(S &&span, Cmp &&cmp = {})
{
  std::sort(begin(span), end(span), cmp);
}

template <typename T, typename K>
constexpr void radix_sort(Span<T>, Span<K>)
{
  // usize frequency[256];
}

template <typename S, typename IndexType, typename Cmp = Lesser>
constexpr void indirect_sort(S &&span, Span<IndexType> indices, Cmp &&cmp = {})
{
  sort(indices,
       [&](IndexType a, IndexType b) { return cmp(span[a], span[b]); });
}

template <OutputRange R, typename Predicate>
constexpr Tuple<Slice, Slice> partition(R &&range, Predicate &&predicate)
{
  OutputIterator auto       iter  = begin(range);
  OutputIterator auto const first = begin(range);

  while (iter != end(range) && predicate(*iter))
  {
    iter++;
  }
  OutputIterator auto next = iter;

  for (; iter != end(range); iter++)
  {
    if (predicate(*iter))
    {
      swap(*iter, *next);
      next++;
    }
  }

  usize const first_size  = static_cast<usize>(next - first);
  usize const second_size = static_cast<usize>(iter - next);

  return Tuple{Slice{0, first_size}, Slice{first_size, second_size}};
}

template <typename S, typename IndexType, typename Cmp = Lesser>
constexpr void stable_indirect_sort(S &&span, Span<IndexType> indices,
                                    Cmp &&cmp = {})
{
  std::stable_sort(begin(indices), end(indices), [&](IndexType a, IndexType b) {
    return cmp(span[a], span[b]);
  });
}

template <typename S, typename IndexType, typename Fn, typename Cmp = Equal>
void for_each_partition_indirect(S &&span, Span<IndexType> indices, Fn &&op,
                                 Cmp &&cmp = {})
{
  IndexType *partition_begin = begin(indices);
  IndexType *iter            = begin(indices);

  while (iter != end(indices))
  {
    while (iter != end(indices) && cmp(span[*partition_begin], span[*iter]))
    {
      iter++;
    }
    op(Span{partition_begin, static_cast<usize>(iter - partition_begin)});
    partition_begin = iter;
  }
}

template <typename T>
void iota(Span<T> s, T first)
{
  for (auto &v : s)
  {
    v = first++;
  };
}

template <typename T>
void riota(Span<T> s, T first)
{
  for (auto &v : s)
  {
    v = first--;
  };
}

}        // namespace  ash
