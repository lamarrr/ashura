#pragma once
#include "ashura/std/types.h"

namespace ash
{

namespace op
{

struct Add
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a + b;
  }
};

struct Sub
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a - b;
  }
};

struct Mul
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a * b;
  }
};

struct Div
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a / b;
  }
};

struct Equal
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a == b;
  }
};

struct NotEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a != b;
  }
};

struct Lesser
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a < b;
  }
};

struct LesserOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a <= b;
  }
};

struct Greater
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a > b;
  }
};

struct GreaterOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a >= b;
  }
};

struct Compare
{
  constexpr i8 operator()(auto const &a, auto const &b) const
  {
    if (a == b)
    {
      return 0;
    }
    if (a > b)
    {
      return -1;
    }
    return 1;
  }
};

struct Min
{
  constexpr auto const &operator()(auto const &a, auto const &b) const
  {
    return a < b ? a : b;
  }
};

struct Max
{
  constexpr auto const &operator()(auto const &a, auto const &b) const
  {
    return a > b ? a : b;
  }
};

struct Swap
{
  template <typename T>
  constexpr void operator()(T &a, T &b) const
  {
    T a_tmp{(T &&) a};
    a = (T &&) b;
    b = (T &&) a_tmp;
  }
};

struct Clamp
{
  template <typename T>
  constexpr T const &operator()(T const &value, T const &min,
                                T const &max) const
  {
    return value < min ? min : (value > max ? max : value);
  }
};

constexpr Add            add;
constexpr Sub            sub;
constexpr Mul            mul;
constexpr Div            div;
constexpr Equal          equal;
constexpr NotEqual       not_equal;
constexpr Lesser         lesser;
constexpr LesserOrEqual  lesser_or_equal;
constexpr Greater        greater;
constexpr GreaterOrEqual greater_or_equal;
constexpr Compare        compare;
constexpr Min            min;
constexpr Max            max;
constexpr Swap           swap;
constexpr Clamp          clamp;

}        // namespace op

namespace alg
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

template <typename A, typename B, typename Swap = op::Swap>
constexpr void swap(Span<A> a, Span<B> b, Swap swap_op = {})
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
constexpr void fill(Span<T> dst, T const &value)
{
  for (T &element : dst)
  {
    element = value;
  }
}

template <typename T, typename Predicate>
constexpr bool all_of(Span<T> span, Predicate predicate)
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
constexpr bool any_of(Span<T> span, Predicate predicate)
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

template <typename T, typename U, typename Cmp = op::Equal>
constexpr bool contains(Span<T const> span, U const &value, Cmp cmp = {})
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

template <typename B, typename H, typename Cmp = op::Equal>
constexpr bool begins_with(Span<B const> body, Span<H const> header,
                           Cmp cmp = {})
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

template <typename B, typename F, typename Cmp = op::Equal>
constexpr bool ends_with(Span<B const> body, Span<F const> footer, Cmp cmp = {})
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
template <typename T, typename U, typename Cmp = op::Equal>
constexpr Span<T> find(Span<T> span, U const &value, Cmp cmp = {})
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
constexpr Span<T> find_if(Span<T> span, Predicate predicate)
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
constexpr Span<T> skip_until(Span<T>, Predicate predicate);

template <typename T, typename Predicate>
constexpr Span<T> skip_while(Span<T>, Predicate predicate);

template <typename T, typename Predicate>
constexpr Span<T> skip_to_last(Span<T>, Predicate predicate);

template <typename T, typename U, typename Cmp = op::Equal>
constexpr void find_mismatch(Span<T>, Span<U>, Span<T> &, Span<U> &,
                             Cmp cmp = {});

template <typename T, typename Element, typename Cmp = op::Equal>
constexpr usize count(Span<T const> span, Element const &element, Cmp cmp = {})
{
  usize count = 0;
  for (usize i = 0; i < span.size; i++)
  {
    if (cmp(span.data[i], element))
    {
      count++;
    }
  }
  return count;
}

template <typename T, typename Predicate>
constexpr usize count_if(Span<T const> span, Predicate predicate)
{
  usize count = 0;
  for (usize i = 0; i < span.size; i++)
  {
    if (predicate(span.data[i]))
    {
      count++;
    }
  }
  return count;
}

template <typename A, typename B, typename Cmp = op::Equal>
constexpr bool equal(Span<A const> a, Span<B const> b, Cmp cmp = {})
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
constexpr void map(Span<Input const> input, Span<Output> output, Map mapper)
{
  for (usize i = 0; i < input.size; i++)
  {
    output.data[i] = mapper(input.data[i]);
  }
}

template <typename Input, typename Init, typename Reduce = op::Add>
constexpr Init reduce(Span<Input const> span, Init init, Reduce reducer = {})
{
  for (usize i = 0; i < span.size; i++)
  {
    init = reducer(init, span.data[i]);
  }

  return (Init &&) init;
}

template <typename Input, typename Output, typename Init, typename Map,
          typename Reduce>
constexpr Init map_reduce(Span<Input const> input, Init init, Map map,
                          Reduce reducer)
{
  for (usize i = 0; i < input.size; i++)
  {
    init = reducer(init, map(input.data[i]));
  }

  return (Init &&) init;
}

template <typename T, typename E, typename R, typename Cmp = op::Equal>
constexpr void replace(Span<T> span, E const &element, R const &replacement,
                       Cmp cmp = {})
{
  for (usize i = 0; i < span.size; i++)
  {
    if (cmp(span.data[i], element))
    {
      span.data[i] = replacement;
    }
  }
}

template <typename T, typename R, typename Test>
constexpr void replace_if(Span<T> span, R const &replacement, Test test)
{
  for (usize i = 0; i < span.size; i++)
  {
    if (test(span.data[i]))
    {
      span.data[i] = replacement;
    }
  }
}

template <typename T, typename Predicate>
constexpr void partition(Span<T>, Span<T> &, Span<T> &, Predicate);

template <typename T, typename Cmp = op::Equal>
constexpr void unique(Span<T>, Cmp cmp = {});        // destroy? retain?

template <typename T, typename Swap = op::Swap>
constexpr void reverse(Span<T> span, Swap swap = {})
{
  for (usize fwd = 0, bwd = span.size - 1; fwd < span.size / 2; fwd++, bwd--)
  {
    swap(span.data[fwd], span.data[bwd]);
  }
}

template <typename T, typename Swap = op::Swap>
constexpr void rotate(Span<T>, Swap swap = {});

template <typename T, typename Cmp = op::Min>
constexpr Span<T> min(Span<T> span, Cmp cmp = {});        // return Span

template <typename T, typename Cmp = op::Max>
constexpr Span<T> max(Span<T> span, Cmp cmp = {});        // return Span

template <typename T, typename LexOrd = op::Compare>
constexpr void min_max(Span<T> span, Span<T> &min, Span<T> &max,
                       LexOrd ord = {});

// once gotten, it will call op(span) for each range
// TODO(lamarrr)
template <typename T, typename U, typename Op, typename Cmp = op::Equal>
constexpr void split(Span<T> span, Span<U const> delimeter, Op op, Cmp cmp = {})
{
}

// first check if src begins with other
// keep advancing whilst src begins with other
// once it doesn't, store present offset,
// slice from present offset to end, and compare for other
// if equal, move back from end - other.size
// if equal again, move back
// move back until it is no longer equal
template <typename T, typename U, typename Cmp = op::Equal>
constexpr Span<T> strip(Span<T> src, Span<U const> other, Cmp cmp = {})
{
}
// title() span, no string_view types

// returns the
template <typename T, typename Predicate>
constexpr void find_reflection(Span<T> span, Span<T> &head, Span<T> &body,
                               Span<T> &tail, Predicate predicate);

}        // namespace alg
}        // namespace  ash
